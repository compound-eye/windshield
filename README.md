## Windshield
Windshield is our software for driving a model car on a race course, using only one camera to detect the track. It competed in
the [DIY Robocars race on January 21, 2017] (https://www.meetup.com/DIYRobocars/events/236393485/), and was the only car that
completed a lap at racetime. The race track was [marked with white tapes](https://www.meetup.com/DIYRobocars/photos/27550435/#457707167).

Windshield consists of three programs:

* `drive` controls the car. It runs on Raspberry 3 with the camera and Navio2 board to control the motors.
   (https://docs.emlid.com/navio2/)
* `look` plays back the video recorded by `drive` for analysis. `drive` records every 5th image frames captured by the camera.
  `look` runs on Linux with GUI.  
  (The directory `think` contains the image processing code, shared by `drive` and `look`.)
* `birdeye` calibrates the camera pose and produces the matrix to tranform the images to bird's-eye view.

### Hardware Requirements
The drive program runs on Raspberry 3 with the camera and Navio2 board to control the motors. The drive program does not use
any sensors other than the camera. You can modify `drive/Motor.cpp` and `drive/Radio.cpp` to support other motor cotrol boards.

As a safety measure, the drive program reads the throttle position of your RC transmitter and use it to set the speed of
the car. You need a RC transmitter and a receiver supported by Navio2.

Our car was based on one of the [rover kits from from Lynxmotion](http://www.lynxmotion.com/c-30-rovers.aspx).

### Software Configuration
Windshield requires OpenCV 3 on both Raspberry Pi 3 and the host computer.

Windshield is built with `cmake`. On Raspberry Pi 3:

    cd drive 
    mkdir build
    cd build
    cmake .. 
    make

Do the same thing on the host computer for `look` and `birdeye`.

### Calibration
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

Our car uses differential drive, so currently `drive/main.cpp` steers the car by setting the speeds for the left and right
motors. As the output from `think/Compute.cpp` is the steering angle, it should be straightforward to adapt
`drive/main.cpp` for conventional steering. If you use the conventional steering, be sure to add a filter to smooth
out the steering control, or you will wear out the steering servo very fast.

### Look at the Video Log
Here are the [images from the successful race lap]( https://github.com/compound-eye/rover-images).

In `look/main.cpp`, uncomment the suitable line to select the image files for replay. Save, build, and run.

* space bar: pause/resume
* up arrow: rewind
* left arrow: previous frame
* right arrow: next frame

In `think/Compute.cpp`, you can comment/uncomment sections of code to display the images at various stage of processing.

## Theory
`think/Compute.cpp` processes the captured image and outputs a new steering angle. 

Our goal is simply to drive between the lines. Our algorithm reduces to:


     WHILE new image {
         find lines
         IF there is a line crossing our path {
             turn parallel to the line to avoid it
         } ELSE {
             drive straight
         }
     }

No, it's not very complicated. KISS.

Notes:

1. The default is to drive straight. If we fail to see the edge of the road, we roll off the track and keep going, until we see something else that looks like a lane marker.
2. We don’t care about lines to our side that we’re not in danger of crossing; in particular we don’t try to hug our lane.

###Finding Lines

We use a standard technique to find lines in each image.

* Convert to grayscale
* Blur
* Canny edge detection
* Hough line search

The Wikipedia articles on [Canny edge detectors](https://en.wikipedia.org/wiki/Canny_edge_detector) and the [Hough transform](https://en.wikipedia.org/wiki/Hough_transform) are good. Here are some of the issues that we ran into:

Scratches in the concrete floor and reflections of the roof structure in puddles of rain generated lots of spurious edges and lines in our tests. A 5x5 Gaussian blur took care of most of those. We also threw away line segments that were very short or that required connecting widely separated dots; those are tunable parameters in the OpenCV implementation of the Hough transform.

We had to tune other Canny and Hough thresholds for the environment: wooden floors in brightly lit homes are not the same as concrete floors in dank warehouses.

The output of these steps is an array of lines, characterized by their start and end points in pixel co-ordinates. Those co-ordinates are not directly useful because of perspective distortion.

###Correcting for perspective

Imagine we’re driving along a straight road that extends to the horizon. From the point of view of a dashcam the lane markers are converging (towards a vanishing point). If the road turns slightly to the right, the left edge will cross our direction of travel, but at a much steeper angle than the steering correction we need to make.

What we really want is a top down or bird’s eye view of the road and our position. We could synthesize that viewpoint if we had a 3D map of the scene from LIDAR or stereo, but we don’t.

Instead we assume that the region of interest (the ROI, in this case the track and all the lane markings) is a horizontal plane. Given that assumption and the pose of our camera, we can compute a 3x3 homography that when applied to the image renders a top down view of the ROI.

We figure out the camera pose ahead of time using a calibration target and the `birdeye` app.

We don’t need to warp the whole image; we only need to warp the lines that we find. This is very fast.

Note:

1. This technique is called the Inverse Perspective Transform in the literature.
2. After calibration the camera cannot move with respect to the robot, so no panning or tilting to look for lines.
3. Outside the ROI the output is geometrically meaningless: walls, spectators, and other cars will not appear as they would in a true top down image.
4. If the course included steep up or down slopes that invalidated the planar assumption, we'd have some issues.

###Choosing a steering angle

Now we have an array of line segments that mostly correspond to the edges of the track, plus some garbage, as seen from above.

The remaining steps are:

* Compute the angle that the line segments make with the horizontal.
* Filter out line segments that appear on one side of the image only and lean outwards, on the assumption that we’re not in danger of crossing those.
* Cluster the rest by angle.
* Find the cluster with the longest combined length, and set that angle as our steering angle.
