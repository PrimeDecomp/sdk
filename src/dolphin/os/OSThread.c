#include <SDKVersion.h>
#include "dolphin/os/OSPriv.h"

static vu32 RunQueueBits;
static volatile BOOL RunQueueHint;
static vs32 Reschedule;

static OSThreadQueue RunQueue[32];
static OSThread IdleThread;
static OSThread DefaultThread;
static OSContext IdleContext;
static void DefaultSwitchThreadCallback(OSThread* from, OSThread* to);
static OSSwitchThreadCallback SwitchThreadCallback = DefaultSwitchThreadCallback;

OSThread* __OSCurrentThread : OS_BASE_CACHED + 0x00E4;
OSThreadQueue __OSActiveThreadQueue : OS_BASE_CACHED + 0x00DC;
volatile OSContext __OSCurrentContext : OS_BASE_CACHED + 0x00D4;
volatile OSContext* __OSFPUContext : OS_BASE_CACHED + 0x00D8;

static void DefaultSwitchThreadCallback(OSThread* from, OSThread* to) {}

extern void __OSUnlockAllMutex(OSThread* thread);
extern OSErrorHandler __OSErrorTable[];
extern u32 __OSFpscrEnableBits;
extern u8 _stack_addr[];
extern u8 _stack_end[];

#define AddTail(queue, thread, link)                                                               \
  do {                                                                                             \
    OSThread* prev;                                                                                \
                                                                                                   \
    prev = (queue)->tail;                                                                          \
    if (prev == NULL)                                                                              \
      (queue)->head = (thread);                                                                    \
    else                                                                                           \
      prev->link.next = (thread);                                                                  \
    (thread)->link.prev = prev;                                                                    \
    (thread)->link.next = NULL;                                                                    \
    (queue)->tail = (thread);                                                                      \
  } while (0)

#define AddPrio(queue, thread, link)                                                               \
  do {                                                                                             \
    OSThread *prev, *next;                                                                         \
                                                                                                   \
    for (next = (queue)->head; next && next->priority <= thread->priority; next = next->link.next) \
      ;                                                                                            \
    if (next == NULL)                                                                              \
      AddTail(queue, thread, link);                                                                \
    else {                                                                                         \
      (thread)->link.next = next;                                                                  \
      prev = next->link.prev;                                                                      \
      next->link.prev = (thread);                                                                  \
      (thread)->link.prev = prev;                                                                  \
      if (prev == NULL)                                                                            \
        (queue)->head = (thread);                                                                  \
      else                                                                                         \
        prev->link.next = (thread);                                                                \
    }                                                                                              \
  } while (0)

#define RemoveItem(queue, thread, link)                                                            \
  do {                                                                                             \
    OSThread *next, *prev;                                                                         \
    next = (thread)->link.next;                                                                    \
    prev = (thread)->link.prev;                                                                    \
    if (next == NULL)                                                                              \
      (queue)->tail = prev;                                                                        \
    else                                                                                           \
      next->link.prev = prev;                                                                      \
    if (prev == NULL)                                                                              \
      (queue)->head = next;                                                                        \
    else                                                                                           \
      prev->link.next = next;                                                                      \
  } while (0)

#define RemoveHead(queue, thread, link)                                                            \
  do {                                                                                             \
    OSThread* __next;                                                                              \
    (thread) = (queue)->head;                                                                      \
    __next = (thread)->link.next;                                                                  \
    if (__next == NULL)                                                                            \
      (queue)->tail = NULL;                                                                        \
    else                                                                                           \
      __next->link.prev = NULL;                                                                    \
    (queue)->head = __next;                                                                        \
  } while (0)

static inline void OSInitMutexQueue(OSMutexQueue* queue) { queue->head = queue->tail = NULL; }

static inline void OSSetCurrentThread(OSThread* thread) {
  SwitchThreadCallback(__OSCurrentThread, thread);
  __OSCurrentThread = thread;
}

void __OSThreadInit() {
  OSThread* thread = &DefaultThread;
  int prio;

  thread->state = OS_THREAD_STATE_RUNNING;
  thread->attr = OS_THREAD_ATTR_DETACH;
  thread->priority = thread->base = 16;
  thread->suspend = 0;
  thread->val = (void*)-1;
  thread->mutex = NULL;
  OSInitThreadQueue(&thread->queueJoin);
  OSInitMutexQueue(&thread->queueMutex);

  __OSFPUContext = &thread->context;

  OSClearContext(&thread->context);
  OSSetCurrentContext(&thread->context);
  thread->stackBase = (void*)_stack_addr;
  thread->stackEnd = (void*)_stack_end;
  *(thread->stackEnd) = OS_THREAD_STACK_MAGIC;

  OSSetCurrentThread(thread);
  OSClearStack(0);

  RunQueueBits = 0;
  RunQueueHint = FALSE;
  for (prio = OS_PRIORITY_MIN; prio <= OS_PRIORITY_MAX; ++prio) {
    OSInitThreadQueue(&RunQueue[prio]);
  }

  OSInitThreadQueue(&__OSActiveThreadQueue);
  AddTail(&__OSActiveThreadQueue, thread, linkActive);
  OSClearContext(&IdleContext);
  Reschedule = 0;
}

void OSInitThreadQueue(OSThreadQueue* queue) { queue->head = queue->tail = NULL; }

OSThread* OSGetCurrentThread() { return __OSCurrentThread; }

s32 OSDisableScheduler() {
  BOOL enabled;
  s32 count;

  enabled = OSDisableInterrupts();
  count = Reschedule++;
  OSRestoreInterrupts(enabled);
  return count;
}

s32 OSEnableScheduler() {
  BOOL enabled;
  s32 count;

  enabled = OSDisableInterrupts();
  count = Reschedule--;
  OSRestoreInterrupts(enabled);
  return count;
}

static void SetRun(OSThread* thread) {
  thread->queue = &RunQueue[thread->priority];
  AddTail(thread->queue, thread, link);
  RunQueueBits |= 1u << (OS_PRIORITY_MAX - thread->priority);
  RunQueueHint = TRUE;
}
#pragma dont_inline on
static void UnsetRun(OSThread* thread) {
  OSThreadQueue* queue;
  queue = thread->queue;
  RemoveItem(queue, thread, link);
  if (queue->head == 0)
    RunQueueBits &= ~(1u << (OS_PRIORITY_MAX - thread->priority));
  thread->queue = NULL;
}
#pragma dont_inline reset

OSPriority __OSGetEffectivePriority(OSThread* thread) {
  OSPriority priority;
  OSMutex* mutex;
  OSThread* blocked;

  priority = thread->base;
  for (mutex = thread->queueMutex.head; mutex; mutex = mutex->link.next) {
    blocked = mutex->queue.head;
    if (blocked && blocked->priority < priority) {
      priority = blocked->priority;
    }
  }
  return priority;
}

static OSThread* SetEffectivePriority(OSThread* thread, OSPriority priority) {
  switch (thread->state) {
  case OS_THREAD_STATE_READY:
    UnsetRun(thread);
    thread->priority = priority;
    SetRun(thread);
    break;
  case OS_THREAD_STATE_WAITING:
    RemoveItem(thread->queue, thread, link);
    thread->priority = priority;
    AddPrio(thread->queue, thread, link);
    if (thread->mutex) {
      return thread->mutex->thread;
    }
    break;
  case OS_THREAD_STATE_RUNNING:
    RunQueueHint = TRUE;
    thread->priority = priority;
    break;
  }
  return NULL;
}

static void UpdatePriority(OSThread* thread) {
  OSPriority priority;

  do {
    if (0 < thread->suspend) {
      break;
    }
    priority = __OSGetEffectivePriority(thread);
    if (thread->priority == priority) {
      break;
    }
    thread = SetEffectivePriority(thread, priority);
  } while (thread);
}

#if SDK_VERSION >= SDK_VERSION_RVL_2008_01
void __OSPromoteThread(OSThread* thread, OSPriority priority) {
  do {
    if (0 < thread->suspend) {
      break;
    }
    if (thread->priority <= priority) {
      break;
    }
    thread = SetEffectivePriority(thread, priority);
  } while (thread);
}

#endif
static void __OSSwitchThread(OSThread* nextThread) {
  OSSetCurrentThread(nextThread);
  OSSetCurrentContext(&nextThread->context);
  OSLoadContext(&nextThread->context);
}

static OSThread* SelectThread(BOOL yield) {
  OSContext* currentContext;
  OSThread* currentThread;
  OSThread* nextThread;
  OSPriority priority;
  OSThreadQueue* queue;

  if (0 < Reschedule) {
    return 0;
  }

  currentContext = OSGetCurrentContext();
  currentThread = OSGetCurrentThread();
  if (currentContext != &currentThread->context) {
    return 0;
  }

  if (currentThread) {
    if (currentThread->state == OS_THREAD_STATE_RUNNING) {
      if (!yield) {
        priority = __cntlzw(RunQueueBits);
        if (currentThread->priority <= priority) {
          return 0;
        }
      }
      currentThread->state = OS_THREAD_STATE_READY;
      SetRun(currentThread);
    }

    if (!(currentThread->context.state & OS_CONTEXT_STATE_EXC) &&
        OSSaveContext(&currentThread->context)) {
      return 0;
    }
  }

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  OSSetCurrentThread(NULL);
#endif
  if (RunQueueBits == 0) {
#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2003_04
    OSSetCurrentThread(NULL);
#endif
    OSSetCurrentContext(&IdleContext);
    do {
      OSEnableInterrupts();
      while (RunQueueBits == 0)
        ;
      OSDisableInterrupts();
    } while (RunQueueBits == 0);

    OSClearContext(&IdleContext);
  }

  RunQueueHint = FALSE;

  priority = __cntlzw(RunQueueBits);
  queue = &RunQueue[priority];
  RemoveHead(queue, nextThread, link);
  if (queue->head == 0) {
    RunQueueBits &= ~(1u << (OS_PRIORITY_MAX - priority));
  }
  nextThread->queue = NULL;
  nextThread->state = OS_THREAD_STATE_RUNNING;
  __OSSwitchThread(nextThread);
  return nextThread;
}

void __OSReschedule() {
  if (!RunQueueHint) {
    return;
  }

  SelectThread(FALSE);
}

void OSYieldThread(void) {
  BOOL enabled;

  enabled = OSDisableInterrupts();
  SelectThread(TRUE);
  OSRestoreInterrupts(enabled);
}
#if SDK_VERSION >= SDK_VERSION_RVL_2005_11

BOOL OSCreateThread(OSThread* thread, void* (*func)(void*), void* param, void* stack,
                    u32 stackSize, OSPriority priority, u16 attr) {
  BOOL enabled;
  u32 sp;
  int i;

  if (priority < OS_PRIORITY_MIN || OS_PRIORITY_MAX < priority) {
    return FALSE;
  }

  thread->state = OS_THREAD_STATE_READY;
  thread->attr = (u16)(attr & OS_THREAD_ATTR_DETACH);
  thread->base = priority;
  thread->priority = priority;
  thread->suspend = 1;
  thread->val = (void*)-1;
  thread->mutex = NULL;
  OSInitThreadQueue(&thread->queueJoin);
  OSInitMutexQueue(&thread->queueMutex);

  sp = (u32)stack;
  sp = sp & ~7;
  sp -= 8;
  *(u32*)sp = 0;
  *(u32*)(sp + 4) = 0;

  OSInitContext(&thread->context, (u32)func, sp);
  thread->context.lr = (u32)OSExitThread;
  thread->context.gpr[3] = (u32)param;

  thread->stackBase = (u8*)stack;
  thread->stackEnd = (u32*)((u8*)stack - stackSize);
  *(thread->stackEnd) = OS_THREAD_STACK_MAGIC;

  thread->error = 0;
  for (i = 0; i < OS_THREAD_SPECIFIC_MAX; ++i) {
    thread->specific[i] = NULL;
  }

  enabled = OSDisableInterrupts();
  if (__OSErrorTable[OS_ERROR_FPE]) {
    thread->context.srr1 |= 0x900;
    thread->context.state |= 1;
#if SDK_VERSION == SDK_VERSION_RVL_2005_11
    thread->context.fpscr = (__OSFpscrEnableBits & 0xF8) | 4;
#else
    thread->context.fpscr = __OSFpscrEnableBits & 0xF8;
    thread->context.fpscr |= 4;
#endif
    for (i = 0; i < 32; ++i) {
      *(u64*)&thread->context.fpr[i] = (u64)-1;
      *(u64*)&thread->context.psf[i] = (u64)-1;
    }
  }

  AddTail(&__OSActiveThreadQueue, thread, linkActive);
  OSRestoreInterrupts(enabled);
  return TRUE;
}

void OSExitThread(void* val) {
  BOOL enabled;
  OSThread* currentThread;

  enabled = OSDisableInterrupts();
  currentThread = OSGetCurrentThread();
  OSClearContext(&currentThread->context);

  if (currentThread->attr & OS_THREAD_ATTR_DETACH) {
    RemoveItem(&__OSActiveThreadQueue, currentThread, linkActive);
    currentThread->state = 0;
  } else {
    currentThread->state = OS_THREAD_STATE_MORIBUND;
    currentThread->val = val;
  }

  __OSUnlockAllMutex(currentThread);
  OSWakeupThread(&currentThread->queueJoin);
  RunQueueHint = TRUE;
  __OSReschedule();
  OSRestoreInterrupts(enabled);
}
#endif

void OSCancelThread(OSThread* thread) {
  BOOL enabled;

  enabled = OSDisableInterrupts();

  switch (thread->state) {
  case OS_THREAD_STATE_READY:
    if (!(0 < thread->suspend)) {
      UnsetRun(thread);
    }
    break;
  case OS_THREAD_STATE_RUNNING:
    RunQueueHint = TRUE;
    break;
  case OS_THREAD_STATE_WAITING:
    RemoveItem(thread->queue, thread, link);
    thread->queue = NULL;
    if (!(0 < thread->suspend) && thread->mutex) {
      UpdatePriority(thread->mutex->thread);
    }
    break;
  default:
    OSRestoreInterrupts(enabled);
    return;
  }

  OSClearContext(&thread->context);
  if (thread->attr & OS_THREAD_ATTR_DETACH) {
    RemoveItem(&__OSActiveThreadQueue, thread, linkActive);
    thread->state = 0;
  } else {
    thread->state = OS_THREAD_STATE_MORIBUND;
  }

  __OSUnlockAllMutex(thread);

  OSWakeupThread(&thread->queueJoin);

  __OSReschedule();
  OSRestoreInterrupts(enabled);

  return;
}

s32 OSResumeThread(OSThread* thread) {
  BOOL enabled;
  s32 suspendCount;

  enabled = OSDisableInterrupts();
  suspendCount = thread->suspend--;
  if (thread->suspend < 0) {
    thread->suspend = 0;
  } else if (thread->suspend == 0) {
    switch (thread->state) {
    case OS_THREAD_STATE_READY:
      thread->priority = __OSGetEffectivePriority(thread);
      SetRun(thread);
      break;
    case OS_THREAD_STATE_WAITING:
      RemoveItem(thread->queue, thread, link);
      thread->priority = __OSGetEffectivePriority(thread);
      AddPrio(thread->queue, thread, link);
      if (thread->mutex) {
        UpdatePriority(thread->mutex->thread);
      }
      break;
    }
    __OSReschedule();
  }
  OSRestoreInterrupts(enabled);
  return suspendCount;
}

s32 OSSuspendThread(OSThread* thread) {
  BOOL enabled;
  s32 suspendCount;

  enabled = OSDisableInterrupts();
  suspendCount = thread->suspend++;
  if (suspendCount == 0) {
    switch (thread->state) {
    case OS_THREAD_STATE_RUNNING:
      RunQueueHint = TRUE;
      thread->state = OS_THREAD_STATE_READY;
      break;
    case OS_THREAD_STATE_READY:
      UnsetRun(thread);
      break;
    case OS_THREAD_STATE_WAITING:
      RemoveItem(thread->queue, thread, link);
      thread->priority = 32;
      AddTail(thread->queue, thread, link);
      if (thread->mutex) {
        UpdatePriority(thread->mutex->thread);
      }
      break;
    }

    __OSReschedule();
  }
  OSRestoreInterrupts(enabled);
  return suspendCount;
}

void OSSleepThread(OSThreadQueue* queue) {
  BOOL enabled;
  OSThread* currentThread;

  enabled = OSDisableInterrupts();
  currentThread = OSGetCurrentThread();

  currentThread->state = OS_THREAD_STATE_WAITING;
  currentThread->queue = queue;
  AddPrio(queue, currentThread, link);
  RunQueueHint = TRUE;
  __OSReschedule();
  OSRestoreInterrupts(enabled);
}

void OSWakeupThread(OSThreadQueue* queue) {
  BOOL enabled;
  OSThread* thread;

  enabled = OSDisableInterrupts();
  while (queue->head) {
    RemoveHead(queue, thread, link);
    thread->state = OS_THREAD_STATE_READY;
    if (!(0 < thread->suspend)) {
      SetRun(thread);
    }
  }
  __OSReschedule();
  OSRestoreInterrupts(enabled);
}

#if SDK_VERSION >= SDK_VERSION_RVL_2008_01
static void SleepAlarmHandler(OSAlarm* alarm, OSContext* context) {
  OSResumeThread((OSThread*)OSGetAlarmUserData(alarm));
}

void OSSleepTicks(OSTime ticks) {
  BOOL enabled;
  OSThread* thread;
  OSAlarm alarm;

  enabled = OSDisableInterrupts();
  thread = __OSCurrentThread;
  if (thread == NULL) {
    OSRestoreInterrupts(enabled);
    return;
  }

  OSCreateAlarm(&alarm);
  OSSetAlarmTag(&alarm, (u32)thread);
  OSSetAlarmUserData(&alarm, thread);
  OSSetAlarm(&alarm, ticks, SleepAlarmHandler);
  OSSuspendThread(thread);
  OSCancelAlarm(&alarm);
  OSRestoreInterrupts(enabled);
}

#elif SDK_VERSION == SDK_VERSION_RVL_2005_11
static BOOL CheckThreadQueue(OSThreadQueue* queue) {
  OSThread* thread;

  if (queue->head != NULL && queue->head->link.prev != NULL) {
    return FALSE;
  }
  if (queue->tail != NULL && queue->tail->link.next != NULL) {
    return FALSE;
  }
  thread = queue->head;
  while (thread) {
    if (thread->link.next != NULL && thread != thread->link.next->link.prev) {
      return FALSE;
    }
    if (thread->link.prev != NULL && thread != thread->link.prev->link.next) {
      return FALSE;
    }
    thread = thread->link.next;
  }
  return TRUE;
}

static BOOL IsMember(OSThreadQueue* queue, OSThread* thread) {
  OSThread* member = queue->head;

  while (member) {
    if (thread == member) {
      return TRUE;
    }
    member = member->link.next;
  }
  return FALSE;
}

#define IsSuspended(suspend) ((suspend) > 0)

#define CheckReport(line, cond)                                                                    \
  if (!(cond)) {                                                                                   \
    OSReport("OSCheckActiveThreads: Failed " #cond " in %d\n", line);                              \
    OSPanic(__FILE__, line, "");                                                                   \
  }

s32 OSCheckActiveThreads(void) {
  OSThread* thread;
  s32 prio;
  s32 cThread;
  BOOL enabled;

  cThread = 0;
  enabled = OSDisableInterrupts();

  for (prio = 0; prio <= OS_PRIORITY_MAX; prio++) {
    if (RunQueueBits & (1 << (OS_PRIORITY_MAX - prio))) {
      CheckReport(1504, RunQueue[prio].head != NULL && RunQueue[prio].tail != NULL);
    } else {
      CheckReport(1509, RunQueue[prio].head == NULL && RunQueue[prio].tail == NULL);
    }
    CheckReport(1511, CheckThreadQueue(&RunQueue[prio]));
  }

  CheckReport(1516, __OSActiveThreadQueue.head == NULL ||
                        __OSActiveThreadQueue.head->linkActive.prev == NULL);
  CheckReport(1518, __OSActiveThreadQueue.tail == NULL ||
                        __OSActiveThreadQueue.tail->linkActive.next == NULL);

  thread = __OSActiveThreadQueue.head;
  while (thread) {
    cThread++;
    CheckReport(1526, thread->linkActive.next == NULL ||
                          thread == thread->linkActive.next->linkActive.prev);
    CheckReport(1528, thread->linkActive.prev == NULL ||
                          thread == thread->linkActive.prev->linkActive.next);
    CheckReport(1531, *(thread->stackEnd) == OS_THREAD_STACK_MAGIC);
    CheckReport(1534,
                OS_PRIORITY_MIN <= thread->priority && thread->priority <= OS_PRIORITY_MAX + 1);
    CheckReport(1535, 0 <= thread->suspend);
    CheckReport(1536, CheckThreadQueue(&thread->queueJoin));

    switch (thread->state) {
    case OS_THREAD_STATE_READY:
      if (thread->suspend <= 0) {
        CheckReport(1542, thread->queue == &RunQueue[thread->priority]);
        CheckReport(1543, IsMember(&RunQueue[thread->priority], thread));
        CheckReport(1544, thread->priority == __OSGetEffectivePriority(thread));
      }
      break;
    case OS_THREAD_STATE_RUNNING:
      CheckReport(1548, !IsSuspended(thread->suspend));
      CheckReport(1549, thread->queue == NULL);
      CheckReport(1550, thread->priority == __OSGetEffectivePriority(thread));
      break;
    case OS_THREAD_STATE_WAITING:
      CheckReport(1553, thread->queue != NULL);
      CheckReport(1554, CheckThreadQueue(thread->queue));
      CheckReport(1555, IsMember(thread->queue, thread));
      if (thread->suspend <= 0) {
        CheckReport(1558, thread->priority == __OSGetEffectivePriority(thread));
      } else {
        CheckReport(1562, thread->priority == 32);
      }
      CheckReport(1564, !__OSCheckDeadLock(thread));
      break;
    case OS_THREAD_STATE_MORIBUND:
      CheckReport(1568, thread->queueMutex.head == NULL && thread->queueMutex.tail == NULL);
      break;
    default:
      OSReport("OSCheckActiveThreads: Failed. unkown thread state (%d) of thread %p\n",
               thread->state, thread);
      OSPanic(__FILE__, 1574, "");
    }
    CheckReport(1579, __OSCheckMutexes(thread));
    thread = thread->linkActive.next;
  }
  OSRestoreInterrupts(enabled);
  return cThread;
}
#endif

void OSClearStack(u8 val) {
  register u32 sp;
  register u32* p;
  register u32 pattern;

  pattern = ((u32)val << 24) | ((u32)val << 16) | ((u32)val << 8) | (u32)val;
  sp = OSGetStackPointer();
  for (p = __OSCurrentThread->stackEnd + 1; p < (u32*)sp; ++p) {
    *p = pattern;
  }
}

#if SDK_VERSION >= SDK_VERSION_RVL_2008_01
// IdleThread has no users but lives on in the retail binary
extern void fake_function(...);
void FORCEACTIVE_OSThread_c(void);
void FORCEACTIVE_OSThread_c(void) { fake_function(&IdleThread); }

#endif
