// style_transfer.cpp : This file contains the 'main' function. Program execution begins and ends there.
// Austin Hester CS642o apr 2021
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <vector>

#include "../include/affine.hpp"
#include "../include/canny.hpp"
#include "../include/cla_parse.hpp"
#include "../include/dir_func.hpp"
#include "../include/rectangle.hpp"
#include "../include/segmentation.hpp"
#include "../include/slic_helper.hpp"

#define DEBUG 1


const std::string WINDOW_NAME = "SLIC Superpixels";


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


std::vector<SLICData>
transfer_style(
    std::string template_image_filename,
    std::string target_image_filename,
    float scale_image_value,
    bool pad_input,
    std::string algorithm_string,
    int region_size,
    float ruler,
    int connectivity,
    bool blur_output,
    bool equalize_output,
    bool sharpen_output
) {
    // open the source image with given options
    SLICData source_data = preprocess_slic(
        template_image_filename,
        scale_image_value,
        pad_input,
        algorithm_string,
        region_size,
        ruler,
        connectivity
    );

    // apply segmentation to source
    process_slic( &source_data );


    // open the target image with given options
    SLICData target_data = preprocess_slic(
        target_image_filename,
        scale_image_value,
        pad_input,
        algorithm_string,
        region_size,
        ruler,
        connectivity,
        source_data.num_superpixels
    );

    // apply segmentation to target with same number of superpixels
    process_slic( &target_data );

    // post-process slic data
    postprocess_slic(
        &target_data,
        blur_output,
        equalize_output,
        sharpen_output
    );

    std::vector<SLICData> data;
    data.push_back( source_data );
    data.push_back( target_data );
    return data;
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

    std::vector<SLICData> style_data = transfer_style(
        template_image_filename,
        target_image_filename,
        scale_image_value,
        pad_input,
        algorithm_string,
        region_size,
        ruler,
        connectivity,
        blur_output,
        equalize_output,
        sharpen_output
    );

    // 'event loop' for keypresses
    while (wait_key());

    cv::destroyAllWindows();

    for (SLICData &data : style_data) {
        data.input_image.release();
        data.markers.release();
        data.input_mask.release();
        data.region_of_interest.release();
        data.marked_up_image.release();
    }

    return 0;
}
