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

VALUE cap2_file_has_cap(VALUE self, VALUE set_sym, VALUE cap_sym) {
  cap_t cap_d;
  VALUE result;

  cap_d = cap2_file_caps(self);

  result = cap2_has_cap(cap_d, set_sym, cap_sym);

  cap_free(cap_d);

  return result;
}

VALUE cap2_file_permit(VALUE self, VALUE cap_sym) {
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
