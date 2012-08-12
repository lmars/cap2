require 'mkmf'

unless have_header('sys/capability.h')
  abort <<-EOS
  -----
  ERROR
  -----
  sys/capability.h is missing. You should install the libcap2 header files

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
