#include <revolution/kpad_2005.h>
#include <dolphin/os.h>
#include <math.h>

#define KPAD_BUTTON_MASK 0x0000ffff
#define KPAD_BUTTON_RPT 0x80000000
#define KPAD_BTN_RPT_TIME_MAX 20000
#define KPAD_BTN_NO_RPT_DELAY 40000
#define KPAD_USE_OBJECTS 2
#define KPAD_MAX_SAMPLES 12
#define WPAD_MAX_CONTROLLERS 4
#define WPAD_DPD_MAX_OBJECTS 4
#define KPAD_DPD_RESO_WX 1024
#define KPAD_DPD_RESO_WY 768

typedef struct { // Guessed name
  Vec2 center;
  s8 error_fg;
  s8 state_fg;
  s8 _fg_1; // Guessed name
  s8 _fg_2; // Guessed name
} KPADObject;

typedef struct { // Guessed name
  KPADStatus status;
  f32 pos_play_radius;
  f32 pos_sensitivity;
  f32 hori_play_radius;
  f32 hori_sensitivity;
  f32 dist_play_radius;
  f32 dist_sensitivity;
  f32 acc_play_radius;
  f32 acc_sensitivity;
  f32 dist_org;
  Vec2 accXY_nrm_hori;
  Vec2 sec_nrm_hori;
  Vec2 center_org;
  f32 dpd2pos_scale;
  KPADObject kobj_sample[4];
  KPADObject kobj_regular[2];
  s16 valid_objs;
  u16 read_count; // Guessed name
  s16 wpad_chan_no;
  s16 wpad_ring_idx;
  union {
    WPADStatus core[KPAD_MAX_SAMPLES];
    WPADFSStatus fs[KPAD_MAX_SAMPLES];
  } wpad_ring_buf; // Guessed name
  f32 sec_length;
  Vec2 sec_nrm;
  f32 sec_dist;
  f32 trust_sec_length;
  KPADVec hard_acc;
  Vec2 obj_horizon;
  Vec2 acc_horizon;
  Vec2 acc_vertical;
  Vec2 ah_circle_pos;
  u16 ah_circle_ct;
  u8 dpd_valid2_ct;
  s8 _fg_4; // Guessed name
  u16 btn_repeat_time;
  u16 btn_repeat_next;
  u16 btn_repeat_delay;
  u16 btn_repeat_pulse;
} KPADInsideStatus;

const char *__KPADVersion =
    "<< Revolution SDK - KPAD\trelease build: Nov 28 2005 14:06:27 (0x2301) >>";

static Vec2 icenter_org = {0.0f, 0.0f};
static f32 idist_org = 1.0f;
static Vec2 iaccXY_nrm_hori = {0.0f, -1.0f};
static Vec2 isec_nrm_hori = {1.0f, 0.0f};
f32 kp_obj_interval = 0.3f;
f32 kp_acc_horizon_pw = 0.05f;
f32 kp_ah_circle_radius = 0.07f;
f32 kp_ah_circle_pw = 0.06f;
u16 kp_ah_circle_ct = 100;
f32 kp_err_outside_frame = 0.05f;
f32 kp_err_dist_max = 3.0f;
f32 kp_err_dist_speed = 0.04f;
f32 kp_err_first_inpr = 0.9f;
f32 kp_err_next_inpr = 0.9f;
f32 kp_err_acc_inpr = 0.9f;
f32 kp_err_up_inpr = 0.7f;
f32 kp_err_near_pos = 0.1f;
static s32 kp_fs_fstick_min = 15;
static s32 kp_fs_fstick_max = 87;
static s32 kp_gc_mstick_min = 15;
static s32 kp_gc_mstick_max = 87;
static s32 kp_gc_cstick_min = 15;
static s32 kp_gc_cstick_max = 74;
static s32 kp_gc_trigger_min = 30;
static s32 kp_gc_trigger_max = 180;
KPADInsideStatus inside_kpads[WPAD_MAX_CONTROLLERS];
static Vec2 Vec2_0 = {0.0f, 0.0f};
static Vec2 kobj_frame_min, kobj_frame_max;
static f32 kp_err_dist_speed_1;
static f32 kp_err_dist_speedM_1;
static f32 kp_ah_circle_radius2;
static f32 kp_dist_vv1;
f32 kp_err_dist_min;

void KPADSetObjInterval(f32 interval) {
  kp_obj_interval = interval;
  kp_err_dist_min = interval / 0.383864f;
  kp_dist_vv1 = interval / 0.383864f;
}

void KPADSetPosParam(s32 chan, f32 play_radius, f32 sensitivity) {
  inside_kpads[chan].pos_play_radius = play_radius;
  inside_kpads[chan].pos_sensitivity = sensitivity;
}

static void calc_dpd2pos_scale(KPADInsideStatus *kp) {
  f32 sx, sy;
  sx = 1.0f;
  sy = (f32)KPAD_DPD_RESO_WY / (f32)KPAD_DPD_RESO_WX;
  kp->dpd2pos_scale = sqrtf(sx * sx + sy * sy);
  if (kp->center_org.x < 0.0f) {
    sx += kp->center_org.x;
  } else {
    sx -= kp->center_org.x;
  }
  if (kp->center_org.y < 0.0f) {
    sy += kp->center_org.y;
  } else {
    sy -= kp->center_org.y;
  }
  if (sx < sy) {
    kp->dpd2pos_scale /= sx;
  } else {
    kp->dpd2pos_scale /= sy;
  }
}

static void reset_kpad(KPADInsideStatus *kp) {
  KPADObject *op;
  KPADStatus *sp = &kp->status;
  KPADEXStatus *ep = &kp->status.ex_status;

  sp->hold = sp->trig = sp->release = 0;
  kp->btn_repeat_time = 0;
  kp->btn_repeat_next = kp->btn_repeat_delay;

  sp->dpd_valid_fg = 0;
  kp->dpd_valid2_ct = 0;

  sp->pos = sp->vec = Vec2_0;
  sp->speed = 0.0f;

  sp->horizon.x = kp->acc_horizon.x = kp->obj_horizon.x = 1.0f;
  sp->horizon.y = kp->acc_horizon.y = kp->obj_horizon.y = 0.0f;
  sp->hori_vec = Vec2_0;
  sp->hori_speed = 0.0f;

  kp->acc_vertical.x = 1.0f;
  kp->acc_vertical.y = 0.0f;

  sp->dist = kp->dist_org;
  sp->dist_vec = sp->dist_speed = 0.0f;

  kp->sec_dist = sp->dist;
  kp->sec_length = kp->trust_sec_length = kp_dist_vv1 / kp->sec_dist;
  kp->sec_nrm = kp->sec_nrm_hori;

  sp->acc.x = sp->acc.z = 0.0f;
  sp->acc.y = -1.0f;
  sp->acc_value = 1.0f;
  sp->acc_speed = 0.0f;
  kp->hard_acc = sp->acc;

  kp->ah_circle_pos = kp->acc_horizon;
  kp->ah_circle_ct = kp_ah_circle_ct;

  kp->valid_objs = 0;

  op = &kp->kobj_sample[WPAD_DPD_MAX_OBJECTS - 1];
  do {
    op->error_fg = -1;
  } while (--op >= kp->kobj_sample);

  op = &kp->kobj_regular[KPAD_USE_OBJECTS - 1];
  do {
    op->error_fg = -1;
  } while (--op >= kp->kobj_regular);

  kp->wpad_ring_idx = -1;
  kp->read_count = 0;
  sp->wpad_err = 0;
  if (kp->wpad_chan_no < 0) {
    WPADControlMotor(kp->wpad_chan_no + WPAD_MAX_CONTROLLERS, WPAD_MOTOR_STOP);
  } else {
    WPADControlMotor(kp->wpad_chan_no, WPAD_MOTOR_STOP);
  }
  switch (sp->dev_type) {
  case WPAD_DEV_FREESTYLE:
    ep->fs.stick = Vec2_0;
    ep->fs.acc.x = ep->fs.acc.z = 0.0f;
    ep->fs.acc.y = -1.0f;
    ep->fs.acc_value = 1.0f;
    ep->fs.acc_speed = 0.0f;
    break;
  case WPAD_DEV_DOLPHIN:
    ep->gc.stick = ep->gc.substick = Vec2_0;
    ep->gc.ltrigger = ep->gc.rtrigger = 0.0f;
    break;
  }
}

static void calc_button_repeat(KPADInsideStatus *kp, KPADStatus *sp) {

  if (sp->trig != 0 || sp->release != 0) {
    kp->btn_repeat_time = 0;
    kp->btn_repeat_next = kp->btn_repeat_delay;
    if (sp->trig && kp->btn_repeat_pulse) {
      sp->hold |= KPAD_BUTTON_RPT;
    }
  } else if (sp->hold != 0) {
    kp->btn_repeat_time += kp->read_count;
    if (kp->btn_repeat_time >= KPAD_BTN_NO_RPT_DELAY) {
      kp->btn_repeat_time -= KPAD_BTN_NO_RPT_DELAY;
    }

    if (kp->btn_repeat_time >= kp->btn_repeat_next) {
      sp->hold |= KPAD_BUTTON_RPT;
      kp->btn_repeat_next += kp->btn_repeat_pulse;
      if (kp->btn_repeat_time >= KPAD_BTN_RPT_TIME_MAX) {
        kp->btn_repeat_time -= KPAD_BTN_RPT_TIME_MAX;
        kp->btn_repeat_next -= KPAD_BTN_RPT_TIME_MAX;
      }
    }
  }
}

static void read_kpad_button(KPADInsideStatus *kp, void *wp, BOOL fs_flag) {
  KPADStatus *sp = &kp->status;
  u32 old_fg, change_fg;
  WPADStatus *core = wp;
  WPADFSStatus *fs = wp;

  if (kp->wpad_chan_no < 0) {
    return;
  }
  old_fg = sp->hold & KPAD_BUTTON_MASK;
  if (fs_flag) {
    if (fs->err != WPAD_ERR_NONE) {
      if (fs->err == WPAD_ERR_NO_CONTROLLER) {
        kp->wpad_chan_no -= WPAD_MAX_CONTROLLERS;
        reset_kpad(kp);
        return;
      }
    } else {
      sp->hold = fs->button;
    }
  } else {
    if (core->err != WPAD_ERR_NONE) {
      if (core->err == WPAD_ERR_NO_CONTROLLER) {
        kp->wpad_chan_no -= WPAD_MAX_CONTROLLERS;
        reset_kpad(kp);
        return;
      }
    } else {
      sp->hold = core->button;
    }
  }
  change_fg = sp->hold ^ old_fg;
  sp->trig = change_fg & sp->hold;
  sp->release = change_fg & old_fg;
  calc_button_repeat(kp, sp);
}

static void calc_acc(KPADInsideStatus *kp, f32 *acc, f32 acc2) {
  f32 f1, f2;

  f2 = acc2 - *acc;

  if (f2 < 0.0f) {
    f1 = -f2;
  } else {
    f1 = f2;
  }

  if (f1 >= kp->acc_play_radius) {
    f1 = 1.0f;
  } else {
    f1 /= kp->acc_play_radius;
    f1 *= f1;
    f1 *= f1;
  }
  f1 *= kp->acc_sensitivity;
  *acc += f1 * f2;
}

static void calc_acc_horizon(KPADInsideStatus *kp) {
  f32 f1, vx, vy, ax, ay;
  f1 = sqrtf(kp->hard_acc.x * kp->hard_acc.x + kp->hard_acc.y * kp->hard_acc.y);
  if (f1 == 0.0f || f1 >= 2.0f) {
    return;
  }
  ax = kp->hard_acc.x / f1;
  ay = kp->hard_acc.y / f1;

  if (f1 > 1.0f) {
    f1 = 2.0f - f1;
  }
  f1 *= f1 * kp_acc_horizon_pw;

  vx = kp->accXY_nrm_hori.x * ax + kp->accXY_nrm_hori.y * ay;
  vy = kp->accXY_nrm_hori.y * ax - kp->accXY_nrm_hori.x * ay;
  ax = (vx - kp->acc_horizon.x) * f1 + kp->acc_horizon.x;
  ay = (vy - kp->acc_horizon.y) * f1 + kp->acc_horizon.y;

  f1 = sqrtf(ax * ax + ay * ay);
  if (f1 == 0.0f) {
    return;
  }
  kp->acc_horizon.x = ax / f1;
  kp->acc_horizon.y = ay / f1;

  kp->ah_circle_pos.x += (kp->acc_horizon.x - kp->ah_circle_pos.x) * kp_ah_circle_pw;
  kp->ah_circle_pos.y += (kp->acc_horizon.y - kp->ah_circle_pos.y) * kp_ah_circle_pw;

  vx = kp->acc_horizon.x - kp->ah_circle_pos.x;
  vy = kp->acc_horizon.y - kp->ah_circle_pos.y;
  if (vx * vx + vy * vy <= kp_ah_circle_radius2) {
    if (kp->ah_circle_ct)
      --kp->ah_circle_ct;
  } else {
    kp->ah_circle_ct = kp_ah_circle_ct;
  }
}

static void calc_acc_vertical(KPADInsideStatus *kp) {
  f32 f1, f2, ax, ay;

  ax = sqrtf(f2 = kp->hard_acc.x * kp->hard_acc.x + kp->hard_acc.y * kp->hard_acc.y);
  ay = -kp->hard_acc.z;
  f1 = sqrtf(f2 + ay * ay);
  if (f1 == 0.0f || f1 >= 2.0f) {
    return;
  }
  f2 = ax / f1;
  ay /= f1;

  if (f1 > 1.0f) {
    f1 = 2.0f - f1;
  }
  f1 *= f1 * kp_acc_horizon_pw;

  ax = (f2 - kp->acc_vertical.x) * f1 + kp->acc_vertical.x;
  ay = (ay - kp->acc_vertical.y) * f1 + kp->acc_vertical.y;

  f1 = sqrtf(ax * ax + ay * ay);
  if (f1 == 0.0f) {
    return;
  }
  kp->acc_vertical.x = ax / f1;
  kp->acc_vertical.y = ay / f1;
}

static void read_kpad_acc(KPADInsideStatus *kp, void *wp, BOOL fs_flag) {
  KPADStatus *sp = &kp->status;
  KPADVec vec;
  WPADFSStatus *fs;
  const f32 acc_scale = 1.0f / 205.0f;

  s32 chan = kp->wpad_chan_no;

  if (chan < 0) {
    return;
  }
  if (fs_flag) {
    fs = (WPADFSStatus *)wp;
    if (fs->err != WPAD_ERR_NONE) {
      if (fs->err == WPAD_ERR_NO_CONTROLLER) {
        kp->wpad_chan_no = chan - WPAD_MAX_CONTROLLERS;
        reset_kpad(kp);
        return;
      }
    } else {
      kp->hard_acc.x = acc_scale * (f32)-fs->accX;
      kp->hard_acc.y = acc_scale * (f32)-fs->accZ;
      kp->hard_acc.z = acc_scale * (f32)fs->accY;
    }
  } else {
    WPADStatus *core = (WPADStatus *)wp;
    if (core->err != WPAD_ERR_NONE) {
      if (core->err == WPAD_ERR_NO_CONTROLLER) {
        kp->wpad_chan_no = chan - WPAD_MAX_CONTROLLERS;
        reset_kpad(kp);
        return;
      }
    } else {
      kp->hard_acc.x = acc_scale * (f32)-core->accX;
      kp->hard_acc.y = acc_scale * (f32)-core->accZ;
      kp->hard_acc.z = acc_scale * (f32)core->accY;
    }
  }
  vec = sp->acc;
  calc_acc(kp, &sp->acc.x, kp->hard_acc.x);
  calc_acc(kp, &sp->acc.y, kp->hard_acc.y);
  calc_acc(kp, &sp->acc.z, kp->hard_acc.z);
  sp->acc_value = sqrtf(sp->acc.x * sp->acc.x + sp->acc.y * sp->acc.y + sp->acc.z * sp->acc.z);
  vec.x -= sp->acc.x;
  vec.y -= sp->acc.y;
  vec.z -= sp->acc.z;
  sp->acc_speed = sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
  calc_acc_horizon(kp);
  calc_acc_vertical(kp);
  if (fs_flag) {
    vec = sp->ex_status.fs.acc;
    calc_acc(kp, &sp->ex_status.fs.acc.x, acc_scale * (f32)-fs->fsAccX);
    calc_acc(kp, &sp->ex_status.fs.acc.y, acc_scale * (f32)-fs->fsAccZ);
    calc_acc(kp, &sp->ex_status.fs.acc.z, acc_scale * (f32)fs->fsAccY);
    sp->ex_status.fs.acc_value = sqrtf(sp->ex_status.fs.acc.x * sp->ex_status.fs.acc.x +
                                       sp->ex_status.fs.acc.y * sp->ex_status.fs.acc.y +
                                       sp->ex_status.fs.acc.z * sp->ex_status.fs.acc.z);
    vec.x -= sp->ex_status.fs.acc.x;
    vec.y -= sp->ex_status.fs.acc.y;
    vec.z -= sp->ex_status.fs.acc.z;
    sp->ex_status.fs.acc_speed = sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
  }
}

static f32 calc_horizon(KPADInsideStatus *kp, Vec2 *p1, Vec2 *p2, Vec2 *hori) {
  f32 f1, f2, vx, vy;

  vx = p2->x - p1->x;
  vy = p2->y - p1->y;
  f2 = sqrtf(vx * vx + vy * vy);
  f1 = 1.0f / f2;
  vx *= f1;
  vy *= f1;

  hori->x = kp->sec_nrm_hori.x * vx + kp->sec_nrm_hori.y * vy;
  hori->y = kp->sec_nrm_hori.y * vx - kp->sec_nrm_hori.x * vy;

  return kp_dist_vv1 * f1;
}

static s8 select_2obj_first(KPADInsideStatus *kp) {
  KPADObject *op1, *op2, *rp1, *rp2;
  Vec2 hori;
  f32 f1, max = kp_err_first_inpr;

  op1 = kp->kobj_sample;
  do {
    if (op1->error_fg != 0) {
      continue;
    }

    op2 = op1 + 1;
    do {
      if (op2->error_fg != 0) {
        continue;
      }

      f1 = calc_horizon(kp, &op1->center, &op2->center, &hori);

      if (f1 <= kp_err_dist_min || f1 >= kp_err_dist_max) {
        continue;
      }

      f1 = kp->acc_horizon.x * hori.x + kp->acc_horizon.y * hori.y;
      if (f1 < 0.0f) {
        if (-f1 > max) {
          max = -f1;
          rp1 = op2;
          rp2 = op1;
        }
      } else {
        if (f1 > max) {
          max = f1;
          rp1 = op1;
          rp2 = op2;
        }
      }

    } while (++op2 <= &kp->kobj_sample[WPAD_DPD_MAX_OBJECTS - 1]);
  } while (++op1 < &kp->kobj_sample[WPAD_DPD_MAX_OBJECTS - 1]);

  if (max == kp_err_first_inpr) {
    return 0;
  }

  kp->kobj_regular[0] = *rp1;
  kp->kobj_regular[1] = *rp2;

  return 2;
}

static s8 select_2obj_continue(KPADInsideStatus *kp) {
  KPADObject *op1, *op2, *rp1, *rp2;
  Vec2 nrm;
  s32 rev_fg;
  f32 f1, f2, f3, vx, vy, min = 2.0f;
  op1 = kp->kobj_sample;
  do {
    if (op1->error_fg != 0) {
      continue;
    }

    op2 = op1 + 1;
    do {
      if (op2->error_fg != 0) {
        continue;
      }
      vx = op2->center.x - op1->center.x;
      vy = op2->center.y - op1->center.y;
      f3 = sqrtf(vx * vx + vy * vy);
      f1 = 1.0f / f3;
      nrm.x = vx * f1;
      nrm.y = vy * f1;

      f1 *= kp_dist_vv1;
      if (f1 <= kp_err_dist_min || f1 >= kp_err_dist_max) {
        continue;
      }

      f1 -= kp->sec_dist;
      if (f1 < 0.0f) {
        f1 *= kp_err_dist_speedM_1;
      } else {
        f1 *= kp_err_dist_speed_1;
      }
      if (f1 >= 1.0f) {
        continue;
      }

      f2 = kp->sec_nrm.x * nrm.x + kp->sec_nrm.y * nrm.y;
      if (f2 < 0.0f) {
        f2 = -f2;
        rev_fg = 1;
      } else {
        rev_fg = 0;
      }
      if (f2 <= kp_err_next_inpr) {
        continue;
      }
      f2 = (1.0f - f2) / (1.0f - kp_err_next_inpr);
      f1 += f2;
      if (f1 < min) {
        min = f1;
        if (rev_fg) {
          rp1 = op2;
          rp2 = op1;
        } else {
          rp1 = op1;
          rp2 = op2;
        }
      }

    } while (++op2 <= &kp->kobj_sample[WPAD_DPD_MAX_OBJECTS - 1]);
  } while (++op1 < &kp->kobj_sample[WPAD_DPD_MAX_OBJECTS - 1]);

  if (min == 2.0f) {
    return 0;
  }

  kp->kobj_regular[0] = *rp1;
  kp->kobj_regular[1] = *rp2;

  return 2;
}

static s8 select_1obj_first(KPADInsideStatus *kp) {
  KPADObject *op1;
  f32 vx, vy;
  Vec2 p1, p2;

  vx = kp->sec_nrm_hori.x * kp->acc_horizon.x + kp->sec_nrm_hori.y * kp->acc_horizon.y;
  vy = kp->sec_nrm_hori.y * kp->acc_horizon.x - kp->sec_nrm_hori.x * kp->acc_horizon.y;

  vx *= kp->trust_sec_length;
  vy *= kp->trust_sec_length;

  op1 = kp->kobj_sample;
  do {
    if (op1->error_fg != 0) {
      continue;
    }

    p1.x = op1->center.x - vx;
    p1.y = op1->center.y - vy;
    p2.x = op1->center.x + vx;
    p2.y = op1->center.y + vy;

    if (p1.x <= kobj_frame_min.x || p1.x >= kobj_frame_max.x || p1.y <= kobj_frame_min.y ||
        p1.y >= kobj_frame_max.y) {
      if (p2.x > kobj_frame_min.x && p2.x < kobj_frame_max.x && p2.y > kobj_frame_min.y &&
          p2.y < kobj_frame_max.y) {
        kp->kobj_regular[1] = *op1;
        kp->kobj_regular[0].center = p1;
        kp->kobj_regular[0].error_fg = 0;
        kp->kobj_regular[0].state_fg = -1;

        return -1;
      }
    } else {
      if (p2.x <= kobj_frame_min.x || p2.x >= kobj_frame_max.x || p2.y <= kobj_frame_min.y ||
          p2.y >= kobj_frame_max.y) {
        kp->kobj_regular[0] = *op1;

        kp->kobj_regular[1].center = p2;
        kp->kobj_regular[1].error_fg = 0;
        kp->kobj_regular[1].state_fg = -1;

        return -1;
      }
    }

  } while (++op1 < &kp->kobj_sample[WPAD_DPD_MAX_OBJECTS]);

  return 0;
}

static s8 select_1obj_continue(KPADInsideStatus *kp) {
  KPADObject *op1, *op2, *rp1, *rp2;
  f32 f1, vx, vy;
  f32 min = kp_err_near_pos * kp_err_near_pos;

  op1 = kp->kobj_regular;
  do {
    if (op1->error_fg != 0) {
      continue;
    }
    if (op1->state_fg != 0) {
      continue;
    }

    op2 = kp->kobj_sample;
    do {
      if (op2->error_fg != 0) {
        continue;
      }

      vx = op1->center.x - op2->center.x;
      vy = op1->center.y - op2->center.y;
      f1 = vx * vx + vy * vy;
      if (f1 < min) {
        min = f1;
        rp1 = op1;
        rp2 = op2;
      }
    } while (++op2 < &kp->kobj_sample[WPAD_DPD_MAX_OBJECTS]);
  } while (++op1 < &kp->kobj_regular[KPAD_USE_OBJECTS]);

  if (min == kp_err_near_pos * kp_err_near_pos)
    return 0;

  *rp1 = *rp2;
  kp->sec_nrm.x = kp->sec_nrm_hori.x * kp->acc_horizon.x + kp->sec_nrm_hori.y * kp->acc_horizon.y;
  kp->sec_nrm.y = kp->sec_nrm_hori.y * kp->acc_horizon.x - kp->sec_nrm_hori.x * kp->acc_horizon.y;

  vx = kp->sec_length * kp->sec_nrm.x;
  vy = kp->sec_length * kp->sec_nrm.y;
  if (rp1 == &kp->kobj_regular[0]) {
    kp->kobj_regular[1].center.x = rp1->center.x + vx;
    kp->kobj_regular[1].center.y = rp1->center.y + vy;
    kp->kobj_regular[1].error_fg = 0;
    kp->kobj_regular[1].state_fg = -1;
  } else {
    kp->kobj_regular[0].center.x = rp1->center.x - vx;
    kp->kobj_regular[0].center.y = rp1->center.y - vy;
    kp->kobj_regular[0].error_fg = 0;
    kp->kobj_regular[0].state_fg = -1;
  }

  if (kp->status.dpd_valid_fg < 0) {
    return -1;
  } else {
    return 1;
  }
}

static void calc_dpd_variable(KPADInsideStatus *kp, s8 valid_fg_next) {
  KPADStatus *sp = &kp->status;
  f32 f1, f2, dist;
  Vec2 pos, vec;

  if (valid_fg_next == 0) {
    sp->dpd_valid_fg = 0;
    return;
  }

  pos.x = kp->sec_nrm_hori.x * kp->sec_nrm.x + kp->sec_nrm_hori.y * kp->sec_nrm.y;
  pos.y = kp->sec_nrm_hori.y * kp->sec_nrm.x - kp->sec_nrm_hori.x * kp->sec_nrm.y;

  if (sp->dpd_valid_fg == 0) {
    sp->horizon = pos;
    sp->hori_vec = Vec2_0;
    sp->hori_speed = 0.0f;
  } else {
    vec.x = pos.x - sp->horizon.x;
    vec.y = pos.y - sp->horizon.y;
    f1 = sqrtf(vec.x * vec.x + vec.y * vec.y);

    if (f1 >= kp->hori_play_radius) {
      f2 = 1.0f;
    } else {
      f2 = f1 / kp->hori_play_radius;
      f2 *= f2;
      f2 *= f2;
    }
    f2 *= kp->hori_sensitivity;
    vec.x = f2 * vec.x + sp->horizon.x;
    vec.y = f2 * vec.y + sp->horizon.y;
    f1 = sqrtf(vec.x * vec.x + vec.y * vec.y);
    vec.x /= f1;
    vec.y /= f1;

    sp->hori_vec.x = vec.x - sp->horizon.x;
    sp->hori_vec.y = vec.y - sp->horizon.y;
    sp->hori_speed = sqrtf(sp->hori_vec.x * sp->hori_vec.x + sp->hori_vec.y * sp->hori_vec.y);

    sp->horizon = vec;
  }

  dist = kp_dist_vv1 / kp->sec_length;

  if (sp->dpd_valid_fg == 0) {
    sp->dist = dist;
    sp->dist_vec = 0.0f;
    sp->dist_speed = 0.0f;
  } else {
    f1 = dist - sp->dist;
    if (f1 < 0.0f) {
      f2 = -f1;
    } else {
      f2 = f1;
    }

    if (f2 >= kp->dist_play_radius) {
      f2 = 1.0f;
    } else {
      f2 /= kp->dist_play_radius;
      f2 *= f2;
      f2 *= f2;
    }
    f2 *= kp->dist_sensitivity;

    sp->dist_vec = f2 * f1;
    if (sp->dist_vec < 0.0f) {
      sp->dist_speed = -sp->dist_vec;
    } else {
      sp->dist_speed = sp->dist_vec;
    }

    sp->dist += sp->dist_vec;
  }

  pos.x = (kp->kobj_regular[0].center.x + kp->kobj_regular[1].center.x) * 0.5f;
  pos.y = (kp->kobj_regular[0].center.y + kp->kobj_regular[1].center.y) * 0.5f;

  f1 = kp->sec_nrm.x * kp->sec_nrm_hori.x + kp->sec_nrm.y * kp->sec_nrm_hori.y;
  f2 = -kp->sec_nrm.y * kp->sec_nrm_hori.x + kp->sec_nrm.x * kp->sec_nrm_hori.y;
  vec.x = f1 * pos.x - f2 * pos.y;
  vec.y = f2 * pos.x + f1 * pos.y;

  vec.x = (kp->center_org.x - vec.x) * kp->dpd2pos_scale;
  vec.y = (kp->center_org.y - vec.y) * kp->dpd2pos_scale;

  pos.x = -kp->accXY_nrm_hori.y * vec.x + kp->accXY_nrm_hori.x * vec.y;
  pos.y = -kp->accXY_nrm_hori.x * vec.x - kp->accXY_nrm_hori.y * vec.y;
  if (sp->dpd_valid_fg == 0) {
    sp->pos = pos;
    sp->vec = Vec2_0;
    sp->speed = 0.0f;
  } else {
    vec.x = pos.x - sp->pos.x;
    vec.y = pos.y - sp->pos.y;
    f1 = sqrtf(vec.x * vec.x + vec.y * vec.y);

    if (f1 >= kp->pos_play_radius) {
      f2 = 1.0f;
    } else {
      f2 = f1 / kp->pos_play_radius;
      f2 *= f2;
      f2 *= f2;
    }
    f2 *= kp->pos_sensitivity;

    sp->vec.x = f2 * vec.x;
    sp->vec.y = f2 * vec.y;
    sp->speed = sqrtf(sp->vec.x * sp->vec.x + sp->vec.y * sp->vec.y);

    sp->pos.x += sp->vec.x;
    sp->pos.y += sp->vec.y;
  }

  sp->dpd_valid_fg = valid_fg_next;
}

static void calc_obj_horizon(KPADInsideStatus *kp) {
  f32 f2, vx, vy;

  vx = kp->kobj_regular[1].center.x - kp->kobj_regular[0].center.x;
  vy = kp->kobj_regular[1].center.y - kp->kobj_regular[0].center.y;
  kp->sec_length = sqrtf(vx * vx + vy * vy);

  f2 = 1.0f / kp->sec_length;
  kp->sec_dist = kp_dist_vv1 * f2;

  vx *= f2;
  vy *= f2;
  kp->sec_nrm.x = vx;
  kp->sec_nrm.y = vy;

  kp->obj_horizon.x = kp->sec_nrm_hori.x * vx + kp->sec_nrm_hori.y * vy;
  kp->obj_horizon.y = kp->sec_nrm_hori.y * vx - kp->sec_nrm_hori.x * vy;
}

static void get_kobj(KPADInsideStatus *kp, DPDObject *wobj_p) {
  const f32 dpd_scale = 2.0f / (f32)KPAD_DPD_RESO_WX;
  const f32 dpd_cx = (f32)(KPAD_DPD_RESO_WX - 1) / (f32)KPAD_DPD_RESO_WX;
  const f32 dpd_cy = (f32)(KPAD_DPD_RESO_WY - 1) / (f32)KPAD_DPD_RESO_WX;

  KPADObject *kobj_p;

  kobj_p = &kp->kobj_sample[WPAD_DPD_MAX_OBJECTS - 1];
  do {
    if (wobj_p->size) {
      kobj_p->center.x = (f32)(s32)wobj_p->x * dpd_scale - dpd_cx;
      kobj_p->center.y = (f32)(s32)wobj_p->y * dpd_scale - dpd_cy;

      kobj_p->error_fg = 0;
      kobj_p->state_fg = 0;
    } else {
      kobj_p->error_fg = -1;
    }

    --wobj_p;
  } while (--kobj_p >= kp->kobj_sample);
}

static void check_kobj_outside_frame(KPADInsideStatus *kp, KPADObject *kobj_t) {
  KPADObject *kobj_p = &kobj_t[WPAD_DPD_MAX_OBJECTS - 1];

  do {
    if (kobj_p->error_fg < 0) {
      continue;
    }

    if (kobj_p->center.x <= kobj_frame_min.x || kobj_p->center.x >= kobj_frame_max.x ||
        kobj_p->center.y <= kobj_frame_min.y || kobj_p->center.y >= kobj_frame_max.y) {
      kobj_p->error_fg |= 1;
    }
  } while (--kobj_p >= kobj_t);
}

static void check_kobj_same_position(KPADObject *kobj_t) {
  KPADObject *op1, *op2;

  op1 = kobj_t;
  do {
    if (op1->error_fg != 0) {
      continue;
    }

    op2 = op1 + 1;
    do {
      if (op2->error_fg != 0) {
        continue;
      }

      if (op1->center.x == op2->center.x && op1->center.y == op2->center.y) {
        op2->error_fg |= 2;
      }
    } while (++op2 <= &kobj_t[WPAD_DPD_MAX_OBJECTS - 1]);
  } while (++op1 < &kobj_t[WPAD_DPD_MAX_OBJECTS - 1]);
}

static void read_kpad_dpd(KPADInsideStatus *kp, void *wp, BOOL fs_flag) {
  KPADStatus *sp = &kp->status;
  KPADObject *op1;
  s8 valid_fg_next;

  WPADStatus *core = wp;
  WPADFSStatus *fs = (WPADFSStatus *)wp;

  s32 chan = kp->wpad_chan_no;

  if (chan < 0) {
    return;
  }
  if (fs_flag) {
    if (fs->err != WPAD_ERR_NONE) {
      if (fs->err == WPAD_ERR_NO_CONTROLLER) {
        kp->wpad_chan_no = chan - WPAD_MAX_CONTROLLERS;
        reset_kpad(kp);
        return;
      }
    } else {
      get_kobj(kp, &fs->obj[WPAD_DPD_MAX_OBJECTS - 1]);
    }
  } else {
    if (core->err != WPAD_ERR_NONE) {
      if (core->err == WPAD_ERR_NO_CONTROLLER) {
        kp->wpad_chan_no = chan - WPAD_MAX_CONTROLLERS;
        reset_kpad(kp);
        return;
      }
    } else {
      get_kobj(kp, &core->obj[WPAD_DPD_MAX_OBJECTS - 1]);
    }
  }

  check_kobj_outside_frame(kp, kp->kobj_sample);
  check_kobj_same_position(kp->kobj_sample);
  kp->valid_objs = 0;
  op1 = &kp->kobj_sample[WPAD_DPD_MAX_OBJECTS - 1];
  do {
    if (op1->error_fg == 0) {
      ++kp->valid_objs;
    }
  } while (--op1 >= kp->kobj_sample);

  if (kp->acc_vertical.x <= kp_err_up_inpr) {
    goto LABEL_select_NG;
  }

  if (sp->dpd_valid_fg == 2 || sp->dpd_valid_fg == -2) {
    if (kp->valid_objs >= 2) {
      valid_fg_next = select_2obj_continue(kp);
      if (valid_fg_next) {
        goto LABEL_select_OK;
      }
    }
    if (kp->valid_objs >= 1) {
      valid_fg_next = select_1obj_continue(kp);
      if (valid_fg_next) {
        goto LABEL_select_OK;
      }
    }
  } else if (sp->dpd_valid_fg == 1 || sp->dpd_valid_fg == -1) {
    if (kp->valid_objs >= 2) {
      valid_fg_next = select_2obj_first(kp);
      if (valid_fg_next) {
        goto LABEL_select_OK;
      }
    }
    if (kp->valid_objs >= 1) {
      valid_fg_next = select_1obj_continue(kp);
      if (valid_fg_next) {
        goto LABEL_select_OK;
      }
    }
  } else {
    if (kp->valid_objs >= 2) {
      valid_fg_next = select_2obj_first(kp);

      if (valid_fg_next) {
        goto LABEL_select_OK;
      }
    }
    if (kp->valid_objs == 1) {
      valid_fg_next = select_1obj_first(kp);
      if (valid_fg_next)
        goto LABEL_select_OK;
    }
  }

LABEL_select_NG:
  valid_fg_next = 0;

LABEL_select_OK:
  if (valid_fg_next) {
    calc_obj_horizon(kp);
    if (kp->ah_circle_ct == 0) {
      if (kp->obj_horizon.x * kp->acc_horizon.x + kp->obj_horizon.y * kp->acc_horizon.y <=
          kp_err_acc_inpr) {
        valid_fg_next = 0;

        kp->kobj_regular[0].error_fg = kp->kobj_regular[1].error_fg = 1;
      }
    }

    if (sp->dpd_valid_fg == 2 && valid_fg_next == 2) {
      if (kp->dpd_valid2_ct == 200) {
        kp->trust_sec_length = kp->sec_length;
      } else {
        ++kp->dpd_valid2_ct;
      }
    } else {
      kp->dpd_valid2_ct = 0;
    }
  } else {
    kp->dpd_valid2_ct = 0;
  }

  calc_dpd_variable(kp, valid_fg_next);
}

static void clamp_trigger(f32 *trigger, s32 tr, s32 min, s32 max) {
  if (tr <= min) {
    *trigger = 0.0f;
  } else if (tr >= max) {
    *trigger = 1.0f;
  } else {
    *trigger = (f32)(tr - min) / (f32)(max - min);
  }
}

static void clamp_stick(Vec2 *stick, s32 sx, s32 sy, s32 min, s32 max) {
  f32 length;

  if (sx < 0) {
    clamp_trigger(&stick->x, -sx, min, max);
    stick->x = -stick->x;
  } else {
    clamp_trigger(&stick->x, sx, min, max);
  }
  if (sy < 0) {
    clamp_trigger(&stick->y, -sy, min, max);
    stick->y = -stick->y;
  } else {
    clamp_trigger(&stick->y, sy, min, max);
  }

  length = stick->x * stick->x + stick->y * stick->y;
  if (length > 1.0f) {
    length = sqrtf(length);
    stick->x /= length;
    stick->y /= length;
  }
}

static s32 check_device(s32 chan, KPADInsideStatus *kp) {
  u32 dev_type;
  s32 err = WPADProbe(chan, &dev_type);
  if (err == WPAD_ERR_NO_CONTROLLER) {
    if (kp->wpad_chan_no >= 0) {
      WPADSetAutoSamplingBuf(chan, NULL, 0);
      kp->wpad_chan_no = chan - WPAD_MAX_CONTROLLERS;
    }
    return -1;
  }
  if (err == WPAD_ERR_NONE && dev_type == kp->status.dev_type) {
    return 0;
  }
  WPADSetAutoSamplingBuf(chan, NULL, 0);
  switch (dev_type) {
  case WPAD_DEV_CORE:
    WPADSetDataFormat(chan, WPAD_FMT_CORE);
    WPADSetAutoSamplingBuf(chan, kp->wpad_ring_buf.core, KPAD_MAX_SAMPLES);
    break;
  case WPAD_DEV_FREESTYLE:
    WPADSetDataFormat(chan, WPAD_FMT_FREESTYLE);
    WPADSetAutoSamplingBuf(chan, kp->wpad_ring_buf.fs, KPAD_MAX_SAMPLES);
    break;
  case WPAD_DEV_DOLPHIN:
    WPADSetDataFormat(chan, WPAD_FMT_DOLPHIN);
    break;
  default:
    kp->wpad_chan_no = chan - WPAD_MAX_CONTROLLERS;
    return -1;
  }
  kp->wpad_chan_no = chan;
  kp->status.dev_type = dev_type;
  reset_kpad(kp);
  return 1;
}

static void read_dolphin(s32 chan, KPADInsideStatus *kp, KPADStatus *sp) {
  PADStatus pad;
  u16 old_fg;
  u32 change_fg;
  WPADRead(chan, &pad);
  if (pad.err == PAD_ERR_NONE) {
    clamp_stick(&sp->ex_status.gc.stick, pad.stickX, pad.stickY, kp_gc_mstick_min,
                kp_gc_mstick_max);
    clamp_stick(&sp->ex_status.gc.substick, pad.substickX, pad.substickY, kp_gc_cstick_min,
                kp_gc_cstick_max);
    clamp_trigger(&sp->ex_status.gc.ltrigger, pad.triggerLeft, kp_gc_trigger_min,
                  kp_gc_trigger_max);
    clamp_trigger(&sp->ex_status.gc.rtrigger, pad.triggerRight, kp_gc_trigger_min,
                  kp_gc_trigger_max);
    old_fg = sp->hold;
    sp->hold = pad.button;
    change_fg = sp->hold ^ old_fg;
    sp->trig = change_fg & sp->hold;
    sp->release = change_fg & old_fg;
    calc_button_repeat(kp, sp);
  }
}

static void read_kpad_stick(KPADInsideStatus *kp, WPADFSStatus *wp) {
  if (kp->wpad_chan_no < 0) {
    return;
  }
  if (wp->err != WPAD_ERR_NONE && wp->err == WPAD_ERR_NO_CONTROLLER) {
    kp->wpad_chan_no -= WPAD_MAX_CONTROLLERS;
    reset_kpad(kp);
  } else {
    clamp_stick(&kp->status.ex_status.fs.stick, wp->fsStickX, wp->fsStickY, kp_fs_fstick_min,
                kp_fs_fstick_max);
  }
}

s32 KPADRead(s32 chan, KPADStatus *samplingBufs, u32 length) {
  KPADInsideStatus *kp = &inside_kpads[chan];
  KPADStatus *sp;
  WPADStatus *core;
  WPADStatus *core_ring = kp->wpad_ring_buf.core;
  WPADFSStatus *fs;
  WPADFSStatus *fs_ring = kp->wpad_ring_buf.fs;
  s32 latest_idx;
  s32 ring_idx;
  s32 idx;
  s32 count;
  u32 return_ct;

  sp = &samplingBufs[length - 1];
  do {
    sp->dev_type = kp->status.dev_type;
  } while (--sp >= samplingBufs);

  if (check_device(chan, kp)) {
    return 0;
  }
  if (kp->status.dev_type == WPAD_DEV_DOLPHIN) {
    read_dolphin(chan, kp, samplingBufs);
    return 1;
  }
  if (kp->status.dev_type != WPAD_DEV_CORE && kp->status.dev_type != WPAD_DEV_FREESTYLE) {
    return 0;
  }
  latest_idx = WPADGetLatestIndexInBuf(kp->wpad_chan_no);
  ring_idx = kp->wpad_ring_idx;
  if (ring_idx < 0) {
    ring_idx = latest_idx - 1;
    if (ring_idx < 0) {
      ring_idx = KPAD_MAX_SAMPLES - 1;
    }
  }
  count = latest_idx - ring_idx;
  kp->wpad_ring_idx = latest_idx;
  if (count == 0) {
    return 0;
  }
  if (count < 0) {
    count += KPAD_MAX_SAMPLES;
  }
  if ((u32)count < length) {
    return_ct = count;
  } else {
    return_ct = length;
  }
  kp->read_count = 0;
  idx = ring_idx;
  sp = samplingBufs + return_ct;
  if (kp->status.dev_type == WPAD_DEV_CORE) {
    do {
      if (++idx == KPAD_MAX_SAMPLES) {
        idx = 0;
      }
      core = &core_ring[idx];
      kp->status.wpad_err = core->err;
      read_kpad_acc(kp, core, FALSE);
      read_kpad_dpd(kp, core, FALSE);
      if (kp->read_count >= count - (s32)return_ct) {
        *--sp = kp->status;
      }
    } while (++kp->read_count < count);
  } else {
    do {
      if (++idx == KPAD_MAX_SAMPLES) {
        idx = 0;
      }
      fs = &fs_ring[idx];
      kp->status.wpad_err = fs->err;
      read_kpad_acc(kp, fs, TRUE);
      read_kpad_dpd(kp, fs, TRUE);
      read_kpad_stick(kp, fs);
      if (kp->read_count >= count - (s32)return_ct) {
        *--sp = kp->status;
      }
    } while (++kp->read_count < count);
  }
  if (kp->status.dev_type == WPAD_DEV_CORE) {
    read_kpad_button(kp, core, FALSE);
  } else {
    read_kpad_button(kp, fs, TRUE);
  }
  count = return_ct;
  do {
    sp->hold = kp->status.hold;
    sp->trig = kp->status.trig;
    sp->release = kp->status.release;
    ++sp;
  } while (--count);
  return return_ct;
}

void KPADInit(void) {
  s32 i;
  KPADInsideStatus *kp;
  WPADInit();
  i = 0;
  do {
    kp = &inside_kpads[i];
    kp->status.dev_type = 0;
    kp->wpad_chan_no = i - WPAD_MAX_CONTROLLERS;
    check_device(i, kp);
    kp->dist_org = idist_org;
    kp->accXY_nrm_hori = iaccXY_nrm_hori;
    kp->sec_nrm_hori = isec_nrm_hori;
    kp->center_org = icenter_org;
    calc_dpd2pos_scale(kp);
    kp->pos_play_radius = kp->hori_play_radius = kp->dist_play_radius = kp->acc_play_radius = 0.0f;
    kp->pos_sensitivity = kp->hori_sensitivity = kp->dist_sensitivity = kp->acc_sensitivity = 1.0f;
    kp->btn_repeat_delay = KPAD_BTN_NO_RPT_DELAY;
    kp->btn_repeat_pulse = 0;
  } while (++i < WPAD_MAX_CONTROLLERS);
  KPADReset();
  OSRegisterVersion(__KPADVersion);
}

void KPADReset(void) {
  KPADInsideStatus *kp;
  KPADSetObjInterval(kp_obj_interval);
  kobj_frame_min.x = -1.0f + kp_err_outside_frame;
  kobj_frame_max.x = 1.0f - kp_err_outside_frame;
  kobj_frame_min.y = -((f32)KPAD_DPD_RESO_WY / KPAD_DPD_RESO_WX) + kp_err_outside_frame;
  kobj_frame_max.y = ((f32)KPAD_DPD_RESO_WY / KPAD_DPD_RESO_WX) - kp_err_outside_frame;
  kp_err_dist_speed_1 = 1.0f / kp_err_dist_speed;
  kp_err_dist_speedM_1 = -1.0f / kp_err_dist_speed;
  kp_ah_circle_radius2 = kp_ah_circle_radius * kp_ah_circle_radius;
  kp = &inside_kpads[WPAD_MAX_CONTROLLERS - 1];
  do {
    reset_kpad(kp);
  } while (--kp >= inside_kpads);
}
