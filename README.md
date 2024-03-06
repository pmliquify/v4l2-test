# V4L2 testing application

## Version 0.3.0

## Build

```bash
sudo apt update
sudo apt install -y build-essential cmake
git clone https://github.com/pmliquify/v4l2-test.git
./v4l2-test/make.sh
```

You can find the executable in the build* directory.

### Arm64 build (cross compiling)

```bash
sudo apt update
sudo apt install -y binutils-aarch64-linux-gnu g++-aarch64-linux-gnu cmake
git clone https://github.com/pmliquify/v4l2-test.git
./v4l2-test/make.sh --cross
```

## Usage

#### Annotation NXP

For NXP plattforms it is necessary to set the image resolution by `v4l2-ctl` before you start using `v4l2-test`.

```bash
v4l2-ctl --set-fmt-video=width=<width>,height=<height>
```

### Image streaming to the console

```bash
$ ./v4l2-test stream -e <exposure> -g <gain> -f <pixelformat> -p 1
Format (width: 1920, height: 1080, pixelformat: RG10, colorspace: SRGB)
[#0001, ts:  252432, t:   0 ms, 1920, 1080, 3840, RG10] (960, 540) 0000001111110010 0000001111101111 0000001111111111 
[#0002, ts:  252449, t:  17 ms, 1920, 1080, 3840, RG10] (960, 540) 0000001111100101 0000001101110001 0000001110111011 
[#0003, ts:  252465, t:  16 ms, 1920, 1080, 3840, RG10] (960, 540) 0000001111111111 0000001111111111 0000001110011010
...
```

Each image is represented by one line.

```text
                                   + image width
                                   |     + image height
                                   |     |     + bytes per line
                                   |     |     |     + pixelformat
                                   |     |     |     |
                                   v     v     v     v
[#0002, ts:  252449, t:  17 ms, 1920, 1080, 3840, RG10] <image data>
     ^            ^       ^
     |            |       |
     |            |       + time between two consecutive images
     |            + time stamp
     + sequence number
```

### Image streaming to the framebuffer

If you have a display attached and a kernel with framebuffer support, you can get a better image output by streaming it to the framebuffer.

```bash
$ ./v4l2-test stream --fb
Format (width: 1920, height: 1080, pixelformat: RG10, colorspace: SRGB)
>>>>>>>>>>>>>>>>>>>>>>>>>>
```

Use the `-p` to additionaly print out one line if image information to the console.
