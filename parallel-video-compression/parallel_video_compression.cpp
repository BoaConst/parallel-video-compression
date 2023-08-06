#include <string>
#include <mpi.h>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if(argc > 1) {
        cout<<"User has chosen to analyze: [" << argv[1] <<"] video file"<<endl;
    } else {
        cout << "User didn't provide input video file" << endl;
        return 1; // Exit the program with the error code 1 indicating that video file type was not provided
    }

    string video_file_type = argv[1];
    string input_video_file_location = "./input/"+video_file_type+".mp4";

    VideoCapture cap(input_video_file_location);
    if (!cap.isOpened()) {
        cout << "Err: Could not open video file" << endl;
        return 2; // Exit the program with the error code 2 indicating that the video file could not be opened
    }

    double fps = cap.get(CAP_PROP_FPS);
    int width = int(cap.get(CAP_PROP_FRAME_WIDTH));
    int height = int(cap.get(CAP_PROP_FRAME_HEIGHT));
    int total_frames = int(cap.get(CAP_PROP_FRAME_COUNT));

    int frames_per_video = total_frames / size;
    
    // Defining the start and end frame for each video file
    int start_frame = rank * frames_per_video;
    int end_frame = (rank + 1) * frames_per_video;

    string output_video_file_location = "./output/parallel_output_video_"+to_string(rank+1)+"_"+video_file_type+".mp4";

    // Compressing to the o/p format avc1
    VideoWriter out = VideoWriter(output_video_file_location, VideoWriter::fourcc('a', 'v', 'c', '1'), fps, Size(width, height), true);
    printf("Video Properties: fps = %f, width = %i, height = %i, total_frames = %i \n", fps, width, height, total_frames);

    cout << "Starting Parallel Video Compression for rank: " <<rank<< endl;

    // start timer
    double start_time = MPI_Wtime(); 

    cap.set(CAP_PROP_POS_FRAMES, start_frame);
    Mat frame;
    for (int i = start_frame; i < end_frame; i++) {
        cap.read(frame);
        if (frame.empty()) break;
        out.write(frame);
    }

    // stop timer
    double end_time = MPI_Wtime(); 

    double elapsed_time = end_time - start_time;
    printf("Process %d: Time elapsed = %f seconds\n", rank, elapsed_time);

    // Release the video input and output objects
    cap.release();
    out.release();

    // Wait for all processes to end
    MPI_Finalize();
    return 0;
}
