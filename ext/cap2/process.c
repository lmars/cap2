#include "cap2.h"

static int cap2_process_pid(VALUE process) {
  VALUE pid;

  pid = rb_iv_get(process, "@pid");

  return FIX2INT(pid);
}

static cap_t cap2_process_caps(VALUE process) {
  cap_t cap_d;
  int pid;

  pid = cap2_process_pid(process);

  cap_d = cap_get_pid(pid);

  if (cap_d == NULL) {
    rb_raise(
      rb_eRuntimeError,
      "Failed to get capabilities for proccess %d: (%s)\n",
      pid, strerror(errno)
    );
  }

  return cap_d;
}

static VALUE cap2_process_set_cap(VALUE process, cap_flag_t set, VALUE cap_sym, cap_flag_value_t set_or_clear) {
  cap_t cap_d;
  int pid;
  cap_value_t caps[1];

  pid = cap2_process_pid(process);

  if((pid_t) pid != getpid())
    rb_raise(
      rb_eRuntimeError,
      "Cannot set capabilities for other processes"
    );

  caps[0] = cap2_sym_to_cap(cap_sym);

  cap_d = cap_get_pid(pid);

  cap_set_flag(cap_d, set, 1, caps, set_or_clear);

  if(cap_set_proc(cap_d) == -1) {
    rb_raise(
      rb_eRuntimeError,
      "Failed to set capabilities for process %d: (%s)\n",
      pid, strerror(errno)
    );
  } else {
    return Qtrue;
  }
}

VALUE cap2_process_has_cap(VALUE self, VALUE set_sym, VALUE cap_sym) {
  cap_t cap_d;
  VALUE result;

  cap_d = cap2_process_caps(self);

  result = cap2_has_cap(cap_d, set_sym, cap_sym);

  cap_free(cap_d);

  return result;
}

VALUE cap2_process_enable(VALUE self, VALUE cap_sym) {
  return cap2_process_set_cap(self, CAP_EFFECTIVE, cap_sym, CAP_SET);
}

VALUE cap2_process_disable(VALUE self, VALUE cap_sym) {
  return cap2_process_set_cap(self, CAP_EFFECTIVE, cap_sym, CAP_CLEAR);
}
