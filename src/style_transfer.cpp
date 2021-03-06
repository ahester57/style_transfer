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
    if ( key_pressed == 27 || key_pressed == 'q' ) {
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
    float scale_image_value = 1.f;
    int quadrant_depth = 2;
    int transfer_mode = 0;
    float w_hue = 0.8;
    float w_sat = 0.9;
    float w_val = 0.15;

    // CLA flags
    bool blur_output = false;
    bool equalize_output = false;
    bool sharpen_output = false;

#if DEBUG
    std::clock_t clock_begin;
    std::clock_t clock_end;
    clock_begin = std::clock();
    // begin clocking pre-process source
#endif

    // parse and save command line args
    int parse_result = parse_arguments(
        argc, argv,
        &template_image_filename,
        &target_image_filename,
        &scale_image_value,
        &quadrant_depth,
        &transfer_mode,
        &w_hue,
        &w_sat,
        &w_val,
        &blur_output,
        &equalize_output,
        &sharpen_output
    );
    if ( parse_result != 1 ) return parse_result;

    // setup images, quadrants, etc.
    StyleTransferData style_data = preprocess_style_data(
        template_image_filename,
        target_image_filename,
        scale_image_value,
        quadrant_depth,
        transfer_mode,
        w_hue,
        w_sat,
        w_val
    );

#if DEBUG
    clock_end = std::clock();
    std::printf( "\nPre-Process Time Elapsed: %.0f (ms)\n", (float)( clock_end - clock_begin ) / CLOCKS_PER_SEC * 1000 );
    clock_begin = std::clock();
    // begin clocking process
#endif

    // 'actually' transfer style
    process_style_data(
        &style_data
    );

#if DEBUG
    clock_end = std::clock();
    std::printf( "\nProcess Time Elapsed: %.0f (ms)\n", (float)( clock_end - clock_begin ) / CLOCKS_PER_SEC * 1000 );
    clock_begin = std::clock();
    // begin clocking post-process
#endif

    // post-process (display) style data
    postprocess_style_data(
        &style_data,
        "TODO",//TODO combine image names without / or .
        blur_output,
        equalize_output,
        sharpen_output
    );

#if DEBUG
    clock_end = std::clock();
    std::printf( "\nPost-Process Time Elapsed: %.0f (ms)\n", (float)( clock_end - clock_begin ) / CLOCKS_PER_SEC * 1000 );
    clock_begin = std::clock();
#endif

    // 'event loop' for keypresses
    while ( wait_key() );

    cv::destroyAllWindows();

    style_data.template_image.release();
    style_data.target_image.release();
    style_data.marked_up_image.release();

    return 0;
}
