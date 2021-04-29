// style_transfer.cpp : This file contains the 'main' function. Program execution begins and ends there.
// Austin Hester CS642o apr 2021
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <vector>

#include "affine.hpp"
#include "canny.hpp"
#include "cla_parse.hpp"
#include "dir_func.hpp"
#include "hsv_convert.hpp"
#include "rectangle.hpp"
#include "segmentation.hpp"
#include "slic_helper.hpp"

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

void
transfer_style(SLICData* src, SLICData* dst)
{
#if DEBUG
    std::clock_t clock_begin;
    std::clock_t clock_end;
    clock_begin = std::clock();
    // begin clocking split and normalize
#endif
    // blur a lot
    cv::Mat src_hsv;
    cv::Mat dst_hsv;
    cv::GaussianBlur( src->input_image, src_hsv, cv::Size( 3, 3 ), 0.5f );
    cv::GaussianBlur( dst->input_image, dst_hsv, cv::Size( 3, 3 ), 0.5f );

    // use hsv
    bgr_to_hsv( src_hsv, &src_hsv );
    bgr_to_hsv( dst_hsv, &dst_hsv );

    // split hsv into planes hue, saturation, vintensity
    std::vector<cv::Mat> src_planes;
    cv::split( src_hsv, src_planes );
    src_hsv.release();

    std::vector<cv::Mat> dst_planes;
    cv::split( dst_hsv, dst_planes );
    dst_hsv.release();

    // normalize markers_32S of src to dst->num_superpixels
    cv::Mat normal_src_markers;
    cv::normalize( src->markers, normal_src_markers, 0, dst->num_superpixels, cv::NORM_MINMAX );

    cv::Mat previous_src_mask;
    cv::Mat src_marker_mask;
    cv::Mat dst_marker_mask;
    // loop thru each superpixel
    for (size_t i = 0; i < dst->num_superpixels; i++) {
        int marker_value = static_cast<int>( i );
        // find the mask for given superpixel
        src_marker_mask = make_background_mask( normal_src_markers, marker_value );
        dst_marker_mask = make_background_mask( dst->markers, marker_value );

        // if blank src mask, use previous
        std::vector<cv::Point> nonzeropixels;
        cv::findNonZero( src_marker_mask, nonzeropixels );
        if (nonzeropixels.size() == 0) {
            previous_src_mask.copyTo( src_marker_mask );
        } else {
            src_marker_mask.copyTo( previous_src_mask );
        }

#if DEBUG > 1
        cv::imshow("src_marker_mask", src_marker_mask);
        cv::imshow("dst_marker_mask", dst_marker_mask);
        cv::waitKey(1);
#endif
        // next, average hue, saturation of src
        cv::Scalar src_mean_hue = cv::mean( src_planes[0], src_marker_mask );
        cv::Scalar src_mean_sat = cv::mean( src_planes[1], src_marker_mask );
        // then average vintensity of both src and dst
        cv::Scalar src_mean_val = cv::mean( src_planes[2], src_marker_mask );
        cv::Scalar dst_mean_val = cv::mean( dst_planes[2], dst_marker_mask );

        // copy hue and saturation from src to dst
        dst_planes[0].setTo( src_mean_hue, dst_marker_mask );
        dst_planes[1].setTo( src_mean_sat, dst_marker_mask );
        // average vintensity of both weighing dst
        dst_planes[2].setTo( (src_mean_val.val[0] + dst_mean_val.val[0] * 5 ) / 6 );

    }
    normal_src_markers.release();
    src_marker_mask.release();
    dst_marker_mask.release();
    previous_src_mask.release();
    for (cv::Mat &img : src_planes) {
        img.release();
    }

    // merge dst_planes back to hsv image
    cv::merge( dst_planes, dst_hsv );
    for (cv::Mat &img : dst_planes) {
        img.release();
    }

    // and convert to bgr
    hsv_to_bgr( dst_hsv, &dst->marked_up_image );
    dst_hsv.release();

#if DEBUG
    clock_end = std::clock();
    std::printf( "Merge Time Elapsed: %.0f (ms)\n", (float)( clock_end - clock_begin ) / CLOCKS_PER_SEC * 1000 );
#endif
}


std::vector<SLICData>
initialize_superimposed_images(
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

    std::vector<SLICData> style_data = initialize_superimposed_images(
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

    transfer_style( &style_data[0], &style_data[1] );

    // post-process target slic data
    postprocess_slic(
        &style_data[1],
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
