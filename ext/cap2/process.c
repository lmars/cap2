#include "cap2.h"

VALUE cap2_process_has_cap(VALUE self, VALUE set_sym, VALUE cap_sym) {
  cap_t cap_d;
  int p;
  VALUE pid, result;

  pid = rb_iv_get(self, "@pid");
  p = FIX2INT(pid);

  cap_d = cap_get_pid(p);

  if (cap_d == NULL) {
    rb_raise(
      rb_eRuntimeError,
      "Failed to get capabilities for proccess %d: (%s)\n",
      p, strerror(errno)
    );
  }

  result = cap2_has_cap(cap_d, set_sym, cap_sym);

  cap_free(cap_d);

  return result;
}
