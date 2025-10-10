
%PHONY: all installDependencies build
installDependencies:
	sudo apt-get update
	sudo apt-get install -y \
		python3-opencv \
		libx264-dev libjpeg-dev \
		libgstreamer1.0-dev \
		libgstreamer-plugins-base1.0-dev \
		libgstreamer-plugins-bad1.0-dev \
		gstreamer1.0-plugins-ugly \
		gstreamer1.0-tools \
		gstreamer1.0-gl \
		gstreamer1.0-gtk3 \
		python3-yaml \
	gstreamer1.0-tools \
	gstreamer1.0-plugins-base \
	gstreamer1.0-plugins-good \
	gstreamer1.0-plugins-bad \
	gstreamer1.0-plugins-ugly \
	gstreamer1.0-libav \
	gstreamer1.0-gl
build:
	./make.sh 
all: installDependencies build