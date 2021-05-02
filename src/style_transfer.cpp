// style_transfer.cpp : This file contains the 'main' function. Program execution begins and ends there.
// Austin Hester CS642o apr 2021
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <vector>

#include "cla_parse.hpp"
#include "style_helper.hpp"

#define DEBUG 1


const std::string WINDOW_NAME = "Style Transfer";


// event loop
// call in a while loop to only register q or <esc>
int
wait_key()
{
    char key_pressed = cv::waitKey(0) & 255;
    // 'q' or  <escape> quits out
    if (key_pressed == 27 || key_pressed == 'q') {
        return 0;
    }
    return 1;
}


int
main(int argc, const char** argv)
{
    // CLA variables
    std::string template_image_filename;
    std::string target_image_filename;
    int region_size = 10;
    float ruler = 10.f;
    std::string algorithm_string = "SLIC";
    int connectivity = 25;
    int quadrant_depth = 1;

    // CLA flags
    float scale_image_value = 1.f;
    bool blur_output = false;
    bool equalize_output = false;
    bool sharpen_output = false;
    bool pad_input = false;

    // parse and save command line args
    int parse_result = parse_arguments(
        argc, argv,
        &template_image_filename,
        &target_image_filename,
        &scale_image_value,
        &blur_output,
        &equalize_output,
        &sharpen_output,
        &region_size,
        &ruler,
        &algorithm_string,
        &connectivity
    );
    if (parse_result != 1) return parse_result;

#if DEBUG
    std::clock_t clock_begin;
    std::clock_t clock_end;
    clock_begin = std::clock();
    // begin clocking pre-process source
#endif

    // setup images, quadrants, etc.
    StyleTransferData style_data = preprocess_style_data(
        template_image_filename,
        target_image_filename,
        scale_image_value,
        pad_input,
        algorithm_string,
        region_size,
        ruler,
        connectivity,
        quadrant_depth
    );

#if DEBUG
    clock_end = std::clock();
    std::printf( "Pre-Process Time Elapsed: %.0f (ms)\n", (float)( clock_end - clock_begin ) / CLOCKS_PER_SEC * 1000 );
    clock_begin = std::clock();
    // begin clocking process
#endif

    // actually transfer style
    process_style_data( &style_data );

#if DEBUG
    clock_end = std::clock();
    std::printf( "Process Time Elapsed: %.0f (ms)\n", (float)( clock_end - clock_begin ) / CLOCKS_PER_SEC * 1000 );
    clock_begin = std::clock();
    // begin clocking post-process
#endif

    // post-process (display) style data
    postprocess_style_data(
        &style_data,
        blur_output,
        equalize_output,
        sharpen_output
    );

#if DEBUG
    clock_end = std::clock();
    std::printf( "Post-Process Time Elapsed: %.0f (ms)\n", (float)( clock_end - clock_begin ) / CLOCKS_PER_SEC * 1000 );
    clock_begin = std::clock();
#endif

    // 'event loop' for keypresses
    while (wait_key());

    cv::destroyAllWindows();

    style_data.template_image.release();
    style_data.target_image.release();
    style_data.input_mask.release();
    style_data.region_of_interest.release();
    style_data.marked_up_image.release();
    style_data.markers.release();

    return 0;
}
