Simplest Raspberry Pi MMAL project

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

