CC1=g++ -I/usr/include/opencv4 
CC2=mpic++ -I/usr/include/opencv4 
CC3=g++ -I/usr/include/opencv4
CC4=g++ -fopenmp -I/usr/include/opencv4

SRC1=./simple-video-compression/simple_video_compression.cpp
SRC2=./parallel-video-compression/parallel_video_compression.cpp
SRC3=./vanilla-video-compression/vanilla_video_compression.cpp
SRC4=./distributed-video-compression/distributed_video_compression.cpp

all: simple parallel vanilla distributed

simple: $(SRC1)
	$(CC1) -o $@ $^ `pkg-config --libs opencv`

parallel: $(SRC2)
	$(CC2) -o $@ $^ `pkg-config --libs opencv`

vanilla: $(SRC3)
	$(CC3) -o $@ $^ `pkg-config --libs opencv`

distributed: $(SRC4)
	$(CC4) -o $@ $^ `pkg-config --libs opencv`