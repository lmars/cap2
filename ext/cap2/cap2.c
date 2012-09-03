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

/*
 * Returns a boolean representing whether cap_d has the given capability enabled
 * in the given set.
 */
VALUE cap2_has_cap(cap_t cap_d, VALUE set_sym, VALUE cap_sym) {
  cap_flag_t set;
  cap_value_t cap;
  cap_flag_value_t flag_value = CAP_CLEAR;

  set = cap2_sym_to_set(set_sym);
  cap = cap2_sym_to_cap(cap_sym);

  cap_get_flag(cap_d, cap, set, &flag_value);

  return flag_value == CAP_SET ? Qtrue : Qfalse;
}

/*
 * Convert @pid stored in the given Process object to an int and return it.
 */
static int cap2_process_pid(VALUE process) {
  VALUE pid;

  pid = rb_iv_get(process, "@pid");

  return FIX2INT(pid);
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
 *  has?(set, capability) -> true or false
 *
 * Return whether the process has the given capability enabled in the given set.
 *
 *   Cap2.process(1).has?(:permitted, :kill)    #=> true
 *   Cap2.process(1000).has?(:permitted, :kill) #=> false
 */
VALUE cap2_process_has_cap(VALUE self, VALUE set_sym, VALUE cap_sym) {
  cap_t cap_d;
  VALUE result;

  cap_d = cap2_process_caps(self);

  result = cap2_has_cap(cap_d, set_sym, cap_sym);

  cap_free(cap_d);

  return result;
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
 * Return a cap_t struct containing the capabilities of the given File object.
 */
static cap_t cap2_file_get_caps(VALUE file) {
  cap_t cap_d;
  char *filename;

  filename = cap2_file_filename(file);

  cap_d = cap_get_file(filename);

  if (cap_d == NULL && errno != ENODATA) {
    rb_raise(
      rb_eRuntimeError,
      "Failed to get capabilities for file %s: (%s)\n",
      filename, strerror(errno)
    );
  }

  return cap_d;
}

/*
 * Set the given capabilities for the given File object.
 */
static VALUE cap2_file_set_caps(VALUE file, cap_t cap_d) {
  char *filename;

  filename = cap2_file_filename(file);

  if(cap_set_file(filename, cap_d) == -1) {
    rb_raise(
      rb_eRuntimeError,
      "Failed to set capabilities for file %s: (%s)\n",
      filename, strerror(errno)
    );
  } else {
    return Qtrue;
  }
}

/*
 * Enable/disable the given capability in the given set for the given File
 * object.
 */
static VALUE cap2_file_set_cap(VALUE file, cap_flag_t set, VALUE cap_sym, cap_flag_value_t set_or_clear) {
  cap_t cap_d;
  cap_value_t caps[1];
  VALUE result;

  cap_d = cap2_file_get_caps(file);

  if(cap_d == NULL)
    cap_d = cap_init();

  caps[0] = cap2_sym_to_cap(cap_sym);

  cap_set_flag(cap_d, set, 1, caps, set_or_clear);

  result = cap2_file_set_caps(file, cap_d);

  cap_free(cap_d);

  return result;
}

/*
 * call-seq:
 *  has?(set, capability) -> true or false
 *
 * Return whether the file has the given capability enabled in the given set.
 *
 *   Cap2.file('/bin/ping').has?(:permitted, :net_raw)    #=> true
 *   Cap2.file('/tmp/ping').has?(:permitted, :net_raw)    #=> false
 */
VALUE cap2_file_has_cap(VALUE self, VALUE set_sym, VALUE cap_sym) {
  cap_t cap_d;
  VALUE result;

  cap_d = cap2_file_get_caps(self);

  result = cap2_has_cap(cap_d, set_sym, cap_sym);

  cap_free(cap_d);

  return result;
}

/*
 * call-seq:
 *  permit(capability) -> true or false
 *
 * Permit processes executing this file to enable the given capability.
 *
 *   file = Cap2.file('/tmp/killer')    #=> <Cap2::File>
 *   file.permitted?(:kill)             #=> false
 *   file.permit(:kill)                 #=> true
 *   file.permitted?(:kill)             #=> true
 */
VALUE cap2_file_permit(VALUE self, VALUE cap_sym) {
  return cap2_file_set_cap(self, CAP_PERMITTED, cap_sym, CAP_SET);
}

/*
 * call-seq:
 *  unpermit(capability) -> true or false
 *
 * Dont permit processes executing ths file to enable the given capability.
 *
 *   file = Cap2.file('/tmp/foo')      #=> <Cap2::File>
 *   file.permit(:kill)                #=> true
 *   file.permitted?(:kill)            #=> true
 *   file.unpermit(:kill)              #=> true
 *   file.permitted?(:kill)            #=> false
 */
VALUE cap2_file_unpermit(VALUE self, VALUE cap_sym) {
  return cap2_file_set_cap(self, CAP_PERMITTED, cap_sym, CAP_CLEAR);
}

/*
 * call-seq:
 *  allow_inherit(capability) -> true or false
 *
 * Allow processes executing this file to inherit the given capability.
 *
 *   file = Cap2.file('/tmp/foo')      #=> <Cap2::File>
 *   file.inheritable?(:kill)          #=> false
 *   file.allow_inherit(:kill)         #=> true
 *   file.inheritable?(:kill)          #=> true
 */
VALUE cap2_file_allow_inherit(VALUE self, VALUE cap_sym) {
  return cap2_file_set_cap(self, CAP_INHERITABLE, cap_sym, CAP_SET);
}

/*
 * call-seq:
 *  disallow_inherit(capability) -> true or false
 *
 * Dont allow processes executing this file to inherit the given capability.
 *
 *   file = Cap2.file('/tmp/foo')      #=> <Cap2::File>
 *   file.inheritable?(:kill)          #=> true
 *   file.allow_inherit(:kill)         #=> true
 *   file.inheritable?(:kill)          #=> false
 */
VALUE cap2_file_disallow_inherit(VALUE self, VALUE cap_sym) {
  return cap2_file_set_cap(self, CAP_INHERITABLE, cap_sym, CAP_CLEAR);
}

/*
 * call-seq:
 *  enable -> true or false
 *
 * Enable the permitted capabilities when a proces executes this file.
 *
 *   file = Cap2.file('/tmp/foo')      #=> <Cap2::File>
 *   file.permitted?(:kill)            #=> true
 *   file.enabled?                     #=> false
 *   file.enable                       #=> true
 *   file.enabled?                     #=> true
 */
VALUE cap2_file_enable(VALUE self) {
  int i;
  bool enabled;
  cap_value_t caps[1];
  cap_t cap_d;
  cap_flag_value_t permitted, inheritable;
  VALUE result;

  cap_d = cap2_file_get_caps(self);

  if(cap_d == NULL)
    return Qfalse;

  enabled = false;

  for(i = 0; i < __CAP_COUNT; i++) {
    caps[0] = cap2_caps[i].value;

    cap_get_flag(cap_d, caps[0], CAP_PERMITTED,   &permitted);
    cap_get_flag(cap_d, caps[0], CAP_INHERITABLE, &inheritable);

    if(permitted | inheritable) {
      enabled = true;
      cap_set_flag(cap_d, CAP_EFFECTIVE, 1, caps, CAP_SET);
    }
  }

  if(enabled) {
    result = cap2_file_set_caps(self, cap_d);
    cap_free(cap_d);
    return result;
  } else {
    return Qfalse;
  }
}

/*
 * call-seq:
 *  disable -> true or false
 *
 * Dont enable the permitted capabilities when a proces executes this file.
 *
 *   file = Cap2.file('/tmp/foo')      #=> <Cap2::File>
 *   file.enabled?                     #=> true
 *   file.disable                      #=> true
 *   file.enabled?                     #=> false
 */
VALUE cap2_file_disable(VALUE self) {
  cap_t cap_d;
  VALUE result;

  cap_d = cap2_file_get_caps(self);

  if(cap_d == NULL)
    return Qtrue;

  cap_clear_flag(cap_d, CAP_EFFECTIVE);

  result = cap2_file_set_caps(self, cap_d);

  cap_free(cap_d);

  return result;
}

void Init_cap2(void) {
  int i;
  VALUE rb_mCap2;
  VALUE rb_cCap2File;
  VALUE rb_cCap2Process;
  VALUE caps_array;

  rb_mCap2 = rb_define_module("Cap2");

  /*
   * Expose the list of capability names as an array of symbols in
   * Cap2::NAMES
   */
  caps_array = rb_ary_new();
  for(i = 0; i < __CAP_COUNT; i++) {
    rb_ary_push(caps_array, ID2SYM(rb_intern(cap2_caps[i].name)));
  }
  rb_define_const(rb_mCap2, "NAMES", caps_array);

  rb_cCap2Process = rb_define_class_under(rb_mCap2, "Process", rb_cObject);
  rb_define_method(rb_cCap2Process, "has?", cap2_process_has_cap, 2);
  rb_define_method(rb_cCap2Process, "enable", cap2_process_enable, 1);
  rb_define_method(rb_cCap2Process, "disable", cap2_process_disable, 1);

  rb_cCap2File = rb_define_class_under(rb_mCap2, "File", rb_cObject);
  rb_define_method(rb_cCap2File, "has?", cap2_file_has_cap, 2);
  rb_define_method(rb_cCap2File, "permit", cap2_file_permit, 1);
  rb_define_method(rb_cCap2File, "unpermit", cap2_file_unpermit, 1);
  rb_define_method(rb_cCap2File, "allow_inherit", cap2_file_allow_inherit, 1);
  rb_define_method(rb_cCap2File, "disallow_inherit", cap2_file_disallow_inherit, 1);
  rb_define_method(rb_cCap2File, "enable", cap2_file_enable, 0);
  rb_define_method(rb_cCap2File, "disable", cap2_file_disable, 0);
}
