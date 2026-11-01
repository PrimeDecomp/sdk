#include <stddef.h>

extern "C" {

#define CTORARG_TYPE int
#define CTORARG_PARTIAL (0)
#define CTORARG_COMPLETE (1)

#define CTORCALL_COMPLETE(ctor, objptr)                                                            \
  (((void (*)(void*, CTORARG_TYPE))ctor)(objptr, CTORARG_COMPLETE))

#define DTORARG_TYPE int

#define DTORCALL_COMPLETE(dtor, objptr) (((void (*)(void*, DTORARG_TYPE))dtor)(objptr, -1))
#define DTORCALL_PARTIAL(dtor, objptr) (((void (*)(void*, DTORARG_TYPE))dtor)(objptr, 0))

typedef short vbase_ctor_arg_type;
typedef char local_cond_type;

typedef struct __eti_init_info {
  void* eti_start;
  void* eti_end;
  void* code_start;
  unsigned long code_size;
} __eti_init_info;

typedef struct CatchInfo {
  void* location;
  void* typeinfo;
  void* dtor;
  void* sublocation;
  long pointercopy;
  void* stacktop;
} CatchInfo;

void __end__catch(CatchInfo* catchinfo);
void __throw(char* throwtype, void* location, void* dtor);
char __throw_catch_compare(const char* throwtype, const char* catchtype, long* offset_result);
void __unexpected(CatchInfo* catchinfo);
void __unregister_fragment(int fragmentID);
int __register_fragment(struct __eti_init_info* info, char* TOC);
int __setjmp(unsigned long* env);
void longjmp(unsigned long* env, int value);

}

typedef unsigned char exaction_type;

#define EXACTION_ENDBIT 0x80
#define EXACTION_MASK 0x7F

// EXAction structs

#define EXACTION_ENDOFLIST 0
#define EXACTION_BRANCH 1

typedef struct ex_branch {
  exaction_type action;
  unsigned char unused;
  unsigned short target;
} ex_branch;

#define EXACTION_DESTROYLOCAL 2

typedef struct ex_destroylocal {
  exaction_type action;
  unsigned char unused;
  short local;
  void* dtor;
} ex_destroylocal;

#define EXACTION_DESTROYLOCALCOND 3

typedef struct ex_destroylocalcond {
  exaction_type action;
  unsigned char dlc_field;
  short cond;
  short local;
  void* dtor;
} ex_destroylocalcond;

#define ex_destroylocalcond_MakeField(regcond) (((regcond) << 7))
#define ex_destroylocalcond_GetRegCond(field) ((field) >> 7)

#define EXACTION_DESTROYLOCALPOINTER 4

typedef struct ex_destroylocalpointer {
  exaction_type action;
  unsigned char dlp_field;
  short pointer;
  void* dtor;
} ex_destroylocalpointer;

#define ex_destroylocalpointer_MakeField(regpointer) (((regpointer) << 7))
#define ex_destroylocalpointer_GetRegPointer(field) ((field) >> 7)

#define EXACTION_DESTROYLOCALARRAY 5

typedef struct ex_destroylocalarray {
  exaction_type action;
  unsigned char unused;
  short localarray;
  unsigned short elements;
  unsigned short element_size;
  void* dtor;
} ex_destroylocalarray;

#define EXACTION_DESTROYBASE 6
#define EXACTION_DESTROYMEMBER 7

typedef struct ex_destroymember {
  exaction_type action;
  unsigned char dm_field;
  short objectptr;
  long offset;
  void* dtor;
} ex_destroymember;

#define ex_destroymember_MakeField(regpointer) (((regpointer) << 7))
#define ex_destroymember_GetRegPointer(field) ((field) >> 7)

#define EXACTION_DESTROYMEMBERCOND 8

typedef struct ex_destroymembercond {
  exaction_type action;
  unsigned char dmc_field;
  short cond;
  short objectptr;
  long offset;
  void* dtor;
} ex_destroymembercond;

#define ex_destroymembercond_MakeField(regcond, regpointer)                                        \
  (((regcond) << 7) | (((regpointer) & 0x1) << 6))
#define ex_destroymembercond_GetRegCond(field) ((field) >> 7)
#define ex_destroymembercond_GetRegPointer(field) (((field) >> 6) & 0x1)

#define EXACTION_DESTROYMEMBERARRAY 9

typedef struct ex_destroymemberarray {
  exaction_type action;
  unsigned char dma_field;
  short objectptr;
  long offset;
  long elements;
  long element_size;
  void* dtor;
} ex_destroymemberarray;

#define ex_destroymemberarray_MakeField(regpointer) (((regpointer) << 7))
#define ex_destroymemberarray_GetRegPointer(field) ((field) >> 7)

#define EXACTION_DELETEPOINTER 10

typedef struct ex_deletepointer {
  exaction_type action;
  unsigned char dp_field;
  short objectptr;
  void* deletefunc;
} ex_deletepointer;

#define ex_deletepointer_MakeField(regpointer) (((regpointer) << 7))
#define ex_deletepointer_GetRegPointer(field) ((field) >> 7)

#define EXACTION_DELETEPOINTERCOND 11

typedef struct ex_deletepointercond {
  exaction_type action;
  unsigned char dpc_field;
  short cond;
  short objectptr;
  void* deletefunc;
} ex_deletepointercond;

#define ex_deletepointercond_MakeField(regcond, regpointer)                                        \
  (((regcond) << 7) | (((regpointer) & 0x1) << 6))
#define ex_deletepointercond_GetRegCond(field) ((field) >> 7)
#define ex_deletepointercond_GetRegPointer(field) (((field) >> 6) & 0x1)

#define EXACTION_CATCHBLOCK 12

typedef struct ex_catchblock {
  exaction_type action;
  unsigned char unused;
  char* catch_type;
  unsigned short catch_pcoffset;
  short cinfo_ref;
} ex_catchblock;

#define EXACTION_ACTIVECATCHBLOCK 13

typedef struct ex_activecatchblock {
  exaction_type action;
  unsigned char unused;
  short cinfo_ref;
} ex_activecatchblock;

#define EXACTION_TERMINATE 14

typedef struct ex_terminate {
  exaction_type action;
  unsigned char unused;
} ex_terminate;

#define EXACTION_SPECIFICATION 15

typedef struct ex_specification {
  exaction_type action;
  unsigned char unused;
  unsigned short specs;
  long pcoffset;
  long cinfo_ref;
  char* spec[];
} ex_specification;

#define EXACTION_CATCHBLOCK_32 16

typedef struct ex_catchblock_32 {
  exaction_type action;
  unsigned char unused;
  char* catch_type;
  long catch_pcoffset;
  long cinfo_ref;
} ex_catchblock_32;

// Other structs

typedef struct ExceptionRangeSmall {
  unsigned short start;
  unsigned short end;
  unsigned short action;
} ExceptionRangeSmall;

typedef struct ExceptionTableSmall {
  unsigned short et_field;
  ExceptionRangeSmall ranges[0];
} ExceptionTableSmall;

typedef struct ExceptionRangeLarge {
  unsigned long start;
  unsigned short size;
  unsigned short action;
} ExceptionRangeLarge;

typedef struct ExceptionTableLarge {
  unsigned short et_field;
  unsigned short et_field2;
  ExceptionRangeLarge ranges[];
} ExceptionTableLarge;

#define ET_MakeField(savedGPRs, savedFPRs, savedCR, hasframeptr, isLarge)                          \
  (((savedGPRs) << 11) | ((savedFPRs & 0x1f) << 6) | ((savedCR & 0x1) << 5) |                      \
   ((hasframeptr & 0x1) << 4) | ((isLarge & 1) << 3))

#define ET_GetSavedGPRs(field) ((field) >> 11)
#define ET_GetSavedFPRs(field) (((field) >> 6) & 0x1f)
#define ET_GetSavedCR(field) (((field) >> 5) & 0x1)
#define ET_GetHasFramePtr(field) (((field) >> 4) & 0x1)
#define ET_IsLargeTable(field) (((field) >> 3) & 0x1)
#define ET_ClearLargeBit(field) ((field) & ~(1 << 3))
#define ET_SetLargeBit(field) ((field) | (1 << 3))

#define ET_HasElfVector(field) (((field) >> 1) & 0x1)

typedef struct ExceptionTableIndex {
  unsigned long functionoffset;
  unsigned long eti_field;
  unsigned long exceptionoffset;
} ExceptionTableIndex;

#define ETI_MakeField(direct, fsize) ((((long)(direct)) << 31) | ((fsize) & 0x7fffffff))
#define ETI_GetDirectStore(field) ((field) >> 31)
#define ETI_GetFunctionSize(field) ((field) & 0x7fffffff)



#if __MWERKS__
#pragma exceptions on
#endif

#define RETURN_ADDRESS 4

namespace std {
void terminate();
void unexpected();

class exception {
public:
  exception() throw() {}
  virtual ~exception() throw() {}
  virtual const char* what() const throw();
};

class bad_exception : public exception {
public:
  bad_exception() throw() {}
  virtual ~bad_exception() throw();
  virtual const char* what() const throw();
};

bad_exception::~bad_exception() throw() {}

const char* bad_exception::what() const throw() { return "bad_exception"; }
} // namespace std

union MWE_GeckoVector64 {
  double d;
  float f[2];
};

typedef union MWE_GeckoVector64 MWE_GeckoVector64;

struct GeckoFPRContext {
  double d;
  MWE_GeckoVector64 v;
};

typedef struct GeckoFPRContext GeckoFPRContext;

typedef struct ThrowContext {
  GeckoFPRContext FPR[32];
  long GPR[32];
  long CR;
  char* SP;
  char* FP;
  char* throwSP;
  char* returnaddr;
  char* throwtype;
  void* location;
  void* dtor;
  CatchInfo* catchinfo;
} ThrowContext;

typedef ThrowContext* ThrowContextPtr;

typedef struct MWExceptionInfo {
  ExceptionTableSmall* exception_record;
  char* current_function;
  char* action_pointer;
  char* code_section;
  char* data_section;
  char* TOC;
} MWExceptionInfo;

typedef struct FragmentInfo {
  ExceptionTableIndex* exception_start;
  ExceptionTableIndex* exception_end;
  char* code_start;
  char* code_end;
  char* data_start;
  char* data_end;
  char* TOC;
  int active;
} FragmentInfo;

typedef struct ProcessInfo {
  __eti_init_info* exception_info;
  char* TOC;
  int active;
} ProcessInfo;

typedef struct ActionIterator {
  MWExceptionInfo info;
  char* current_SP;
  char* current_FP;
  long current_R31;
} ActionIterator;

#define MAXFRAGMENTS 1
static ProcessInfo fragmentinfo[MAXFRAGMENTS];

typedef void (*DeleteFunc)(void*);

int __register_fragment(struct __eti_init_info* info, char* TOC) {
  ProcessInfo* f;
  int i;

  for (i = 0, f = fragmentinfo; i < MAXFRAGMENTS; ++i, ++f) {
    if (f->active == 0) {
      f->exception_info = info;
      f->TOC = TOC;
      f->active = 1;
      return i;
    }
  }

  return -1;
}

void __unregister_fragment(int fragmentID) {
  ProcessInfo* f;
  if (fragmentID >= 0 && fragmentID < MAXFRAGMENTS) {
    f = &fragmentinfo[fragmentID];
    f->exception_info = 0;
    f->TOC = 0;
    f->active = 0;
  }
}

static inline int ExPPC_FindExceptionFragment(char* returnaddr, FragmentInfo* frag) {
  ProcessInfo* f;
  int i;
  __eti_init_info* eti_info;

  for (i = 0, f = fragmentinfo; i < MAXFRAGMENTS; ++i, ++f) {
    if (f->active) {
      eti_info = f->exception_info;
      while (1) {
        if (eti_info->code_size == 0)
          break;
        if (returnaddr >= eti_info->code_start &&
            returnaddr < (char*)eti_info->code_start + eti_info->code_size) {
          frag->exception_start = (ExceptionTableIndex*)eti_info->eti_start;
          frag->exception_end = (ExceptionTableIndex*)eti_info->eti_end;
          frag->code_start = 0;
          frag->code_end = 0;
          frag->data_start = 0;
          frag->data_end = 0;
          frag->TOC = f->TOC;
          frag->active = f->active;
          return 1;
        }
        eti_info++;
      }
    }
  }

  return 0;
}

void ExPPC_FindExceptionRecord(char* returnaddr, MWExceptionInfo* info) {
  FragmentInfo* fragment;
  FragmentInfo frag;
  ExceptionTableIndex *exceptionindex, *p;
  unsigned long returnoffset;
  long i, m, n;

  info->exception_record = 0;
  info->action_pointer = 0;

  if ((ExPPC_FindExceptionFragment(returnaddr, &frag)) == 0)
    return;
  fragment = &frag;

  info->code_section = fragment->code_start;
  info->data_section = fragment->data_start;
  info->TOC = fragment->TOC;

  returnoffset = returnaddr - fragment->code_start;
  exceptionindex = fragment->exception_start;
  for (i = 0, n = fragment->exception_end - fragment->exception_start;;) {
    if (i > n)
      return;
    p = &exceptionindex[m = (i + n) / 2];

    if (returnoffset < p->functionoffset) {
      n = m - 1;
    } else if (returnoffset > p->functionoffset + ETI_GetFunctionSize(p->eti_field)) {
      i = m + 1;
    } else
      break;
  }
  info->current_function = fragment->code_start + p->functionoffset;
  info->exception_record = ETI_GetDirectStore(p->eti_field)
                               ? (ExceptionTableSmall*)(&p->exceptionoffset)
                               : (ExceptionTableSmall*)(fragment->data_start + p->exceptionoffset);

  returnoffset -= p->functionoffset;

  if (ET_IsLargeTable(info->exception_record->et_field)) {
    ExceptionTableLarge* etl = (ExceptionTableLarge*)info->exception_record;
    ExceptionRangeLarge* erl;

    for (erl = etl->ranges; erl->start != 0; erl++) {
      unsigned long range_end = erl->start + (erl->size * 4);

      if (erl->start <= returnoffset && range_end >= returnoffset) {
        info->action_pointer = (char*)etl + erl->action;
        break;
      }
    }
  } else {
    ExceptionTableSmall* ets = (ExceptionTableSmall*)info->exception_record;
    ExceptionRangeSmall* ers;

    for (ers = ets->ranges; ers->start != 0; ers++) {
      if (ers->start <= returnoffset && ers->end >= returnoffset) {
        info->action_pointer = (char*)ets + ers->action;
        break;
      }
    }
  }
}

static inline long ExPPC_PopR31(char* SP, MWExceptionInfo* info) {
  double* FPR_save_area;
  long* GPR_save_area;
  int saved_GPRs, saved_FPRs;

  saved_FPRs = ET_GetSavedFPRs(info->exception_record->et_field);
  FPR_save_area = (double*)(SP - saved_FPRs * 8);
  saved_GPRs = ET_GetSavedGPRs(info->exception_record->et_field);
  GPR_save_area = (long*)FPR_save_area;

  return GPR_save_area[-1];
}

static inline exaction_type ExPPC_CurrentAction(const ActionIterator* iter) {
  if (iter->info.action_pointer == 0) {
    return EXACTION_ENDOFLIST;
  }

  return ((ex_destroylocal*)iter->info.action_pointer)->action & EXACTION_MASK;
}

exaction_type ExPPC_NextAction(ActionIterator* iter) {
  exaction_type action;

  for (;;) {
    if (iter->info.action_pointer == 0 ||
        ((action = ((ex_destroylocal*)iter->info.action_pointer)->action) & EXACTION_ENDBIT) != 0) {
      char *return_addr, *callers_SP;

      callers_SP = *(char**)iter->current_SP;

      if (ET_GetSavedGPRs(iter->info.exception_record->et_field)) {
        iter->current_R31 = ExPPC_PopR31(callers_SP, &iter->info);
      }

      return_addr = *(char**)(callers_SP + RETURN_ADDRESS);

      ExPPC_FindExceptionRecord(return_addr, &iter->info);

      if (iter->info.exception_record == 0) {
        std::terminate();
      }

      iter->current_SP = callers_SP;
      iter->current_FP = (ET_GetHasFramePtr(iter->info.exception_record->et_field))
                             ? (char*)iter->current_R31
                             : iter->current_SP;

      if (iter->info.action_pointer == 0)
        continue;
    } else {
      switch (action) {
      case EXACTION_DESTROYLOCAL:
        iter->info.action_pointer += sizeof(ex_destroylocal);
        break;
      case EXACTION_DESTROYLOCALCOND:
        iter->info.action_pointer += sizeof(ex_destroylocalcond);
        break;
      case EXACTION_DESTROYLOCALPOINTER:
        iter->info.action_pointer += sizeof(ex_destroylocalpointer);
        break;
      case EXACTION_DESTROYLOCALARRAY:
        iter->info.action_pointer += sizeof(ex_destroylocalarray);
        break;
      case EXACTION_DESTROYBASE:
      case EXACTION_DESTROYMEMBER:
        iter->info.action_pointer += sizeof(ex_destroymember);
        break;
      case EXACTION_DESTROYMEMBERCOND:
        iter->info.action_pointer += sizeof(ex_destroymembercond);
        break;
      case EXACTION_DESTROYMEMBERARRAY:
        iter->info.action_pointer += sizeof(ex_destroymemberarray);
        break;
      case EXACTION_DELETEPOINTER:
        iter->info.action_pointer += sizeof(ex_deletepointer);
        break;
      case EXACTION_DELETEPOINTERCOND:
        iter->info.action_pointer += sizeof(ex_deletepointercond);
        break;
      case EXACTION_CATCHBLOCK:
        iter->info.action_pointer += sizeof(ex_catchblock);
        break;
      case EXACTION_CATCHBLOCK_32:
        iter->info.action_pointer += sizeof(ex_catchblock_32);
        break;
      case EXACTION_ACTIVECATCHBLOCK:
        iter->info.action_pointer += sizeof(ex_activecatchblock);
        break;
      case EXACTION_SPECIFICATION:
        iter->info.action_pointer +=
            sizeof(ex_specification) +
            ((ex_specification*)iter->info.action_pointer)->specs * sizeof(void*);
        break;
      default:
        std::terminate();
      }
    }

    action = ((ex_destroylocal*)iter->info.action_pointer)->action & EXACTION_MASK;

    if (action == EXACTION_BRANCH) {
      iter->info.action_pointer =
          ((char*)iter->info.exception_record) + ((ex_branch*)iter->info.action_pointer)->target;
      action = ((ex_destroylocal*)iter->info.action_pointer)->action & EXACTION_MASK;
    }
    return action;
  }
}

char* ExPPC_PopStackFrame(ThrowContext* context, MWExceptionInfo* info) {
  char *SP, *callers_SP;
  double* FPR_save_area;
  long* GPR_save_area;
  int saved_GPRs, saved_FPRs;
  GeckoFPRContext* Vector_save_area;
  int i, j;

  SP = context->SP;
  callers_SP = *(char**)SP;
  saved_FPRs = ET_GetSavedFPRs(info->exception_record->et_field);

  if (ET_HasElfVector(info->exception_record->et_field)) {
    Vector_save_area = (GeckoFPRContext*)(callers_SP - saved_FPRs * 16);
    FPR_save_area = (double*)Vector_save_area;
  } else {
    FPR_save_area = (double*)(callers_SP - saved_FPRs * 8);
  }

  if (ET_HasElfVector(info->exception_record->et_field)) {
    for (i = 32 - saved_FPRs, j = 0; i < 32; ++i, ++j) {
      context->FPR[i].v.f[0] = Vector_save_area[j].v.f[0];
      context->FPR[i].v.f[1] = Vector_save_area[j].v.f[1];
      context->FPR[i].d = Vector_save_area[j].d;
    }
  } else {
    for (i = 32 - saved_FPRs, j = 0; i < 32; ++i, ++j) {
      context->FPR[i].d = FPR_save_area[j];
    }
  }

  saved_GPRs = ET_GetSavedGPRs(info->exception_record->et_field);
  GPR_save_area = (long*)FPR_save_area;
  GPR_save_area -= saved_GPRs;

  for (i = 32 - saved_GPRs, j = 0; i < 32; ++i, ++j) {
    context->GPR[i] = GPR_save_area[j];
  }

  context->SP = callers_SP;
  return *(char**)(callers_SP + RETURN_ADDRESS);
}

static inline void ExPPC_DestroyLocal(ThrowContext* context, const ex_destroylocal* ex) {
  DTORCALL_COMPLETE(ex->dtor, context->FP + ex->local);
}

static inline void ExPPC_DestroyLocalCond(ThrowContext* context, const ex_destroylocalcond* ex) {
  int cond = ex_destroylocalcond_GetRegCond(ex->dlc_field)
                 ? (local_cond_type)context->GPR[ex->cond]
                 : *(local_cond_type*)(context->FP + ex->cond);

  if (cond) {
    DTORCALL_COMPLETE(ex->dtor, context->FP + ex->local);
  }
}

static inline void ExPPC_DestroyLocalPointer(ThrowContext* context,
                                             const ex_destroylocalpointer* ex) {
  void* pointer = ex_destroylocalpointer_GetRegPointer(ex->dlp_field)
                      ? (void*)context->GPR[ex->pointer]
                      : *(void**)(context->FP + ex->pointer);

  DTORCALL_COMPLETE(ex->dtor, pointer);
}

static inline void ExPPC_DestroyLocalArray(ThrowContext* context, const ex_destroylocalarray* ex) {
  char* ptr = context->FP + ex->localarray;
  long n = ex->elements;
  long size = ex->element_size;

  for (ptr = ptr + size * n; n > 0; n--) {
    ptr -= size;
    DTORCALL_COMPLETE(ex->dtor, ptr);
  }
}

static inline void ExPPC_DestroyMember(ThrowContext* context, const ex_destroymember* ex) {
  char* objectptr = ex_destroymember_GetRegPointer(ex->dm_field)
                        ? (char*)context->GPR[ex->objectptr]
                        : *(char**)(context->FP + ex->objectptr);

  DTORCALL_COMPLETE(ex->dtor, objectptr + ex->offset);
}

static inline void ExPPC_DestroyBase(ThrowContext* context, const ex_destroymember* ex) {
  char* objectptr = ex_destroymember_GetRegPointer(ex->dm_field)
                        ? (char*)context->GPR[ex->objectptr]
                        : *(char**)(context->FP + ex->objectptr);

  DTORCALL_PARTIAL(ex->dtor, objectptr + ex->offset);
}

static inline void ExPPC_DestroyMemberCond(ThrowContext* context, const ex_destroymembercond* ex) {
  char* objectptr = ex_destroymembercond_GetRegPointer(ex->dmc_field)
                        ? (char*)context->GPR[ex->objectptr]
                        : *(char**)(context->FP + ex->objectptr);
  int cond = ex_destroymembercond_GetRegCond(ex->dmc_field)
                 ? (vbase_ctor_arg_type)context->GPR[ex->cond]
                 : *(vbase_ctor_arg_type*)(context->FP + ex->cond);

  if (cond) {
    DTORCALL_PARTIAL(ex->dtor, objectptr + ex->offset);
  }
}

static inline void ExPPC_DestroyMemberArray(ThrowContext* context,
                                            const ex_destroymemberarray* ex) {
  char* ptr = ex_destroymemberarray_GetRegPointer(ex->dma_field)
                  ? (char*)context->GPR[ex->objectptr]
                  : *(char**)(context->FP + ex->objectptr);
  long n = ex->elements;
  long size = ex->element_size;

  ptr += ex->offset;

  for (ptr = ptr + size * n; n > 0; n--) {
    ptr -= size;
    DTORCALL_COMPLETE(ex->dtor, ptr);
  }
}

static inline void ExPPC_DeletePointer(ThrowContext* context, const ex_deletepointer* ex) {
  char* objectptr = ex_deletepointer_GetRegPointer(ex->dp_field)
                        ? (char*)context->GPR[ex->objectptr]
                        : *(char**)(context->FP + ex->objectptr);

  ((DeleteFunc)ex->deletefunc)(objectptr);
}

static inline void ExPPC_DeletePointerCond(ThrowContext* context, const ex_deletepointercond* ex) {
  char* objectptr = ex_deletepointercond_GetRegPointer(ex->dpc_field)
                        ? (char*)context->GPR[ex->objectptr]
                        : *(char**)(context->FP + ex->objectptr);
  int cond = ex_deletepointercond_GetRegCond(ex->dpc_field)
                 ? (local_cond_type)context->GPR[ex->cond]
                 : *(local_cond_type*)(context->FP + ex->cond);

  if (cond) {
    ((DeleteFunc)ex->deletefunc)(objectptr);
  }
}

void ExPPC_UnwindStack(ThrowContext* context, MWExceptionInfo* info, void* catcher) {
  exaction_type action;

#pragma exception_terminate

  for (;;) {
    if (info->action_pointer == 0) {
      char* return_addr;

      return_addr = ExPPC_PopStackFrame(context, info);
      ExPPC_FindExceptionRecord(return_addr, info);

      if (info->exception_record == 0) {
        std::terminate();
      }

      context->FP = (ET_GetHasFramePtr(info->exception_record->et_field)) ? (char*)context->GPR[31]
                                                                          : context->SP;
      continue;
    }

    action = ((ex_destroylocal*)info->action_pointer)->action;

    switch (action & EXACTION_MASK) {
    case EXACTION_BRANCH:
      info->action_pointer =
          ((char*)info->exception_record) + ((ex_branch*)info->action_pointer)->target;
      break;
    case EXACTION_DESTROYLOCAL:
      ExPPC_DestroyLocal(context, (ex_destroylocal*)info->action_pointer);
      info->action_pointer += sizeof(ex_destroylocal);
      break;
    case EXACTION_DESTROYLOCALCOND:
      ExPPC_DestroyLocalCond(context, (ex_destroylocalcond*)info->action_pointer);
      info->action_pointer += sizeof(ex_destroylocalcond);
      break;
    case EXACTION_DESTROYLOCALPOINTER:
      ExPPC_DestroyLocalPointer(context, (ex_destroylocalpointer*)info->action_pointer);
      info->action_pointer += sizeof(ex_destroylocalpointer);
      break;
    case EXACTION_DESTROYLOCALARRAY:
      ExPPC_DestroyLocalArray(context, (ex_destroylocalarray*)info->action_pointer);
      info->action_pointer += sizeof(ex_destroylocalarray);
      break;
    case EXACTION_DESTROYBASE:
      ExPPC_DestroyBase(context, (ex_destroymember*)info->action_pointer);
      info->action_pointer += sizeof(ex_destroymember);
      break;
    case EXACTION_DESTROYMEMBER:
      ExPPC_DestroyMember(context, (ex_destroymember*)info->action_pointer);
      info->action_pointer += sizeof(ex_destroymember);
      break;
    case EXACTION_DESTROYMEMBERCOND:
      ExPPC_DestroyMemberCond(context, (ex_destroymembercond*)info->action_pointer);
      info->action_pointer += sizeof(ex_destroymembercond);
      break;
    case EXACTION_DESTROYMEMBERARRAY:
      ExPPC_DestroyMemberArray(context, (ex_destroymemberarray*)info->action_pointer);
      info->action_pointer += sizeof(ex_destroymemberarray);
      break;
    case EXACTION_DELETEPOINTER:
      ExPPC_DeletePointer(context, (ex_deletepointer*)info->action_pointer);
      info->action_pointer += sizeof(ex_deletepointer);
      break;
    case EXACTION_DELETEPOINTERCOND:
      ExPPC_DeletePointerCond(context, (ex_deletepointercond*)info->action_pointer);
      info->action_pointer += sizeof(ex_deletepointercond);
      break;
    case EXACTION_CATCHBLOCK:
      if (catcher == (void*)info->action_pointer)
        return;
      info->action_pointer += sizeof(ex_catchblock);
      break;
    case EXACTION_CATCHBLOCK_32:
      if (catcher == (void*)info->action_pointer)
        return;
      info->action_pointer += sizeof(ex_catchblock_32);
      break;
    case EXACTION_ACTIVECATCHBLOCK: {
      CatchInfo* catchinfo;

      catchinfo =
          (CatchInfo*)(context->FP + ((ex_activecatchblock*)info->action_pointer)->cinfo_ref);

      if (catchinfo->dtor) {
        if (context->location == catchinfo->location) {
          context->dtor = catchinfo->dtor;
        } else {
          DTORCALL_COMPLETE(catchinfo->dtor, catchinfo->location);
        }
      }
      info->action_pointer += sizeof(ex_activecatchblock);
    } break;
    case EXACTION_SPECIFICATION:
      if (catcher == (void*)info->action_pointer)
        return;
      info->action_pointer += sizeof(ex_specification) +
                              ((ex_specification*)info->action_pointer)->specs * sizeof(void*);
      break;
    default:
      std::terminate();
    }

    if (action & EXACTION_ENDBIT)
      info->action_pointer = 0;
  }
}

static inline int ExPPC_IsInSpecification(char* extype, ex_specification* spec) {
  long i, offset;

  for (i = 0; i < spec->specs; i++) {
    if (__throw_catch_compare(extype, spec->spec[i], &offset))
      return 1;
  }

  return 0;
}

extern void __unexpected(CatchInfo* catchinfo) {
  ex_specification* unexp = (ex_specification*)catchinfo->stacktop;

#pragma exception_magic

  try {
    std::unexpected();
  } catch (...) {
    if (ExPPC_IsInSpecification((char*)((CatchInfo*)&__exception_magic)->typeinfo, unexp)) {
      throw;
    }
    if (ExPPC_IsInSpecification("!bad_exception!!", unexp)) {
      throw std::bad_exception();
    }
    if (ExPPC_IsInSpecification("!std::bad_exception!!", unexp)) {
      throw std::bad_exception();
    }
  }
  std::terminate();
}

asm static void ExPPC_LongJump(register ThrowContext* context, register void* newRTOC,
                               register void* newPC) {
  /* clang-format off */
  nofralloc

  mr r8, newPC
  mr RTOC, newRTOC
  lwz r0, context->CR
  mtcrf 255, r0

  lmw r13, context->GPR[13]

  la r7, context->FPR[14].v
  psq_lx fp14, 0, r7, 0, 0
  lfd fp14, context->FPR[14].d

  la r7, context->FPR[15].v
  psq_lx fp15, 0, r7, 0, 0
  lfd fp15, context->FPR[15].d

  la r7, context->FPR[16].v
  psq_lx fp16, 0, r7, 0, 0
  lfd fp16, context->FPR[16].d

  la r7, context->FPR[17].v
  psq_lx fp17, 0, r7, 0, 0
  lfd fp17, context->FPR[17].d

  la r7, context->FPR[18].v
  psq_lx fp18, 0, r7, 0, 0
  lfd fp18, context->FPR[18].d

  la r7, context->FPR[19].v
  psq_lx fp19, 0, r7, 0, 0
  lfd fp19, context->FPR[19].d

  la r7, context->FPR[20].v
  psq_lx fp20, 0, r7, 0, 0
  lfd fp20, context->FPR[20].d

  la r7, context->FPR[21].v
  psq_lx fp21, 0, r7, 0, 0
  lfd fp21, context->FPR[21].d

  la r7, context->FPR[22].v
  psq_lx fp22, 0, r7, 0, 0
  lfd fp22, context->FPR[22].d

  la r7, context->FPR[23].v
  psq_lx fp23, 0, r7, 0, 0
  lfd fp23, context->FPR[23].d

  la r7, context->FPR[24].v
  psq_lx fp24, 0, r7, 0, 0
  lfd fp24, context->FPR[24].d

  la r7, context->FPR[25].v
  psq_lx fp25, 0, r7, 0, 0
  lfd fp25, context->FPR[25].d

  la r7, context->FPR[26].v
  psq_lx fp26, 0, r7, 0, 0
  lfd fp26, context->FPR[26].d

  la r7, context->FPR[27].v
  psq_lx fp27, 0, r7, 0, 0
  lfd fp27, context->FPR[27].d

  la r7, context->FPR[28].v
  psq_lx fp28, 0, r7, 0, 0
  lfd fp28, context->FPR[28].d

  la r7, context->FPR[29].v
  psq_lx fp29, 0, r7, 0, 0
  lfd fp29, context->FPR[29].d

  la r7, context->FPR[30].v
  psq_lx fp30, 0, r7, 0, 0
  lfd fp30, context->FPR[30].d

  la r7, context->FPR[31].v
  psq_lx fp31, 0, r7, 0, 0
  lfd fp31, context->FPR[31].d

  mtlr r8

  lwz SP, context->throwSP
  lwz r3, context->SP
  lwz r3, 0(r3)
  stw r3, 0(SP)
  blr
  /* clang-format on */
}

static inline void ExPPC_HandleUnexpected(ThrowContext* context, MWExceptionInfo* info,
                                          ex_specification* unexp) {
  CatchInfo* catchinfo;

#pragma exception_terminate

  ExPPC_UnwindStack(context, info, unexp);

  catchinfo = (CatchInfo*)(context->FP + unexp->cinfo_ref);
  catchinfo->location = context->location;
  catchinfo->typeinfo = context->throwtype;
  catchinfo->dtor = context->dtor;
  catchinfo->stacktop = unexp;

  ExPPC_LongJump(context, info->TOC, info->current_function + unexp->pcoffset);
}

void ExPPC_ThrowHandler(ThrowContext* context) {
  ActionIterator iter;
  MWExceptionInfo info;
  exaction_type action;
  CatchInfo* catchinfo;
  long offset;

  ExPPC_FindExceptionRecord(context->returnaddr, &info);

  if (info.exception_record == 0) {
    std::terminate();
  }

  context->FP =
      (ET_GetHasFramePtr(info.exception_record->et_field)) ? (char*)context->GPR[31] : context->SP;

  if (context->throwtype == 0) {
    iter.info = info;
    iter.current_SP = context->SP;
    iter.current_FP = context->FP;
    iter.current_R31 = context->GPR[31];

    for (action = ExPPC_CurrentAction(&iter);; action = ExPPC_NextAction(&iter)) {
      switch (action) {
      case EXACTION_ACTIVECATCHBLOCK:
        break;
      case EXACTION_ENDOFLIST:
      case EXACTION_DESTROYLOCAL:
      case EXACTION_DESTROYLOCALCOND:
      case EXACTION_DESTROYLOCALPOINTER:
      case EXACTION_DESTROYLOCALARRAY:
      case EXACTION_DESTROYBASE:
      case EXACTION_DESTROYMEMBER:
      case EXACTION_DESTROYMEMBERCOND:
      case EXACTION_DESTROYMEMBERARRAY:
      case EXACTION_DELETEPOINTER:
      case EXACTION_DELETEPOINTERCOND:
      case EXACTION_CATCHBLOCK:
      case EXACTION_CATCHBLOCK_32:
      case EXACTION_SPECIFICATION:
        continue;
      case EXACTION_TERMINATE:
      default:
        std::terminate();
      }
      break;
    }

    catchinfo =
        (CatchInfo*)(iter.current_FP + ((ex_activecatchblock*)iter.info.action_pointer)->cinfo_ref);
    context->throwtype = (char*)catchinfo->typeinfo;
    context->location = catchinfo->location;
    context->dtor = 0;
    context->catchinfo = catchinfo;
  } else {
    context->catchinfo = 0L;
  }

  iter.info = info;
  iter.current_SP = context->SP;
  iter.current_FP = context->FP;
  iter.current_R31 = context->GPR[31];

  for (action = ExPPC_CurrentAction(&iter);; action = ExPPC_NextAction(&iter)) {
    switch (action) {
    case EXACTION_CATCHBLOCK_32:
      if (__throw_catch_compare(context->throwtype,
                                ((ex_catchblock_32*)iter.info.action_pointer)->catch_type,
                                &offset)) {
        break;
      }
      continue;
    case EXACTION_CATCHBLOCK:
      if (__throw_catch_compare(context->throwtype,
                                ((ex_catchblock*)iter.info.action_pointer)->catch_type, &offset)) {
        break;
      }
      continue;
    case EXACTION_SPECIFICATION:
      if (!ExPPC_IsInSpecification(context->throwtype,
                                   (ex_specification*)iter.info.action_pointer)) {
        ExPPC_HandleUnexpected(context, &info, (ex_specification*)iter.info.action_pointer);
      }
      continue;
    case EXACTION_ENDOFLIST:
    case EXACTION_DESTROYLOCAL:
    case EXACTION_DESTROYLOCALCOND:
    case EXACTION_DESTROYLOCALPOINTER:
    case EXACTION_DESTROYLOCALARRAY:
    case EXACTION_DESTROYBASE:
    case EXACTION_DESTROYMEMBER:
    case EXACTION_DESTROYMEMBERCOND:
    case EXACTION_DESTROYMEMBERARRAY:
    case EXACTION_DELETEPOINTER:
    case EXACTION_DELETEPOINTERCOND:
    case EXACTION_ACTIVECATCHBLOCK:
      continue;
    case EXACTION_TERMINATE:
    default:
      std::terminate();
    }
    break;
  }

  if (action == EXACTION_CATCHBLOCK_32) {
    ex_catchblock_32* catchblock_32;
    catchblock_32 = (ex_catchblock_32*)iter.info.action_pointer;

    ExPPC_UnwindStack(context, &info, catchblock_32);

    catchinfo = (CatchInfo*)(context->FP + catchblock_32->cinfo_ref);
    catchinfo->location = context->location;
    catchinfo->typeinfo = context->throwtype;
    catchinfo->dtor = context->dtor;

    if (*context->throwtype == '*') {
      catchinfo->sublocation = &catchinfo->pointercopy;
      catchinfo->pointercopy = *(long*)context->location + offset;
    } else {
      catchinfo->sublocation = (char*)context->location + offset;
    }

    ExPPC_LongJump(context, info.TOC, info.current_function + catchblock_32->catch_pcoffset);
  } else {
    ex_catchblock* catchblock;

    catchblock = (ex_catchblock*)iter.info.action_pointer;
    ExPPC_UnwindStack(context, &info, catchblock);

    catchinfo = (CatchInfo*)(context->FP + catchblock->cinfo_ref);
    catchinfo->location = context->location;
    catchinfo->typeinfo = context->throwtype;
    catchinfo->dtor = context->dtor;

    if (*context->throwtype == '*') {
      catchinfo->sublocation = &catchinfo->pointercopy;
      catchinfo->pointercopy = *(long*)context->location + offset;
    } else {
      catchinfo->sublocation = (char*)context->location + offset;
    }

    ExPPC_LongJump(context, info.TOC, info.current_function + catchblock->catch_pcoffset);
  }
}

void __end__catch(CatchInfo* catchinfo) {
  if (catchinfo->location && catchinfo->dtor) {
    DTORCALL_COMPLETE(catchinfo->dtor, catchinfo->location);
  }
}

asm void __throw(char* throwtype, void* location, void* dtor) {
  /* clang-format off */
  ThrowContext throwcontext;

  fralloc

  stmw r13, throwcontext.GPR[13]

  stfd fp14, throwcontext.FPR[14].d
  la r3, throwcontext.FPR[14].v
  psq_stx fp14, 0, r3, 0, 0

  stfd fp15, throwcontext.FPR[15].d
  la r3, throwcontext.FPR[15].v
  psq_stx fp15, 0, r3, 0, 0

  stfd fp16, throwcontext.FPR[16].d
  la r3, throwcontext.FPR[16].v
  psq_stx fp16, 0, r3, 0, 0

  stfd fp17, throwcontext.FPR[17].d
  la r3, throwcontext.FPR[17].v
  psq_stx fp17, 0, r3, 0, 0

  stfd fp18, throwcontext.FPR[18].d
  la r3, throwcontext.FPR[18].v
  psq_stx fp18, 0, r3, 0, 0

  stfd fp19, throwcontext.FPR[19].d
  la r3, throwcontext.FPR[19].v
  psq_stx fp19, 0, r3, 0, 0

  stfd fp20, throwcontext.FPR[20].d
  la r3, throwcontext.FPR[20].v
  psq_stx fp20, 0, r3, 0, 0

  stfd fp21, throwcontext.FPR[21].d
  la r3, throwcontext.FPR[21].v
  psq_stx fp21, 0, r3, 0, 0

  stfd fp22, throwcontext.FPR[22].d
  la r3, throwcontext.FPR[22].v
  psq_stx fp22, 0, r3, 0, 0

  stfd fp23, throwcontext.FPR[23].d
  la r3, throwcontext.FPR[23].v
  psq_stx fp23, 0, r3, 0, 0

  stfd fp24, throwcontext.FPR[24].d
  la r3, throwcontext.FPR[24].v
  psq_stx fp24, 0, r3, 0, 0

  stfd fp25, throwcontext.FPR[25].d
  la r3, throwcontext.FPR[25].v
  psq_stx fp25, 0, r3, 0, 0

  stfd fp26, throwcontext.FPR[26].d
  la r3, throwcontext.FPR[26].v
  psq_stx fp26, 0, r3, 0, 0

  stfd fp27, throwcontext.FPR[27].d
  la r3, throwcontext.FPR[27].v
  psq_stx fp27, 0, r3, 0, 0

  stfd fp28, throwcontext.FPR[28].d
  la r3, throwcontext.FPR[28].v
  psq_stx fp28, 0, r3, 0, 0

  stfd fp29, throwcontext.FPR[29].d
  la r3, throwcontext.FPR[29].v
  psq_stx fp29, 0, r3, 0, 0

  stfd fp30, throwcontext.FPR[30].d
  la r3, throwcontext.FPR[30].v
  psq_stx fp30, 0, r3, 0, 0

  stfd fp31, throwcontext.FPR[31].d
  la r3, throwcontext.FPR[31].v
  psq_stx fp31, 0, r3, 0, 0

  mfcr r3
  stw r3, throwcontext.CR;

  lwz r3, 0(sp)
  lwz r4, RETURN_ADDRESS(r3)
  stw r3, throwcontext.SP;
  stw r3, throwcontext.throwSP;
  stw r4, throwcontext.returnaddr;

  lwz r3, throwtype
  stw r3, throwcontext.throwtype
  lwz r3, location
  stw r3, throwcontext.location
  lwz r3, dtor
  stw r3, throwcontext.dtor
  la r3, throwcontext
  bl ExPPC_ThrowHandler
  nop
  frfree
  blr
  /* clang-format on */
}

extern "C" {

asm int __setjmp(register unsigned long* env) {
  nofralloc
  mflr r5
  mfcr r6
  stw r5, 0(r3)
  stw r6, 4(r3)
  stw SP, 8(r3)
  stw RTOC, 12(r3)
  stmw r13, 20(r3)
  mffs fp0
  stfd fp14, 96(r3)
  la r4, 104(r3)
  psq_stx fp14, 0, r4, 0, 0
  stfd fp15, 112(r3)
  la r4, 120(r3)
  psq_stx fp15, 0, r4, 0, 0
  stfd fp16, 128(r3)
  la r4, 136(r3)
  psq_stx fp16, 0, r4, 0, 0
  stfd fp17, 144(r3)
  la r4, 152(r3)
  psq_stx fp17, 0, r4, 0, 0
  stfd fp18, 160(r3)
  la r4, 168(r3)
  psq_stx fp18, 0, r4, 0, 0
  stfd fp19, 176(r3)
  la r4, 184(r3)
  psq_stx fp19, 0, r4, 0, 0
  stfd fp20, 192(r3)
  la r4, 200(r3)
  psq_stx fp20, 0, r4, 0, 0
  stfd fp21, 208(r3)
  la r4, 216(r3)
  psq_stx fp21, 0, r4, 0, 0
  stfd fp22, 224(r3)
  la r4, 232(r3)
  psq_stx fp22, 0, r4, 0, 0
  stfd fp23, 240(r3)
  la r4, 248(r3)
  psq_stx fp23, 0, r4, 0, 0
  stfd fp24, 256(r3)
  la r4, 264(r3)
  psq_stx fp24, 0, r4, 0, 0
  stfd fp25, 272(r3)
  la r4, 280(r3)
  psq_stx fp25, 0, r4, 0, 0
  stfd fp26, 288(r3)
  la r4, 296(r3)
  psq_stx fp26, 0, r4, 0, 0
  stfd fp27, 304(r3)
  la r4, 312(r3)
  psq_stx fp27, 0, r4, 0, 0
  stfd fp28, 320(r3)
  la r4, 328(r3)
  psq_stx fp28, 0, r4, 0, 0
  stfd fp29, 336(r3)
  la r4, 344(r3)
  psq_stx fp29, 0, r4, 0, 0
  stfd fp30, 352(r3)
  la r4, 360(r3)
  psq_stx fp30, 0, r4, 0, 0
  stfd fp31, 368(r3)
  la r4, 376(r3)
  psq_stx fp31, 0, r4, 0, 0
  stfd fp0, 384(r3)
  li r3, 0
  blr
}

asm void longjmp(register unsigned long* env, register int value) {
  nofralloc
  lwz r5, 0(r3)
  lwz r6, 4(r3)
  mtlr r5
  mtcr r6
  lwz SP, 8(r3)
  lwz RTOC, 12(r3)
  lmw r13, 20(r3)
  lfd fp14, 96(r3)
  la r7, 104(r3)
  psq_lx fp14, 0, r7, 0, 0
  lfd fp15, 112(r3)
  la r7, 120(r3)
  psq_lx fp15, 0, r7, 0, 0
  lfd fp16, 128(r3)
  la r7, 136(r3)
  psq_lx fp16, 0, r7, 0, 0
  lfd fp17, 144(r3)
  la r7, 152(r3)
  psq_lx fp17, 0, r7, 0, 0
  lfd fp18, 160(r3)
  la r7, 168(r3)
  psq_lx fp18, 0, r7, 0, 0
  lfd fp19, 176(r3)
  la r7, 184(r3)
  psq_lx fp19, 0, r7, 0, 0
  lfd fp20, 192(r3)
  la r7, 200(r3)
  psq_lx fp20, 0, r7, 0, 0
  lfd fp21, 208(r3)
  la r7, 216(r3)
  psq_lx fp21, 0, r7, 0, 0
  lfd fp22, 224(r3)
  la r7, 232(r3)
  psq_lx fp22, 0, r7, 0, 0
  lfd fp23, 240(r3)
  la r7, 248(r3)
  psq_lx fp23, 0, r7, 0, 0
  lfd fp24, 256(r3)
  la r7, 264(r3)
  psq_lx fp24, 0, r7, 0, 0
  lfd fp25, 272(r3)
  la r7, 280(r3)
  psq_lx fp25, 0, r7, 0, 0
  lfd fp26, 288(r3)
  la r7, 296(r3)
  psq_lx fp26, 0, r7, 0, 0
  lfd fp27, 304(r3)
  la r7, 312(r3)
  psq_lx fp27, 0, r7, 0, 0
  lfd fp28, 320(r3)
  la r7, 328(r3)
  psq_lx fp28, 0, r7, 0, 0
  lfd fp29, 336(r3)
  la r7, 344(r3)
  psq_lx fp29, 0, r7, 0, 0
  lfd fp30, 352(r3)
  la r7, 360(r3)
  psq_lx fp30, 0, r7, 0, 0
  lfd fp31, 368(r3)
  la r7, 376(r3)
  psq_lx fp31, 0, r7, 0, 0
  lfd fp0, 384(r3)
  cmpwi r4, 0
  mr r3, r4
  mtfsf 255, fp0
  bnelr
  li r3, 1
  blr
}

}
