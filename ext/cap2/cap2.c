#include <ruby.h>
#include <errno.h>
#include <sys/capability.h>

static VALUE cap2_has_cap(VALUE self, VALUE pid, VALUE set, VALUE cap) {
  cap_t cap_d;
  cap_flag_value_t flag_value;

  Check_Type(pid, T_FIXNUM);

  cap_d = cap_get_pid(FIX2INT(pid));

  if (cap_d == NULL) {
    rb_raise(
      rb_eRuntimeError,
      "Failed to get capabilities for proccess %d: (%s)\n",
      FIX2INT(pid), strerror(errno)
    );
  } else {
    cap_get_flag(
      cap_d,
      (cap_value_t) cap,
      (cap_flag_t) set,
      &flag_value
    );

    cap_free(cap_d);
  }

  return flag_value == CAP_SET ? Qtrue : Qfalse;
}

void Init_cap2(void) {
  VALUE rb_mCap2;

  rb_mCap2 = rb_define_module("Cap2");

  rb_define_const(rb_mCap2, "PERMITTED", CAP_PERMITTED);

  rb_define_const(rb_mCap2, "DAC_OVERRIDE", CAP_DAC_OVERRIDE);

  rb_define_module_function(rb_mCap2, "has_capability?", cap2_has_cap, 3);
}
