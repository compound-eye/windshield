# Windshield
Windshield is the software for driving a model car on a race course, using the camera to detect the track. It competed in
the DIY Robocars race on January 21, 2017 (https://www.meetup.com/DIYRobocars/events/236393485/), and was the only car that
successfully finished the lap. The race track was marked with white tapes, like in the photo:
https://www.meetup.com/DIYRobocars/photos/27550435/#457707167

Windshield consists of three programs:
* `drive` controls the car. It runs on Raspberry 3 with the camera and Navio2 board to control the motors.
   (https://docs.emlid.com/navio2/)
* `look` plays back the video recorded by `drive` for analysis. `drive` records every 5th image frames captured by the camera.
  `look` runs on Linux with GUI.  
  (The directory `think` contains the image processing code, shared by `drive` and `look`.)
* `birdeye` calibrates the camera pose and produces the matrix to tranform the images to bird's-eye view.

## Hardware Requirements
The drive program runs on Raspberry 3 with the camera and Navio2 board to control the motors. The drive program does not use
any sensors other than the camera. You can modify `drive/Motor.cpp` and `drive/Radio.cpp` to support other motor cotrol boards.

As a safety measure, the drive program reads the throttle position of your RC transmitter and use it to set the speed of
the car. You need a RC transmitter and a receiver supported by Navio2.

## Software Configuration
Windshield requires OpenCV 3 on both Raspberry Pi 3 and the host computer.

Windshield is built with `cmake`. On Raspberry Pi 3
```
cd drive
mkdir build
cd build
cmake ..
make
```
Do the same thing on the host computer for `look` and `birdeye`.

## Calibration
Download the OpenCV chessboard pattern https://github.com/opencv/opencv/blob/master/doc/pattern.png  
Print it out. Tape it on the floor in front the car. Take a picture of it from the pi camera using `raspistill`
(https://www.raspberrypi.org/documentation/usage/camera/raspicam/raspistill.md). It should look like `pic3.jpg` in `birdeye`
directory: https://github.com/compound-eye/windshield/blob/master/birdeye/pic3.jpg

Replace `birdeye/pic3.jpg` with the picture you just took. Run `birdeye`. Copy the output matrix to the variable `Hdata` in
`think/Compute.cpp`.

If your camera has a different resolution than 320x240, change the variable `dst` in `birdeye.cpp` to reflect that. When you
take the chessboard picture, use the highest resolution available on the camera, and let `birdeye` scale it down.

## Drive
On Raspbery Pi 3, create the directory `rover-images` under the home directory. The drive program will save every 5th images
from the camera in `rover-images`.

Turn on the RC transmitter. Make sure the light on the receiver is on. Move the throttle to the lowest position.

From `drive` directory, start the program with the command `sudo build/rover`. Raise the throttle.
(The drive program needs su privilege for the access to Navio2.)

Control-C to stop the drive program.

## Look at the Video Log
Here are the images from the successful race lap: https://github.com/compound-eye/rover-images

In `look/main.cpp`, uncomment the suitable line to select the image files for replay. Save, build, and run.

* space bar: pause/resume
* up arrow: rewind
* left arrow: previous frame
* right arrow: next frame

In `think/Compute.cpp`, uncomment the suitable section to display the images at various stage of processing.

## Customization
`think/Compute.cpp` processes the captured image and results in the steering angle. You will have to tweak it for different
race courses (until we have machine learning to figure it out all by itself).

For every images from the camera, `think/Compute.cpp` performs these steps:

1. Convert it to grayscale. (cv::cvtColor)
2. Smooth it out. (cv::blur)
3. Detect the edges. (cv::Canny)
4. Find the line segments. (cv::HoughLinesP)
5. Transform the line segments to bird's-eye view. (cv::perspectiveTransform)
6. Group the line segments into clusters by their angles.
7. Pick the steering angle from the cluster of lines with the longest combined length.

Our car uses differential drive, so currently `drive/main.cpp` steers the car by setting the speeds for the left and right
motors. However, as the output from `think/Compute.cpp` is the steering angle, it should be straightforward to adapt
`drive/main.cpp` for conventional steering. However, if you use the conventional steering, be sure to add a filter to smooth
out the steering control, or you will wear out the steering servo very fast.
