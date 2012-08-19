#include "cap2.h"

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

VALUE cap2_file_has_cap(VALUE self, VALUE set_sym, VALUE cap_sym) {
  cap_t cap_d;
  VALUE result;

  cap_d = cap2_file_caps(self);

  result = cap2_has_cap(cap_d, set_sym, cap_sym);

  cap_free(cap_d);

  return result;
}

VALUE cap2_file_permit(VALUE self, VALUE cap_sym) {
  return cap2_file_set_cap(self, CAP_PERMITTED, cap_sym, CAP_SET);
}

VALUE cap2_file_unpermit(VALUE self, VALUE cap_sym) {
  return cap2_file_set_cap(self, CAP_PERMITTED, cap_sym, CAP_CLEAR);
}

VALUE cap2_file_allow_inherit(VALUE self, VALUE cap_sym) {
  return cap2_file_set_cap(self, CAP_INHERITABLE, cap_sym, CAP_SET);
}

VALUE cap2_file_disallow_inherit(VALUE self, VALUE cap_sym) {
  return cap2_file_set_cap(self, CAP_INHERITABLE, cap_sym, CAP_CLEAR);
}
