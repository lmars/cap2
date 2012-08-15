#include <ruby.h>
#include <errno.h>
#include <sys/capability.h>

static VALUE cap2_has_cap(VALUE self, VALUE pid_or_filename, VALUE set, VALUE cap) {
  cap_t cap_d;
  cap_flag_value_t flag_value = CAP_CLEAR;

  if(TYPE(pid_or_filename) == T_FIXNUM) {
    int pid = FIX2INT(pid_or_filename);

    cap_d = cap_get_pid(pid);

    if (cap_d == NULL) {
      rb_raise(
        rb_eRuntimeError,
        "Failed to get capabilities for proccess %d: (%s)\n",
        pid, strerror(errno)
      );
    }
  } else if(TYPE(pid_or_filename) == T_STRING) {
    char *filename = StringValueCStr(pid_or_filename);

    cap_d = cap_get_file(filename);

    if (cap_d == NULL && errno != ENODATA) {
      rb_raise(
        rb_eRuntimeError,
        "Failed to get capabilities for file %s: (%s)\n",
        filename, strerror(errno)
      );
    }
  } else {
    rb_raise(
      rb_eArgError,
      "wrong argument type, expected Fixnum or String\n"
    );
  }

  cap_get_flag(
    cap_d,
    (cap_value_t) cap,
    (cap_flag_t) set,
    &flag_value
  );

  cap_free(cap_d);

  return flag_value == CAP_SET ? Qtrue : Qfalse;
}

void Init_cap2(void) {
  VALUE rb_mCap2;

  rb_mCap2 = rb_define_module("Cap2");

  rb_define_const(rb_mCap2, "PERMITTED", CAP_PERMITTED);
  rb_define_const(rb_mCap2, "EFFECTIVE", CAP_EFFECTIVE);

  rb_define_const(rb_mCap2, "DAC_OVERRIDE", CAP_DAC_OVERRIDE);

  rb_define_module_function(rb_mCap2, "has_capability?", cap2_has_cap, 3);
}
