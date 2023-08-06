#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

Mat dct(Mat block) {
  Mat dct_block = Mat::zeros(block.size(), block.type());

  // c_u and c_v are scaling coefficients that depend on the values of u and v, which are the indices of the frequency coefficients in the 2D DCT matrix.
  float c_u, c_v, sum;
  
  for (int u = 0; u < block.rows; ++u) {
    for (int v = 0; v < block.cols; ++v) {
      sum = 0.0f;
      for (int i = 0; i < block.rows; ++i) {
        for (int j = 0; j < block.cols; ++j) {
          // Normalizing the coefficients
          c_u = (u == 0) ? sqrt(1.0 / block.rows) : sqrt(2.0 / block.rows);
          c_v = (v == 0) ? sqrt(1.0 / block.cols) : sqrt(2.0 / block.cols);
          sum += c_u * c_v * block.at<float>(i, j)* cos((2 * i + 1) * u * M_PI / (2 * block.rows))* cos((2 * j + 1) * v * M_PI / (2 * block.cols));
        }
      }
      dct_block.at<float>(u, v) = sum;
    }
  }

  return dct_block;
}

Mat idct(Mat dct_block) {
  Mat block = Mat::zeros(dct_block.size(), dct_block.type());

  double c_u, c_v, sum;
  // Loop interchange to do inverse DCT
  for (int i = 0; i < dct_block.rows; ++i) {
    for (int j = 0; j < dct_block.cols; ++j) {
      sum = 0.0f;
      for (int u = 0; u < dct_block.rows; ++u) {
        for (int v = 0; v < dct_block.cols; ++v) {
          c_u = (u == 0) ? sqrt(1.0 / dct_block.rows) : sqrt(2.0 / dct_block.rows);
          c_v = (v == 0) ? sqrt(1.0 / dct_block.cols) : sqrt(2.0 / dct_block.cols);
          sum += c_u * c_v * dct_block.at<float>(u, v)* cos((2 * i + 1) * u * M_PI / (2 * dct_block.rows))* cos((2 * j + 1) * v * M_PI / (2 * dct_block.cols));
        }
      }
      block.at<float>(i, j) = sum;
    }
  }

  return block;
}

int main(int argc, char* argv[]) {
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

    string output_video_file_location = "./output/vanilla_output_video_"+video_file_type+".mp4";

    // Using the o/p format as mp4v (same as input) so that no compression is done by OpenCV 
    VideoWriter out = VideoWriter(output_video_file_location, VideoWriter::fourcc('m', 'p', '4', 'v'), fps, Size(width, height), true);

    // Quantization Matrix Sourced from https://www.math.cuhk.edu.hk/~lmlui/dct.pdf under the section Quantization of the pdf. 
    Mat quantization_matrix = (Mat_<float>(8, 8) << 16, 11, 10, 16, 24, 40, 51, 61,
                                                     12, 12, 14, 19, 26, 58, 60, 55,
                                                     14, 13, 16, 24, 40, 57, 69, 56,
                                                     14, 17, 22, 29, 51, 87, 80, 62,
                                                     18, 22, 37, 56, 68, 109, 103, 77,
                                                     24, 35, 55, 64, 81, 104, 113, 92,
                                                     49, 64, 78, 87, 103, 121, 120, 101,
                                                     72, 92, 95, 98, 112, 100, 103, 99);

    // Start the timer
    timespec vid_cmp_start, vid_cmp_stop;
    clock_gettime(CLOCK_REALTIME, &vid_cmp_start);

    cout << "Starting Vanilla Video Compression" << endl;
    printf("Video Properties: fps = %f, width = %i, height = %i, total_frames = %i \n", fps, width, height, total_frames);

    Mat frame;
    for (int i = 0; i < total_frames; i++) {
        cap.read(frame);
        if (frame.empty()) break;

         // Convert the input frame to grayscale
        Mat gray;
        cvtColor(frame, gray, COLOR_BGR2GRAY);

        // Block size (8x8)
        int block_size = 8;

        /* I tried an implementation where I was DCTing->quantizing->dequantizing->inverseDCTing the frame in a single loop, but the performance was not optimal so 
        I broke this into 2 separate loops
        */

        // To store dct_blocks of the input frame
        Mat dct_blocks = Mat::zeros(gray.size(), CV_32FC1);

        for (int j = 0; j < gray.rows; j += block_size) {
            for (int k = 0; k < gray.cols; k += block_size) {
                // Extract the 8x8 block
                Mat region = gray(Rect(k, j, block_size, block_size));
                Mat block;
                region.convertTo(block, CV_32FC1);

                // Apply DCT to the block
                Mat dct_block;
                dct(block, dct_block);

                // Apply Quantization
                dct_block /= quantization_matrix;

                // Store the transformed block in the dct_blocks matrix
                dct_block.copyTo(dct_blocks(Rect(k, j, block_size, block_size)));
            }
        }

        // Convert the transformed blocks back to the original image
        Mat idct_frame = Mat::zeros(gray.size(), CV_32FC1);

        for (int j = 0; j < gray.rows; j += block_size) {
            for (int k = 0; k < gray.cols; k += block_size) {
                // Extract the transformed block
                Mat dct_block = dct_blocks(Rect(k, j, block_size, block_size));

                // Apply Inverse Quantization
                dct_block *= quantization_matrix;

                // Apply inverse DCT to the block
                Mat block;
                idct(dct_block, block);

                // Copy the reconstructed block to the output image
                block.copyTo(idct_frame(Rect(k, j, block_size, block_size)));
            }
        }

        Mat output;
        idct_frame.convertTo(output, CV_8UC1);

        Mat result;
        cvtColor(output, result, COLOR_GRAY2BGR);
      
        out.write(result);
    }

    // Stop the timer
    clock_gettime(CLOCK_REALTIME, &vid_cmp_stop);
    double elapsed_time = ((vid_cmp_stop.tv_sec - vid_cmp_start.tv_sec) + (vid_cmp_stop.tv_nsec - vid_cmp_start.tv_nsec) / 1E9);
    printf("Time elapsed = %f seconds\n", elapsed_time);

    // Release the video input and output objects
    cap.release();
    out.release();

    return 0;
}
