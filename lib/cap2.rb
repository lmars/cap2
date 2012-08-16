require 'cap2.so'
require 'cap2/entity'
require 'cap2/process'
require 'cap2/file'

module Cap2
  class << self
    def has_capability?(pid_or_filename, set, cap)
      set = SETS[set.to_s]
      cap = CAPS[cap.to_s]

      case pid_or_filename
      when Fixnum
        pid_has_cap?(pid_or_filename, set, cap)
      when String
        file_has_cap?(pid_or_filename, set, cap)
      else
        raise ArgumentError, "wrong argument type, expected Fixnum or String, got #{pid_or_filename.class}"
      end
    end

    def process(pid = ::Process.pid)
      check_pid! pid

      Process.new(pid)
    end

    def file(filename)
      check_file! filename

      File.new(filename)
    end

    private
    def check_pid!(pid)
      ::Process.kill(0, pid)
    rescue Exception => e
      if Errno::ESRCH === e
        raise ProcessNotFound
      end
    end

    def check_file!(filename)
      unless ::File.exists?(filename)
        raise FileNotFound
      end
    end
  end

  class ProcessNotFound < StandardError; end
  class FileNotFound < StandardError; end
end
