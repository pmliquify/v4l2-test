#!/bin/bash

cd /home/forlinx/v4l2-test

rm -rf *.jpg
COUNT=100

v4l2-ctl --all -d /dev/v4l-subdev2 -c horizontal_flip=1 -c vertical_flip=1
v4l2-ctl --all -d /dev/v4l-subdev7 -c horizontal_flip=1 -c vertical_flip=1
v4l2-ctl --all -d /dev/v4l-subdev12 -c horizontal_flip=1 -c vertical_flip=1
v4l2-ctl --all -d /dev/v4l-subdev17 -c horizontal_flip=1 -c vertical_flip=1
v4l2-ctl --all -d /dev/v4l-subdev22 -c horizontal_flip=1 -c vertical_flip=1

./v4l2-test imagesaver --prefix CAM1 --count $COUNT --timeout 200 -d /dev/video55 &
./v4l2-test imagesaver --prefix CAM2 --count $COUNT --timeout 200 -d /dev/video64 &
./v4l2-test imagesaver --prefix CAM3 --count $COUNT --timeout 200 -d /dev/video73 &
./v4l2-test imagesaver --prefix CAM4 --count $COUNT --timeout 200 -d /dev/video82 &
./v4l2-test imagesaver --prefix CAM5 --count $COUNT --timeout 200 -d /dev/video91 
