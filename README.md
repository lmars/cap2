Cap2
====

Cap2 is a Ruby library for managing the POSIX 1003.1e capabilities available in Linux kernels. These capabilities are a partitioning of the all powerful root privilege into a set of distinct privileges. See capabilites(7) for more information.

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

Querying Capabilities
---------------------

### Processes

Suppose a process with pid 1000 exists with the following capabilities:

```
Permitted   - CAP_CHOWN, CAP_LEASE
Effective   - CAP_CHOWN
Inheritable - CAP_LEASE
```

Then:

```
process = Cap2.process(1000)  # => #<Cap2::Process>

process.permitted?(:chown)    # => true
process.permitted?(:lease)    # => true
process.permitted?(:fowner)   # => false

process.effective?(:chown)    # => true
process.effective?(:lease)    # => false

process.inheritable?(:chown)  # => false
process.inheritable?(:lease)  # => true
```

### Files

Suppose the file "/tmp/cap_test" exists with the following capabilities:

```
Permitted   - CAP_CHOWN, CAP_LEASE
Effective   - CAP_CHOWN
Inheritable - CAP_LEASE
```

Then:

```
file = Cap2.file('/tmp/cap_test')  # => #<Cap2::File>

file.permitted?(:chown)            # => true
file.permitted?(:lease)            # => true
file.permitted?(:fowner)           # => false

file.effective?(:chown)            # => true
file.effective?(:lease)            # => false

file.inheritable?(:chown)          # => false
file.inheritable?(:lease)          # => true
```
