Raspiberry Pi MMAL movement detection project


Introduction
------------

This project has mainly 2 aims:
1) offer a clear example how to use mmal library in c in raspberry pi
2) provide a movement detection algorithm able to create videos of the scenes where something moves.


Current state:
--------------

Text mode version, capable of produce videos when something moves on the screen.
As movement is not detected the frames are ignored.


Current issues / improvements to be done:
---------------
1) the playback video is faster then it should be.
It seems that this problems originates from the option encoding type=MMAL\_ENCODING\_I420.
If one uses encoding MMAL\_ENCODING\_OPAQUE, this effects disapear. 
The problem is that, if one uses ENCODING\_OPAQUE just the headers are visible in user space, and we need the whole frame data to detect movement.

2) currently there is no way to select a portion of the image and ignore the movement on the rest.

3) improve algorithm of detection: the current algorithm is just a comparison between the current frame with a frame from about a second ago. As soon as a difference greater than threshold is found, it is considered that something moved.

Points 1) and 3) can be improved with motion vectors, probably the next step.

Projects
--------

test\_preview: the simplest variant to get the raspicam working.

test\_preview\_no\_tunneling: the worst problem so far is to understand how the connections and the callbacks work.

This source changes the test\_preview code, taking out the tunneling flag on the connections and passing the buffers manually.

test\_encode: A simple version of encoding the output as h264 and saving to a file

test\_encode\_no\_tunneling: Modified version of test\_encode not using tunneling connections

test\_movement\_detection: Starts capturing the scene until it stops moving (calibration time). 
When the scene is static the program waits until something changes.
When the movement is detected, all frames are sent to output, until n frames come where no movement is registered.
For this first version, n=50 (about 2 seconds).
After ~2 seconds with no move, the program exits.

run\_motion.sh: runs a loop with a counter. Every iteration it start test\_movement with a new name and the desired output name.
This way, every registered motion becomes a video saved on the chosen directory to be possibly exported somewhere else.

Notes:

Currently the move detection is done in memory and by hand.

There are two clear improvements to be done. The first would be to create a user defined component and pack the move
detection code on it. This way we can use tunneling connection to get a better hardware utilization.

The second will be to use motion vectors. This is done directly in hardware and could than possibly be used together with 
tunneling connections.



Build
-----
0. The reader should have the userland-master branch download and compiled 

1. Clone this project in your home directory
   
2. Take a look on the CMakeLists.txt... 

3. cmake .

4. make

5. If everything goes fine, you should have the binaries in the project directory. Take a look at run\_motion.sh and have fun.


Short demonstration
-------------------
http://www.youtube.com/watch?v=x9Po-ugkrvw
