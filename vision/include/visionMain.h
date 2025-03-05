#ifndef VISION_PIPE_H
#define VISION_PIPE_H

#include "../../food_item.h"
#include "../../pipes.h"
#include "../include/ImageProcessor.h"
#include "../include/helperFunctions.h"
#include <iostream>

void visionEntry(struct Pipes);
bool startPythonServer();
bool isPythonServerReady();
#endif
