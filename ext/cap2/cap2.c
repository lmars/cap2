#include <ruby.h>
#include <errno.h>
#include <sys/capability.h>

static VALUE cap2_has_cap(VALUE self, VALUE pid_or_filename, VALUE set, VALUE cap) {
  cap_t cap_d;
  cap_flag_value_t flag_value = CAP_CLEAR;
  char *set_s;
  char *cap_s;

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

  Check_Type(set, T_SYMBOL);

  set = rb_sym_to_s(set);
  set_s = StringValueCStr(set);

  if(strcmp(set_s, "permitted") == 0)
    set = CAP_PERMITTED;
  else if(strcmp(set_s, "effective") == 0)
    set = CAP_EFFECTIVE;
  else if(strcmp(set_s, "inheritable") == 0)
    set = CAP_INHERITABLE;
  else
    rb_raise(
      rb_eArgError,
      "wrong set type, expected one of :permitted, :effective, :inheritable, got :%s\n",
      set_s
    );

  Check_Type(cap, T_SYMBOL);

  cap = rb_sym_to_s(cap);
  cap_s = StringValueCStr(cap);

  if(strcmp(cap_s, "dac_override") == 0)
    cap = CAP_DAC_OVERRIDE;
  else
    rb_raise(
      rb_eArgError,
      "wrong capability type, expected one of :dac_override, got :%s\n",
      cap_s
    );

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

  rb_define_module_function(rb_mCap2, "has_capability?", cap2_has_cap, 3);
}
