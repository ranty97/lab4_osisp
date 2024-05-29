# consumer/producer

### What does this program do?

This program demonstrates multiprocess communication using shared memory and mutexes to suppress race conditions.

### How to build this project

This project can be built in release and debug modes using `make` utility. 

```
make MODE=release
```

or

```
make MODE=debug
```

Use `l` key to create a producer. Use `k` key to kill it.

Use `o` key to create a consumer. Use `i` key to kill it.

Use `q` key to kill all consumers and producers and terminate the program.
