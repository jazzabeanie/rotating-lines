# Rotating Lines Face

A minimal Pebble watchface that draws the largest circle that fits on the
display. Runs on every Pebble platform: Aplite, Basalt, Chalk, Diorite, Emery,
and Gabbro (Pebble 2 Duo / Core Devices).

## Prerequisites

- [Pebble SDK](https://github.com/pebble-dev/pebble-tool) (the community
  `pebble-tool` fork works well on modern systems and adds the Gabbro target)
- Python 2.7 (required by the SDK build scripts)
- Node.js (for the JS shim bundled with the app)

## Build

```sh
pebble build
```

The compiled `.pbw` bundle will be written to `build/rotating-lines-face.pbw`.

## Install

With a phone running the Pebble app on the same network as your computer:

```sh
pebble install --phone <PHONE_IP>
```

Or install in the emulator for each platform:

```sh
pebble install --emulator aplite
pebble install --emulator basalt
pebble install --emulator chalk
pebble install --emulator diorite
pebble install --emulator emery
pebble install --emulator gabbro
```

## How it works

`src/c/main.c` creates a full-screen `Layer` and, in its update procedure,
computes the radius as half of the smaller display dimension, then strokes a
circle centered on the screen. Because the radius is derived from
`layer_get_bounds` at runtime, the circle automatically fills each device's
display — square on rectangular Pebbles, inscribed in the round Chalk face.
