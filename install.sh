#!/bin/bash

sudo apt update && sudo apt full-upgrade -y

# Build tools
sudo apt install make g++ cmake

# Camera
sudo chmod -R 777 /home/pi/Documents/raspi-yolo/images
sudo apt install libcamera-apps libcamera-dev

# Glog
sudo apt install libgoogle-glog-dev

# OPENCV
sudo apt install libopencv-dev

# Protobuf
sudo apt install protobuf-compiler libprotobuf-dev

# SDL
sudo apt install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev

# SQLite
sudo apt install sqlite3 libsqlite3-dev

# WiringPi
git clone https://github.com/WiringPi/WiringPi.git
cd WiringPi
./build debian
mv debian-template/wiringpi_3.14_arm64.deb .
sudo apt install ./wiringpi_3.14_arm64.deb
cd ..
rm -rf WiringPi

# zeroMQ
sudo apt-get install libzmq3-dev
mkdir zero
cd zero
git clone https://github.com/zeromq/zmqpp.git
cd zmqpp
mkdir build
cd build
cmake ..
make
sudo make install
cd ../../..
rm -rf zero

#json
sudo apt install nlohmann-json3-dev

# Python virtual envrionment
mkdir build
cd build
python3 -m venv models-venv
source models-venv/bin/activate
pip install easyocr
pip install ultralytics
pip install pyzmq
deactivate

# Compile
cmake ..
make
