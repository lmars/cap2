#include <ruby.h>
#include <errno.h>
#include <unistd.h>
#include <sys/capability.h>

/*
 * Converts a Ruby symbol into cap_flag_t set, defined in <sys/capability.h>
 *
 * Raises an ArgumentError if set is not a valid capability set
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
 * Converts a Ruby symbol into cap_value_t capability value, defined
 * in <linux/capability.h>
 *
 * Raises an ArgumentError if cap is not a valid capability value.
 */
cap_value_t cap2_sym_to_cap(VALUE cap) {
  char *cap_s;

  Check_Type(cap, T_SYMBOL);

  cap = rb_sym_to_s(cap);

  cap_s = StringValueCStr(cap);

       if(strcmp(cap_s, "chown")            == 0) return CAP_CHOWN;
  else if(strcmp(cap_s, "dac_override")     == 0) return CAP_DAC_OVERRIDE;
  else if(strcmp(cap_s, "dac_read_search")  == 0) return CAP_DAC_READ_SEARCH;
  else if(strcmp(cap_s, "fowner")           == 0) return CAP_FOWNER;
  else if(strcmp(cap_s, "fsetid")           == 0) return CAP_FSETID;
  else if(strcmp(cap_s, "kill")             == 0) return CAP_KILL;
  else if(strcmp(cap_s, "setgid")           == 0) return CAP_SETGID;
  else if(strcmp(cap_s, "setuid")           == 0) return CAP_SETUID;
  else if(strcmp(cap_s, "setpcap")          == 0) return CAP_SETPCAP;
  else if(strcmp(cap_s, "linux_immutable")  == 0) return CAP_LINUX_IMMUTABLE;
  else if(strcmp(cap_s, "net_bind_service") == 0) return CAP_NET_BIND_SERVICE;
  else if(strcmp(cap_s, "net_broadcast")    == 0) return CAP_NET_BROADCAST;
  else if(strcmp(cap_s, "net_admin")        == 0) return CAP_NET_ADMIN;
  else if(strcmp(cap_s, "net_raw")          == 0) return CAP_NET_RAW;
  else if(strcmp(cap_s, "ipc_lock")         == 0) return CAP_IPC_LOCK;
  else if(strcmp(cap_s, "ipc_owner")        == 0) return CAP_IPC_OWNER;
  else if(strcmp(cap_s, "sys_module")       == 0) return CAP_SYS_MODULE;
  else if(strcmp(cap_s, "sys_rawio")        == 0) return CAP_SYS_RAWIO;
  else if(strcmp(cap_s, "sys_chroot")       == 0) return CAP_SYS_CHROOT;
  else if(strcmp(cap_s, "sys_ptrace")       == 0) return CAP_SYS_PTRACE;
  else if(strcmp(cap_s, "sys_pacct")        == 0) return CAP_SYS_PACCT;
  else if(strcmp(cap_s, "sys_admin")        == 0) return CAP_SYS_ADMIN;
  else if(strcmp(cap_s, "sys_boot")         == 0) return CAP_SYS_BOOT;
  else if(strcmp(cap_s, "sys_nice")         == 0) return CAP_SYS_NICE;
  else if(strcmp(cap_s, "sys_resource")     == 0) return CAP_SYS_RESOURCE;
  else if(strcmp(cap_s, "sys_time")         == 0) return CAP_SYS_TIME;
  else if(strcmp(cap_s, "sys_tty_config")   == 0) return CAP_SYS_TTY_CONFIG;
  else if(strcmp(cap_s, "mknod")            == 0) return CAP_MKNOD;
  else if(strcmp(cap_s, "lease")            == 0) return CAP_LEASE;
  else if(strcmp(cap_s, "audit_write")      == 0) return CAP_AUDIT_WRITE;
  else if(strcmp(cap_s, "audit_control")    == 0) return CAP_AUDIT_CONTROL;
  else if(strcmp(cap_s, "setfcap")          == 0) return CAP_SETFCAP;
  else if(strcmp(cap_s, "mac_override")     == 0) return CAP_MAC_OVERRIDE;
  else if(strcmp(cap_s, "mac_admin")        == 0) return CAP_MAC_ADMIN;
  else if(strcmp(cap_s, "syslog")           == 0) return CAP_SYSLOG;
  else if(strcmp(cap_s, "wake_alarm")       == 0) return CAP_WAKE_ALARM;
  else rb_raise(rb_eArgError, "unknown capability %s", cap_s);
}

/*
 * Returns a boolean representing whether cap_d has the given capability enabled
 * in the given set
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
static cap_t cap2_file_caps(VALUE file) {
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
 * Enable/disable the given capability in the given set for the given File
 * object.
 */
static VALUE cap2_file_set_cap(VALUE file, cap_flag_t set, VALUE cap_sym, cap_flag_value_t set_or_clear) {
  cap_t cap_d;
  char *filename;
  cap_value_t caps[1];

  filename = cap2_file_filename(file);

  caps[0] = cap2_sym_to_cap(cap_sym);

  cap_d = cap_get_file(filename);

  if(cap_d == NULL)
    cap_d = cap_init();

  cap_set_flag(cap_d, set, 1, caps, set_or_clear);

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

  cap_d = cap2_file_caps(self);

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
 *  set_effective(capability) -> true or false
 *
 * Enable the given capability when a proces executes this file.
 *
 *   file = Cap2.file('/tmp/foo')      #=> <Cap2::File>
 *   file.effective?(:kill)            #=> false
 *   file.set_effective(:kill)         #=> true
 *   file.effective?(:kill)            #=> true
 */
VALUE cap2_file_set_effective(VALUE self, VALUE cap_sym) {
  return cap2_file_set_cap(self, CAP_EFFECTIVE, cap_sym, CAP_SET);
}

/*
 * call-seq:
 *  disable_on_exec(capability) -> true or false
 *
 * Dont enable the given capability when a process executes this file.
 *
 *   file = Cap2.file('/tmp/foo')      #=> <Cap2::File>
 *   file.effective?(:kill)            #=> true
 *   file.disable_on_exec(:kill)       #=> true
 *   file.effective?(:kill)            #=> false
 */
VALUE cap2_file_clear_effective(VALUE self, VALUE cap_sym) {
  return cap2_file_set_cap(self, CAP_EFFECTIVE, cap_sym, CAP_CLEAR);
}
void Init_cap2(void) {
  VALUE rb_mCap2;
  VALUE rb_cCap2File;
  VALUE rb_cCap2Process;

  rb_mCap2 = rb_define_module("Cap2");

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
  rb_define_method(rb_cCap2File, "set_effective", cap2_file_set_effective, 1);
  rb_define_method(rb_cCap2File, "disable_on_exec", cap2_file_clear_effective, 1);
}
