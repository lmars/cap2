Cap2
====

Cap2 is a Ruby library for managing Linux process and file capabilities

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
