// slic_helper.cpp : This file contains the helper functions for the main
// Austin Hester CS642o apr 2021
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/ximgproc/slic.hpp>

#include <iostream>
#include <map>

#include "slic_helper.hpp"

#include "affine.hpp"
#include "bitwise_porter_duff_ops.hpp"
#include "canny.hpp"
#include "equalize.hpp"
#include "mouse_callback.hpp"
#include "rectangle.hpp"
#include "region_of_interest.hpp"
#include "segmentation.hpp"

#define DEBUG 1

#if DEBUG
    #include <opencv2/highgui/highgui.hpp>
    #include <ctime>

    #include "dir_func.hpp"
    #include "string_helper.hpp"
#endif

const std::string WINDOW_NAME = "SLIC Superpixels";


// initialize input data
StyleTransferData
preprocess_slic(
    std::string input_image_filename,
    float scale_image_value,
    bool pad_input,
    std::string algorithm_string,
    int region_size,
    float ruler,
    int connectivity,
    int num_superpixels,
    int quadrant_depth
) {
    cv::Mat input_image = open_image( input_image_filename );

    // scale the input size if given 's' flag
    if (scale_image_value != 1.f) {
        input_image = resize_affine( input_image, scale_image_value );
    }

    // crop if odd resolution
    input_image = input_image(
        cv::Rect( 0, 0, input_image.cols & -2, input_image.rows & -2 )
    );
    std::cout << "Scaled Image size is:\t\t" << input_image.cols << "x" << input_image.rows << std::endl;

    // pad the input image if given flag
    if (pad_input) {
        cv::copyMakeBorder( input_image, input_image, 50, 50, 50, 50, cv::BORDER_CONSTANT, cv::Scalar(0) );
        std::cout << "Padded Image size is:\t\t" << input_image.cols << "x" << input_image.rows << std::endl;
    }

    // blur
    cv::GaussianBlur( input_image, input_image, cv::Size( 3, 3 ), 0.5f );

    // convert to CieLAB
    cv::cvtColor( input_image, input_image, cv::COLOR_BGR2Lab );

#if DEBUG > 1
    cv::imshow( WINDOW_NAME, input_image );
    // 'event loop' for keypresses
    while (wait_key());
#endif

    std::string output_window_name = WINDOW_NAME + " Output Image";

    // initialize SLICData object
    StyleTransferData image_data;
    image_data.window_name = output_window_name;
    input_image.copyTo( image_data.template_image );
    input_image.release();

    // create mask, only distance filter on foreground
    //TODO make this better at background detection, not just black backgrounds
    image_data.input_mask = make_background_mask( image_data.template_image );

    // get the algorithm parameters
    image_data.algorithm = slic_string_to_int( algorithm_string );
    image_data.region_size = region_size;
    image_data.ruler = ruler;
    image_data.connectivity = connectivity;
    image_data.num_superpixels = num_superpixels;

    // if num_superpixels provided, set the region size accordingly
    if (image_data.num_superpixels != 0) {
        image_data.region_size = static_cast<int>( std::sqrt( image_data.template_image.size().area() / num_superpixels ) );
    }

    return image_data;
}

// apply segmentation
void
process_slic(StyleTransferData* image_data)
{
    // segment the image by intensity
    superpixel_slic( image_data );
    // convert back to RGB
    cv::cvtColor( image_data->template_image, image_data->template_image, cv::COLOR_Lab2BGR );
    // zero-out region of interest
    image_data->marked_up_image = cv::Mat::zeros( image_data->template_image.size(), image_data->template_image.type() );
    // draw original map back on
    draw_on_original( image_data );
}

// apply input filters, show, save, and initialize mouse callback
void
postprocess_slic(
    StyleTransferData* image_data,
    bool blur_output,
    bool equalize_output,
    bool sharpen_output
) {
    // blur the output if given 'b' flag
    if (blur_output) {
        cv::GaussianBlur( image_data->marked_up_image, image_data->marked_up_image, cv::Size( 3, 3 ), 3.5f );
    }

    if (sharpen_output) {
        cv::Mat tmp;
        cv::GaussianBlur( image_data->marked_up_image, tmp, cv::Size( 0, 0 ), 3 );
        cv::addWeighted( image_data->marked_up_image, 1.42, tmp, -0.42, 0, image_data->marked_up_image );
        tmp.release();
    }

    // equalize the output if given 'e' flag
    if (equalize_output) {
        equalize_image( &image_data->marked_up_image );
    }

    char metadata[50];
    std::sprintf( metadata, "a_%d_s_%d_r_%.0f_c_%d.png",
        image_data->algorithm,
        image_data->region_size,
        image_data->ruler,
        image_data->connectivity
    );
    cv::imshow( image_data->window_name, image_data->marked_up_image );
    write_img_to_file( image_data->marked_up_image, "./out/slic_output", metadata );

    // initialize the mouse callback
    init_callback( image_data );
    init_callback( image_data, "SLIC Label Contours");
    init_callback( image_data, "SLIC Label Markers" );
}

// segment images into markers and contours using SLIC algorithms
void
superpixel_slic(StyleTransferData* image_data)
{
#if DEBUG
    std::clock_t clock_begin, clock_end;
    clock_begin = std::clock();
#endif

    cv::Ptr<cv::ximgproc::SuperpixelSLIC> superpixels;
    superpixels = cv::ximgproc::createSuperpixelSLIC(
        image_data->template_image,
        image_data->algorithm,
        image_data->region_size,
        image_data->ruler
    );

    // generate the segments
    superpixels.get()->iterate(10);
    // level of connectivity
    superpixels.get()->enforceLabelConnectivity( image_data->connectivity );
    // label contours
    superpixels.get()->getLabelContourMask( image_data->input_mask );
    // labels
    superpixels.get()->getLabels( image_data->markers );
    // num superpixels
    image_data->num_superpixels = superpixels.get()->getNumberOfSuperpixels();
    superpixels.release();

#if DEBUG
    clock_end = std::clock();
    std::printf( "\nSuperpixel Time Elapsed: %.0f (ms)\n", (float)( clock_end - clock_begin ) / CLOCKS_PER_SEC * 1000 );

    char metadata[50];
    std::sprintf( metadata, "a_%d_s_%d_r_%.0f_c_%d.png",
        image_data->algorithm,
        image_data->region_size,
        image_data->ruler,
        image_data->connectivity
    );

    // normalize markers for output (won't be recoverable, but looks cool)
    cv::Mat markers;
    cv::normalize( image_data->markers, markers, 0, 255, cv::NORM_MINMAX );
    markers.convertTo( markers, CV_8U );

    // save both
    cv::imshow( "SLIC Label Markers", markers );
    write_img_to_file( markers, "./out/slic_markers", metadata );
    write_img_to_file( image_data->markers, "./out/slic_markers_32S", metadata );
    markers.release();

    cv::imshow( "SLIC Label Contours", image_data->input_mask );
    write_img_to_file( image_data->input_mask, "./out/slic_contours", metadata );
#endif

#if DEBUG
    std::cout << std::endl << "num_pixels:\t" << image_data->template_image.size().area() << std::endl;
    std::cout << "num_superpixels:\t" << image_data->num_superpixels << std::endl;
    std::cout << "region_size:\t" << image_data->region_size << std::endl << std::endl;
#endif

}

// convert given slic_string type to enum int
int
slic_string_to_int(std::string algorithm_string)
{
    std::map<std::string, int> algorithm_string_map = {
        { "SLIC",  cv::ximgproc::SLIC  },
        { "SLICO", cv::ximgproc::SLICO },
        { "MSLIC", cv::ximgproc::MSLIC }
    };
    int algorithm = algorithm_string_map[algorithm_string];
    // assertion to prevent them from typing wrong and getting translation
    assert(algorithm > 99 && algorithm < 103);
    return algorithm;
}


// select a region. called from mouse listener
void
select_region(StyleTransferData* image_data, int marker_value)
{

    // zero-out region of interest
    image_data->marked_up_image = cv::Mat::zeros( image_data->template_image.size(), image_data->template_image.type() );

    // draw original map back on
    draw_on_original( image_data, marker_value );

    // highlight selected region
    // draw_in_roi( image_data, marker_value );

    // get bounding rect
    // cv::Rect bounding_rect = image_data->boundaries[marker_value - 1];

    // extract the ROI
    // cv::Mat region_only = extract_selected_region( image_data, marker_value );

    // double the size
    // region_only = resize_affine( region_only, 2.f );

    // copy the region to image_data
    // region_only.copyTo( image_data->region_of_interest );
    // region_only.release();

    // double size of rect of roi
    // bounding_rect = center_and_double_rect( bounding_rect, image_data->marked_up_image.size() );

}

// paint real map atop the region of interest's mask
cv::Mat
paint_map_atop_region(StyleTransferData* image_data, int marker_value, cv::Mat drawn_contour)
{
    // create single channel mask
    cv::Mat map_mask_8u;
    image_data->input_mask.convertTo( map_mask_8u, CV_8U );

    // create 3 channel contour
    cv::Mat contour_8u3;
    drawn_contour.convertTo( contour_8u3, CV_8UC3 );
    cv::cvtColor( contour_8u3, contour_8u3, cv::COLOR_GRAY2BGR );

    // paint region using porter duff
    cv::Mat painted_region;
    try {
        painted_region = bitwise_i1_atop_i2(
            image_data->template_image,
            contour_8u3,
            map_mask_8u,
            drawn_contour
        );
    } catch (std::string &str) {
        std::cerr << "ERROR : paint_map_atop_region" << std::endl;
        std::cerr << "Error: " << str << std::endl;
    } catch (cv::Exception &e) {
        std::cerr << "ERROR : paint_map_atop_region" << std::endl;
        std::cerr << "Error: " << e.msg << std::endl;
    } catch (std::runtime_error &re) {
        std::cerr << "ERROR : paint_map_atop_region" << std::endl;
        std::cerr << "Error: " << re.what() << std::endl;
    }

    if (painted_region.empty()) {
        image_data->template_image.copyTo( painted_region );
    }

    map_mask_8u.release();
    contour_8u3.release();
    return painted_region;
}

// draw original states back onto marked_up_image
void
draw_on_original(StyleTransferData* image_data, int marker_value)
{
    // create single channel mask
    cv::Mat mask_8u;
    image_data->input_mask.convertTo( mask_8u, CV_8U );

    // fill in states
    for (int i = 0; i < image_data->markers.rows; i++)
    {
        for (int j = 0; j < image_data->markers.cols; j++)
        {
            // skip if not in mask (draw borders)
            if (mask_8u.at<uchar>( i, j ) != (uchar) 0) {
                continue;
            }
            int pixel = image_data->markers.at<int>( i, j );
            if (marker_value > -1 && pixel != marker_value) {
                continue;
            }
            if (pixel >= 0 && pixel <= static_cast<int>(image_data->num_superpixels)) {
                image_data->marked_up_image.at<cv::Vec3b>( i, j ) = image_data->template_image.at<cv::Vec3b>( i, j );
            }
        }
    }
    mask_8u.release();
}
