#!/bin/bash

cd build
source models-venv/bin/activate
cd ../vision/Models
python3 server.py
