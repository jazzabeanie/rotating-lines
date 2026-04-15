# Rotating Lines Face

A minimal Pebble watchface that draws the largest circle that fits on the
display. Runs on every Pebble platform: Aplite, Basalt, Chalk, Diorite, Emery,
and Gabbro (Pebble 2 Duo / Core Devices).

## Building

- Install the SDK almost according to https://developer.repebble.com/sdk/
  - Install node
  - `sudo apt install libsdl1.2debian libfdt1`
  - `brew install uv`
- Create a virtual env: `conda create --prefix conda-env python=3.13` and
  activate it with `conda activate conda-env/` (it should also work down to
  Python 3.10).
- `uv pip install pebble-tool`
- `pebble sdk install latest`
- `pebble build`
- `pebble install --emulator basalt`

Once it's set up, just run:

- `conda activate conda-env`
- `pebble build`
- `pebble install --emulator basalt`

The compiled `.pbw` bundle will be written to `build/rotating-lines-face.pbw`.

If installing to the emulator fails:

- Build and install a known-working project (e.g. pebble-demo)
- `pebble clean`
- `uv pip install --upgrade --reinstall pebble-tool`
- `rm -rf ~/.pebble-sdk`
- `pebble sdk install latest`

## Installing to a watch

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
