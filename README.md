# Modular Proxy

A network proxy that lets you easily load and unload modules at runtime.
All you have to do is put them in a directory.

## Compiling & Running

The program uses CMake, so this compiles the program:

```bash
cmake -B build/
cmake --build build/
```

To run, just execute the generated binary with the required arguments:

```bash
./build/mproxy -d <modules_dir> -o <outbound_port>
```

## Modules

Check out the example module at [`modules/hello_world.c`](modules/hello_world.c).

To compile a module, just use the `-shared` flag and output to a file with the extension `.so`.

## TODOs

- Load modules on program start instead of only when events are fired through inotify;
- Console instead of getchar() on main thread while running;
- Improve the "closing the program" case;
- Allow for multiple clients on the same proxy.
