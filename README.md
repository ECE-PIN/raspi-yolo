# raspi-yolo

## Function Comment Format
/**  
 \* Detailed description of what function's purpose is  
 \*  
 \* Input:  
 \* @param Name of Input 1 Description of Input 1  
 \* @param Name of Input 2 Description of Input 2  
 \* @return Description of return value
 */  

# Create a build and a YOLO directory within the root of the repo

Navigate into YOLO and run:
git clone https://github.com/lpelkey23/yolov4-tiny

cd into the directory it creates (yolov4-tiny)

run make

grab the weights file for tinyv4:
wget https://github.com/AlexeyAB/darknet/releases/download/yolov4/yolov4-tiny.weights

cd ../../build

run cmake ..
run make

# Had to install to run repo:

sudo apt install make
sudo apt install g++
sudo apt install cmake
sudo apt install libgoogle-glog-dev
sudo apt install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev  
libopencv-dev



MESA: error: ZINK: failed to choose pdev: sudo apt install vulkan-tools

What it means: This error comes from the Zink driver, which is a Mesa driver that translates Vulkan calls to OpenGL. The error suggests that Zink failed to choose a physical device (pdev) for rendering. This typically occurs when:

Vulkan is not properly configured or no Vulkan-compatible GPU is detected.
There’s a misconfiguration in your graphics driver or Vulkan loader.
Your system might not have the necessary Vulkan support libraries installed.




glx: failed to create drisw screen: 
sudo apt install mesa-utils (MAYBE NOT NEEDED)
sudo add-apt-repository ppa:kisak/kisak-mesa
sudo apt update
sudo apt upgrade

What it means: This error comes from the GLX extension, which is used for integrating OpenGL with the X Window System. The error indicates that the driver failed to create a DRI software (drisw) screen, which happens when:

There’s a problem with the Mesa software renderer.
The system cannot access the GPU or software rendering fallback (DRISW) is misconfigured.
There’s a mismatch between the SDL backend, OpenGL/GLX, and the Mesa configuration.
