# Pong
A simple pong clone written in C using SDL3.

<p align="center">
  <img src="https://github.com/snddude/c-pong/blob/main/repo/preview.png" alt="A preview of the debug console window.">
</p>

## Controls
- W and S to move the left paddle.
- Up and Down arrows to move the right paddle.

## Building and running
### Dependencies:
- [SDL3 build dependencies for linux.](https://wiki.libsdl.org/SDL3/README-linux#build-dependencies)
- CMake 4.3 or newer.

```bash
git clone https://github.com/snddude/c-pong && cd c-pong
mkdir build && cd build
cmake ..
make
./dist/pong
```

## License

[MIT](https://en.wikipedia.org/wiki/MIT_License)
