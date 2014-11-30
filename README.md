Simplest Raspberry Pi MMAL project

Projects
--------

test\_preview: the simplest variant to get the raspicam working.

test\_preview\_no\_tunneling: the worst problem so far is to understand how the connections and the callbacks work.

This source changes the test\_preview code, taking out the tunneling flag on the connections and passing the buffers manually.

test\_encoding: A simple version of encoding the output as h264 and saving to a file

test\_encode\_on\_move: A simple version of movement detection.

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

5. If everything goes fine, you have the binary "test\_preview" and "test\_preview\_no\_tunneling in the project directory
