# Noice

Noice is a command line Noise generator utility, by Kleber Garcia.

![blue perlin example](Misc/noisedemosall.png?raw=true "noise demos.")

## Usage

Check the help menu for information on all the switches:

```shell
## long syntax 
noice --help

## short syntax
noice -h
```

Noice provides ways to pack custom noise patterns per channel. For example, the following command creates a texture
with blue noise in the red channel, and white noise in the green channel:

```shell
noice -cR -n blue -cG -n white
```

The -cR and -cG switches activate a particular channel. After a channel switch is parsed, the rest of the commands, such as -n 
(which is the noise type) will operate on that specific switch. When another switch channel is found, then the tool repeats the process.
This allows to configure any packing at will if desired.

Dimensions can be specified using the -d switch. Noice also accepts a more verbose syntax, with the '--' prefix. For example:

```shell
noice  -n blue --dimensions=128x128
```

is the same as writting:

```shell
noice -n blue -d 128x128
```

Channel switches are not mandatory. By default, Noice only writes to the red channel.

Noice can also generate volume textures, for example to create a cloud perlin noise texture:

```shell
noirce -n perlin -d 32x32x32 
```

Resulting in this:
![3d perlin](Misc/3dperlin.png?raw=true "3d perlin.")

Volume textures are layed out vertically. Engines, such as unity can provide simple tools to import these textures.

## Features

- Noice only supports white, blue and perlin for now. New patterns will be added and are very welcome to be added.
- Noice only writes EXR 32 bit float files. Meant to provide highest precision and let texture pipelines decide on compression.
- Only supporting x64 intel architecture. The code should be easy to port to other architectures, but will require managing OpenEXR, Zlib, and whatever else necessary.

## Building on windows


To build on windows the following requirements are necessary:

* Windows 10, version 19042 or higher
* Visual Studio 2019

ISPC, tundra and OpenEXR have been precompiled for compilation convenience.

To compile in windows, run:

```batch
build.bat release
```

To generate the solution, run:

```batch
gensln.bat
```

All build artifacts will be located in the t2-output directory.

## Building on linux

Linux has only been tested on debian ubuntu.
To build on linux, first launch the script that will download the required dependencies:

```shell
./linux-deps.sh
```
This should download:
* OpenEXR version 2.3.0 debian
* libTbb version 2020.1-2

ISPC is precompiled for convenience.

After this, just run

```shell
./build.sh release
```

All build artifacts will be located in the t2-output directory.

## Contributing

Contributions and pull requests are very much welcome! please run the tests when doing so, and ensure to add new tests if necessary.
To run the tests, just find the built noice_test application and run it.






