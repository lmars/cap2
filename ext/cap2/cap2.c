#include <ruby.h>
#include <errno.h>
#include <sys/capability.h>

static VALUE cap2_has_cap(cap_t cap_d, cap_flag_t set, cap_value_t cap) {
  cap_flag_value_t flag_value = CAP_CLEAR;

  cap_get_flag(cap_d, cap, set, &flag_value);

  return flag_value == CAP_SET ? Qtrue : Qfalse;
}

static VALUE cap2_pid_has_cap(VALUE self, VALUE pid, VALUE set, VALUE cap) {
  cap_t cap_d;
  VALUE result;

  int p = FIX2INT(pid);

  cap_d = cap_get_pid(p);

  if (cap_d == NULL) {
    rb_raise(
      rb_eRuntimeError,
      "Failed to get capabilities for proccess %d: (%s)\n",
      p, strerror(errno)
    );
  }

  set = FIX2INT(set);
  cap = FIX2INT(cap);

  result = cap2_has_cap(
    cap_d,
    (cap_flag_t) set,
    (cap_value_t) cap
  );

  cap_free(cap_d);

  return result;
}

static VALUE cap2_file_has_cap(VALUE self, VALUE filename, VALUE set, VALUE cap) {
  cap_t cap_d;
  VALUE result;

  char *f = StringValueCStr(filename);

  cap_d = cap_get_file(f);

  if (cap_d == NULL && errno != ENODATA) {
    rb_raise(
      rb_eRuntimeError,
      "Failed to get capabilities for file %s: (%s)\n",
      f, strerror(errno)
    );
  }

  set = FIX2INT(set);
  cap = FIX2INT(cap);

  result = cap2_has_cap(
    cap_d,
    (cap_flag_t) set,
    (cap_value_t) cap
  );

  cap_free(cap_d);

  return result;
}

#define SetsHashSet(key,val) \
  rb_hash_aset(sets_hash, rb_str_new2(key), INT2FIX(val))

#define CapsHashSet(key,val) \
  rb_hash_aset(caps_hash, rb_str_new2(key), INT2FIX(val))

void Init_cap2(void) {
  VALUE rb_mCap2, sets_hash, caps_hash;

  rb_mCap2 = rb_define_module("Cap2");

  sets_hash = rb_hash_new();
  SetsHashSet("permitted",    CAP_PERMITTED);
  SetsHashSet("effective",    CAP_EFFECTIVE);
  SetsHashSet("inheritable",  CAP_INHERITABLE);
  rb_define_const(rb_mCap2, "SETS", sets_hash);

  caps_hash = rb_hash_new();
  CapsHashSet("dac_override", CAP_DAC_OVERRIDE);
  rb_define_const(rb_mCap2, "CAPS", caps_hash);

  rb_define_module_function(
    rb_mCap2,
    "pid_has_cap?",
    cap2_pid_has_cap,
    3
  );

  rb_define_module_function(
    rb_mCap2,
    "file_has_cap?",
    cap2_file_has_cap,
    3
  );
}
