#include <ruby.h>
#include <errno.h>
#include <sys/capability.h>

static cap_flag_t cap2_sym_to_set(VALUE set) {
  char *set_s;

  Check_Type(set, T_SYMBOL);

  set = rb_sym_to_s(set);

  set_s = StringValueCStr(set);

       if(strcmp(set_s, "permitted")   == 0) return CAP_PERMITTED;
  else if(strcmp(set_s, "effective")   == 0) return CAP_EFFECTIVE;
  else if(strcmp(set_s, "inheritable") == 0) return CAP_INHERITABLE;
  else rb_raise(rb_eArgError, "unknown set %s", set_s);
}

static cap_value_t cap2_sym_to_cap(VALUE cap) {
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

static VALUE cap2_has_cap(cap_t cap_d, VALUE set_sym, VALUE cap_sym) {
  cap_flag_t set;
  cap_value_t cap;
  cap_flag_value_t flag_value = CAP_CLEAR;

  set = cap2_sym_to_set(set_sym);
  cap = cap2_sym_to_cap(cap_sym);

  cap_get_flag(cap_d, cap, set, &flag_value);

  return flag_value == CAP_SET ? Qtrue : Qfalse;
}

static VALUE cap2_process_has_cap(VALUE self, VALUE set_sym, VALUE cap_sym) {
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

static char *cap2_file_filename(VALUE file) {
  VALUE filename;

  filename = rb_iv_get(file, "@filename");

  return StringValueCStr(filename);
}

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

static VALUE cap2_file_has_cap(VALUE self, VALUE set_sym, VALUE cap_sym) {
  cap_t cap_d;
  VALUE result;

  cap_d = cap2_file_caps(self);

  result = cap2_has_cap(cap_d, set_sym, cap_sym);

  cap_free(cap_d);

  return result;
}

static VALUE cap2_file_permit(VALUE self, VALUE cap_sym) {
  cap_t cap_d;
  int result;
  char *filename;
  cap_value_t caps_to_set[1];

  filename = cap2_file_filename(self);

  caps_to_set[0] = cap2_sym_to_cap(cap_sym);

  cap_d = cap_get_file(filename);

  if(cap_d == NULL)
    cap_d = cap_init();

  cap_set_flag(cap_d, CAP_PERMITTED, 1, caps_to_set, CAP_SET);

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

void Init_cap2(void) {
  VALUE rb_mCap2;
  VALUE rb_cCap2File;
  VALUE rb_cCap2Process;

  rb_mCap2 = rb_define_module("Cap2");

  rb_cCap2Process = rb_define_class_under(rb_mCap2, "Process", rb_cObject);
  rb_define_method(rb_cCap2Process, "has?", cap2_process_has_cap, 2);

  rb_cCap2File = rb_define_class_under(rb_mCap2, "File", rb_cObject);
  rb_define_method(rb_cCap2File, "has?", cap2_file_has_cap, 2);
  rb_define_method(rb_cCap2File, "permit", cap2_file_permit, 1);
}
