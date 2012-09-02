require 'mkmf'

unless have_header('sys/capability.h')
  abort <<-EOS
  -----
  ERROR
  -----
  sys/capability.h is missing. You should install the libcap2 header files

  EOS
end

# Generate cap2.h dynamically to define cap2_caps, an array of capability
# name / value pairs, with values as defined in <linux/capability.h>
File.open(File.dirname(__FILE__) + '/cap2.h', 'w') do |file|
  cap_count = 0
  caps = []

  File.
    readlines('/usr/include/linux/capability.h').
    grep(/#define CAP_([\w_]+)\s+(\d+)/) do
      caps << %[{ #{$2}, "#{$1.downcase}" }]
      cap_count += 1
    end

  file.puts <<-EOS
struct {
  int value;
  char name[32];
} cap2_caps[] = {
  #{caps.join(",\n  ")}
};

#define __CAP_COUNT #{cap_count}
EOS
end

unless have_library('cap')
  abort <<-EOS
  -----
  ERROR
  -----
  libcap is missing. You should install the libcap2 library

  EOS
end

create_makefile('cap2')
