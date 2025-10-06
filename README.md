# V4L2 testing application

## Version 0.3.0

## Build

```bash
sudo apt update
sudo apt install -y build-essential cmake
git clone https://github.com/pmliquify/v4l2-test.git
./v4l2-test/make.sh
```
# Requirements
#### Gstreamer Support 

```bash
#For opencv support 
sudo apt-get install python3-opencv
# install a missing dependency
sudo apt-get install libx264-dev libjpeg-dev
# install the remaining plugins
sudo apt-get install libgstreamer1.0-dev \
     libgstreamer-plugins-base1.0-dev \
     libgstreamer-plugins-bad1.0-dev \
     gstreamer1.0-plugins-ugly \
     gstreamer1.0-tools \
     gstreamer1.0-gl \
     gstreamer1.0-gtk3 \
     python3-yaml
# if you have Qt5 install this plugin
sudo apt-get install gstreamer1.0-qt5
```

#### On RaspberryPi OS Lite  install also:
```bash
sudo apt-get install \
  gstreamer1.0-tools \
  gstreamer1.0-plugins-base \
  gstreamer1.0-plugins-good \
  gstreamer1.0-plugins-bad \
  gstreamer1.0-plugins-ugly \
  gstreamer1.0-libav \
  gstreamer1.0-gl
```

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


# Gstreamer

## Gstreamer UDP 

Streaming over ethernet is possible by udp 
First, on the host the server has to be started by 
```bash
gst-launch-1.0 udpsrc port=5000 caps="application/x-rtp, media=(string)video, encoding-name=(string)H264" ! rtph264depay ! avdec_h264 ! videoconvert ! autovideosink
```
On the remote device, the v4l2-test tool has to be started by the command 

```bash
./v4l2-test gstreamer --gst udp --udphost=<IP HOST> --port 5000
```
