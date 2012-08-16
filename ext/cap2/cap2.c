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
  VALUE rb_mCap2;

  // sets_hash and caps_hash act as maps between lower cased
  // names of capabilities (e.g. 'dac_override') and the CAP_
  // constants defined in linux/capability.h and
  // sys/capability.h (e.g. CAP_DAC_OVERRIDE). They are
  // assigned to Cap2::SETS and Cap2::CAPS respectively.
  VALUE sets_hash, caps_hash;

  rb_mCap2 = rb_define_module("Cap2");

  sets_hash = rb_hash_new();
  SetsHashSet("permitted",    CAP_PERMITTED);
  SetsHashSet("effective",    CAP_EFFECTIVE);
  SetsHashSet("inheritable",  CAP_INHERITABLE);
  rb_define_const(rb_mCap2, "SETS", sets_hash);

  caps_hash = rb_hash_new();
  CapsHashSet("chown",            CAP_CHOWN);
  CapsHashSet("dac_override",     CAP_DAC_OVERRIDE);
  CapsHashSet("dac_read_search",  CAP_DAC_READ_SEARCH);
  CapsHashSet("fowner",           CAP_FOWNER);
  CapsHashSet("fsetid",           CAP_FSETID);
  CapsHashSet("kill",             CAP_KILL);
  CapsHashSet("setgid",           CAP_SETGID);
  CapsHashSet("setuid",           CAP_SETUID);
  CapsHashSet("setpcap",          CAP_SETPCAP);
  CapsHashSet("linux_immutable",  CAP_LINUX_IMMUTABLE);
  CapsHashSet("net_bind_service", CAP_NET_BIND_SERVICE);
  CapsHashSet("net_broadcast",    CAP_NET_BROADCAST);
  CapsHashSet("net_admin",        CAP_NET_ADMIN);
  CapsHashSet("net_raw",          CAP_NET_RAW);
  CapsHashSet("ipc_lock",         CAP_IPC_LOCK);
  CapsHashSet("ipc_owner",        CAP_IPC_OWNER);
  CapsHashSet("sys_module",       CAP_SYS_MODULE);
  CapsHashSet("sys_rawio",        CAP_SYS_RAWIO);
  CapsHashSet("sys_chroot",       CAP_SYS_CHROOT);
  CapsHashSet("sys_ptrace",       CAP_SYS_PTRACE);
  CapsHashSet("sys_pacct",        CAP_SYS_PACCT);
  CapsHashSet("sys_admin",        CAP_SYS_ADMIN);
  CapsHashSet("sys_boot",         CAP_SYS_BOOT);
  CapsHashSet("sys_nice",         CAP_SYS_NICE);
  CapsHashSet("sys_resource",     CAP_SYS_RESOURCE);
  CapsHashSet("sys_time",         CAP_SYS_TIME);
  CapsHashSet("sys_tty_config",   CAP_SYS_TTY_CONFIG);
  CapsHashSet("mknod",            CAP_MKNOD);
  CapsHashSet("lease",            CAP_LEASE);
  CapsHashSet("audit_write",      CAP_AUDIT_WRITE);
  CapsHashSet("audit_control",    CAP_AUDIT_CONTROL);
  CapsHashSet("setfcap",          CAP_SETFCAP);
  CapsHashSet("mac_override",     CAP_MAC_OVERRIDE);
  CapsHashSet("mac_admin",        CAP_MAC_ADMIN);
  CapsHashSet("syslog",           CAP_SYSLOG);
  CapsHashSet("wake_alarm",       CAP_WAKE_ALARM);
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
