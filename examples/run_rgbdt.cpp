// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2015 Intel Corporation. All Rights Reserved.

///////////////////////////////////////////////////////////
// librealsense tutorial #2 - Accessing multiple streams //
///////////////////////////////////////////////////////////

// First include the librealsense C++ header file
#include <librealsense/rs.hpp>
#include <cstdio>
#include <sys/time.h>

// Also include GLFW to allow for graphical display
#include <GLFW/glfw3.h>
#include <opencv2/opencv.hpp>
#include <fstream>

using namespace cv;
int main() try
{
    struct timeval tp;
    gettimeofday(&tp, NULL);
	long unsigned int ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;

    char com[512];
    printf(com);

    printf("mkdir %u", ms);
    sprintf(com, "mkdir %u", ms);
    system(com);

    cv::VideoCapture * cap0 = new cv::VideoCapture(1); // open the default camera
    cv::VideoCapture * cap1 = new cv::VideoCapture(2); // open the default camera

    cv::Mat im0;
    cv::Mat im1;

    // Create a context object. This object owns the handles to all connected realsense devices.
    rs::context ctx;
    printf("There are %d connected RealSense devices.\n", ctx.get_device_count());
    if(ctx.get_device_count() == 0) return EXIT_FAILURE;

    // This tutorial will access only a single device, but it is trivial to extend to multiple devices
    rs::device * dev = ctx.get_device(0);
    printf("\nUsing device 0, an %s\n", dev->get_name());
    printf("    Serial number: %s\n", dev->get_serial());
    printf("    Firmware version: %s\n", dev->get_firmware_version());

    // Configure all streams to run at VGA resolution at 60 frames per second
    dev->enable_stream(rs::stream::depth, 640, 480, rs::format::z16, 60);
    dev->enable_stream(rs::stream::color, 640, 480, rs::format::rgb8, 60);
    dev->enable_stream(rs::stream::infrared, 640, 480, rs::format::y8, 60);
    try { dev->enable_stream(rs::stream::infrared2, 640, 480, rs::format::y8, 60); }
    catch(...) { printf("Device does not provide infrared2 stream.\n"); }
    dev->start();


    cv::namedWindow("rgb");
    cv::namedWindow("depth");
    cv::namedWindow("rgb_t");
    cv::namedWindow("thermal");
    printf("Camera intrinsics: fx, fy, cx, cy\n");
    auto intrin = dev->get_stream_intrinsics(rs::stream::color);
    printf("%d %d %f %f %f %f\n",intrin.width,intrin.height,intrin.fx,intrin.fy,intrin.ppx,intrin.ppy);
    for(int i=0; i < 5; i++)
        printf("%f ",intrin.coeffs[i]);
    printf("Distortion model: %d\n",intrin.model());

    int fid = 0;
    while(1)
    {
        fid++;
        // Wait for new frame data
        glfwPollEvents();
        dev->wait_for_frames();
        (*cap0) >> im0;
        (*cap1) >> im1;
        cv::Mat rgb = cv::Mat(480, 640, CV_8UC3, (char*)dev->get_frame_data(rs::stream::color));
        cv::Mat aligned_depth = cv::Mat(480, 640, CV_16UC1, (char*)dev->get_frame_data(rs::stream::depth_aligned_to_color));
        cv::Mat depth = cv::Mat(480, 640, CV_16UC1, (char*)dev->get_frame_data(rs::stream::depth));

        sprintf(com, "%u/rgb_%u.png", ms, fid);
        imwrite(com, rgb);
        sprintf(com, "%u/depth_%u.png", ms, fid);
        imwrite(com, depth);
        sprintf(com, "%u/aligned_depth_%u.png", ms, fid);
        imwrite(com, aligned_depth);
        sprintf(com, "%u/thermal_it_%u.png", ms, fid);
        imwrite(com, im0);
        sprintf(com, "%u/thermal_rgb_%u.png", ms, fid);
        imwrite(com, im1);
        sprintf(com, "%u/bin_aligned_depth_%u.bin", ms, fid);
        std::ofstream myFile (com, std::ios::out | std::ios::binary);
        myFile.write( (char*) dev->get_frame_data(rs::stream::depth_aligned_to_color), 480*640*2);
        myFile.close();

        // Comment out this block while collecting data
        imshow("rgb", rgb);
        imshow("depth", aligned_depth);
        imshow("thermal", im0);
        imshow("rgb_t", im1);
        waitKey(25);

    }

    return EXIT_SUCCESS;
}
catch(const rs::error & e)
{
    // Method calls against librealsense objects may throw exceptions of type rs::error
    printf("rs::error was thrown when calling %s(%s):\n", e.get_failed_function().c_str(), e.get_failed_args().c_str());
    printf("    %s\n", e.what());
    return EXIT_FAILURE;
}