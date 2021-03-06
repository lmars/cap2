Cap2
====

Cap2 is a Ruby library for managing the POSIX 1003.1e capabilities available in Linux kernels. These capabilities are a partitioning of the all powerful root privilege into a set of distinct privileges. See capabilities(7) for more information.

Installation
------------

To install this library, you will need to be running Linux and have the libcap2 library and header files installed.

### Install libcap2

#### Ubuntu

```
$ sudo apt-get install libcap2 libcap-dev
```

#### From source

```
$ curl -O http://ftp.de.debian.org/debian/pool/main/libc/libcap2/libcap2_2.22.orig.tar.gz
$ tar xzf libcap2_2.22.orig.tar.gz
$ cd libcap2-2.22
$ make
$ sudo make install
```

### Install the gem

```
$ gem install cap2
```

Process and File Objects
------------------------

Cap2 provides methods for querying and modifying capabilities for both processes and files.

Process capabilities are accessed via a `Cap2::Process` object (returned from `Cap2.process`) and file capabilities via a `Cap2::File` object (returned from `Cap2.file`):

```
Cap2.process              # Returns a Cap2::Process for the current process
Cap2.process(1000)        # Returns a Cap2::Process for the process with pid 1000
Cap2.file('/sbin/init')   # Returns a Cap2::File for the /sbin/init file
```

Querying Capabilities
---------------------

Capabilities are referenced using lower cased symbols, and without the CAP_ prefix (e.g. `:kill` represents CAP_KILL).

There are three methods defined on both `Cap2::Process` and `Cap2::File` for querying capabilities:

* `permitted?`
* `enabled?`
* `inheritable?`

Each of these methods, `Cap2::File#enabled?` being the exception, take a list of capability symbols and return true / false if the capabilities are in / not in the relevant set:

```
# the init daemon - all caps permitted & enabled but not inheritable
init = Cap2.process(1)            # => #<Cap2::Process @pid=1>

init.permitted?(:kill)            # => true
init.permitted?(:chown, :fowner)  # => true

init.enabled?(:fowner)            # => true
init.enabled?(:kill, :chown)      # => true

init.inheritable?(:chown)         # => false
init.inheritable?(:fowner, :kill) # => false
```

```
# assume /bin/ping is using file capabilities to enable CAP_NET_RAW on exec
ping = Cap2.file('/bin/ping')   # => #<Cap2::File @filename="/bin/ping">

ping.permitted?(:net_raw)       # => true
ping.permitted?(:mknod, :chown) # => false

ping.enabled?                   # => true

ping.inheritable?(:net_raw)     # => false
```

### Why does Cap2::File#enabled? not take an arguement?

Under the hood, the file effective set is just a single bit. When enabled, any process which exec's the file will have the capabilities from it's resulting permitted set also in it's effective set.

Modifying File Capabilities
---------------------------

The file capability modification methods are:

* `permit(capabilities)`
* `unpermit(capabilities)`
* `allow_inherit(capabilities)`
* `disallow_inherit(capabilities)`
* `enable`
* `disable`

where `capabilities` can take any of the following forms:

```
# list of one or more symbols
(:chown)
(:mknod, :kill, :mac_admin)

# hash with an :only key
(:only => :fowner)
(:only => [:lease, :net_raw])

# hash with an :except key
(:except => :sys_time)
(:except => [:syslog, :fsetid])
```

### Permitting capabilities

To modify the permitted capabilities of a file (i.e. the capabilities which will be permitted in any process that exec's the file), use `Cap2::File#permit` and `Cap2::File#unpermit`:

```
Cap2.process.enabled?(:setfcap)          # => true - needed to set file capabilities

file = Cap2.file('/tmp/file')            # => #<Cap2::File @filename="/tmp/file">

file.permitted?(:mknod, :chown, :fowner) # => false

file.permit(:mknod)                      # => true
file.permit(:chown, :fowner)             # => true
file.permitted?(:mknod, :chown, :fowner) # => true

file.unpermit(:mknod)                    # => true
file.unpermit(:chown, :fowner)           # => true
file.permitted?(:mknod, :chown, :fowner) # => false
```

### Enabling capabilities

To modify the effective bit of a file (i.e. whether the resulting permitted capabilities of any process that exec's the file will also be enabled in it's effective set), use `Cap2::File#enable` and `Cap2::File#disable`:

```
Cap2.process.enabled?(:setfcap)     # => true - needed to set file capabilities

file = Cap2.file('/tmp/file')       # => #<Cap2::File @filename="/tmp/file">

file.permitted?(:net_raw)           # => true
file.enabled?                       # => false

file.enable                         # => true
file.enabled?                       # => true

file.disable                        # => true
file.enabled?                       # => false
```

### Inheriting capabilities

To modify the inheritable capabilities of a file (i.e. the capabilities which will be ANDed with the inheritable set of any process that exec's the file to determine which capabilities are in the permitted set of the process), use `Cap2::File#allow_inherit` and `Cap2::File#disallow_inherit`:

```
Cap2.process.enabled?(:setfcap)     # => true - needed to set file capabilities

file = Cap2.file('/tmp/file')       # => #<Cap2::File @filename="/tmp/file">

file.inheritable?(:fowner)          # => false

file.allow_inherit(:fowner)         # => true
file.inheritable?(:fowner)          # => true

file.disallow_inherit(:fowner)      # => true
file.inheritable?(:fowner)          # => false
```

### Clearing capabilities

To clear all capabilities for a file, use `Cap2::File#clear`:

```
Cap2.process.enabled?(:setfcap)     # => true - needed to set file capabilities

file = Cap2.file('/tmp/file')       # => #<Cap2::File @filename="/tmp/file">

file.permit(:kill, :mknod)          # => true
file.allow_inherit(:kill, :mknod)   # => true
file.enable                         # => true

file.permitted?(:kill, :mknod)      # => true
file.inheritable?(:kill, :mknod)    # => true
file.enabled?                       # => true

file.clear                          # => true

file.permitted?(:kill, :mknod)      # => false
file.inheritable?(:kill, :mknod)    # => false
file.enabled?                       # => false
```

Modifying Process Capabilities
------------------------------

Cap2 can be used to enable / disable capabilities of the current Ruby process.

Suppose the ruby binary file permits `:kill`, but does not enable it on exec:

```
ruby = Cap2.file('/usr/bin/ruby')   # => #<Cap2::File @filename="/usr/bin/ruby">
ruby.permitted?(:kill)              # => true
ruby.enabled?                       # => false
```

and we want to kill a process running as root with pid 1000:

```
Cap2.process.enabled?(:kill)    # => false
Process.kill("TERM", 1000)      # => Errno::EPERM: Operation not permitted

Cap2.process.enable(:kill)      # => true
Cap2.process.enabled?(:kill)    # => true
Process.kill("TERM", 1000)      # => 1
```

Notes
-----

* Cap2 cannot be used to modify capabilities of other processes. If you are curious as to why, see the Notes section of [cap_set_proc(3)](http://linux.die.net/man/3/cap_set_proc) for an explanation.
* File capabilities are not supported for shell scripts due to security implications, similar to the reasons setuid is disabled, [see here](http://unix.stackexchange.com/questions/364/allow-setuid-on-shell-scripts#answer-2910) for more information.
