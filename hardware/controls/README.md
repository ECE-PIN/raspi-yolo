# Hardware Errata

## rpi camera
```plaintext
https://www.raspberrypi.com/documentation/computers/camera_software.html
```

--autofocus-mode Specify the autofocus mode <string>
Specifies the autofocus mode to use, which may be one of

default (also the default if the option is omitted) - normally puts the camera into continuous autofocus mode, except if either --lens-position or --autofocus-on-capture is given, in which case manual mode is chosen instead

manual - do not move the lens at all, but it can be set with the --lens-position option

auto - does not move the lens except for an autofocus sweep when the camera starts (and for libcamera-still, just before capture if --autofocus-on-capture is given)

continuous - adjusts the lens position automatically as the scene changes.

This option is only supported for certain camera modules (such as the Raspberry Pi Camera Module 3).

--autofocus-range Specify the autofocus range <string>
Specifies the autofocus range, which may be one of

normal (the default) - focuses from reasonably close to infinity

macro - focuses only on close objects, including the closest focal distances supported by the camera

full - will focus on the entire range, from the very closest objects to infinity.

This option is only supported for certain camera modules (such as the Raspberry Pi Camera Module 3).

--autofocus-speed Specify the autofocus speed <string>
Specifies the autofocus speed, which may be one of

normal (the default) - the lens position will change at the normal speed

fast - the lens position may change more quickly.

This option is only supported for certain camera modules (such as the Raspberry Pi Camera Module 3).

--autofocus-window Specify the autofocus window
Specifies the autofocus window, in the form x,y,width,height where the coordinates are given as a proportion of the entire image. For example, --autofocus-window 0.25,0.25,0.5,0.5 would choose a window that is half the size of the output image in each dimension, and centred in the middle.

The default value causes the algorithm to use the middle third of the output image in both dimensions (so 1/9 of the total image area).

This option is only supported for certain camera modules (such as the Raspberry Pi Camera Module 3).

--lens-position Set the lens to a given position <string>
Moves the lens to a fixed focal distance, normally given in dioptres (units of 1 / distance in metres). We have

0.0 will move the lens to the "infinity" position

Any other number: move the lens to the 1 / number position, so the value 2 would focus at approximately 0.5m

default - move the lens to a default position which corresponds to the hyperfocal position of the lens.

It should be noted that lenses can only be expected to be calibrated approximately, and there may well be variation between different camera modules.

This option is only supported for certain camera modules (such as the Raspberry Pi Camera Module 3).