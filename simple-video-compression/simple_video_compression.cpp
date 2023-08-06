#include <opencv2/opencv.hpp>
#include <time.h>

using namespace cv;
using namespace std;

int main(int argc, char** argv) {
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

    string output_video_file_location = "./output/simple_output_video_"+video_file_type+".mp4";

    // Compressing to the o/p format avc1
    VideoWriter out = VideoWriter(output_video_file_location, VideoWriter::fourcc('a', 'v', 'c', '1'), fps, Size(width, height), true);
    printf("Video Properties: fps = %f, width = %i, height = %i, total_frames = %i \n", fps, width, height, total_frames);

    cout << "Starting Simple Video Compression" << endl;

    timespec vid_cmp_start, vid_cmp_stop;
    clock_gettime(CLOCK_REALTIME, &vid_cmp_start);

    Mat frame;
    for (int i = 0; i < total_frames; i++) {
        cap.read(frame);
        if (frame.empty()) break;
        out.write(frame);
    }

    clock_gettime(CLOCK_REALTIME, &vid_cmp_stop);
    double elapsed_time = ((vid_cmp_stop.tv_sec - vid_cmp_start.tv_sec) + (vid_cmp_stop.tv_nsec - vid_cmp_start.tv_nsec) / 1E9);
    printf("Time elapsed = %f seconds\n", elapsed_time);

    cap.release();
    out.release();
    return 0;
}
