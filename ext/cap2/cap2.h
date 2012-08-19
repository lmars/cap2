#include <ruby.h>
#include <errno.h>
#include <sys/capability.h>

cap_flag_t cap2_sym_to_set(VALUE set);
cap_value_t cap2_sym_to_cap(VALUE cap);
VALUE cap2_has_cap(cap_t cap_d, VALUE set_sym, VALUE cap_sym);

VALUE cap2_file_has_cap(VALUE self, VALUE set_sym, VALUE cap_sym);
VALUE cap2_file_permit(VALUE self, VALUE cap_sym);
VALUE cap2_file_unpermit(VALUE self, VALUE cap_sym);
VALUE cap2_file_allow_inherit(VALUE self, VALUE cap_sym);
VALUE cap2_file_disallow_inherit(VALUE self, VALUE cap_sym);
VALUE cap2_file_set_effective(VALUE self, VALUE cap_sym);
VALUE cap2_file_clear_effective(VALUE self, VALUE cap_sym);

VALUE cap2_process_has_cap(VALUE self, VALUE set_sym, VALUE cap_sym);
