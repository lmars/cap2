#include <ruby.h>
#include <errno.h>
#include <sys/capability.h>

static VALUE cap2_getpcaps(VALUE self, VALUE pid) {
  cap_t cap_d;
  ssize_t length;
  char *caps;
  VALUE result;

  Check_Type(pid, T_FIXNUM);

  cap_d = cap_get_pid(FIX2INT(pid));

  if (cap_d == NULL) {
    rb_raise(
      rb_eRuntimeError,
      "Failed to get cap's for proccess %d: (%s)\n",
      FIX2INT(pid), strerror(errno)
    );
  } else {
    caps = cap_to_text(cap_d, &length);
    result = rb_str_new(caps, length);
    cap_free(caps);
    cap_free(cap_d);
  }

  return result;
}

void Init_cap2(void) {
  VALUE rb_mCap2;

  rb_mCap2 = rb_define_module("Cap2");

  rb_define_module_function(rb_mCap2, "getpcaps", cap2_getpcaps, 1);
}
