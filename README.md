# Modular Proxy

A network proxy that lets you easily load and unload modules at runtime.
All you have to do is put them in a directory.

## Compiling & Running

The program uses CMake, so this compiles the program:

```bash
cmake -B build/
cmake --build build/
```

To run, just execute the generated binary:

```bash
./build/mproxy <modules_dir> <outbound_port>
```

Currently the proxy only binds to `0.0.0.0:0` and connects to `127.0.0.1:<outbound_port>`.
First item on the TODO list is here to change that...

## Modules

Check out the example module at [`modules/hello_world.c`](modules/hello_world.c).

To compile a module, just use the `-shared` flag and output to a file with the extension `.so`.

## TODOs

- Properly parse arguments (and maybe rethink them);
- Load modules on program start instead of only when events are fired through inotify;
- Console instead of getchar() on main thread while running;
- Improve the "closing the program" case;
- Allow for multiple clients on the same proxy.
