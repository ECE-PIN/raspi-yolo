#!/bin/bash

sudo apt update

# Build tools
sudo apt install make g++ cmake

# Glog
sudo apt install libgoogle-glog-dev

# SDL
sudo apt install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev

# OPENCV
sudo apt install libopencv-dev

# SQLite
sudo apt install sqlite3 libsqlite3-dev

# Protobuf
sudo apt install protobuf-compiler libprotobuf-dev

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

# Python virtual envrionment
mkdir build
cd build
python3 -m venv models-venv
source models-venv/bin/activate
pip install easyocr
pip install efficientnet
pip install tensorflow
pip install pyzmq
deactivate

# Compile
cmake ..
make
