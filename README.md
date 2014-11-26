Simplest Raspberry Pi MMAL project

Projects
--------

test\_preview: the simplest variant to get the raspicam working.

test\_preview\_no\_tunneling: the worst problem so far is to understand how the connections and the callbacks work.

This source changes the test\_preview code, taking out the tunneling flag on the connections and passing the buffers manually.

An issue I am currently facing is that I need to set some sleeping time, so the queues don't get stucked.

comming up:

test\_encoding: A simple version of encoding the output as h264 and saving to a file

test\_encode\_on\_move

Build
-----
0. The reader should have the userland-master branch download and compiled 

1. Clone this project in your home directory
   
2. Take a look on the CMakeLists.txt... 

3. cmake .

4. make

5. If everything goes fine, you have the binary "test\_preview" and "test\_preview\_no\_tunneling in the project directory
