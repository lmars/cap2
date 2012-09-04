#include <ruby.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <sys/capability.h>
#include "cap2.h"

/*
 * Converts a Ruby symbol into cap_flag_t set, defined in <sys/capability.h>
 *
 * Raises an ArgumentError if set is not a valid capability set.
 */
cap_flag_t cap2_sym_to_set(VALUE set) {
  char *set_s;

  Check_Type(set, T_SYMBOL);

  set = rb_sym_to_s(set);

  set_s = StringValueCStr(set);

       if(strcmp(set_s, "permitted")   == 0) return CAP_PERMITTED;
  else if(strcmp(set_s, "effective")   == 0) return CAP_EFFECTIVE;
  else if(strcmp(set_s, "inheritable") == 0) return CAP_INHERITABLE;
  else rb_raise(rb_eArgError, "unknown set %s", set_s);
}

/*
 * Lookup the value of a capability in cap2_caps, defined in cap2.h
 * (cap2.h is generated dynamically by extconf.rb).
 *
 * Raises an ArgumentError if name is not a valid capability name.
 */
cap_value_t cap2_cap_value(const char *name) {
  int i;

  for(i = 0; i < __CAP_COUNT; i++) {
    if(strcmp(cap2_caps[i].name, name) == 0)
      return cap2_caps[i].value;
  }

  rb_raise(rb_eArgError, "unknown capability %s", name);
}

/*
 * Converts a Ruby symbol into a cap_value_t capability value.
 */
cap_value_t cap2_sym_to_cap(VALUE cap) {
  Check_Type(cap, T_SYMBOL);

  cap = rb_sym_to_s(cap);

  return cap2_cap_value(StringValueCStr(cap));
}

VALUE cap2_caps_to_hash(cap_t cap_d) {
  int i;
  cap_flag_value_t cap_value;
  VALUE caps, permitted, effective, inheritable;

  permitted = rb_ary_new();
  effective = rb_ary_new();
  inheritable = rb_ary_new();

  for(i = 0; i < __CAP_COUNT; i++) {
    cap_get_flag(cap_d, cap2_caps[i].value, CAP_PERMITTED, &cap_value);
    if(cap_value == CAP_SET)
      rb_ary_push(permitted, ID2SYM(rb_intern(cap2_caps[i].name)));

    cap_get_flag(cap_d, cap2_caps[i].value, CAP_EFFECTIVE, &cap_value);
    if(cap_value == CAP_SET)
      rb_ary_push(effective, ID2SYM(rb_intern(cap2_caps[i].name)));

    cap_get_flag(cap_d, cap2_caps[i].value, CAP_INHERITABLE, &cap_value);
    if(cap_value == CAP_SET)
      rb_ary_push(inheritable, ID2SYM(rb_intern(cap2_caps[i].name)));
  }

  caps = rb_hash_new();

  rb_hash_aset(
    caps,
    ID2SYM(rb_intern("permitted")),
    rb_funcall(permitted, rb_intern("to_set"), 0)
  );

  rb_hash_aset(
    caps,
    ID2SYM(rb_intern("effective")),
    rb_funcall(effective, rb_intern("to_set"), 0)
  );

  rb_hash_aset(
    caps,
    ID2SYM(rb_intern("inheritable")),
    rb_funcall(inheritable, rb_intern("to_set"), 0)
  );

  return caps;
}

/*
 * Convert @pid stored in the given Process object to an int and return it.
 */
static int cap2_process_pid(VALUE process) {
  VALUE pid;

  pid = rb_iv_get(process, "@pid");

  return FIX2INT(pid);
}

VALUE cap2_process_getcaps(VALUE self) {
  cap_t cap_d;
  int pid;
  VALUE result;

  pid = cap2_process_pid(self);

  cap_d = cap_get_pid(pid);

  if (cap_d == NULL) {
    rb_raise(
      rb_eRuntimeError,
      "Failed to get capabilities for proccess %d: (%s)\n",
      pid, strerror(errno)
    );
  } else {
    result = cap2_caps_to_hash(cap_d);
    cap_free(cap_d);
    return result;
  }
}

/*
 * Return a cap_t struct containing the capabilities of the given Process object.
 */
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

/*
 * Enable/disable the given capability in the given set for the given Process
 * object.
 */
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

/*
 * call-seq:
 *  enable(capability) -> true or false
 *
 * Enable the given capability for this process.
 *
 * Raises a RuntimeError if the process's pid is not the same as the current
 * pid (you cannot enable capabilities for other processes, that's their job).
 *
 *   process = Cap2.process              #=> <Cap2::Process>
 *   process.permitted?(:kill)           #=> true
 *   process.effective?(:kill)           #=> false
 *   process.enable(:kill)               #=> true
 *   process.effective?(:kill)           #=> true
 */
VALUE cap2_process_enable(VALUE self, VALUE cap_sym) {
  return cap2_process_set_cap(self, CAP_EFFECTIVE, cap_sym, CAP_SET);
}

/*
 * call-seq:
 *  disable(capability) -> true or false
 *
 * Disable the given capability for this process.
 *
 *   process = Cap2.process              #=> <Cap2::Process>
 *   process.permitted?(:kill)           #=> true
 *   process.effective?(:kill)           #=> true
 *   process.disable(:kill)              #=> true
 *   process.effective?(:kill)           #=> false
 */
VALUE cap2_process_disable(VALUE self, VALUE cap_sym) {
  return cap2_process_set_cap(self, CAP_EFFECTIVE, cap_sym, CAP_CLEAR);
}

/*
 * Convert @filename stored in the given File object to a char* and return it.
 */
static char *cap2_file_filename(VALUE file) {
  VALUE filename;

  filename = rb_iv_get(file, "@filename");

  return StringValueCStr(filename);
}

/*
 * Return a caps hash containing the capabilities of self.
 */
VALUE cap2_file_getcaps(VALUE self) {
  cap_t cap_d;
  char *filename;
  VALUE result;

  filename = cap2_file_filename(self);

  cap_d = cap_get_file(filename);

  if (cap_d == NULL && errno != ENODATA) {
    rb_raise(
      rb_eRuntimeError,
      "Failed to get capabilities for file %s: (%s)\n",
      filename, strerror(errno)
    );
  } else {
    result = cap2_caps_to_hash(cap_d);
    cap_free(cap_d);
    return result;
  }
}

/*
 * Set the capabilities for self from the caps hash stored in @caps.
 */
VALUE cap2_file_setcaps(VALUE self) {
  int i;
  cap_t cap_d;
  char *filename;
  VALUE caps, cap_array, cap_sym;
  cap_value_t cap_values[__CAP_COUNT];

  cap_d = cap_init();

  caps = rb_iv_get(self, "@caps");

  // permitted
  cap_array = rb_funcall(
    rb_hash_aref(caps, ID2SYM(rb_intern("permitted"))),
    rb_intern("to_a"),
    0
  );

  for(i = 0; i < RARRAY_LEN(cap_array); i++) {
    cap_sym = RARRAY_PTR(cap_array)[i];
    cap_values[i] = cap2_sym_to_cap(cap_sym);
  }

  cap_set_flag(cap_d, CAP_PERMITTED, i, cap_values, CAP_SET);

  // effective
  cap_array = rb_funcall(
    rb_hash_aref(caps, ID2SYM(rb_intern("effective"))),
    rb_intern("to_a"),
    0
  );

  for(i = 0; i < RARRAY_LEN(cap_array); i++) {
    cap_sym = RARRAY_PTR(cap_array)[i];
    cap_values[i] = cap2_sym_to_cap(cap_sym);
  }

  cap_set_flag(cap_d, CAP_EFFECTIVE, i, cap_values, CAP_SET);

  // inheritable
  cap_array = rb_funcall(
    rb_hash_aref(caps, ID2SYM(rb_intern("inheritable"))),
    rb_intern("to_a"),
    0
  );

  for(i = 0; i < RARRAY_LEN(cap_array); i++) {
    cap_sym = RARRAY_PTR(cap_array)[i];
    cap_values[i] = cap2_sym_to_cap(cap_sym);
  }

  cap_set_flag(cap_d, CAP_INHERITABLE, i, cap_values, CAP_SET);

  filename = cap2_file_filename(self);

  if(cap_set_file(filename, cap_d) == -1) {
    rb_raise(
      rb_eRuntimeError,
      "Failed to set capabilities for file %s: (%s)\n",
      filename, strerror(errno)
    );
  } else {
    cap_free(cap_d);
    return Qtrue;
  }
}

void Init_cap2(void) {
  int i;
  VALUE rb_mCap2;
  VALUE rb_cCap2File;
  VALUE rb_cCap2Process;

  rb_mCap2 = rb_define_module("Cap2");

  rb_require("set");

  rb_cCap2Process = rb_define_class_under(rb_mCap2, "Process", rb_cObject);
  rb_define_method(rb_cCap2Process, "getcaps", cap2_process_getcaps, 0);
  rb_define_method(rb_cCap2Process, "enable", cap2_process_enable, 1);
  rb_define_method(rb_cCap2Process, "disable", cap2_process_disable, 1);

  rb_cCap2File = rb_define_class_under(rb_mCap2, "File", rb_cObject);
  rb_define_method(rb_cCap2File, "getcaps", cap2_file_getcaps, 0);
  rb_define_method(rb_cCap2File, "save", cap2_file_setcaps, 0);
}
