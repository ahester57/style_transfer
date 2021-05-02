// style_helper.cpp : This file contains the helper functions for the main
// Austin Hester CS642o may 2021
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/ximgproc/slic.hpp>

#include <iostream>
#include <map>

#include "style_helper.hpp"

#include "affine.hpp"
#include "dir_func.hpp"
#include "equalize.hpp"
#include "mouse_callback.hpp"
#include "quadrant.hpp"
#include "segmentation.hpp"

#define DEBUG 1

#if DEBUG
    #include <opencv2/highgui/highgui.hpp>
    #include <ctime>

    #include "string_helper.hpp"
#endif

const std::string WINDOW_NAME = "Style Transfer";


// initialize input data
StyleTransferData
preprocess_style_data(
    std::string template_filename,
    std::string target_filename,
    float scale_image_value,
    bool pad_input,
    std::string algorithm_string,
    int region_size,
    float ruler,
    int connectivity,
    int num_superpixels,
    int quadrant_depth
) {
    cv::Mat template_image = open_image( template_filename );
    cv::Mat target_image = open_image( target_filename );

    // scale the input size if given 's' flag
    if (scale_image_value != 1.f) {
        template_image = resize_affine( template_image, scale_image_value );
    }

    // crop if odd resolution
    template_image = template_image(
        cv::Rect( 0, 0, template_image.cols & -2, template_image.rows & -2 )
    );
    std::cout << "Scaled Image size is:\t\t" << template_image.cols << "x" << template_image.rows << std::endl;

    // pad the input image if given flag
    if (pad_input) {
        cv::copyMakeBorder( template_image, template_image, 50, 50, 50, 50, cv::BORDER_CONSTANT, cv::Scalar(0) );
        std::cout << "Padded Image size is:\t\t" << template_image.cols << "x" << template_image.rows << std::endl;
    }

    // blur
    cv::GaussianBlur( template_image, template_image, cv::Size( 3, 3 ), 0.5f );

    // convert to CieLAB
    cv::cvtColor( template_image, template_image, cv::COLOR_BGR2HSV );

#if DEBUG > 1
    cv::imshow( WINDOW_NAME, input_image );
    // 'event loop' for keypresses
    while (wait_key());
#endif

    std::string output_window_name = WINDOW_NAME + " Output Image";

    // initialize StyleTransferData object
    StyleTransferData style_data;
    style_data.window_name = output_window_name;
    template_image.copyTo( style_data.template_image );
    template_image.release();

    // create mask, only distance filter on foreground
    //TODO make this better at background detection, not just black backgrounds
    style_data.input_mask = make_background_mask( style_data.template_image );

    // get the algorithm parameters
    // image_data.algorithm = slic_string_to_int( algorithm_string );
    style_data.region_size = region_size;
    style_data.ruler = ruler;
    style_data.connectivity = connectivity;
    style_data.num_superpixels = num_superpixels;

    // if num_superpixels provided, set the region size accordingly
    if (style_data.num_superpixels != 0) {
        style_data.region_size = static_cast<int>( std::sqrt( style_data.template_image.size().area() / num_superpixels ) );
    }

    return style_data;
}

// apply output filters, show, save, and initialize mouse callback
void
postprocess_style_data(
    StyleTransferData* style_data,
    bool blur_output,
    bool equalize_output,
    bool sharpen_output
) {
    // blur the output if given 'b' flag
    if (blur_output) {
        cv::GaussianBlur( style_data->marked_up_image, style_data->marked_up_image, cv::Size( 3, 3 ), 3.5f );
    }

    if (sharpen_output) {
        cv::Mat tmp;
        cv::GaussianBlur( style_data->marked_up_image, tmp, cv::Size( 0, 0 ), 3 );
        cv::addWeighted( style_data->marked_up_image, 1.42, tmp, -0.42, 0, style_data->marked_up_image );
        tmp.release();
    }

    // equalize the output if given 'e' flag
    if (equalize_output) {
        equalize_image( &style_data->marked_up_image );
    }

    char metadata[50];
    std::sprintf( metadata, "a_%d_s_%d_r_%.0f_c_%d.png",
        style_data->algorithm,
        style_data->region_size,
        style_data->ruler,
        style_data->connectivity
    );
    cv::imshow( style_data->window_name, style_data->marked_up_image );
    write_img_to_file( style_data->marked_up_image, "./out/style_output", metadata );

    // initialize the mouse callback
    init_callback( style_data );
}
