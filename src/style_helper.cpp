// style_helper.cpp : This file contains the helper functions for the main
// Austin Hester CS642o may 2021
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/ximgproc/slic.hpp>

#include <iostream>

#include "style_helper.hpp"

#include "affine.hpp"
#include "dir_func.hpp"
#include "equalize.hpp"
#include "hsv_convert.hpp"
#include "mouse_callback.hpp"
#include "quadrant.hpp"

#define DEBUG 1

#if DEBUG
    #include <opencv2/highgui/highgui.hpp>
    #include <ctime>
#endif

const std::string WINDOW_NAME = "Style Transfer";


// initialize input data
StyleTransferData
preprocess_style_data(
    std::string template_filename,
    std::string target_filename,
    float scale_image_value,
    int quadrant_depth
) {
    // open images by filename
    cv::Mat template_image = open_image( template_filename );
    cv::Mat target_image = open_image( target_filename );

    // scale the input size if given 's' flag
    if ( scale_image_value != 1.f ) {
        template_image = resize_affine( template_image, scale_image_value );
        target_image = resize_affine( target_image, scale_image_value );
    }

    // crop if odd resolution
    template_image = template_image(
        cv::Rect( 0, 0, template_image.cols & -2, template_image.rows & -2 )
    );
    target_image = target_image(
        cv::Rect( 0, 0, target_image.cols & -2, target_image.rows & -2 )
    );

    std::cout << "Scaled Template Image size is:\t\t" << template_image.cols << "x" << template_image.rows << std::endl;
    std::cout << "Scaled Target Image size is:\t\t" << target_image.cols << "x" << target_image.rows << std::endl;

    // blur
    cv::GaussianBlur( template_image, template_image, cv::Size( 3, 3 ), 0.5f );
    cv::GaussianBlur( target_image, target_image, cv::Size( 3, 3 ), 0.5f );

    // convert to HSV
    bgr_to_hsv( template_image, &template_image );
    bgr_to_hsv( target_image, &target_image );

#if DEBUG > 1
    cv::imshow( WINDOW_NAME + " Template Image", template_image );
    cv::imshow( WINDOW_NAME + " Target Image", target_image );
    cv::waitKey(0);
#endif

    std::string output_window_name = WINDOW_NAME + " Output Image";

    // initialize StyleTransferData object
    StyleTransferData style_data;
    style_data.window_name = output_window_name;

    // copy input images
    template_image.copyTo( style_data.template_image );
    target_image.copyTo( style_data.target_image );
    template_image.release();
    target_image.release();

    // SET the algorithm parameters
    style_data.quadrant_depth = quadrant_depth;

    // TEMPLATE
    // split images into equal amount of quadrants.
    cv::Rect template_rect = cv::Rect( {}, style_data.template_image.size() );
    style_data.template_quadrants = quadrant_split_recursive( template_rect, quadrant_depth );

    // TARGET
    // split images into equal amount of quadrants.
    cv::Rect target_rect = cv::Rect( {}, style_data.target_image.size() );
    style_data.target_quadrants = quadrant_split_recursive( target_rect, quadrant_depth );

#if DEBUG
    std::cout << std::endl << "Template Quadrants:\t" << style_data.template_quadrants.size() << std::endl;
    std::cout << "Target Quadrants:\t" << style_data.target_quadrants.size() << std::endl;
#endif

    return style_data;
}


void
process_style_data(StyleTransferData* style_data)
{
    assert( style_data != NULL);
    assert( !style_data->template_image.empty() );
    assert( !style_data->target_image.empty() );
    assert( style_data->template_quadrants.size() > 0 );
    assert( style_data->target_quadrants.size() > 0 );

#if DEBUG
    std::clock_t clock_begin;
    std::clock_t clock_end;
    clock_begin = std::clock();
    std::cout << std::endl << "Begin Style Transfer" << std::endl;
#endif

    // split hsv into planes hue[0], saturation[1], vintensity[2]
    std::vector<cv::Mat> src_planes;
    cv::split( style_data->template_image, src_planes );

    std::vector<cv::Mat> dst_planes;
    cv::split( style_data->target_image, dst_planes );

    // loop thru each superpixel to transfer style (mean)
    for ( int i = 0; i < std::pow( 4, style_data->quadrant_depth ); i++ ) {

        // find the mask for given superpixel
        cv::Mat src_quad_roi = quadrant_mask_generator(
            style_data->template_image,
            style_data->template_quadrants.at( i )
        );
        cv::Mat dst_quad_roi = quadrant_mask_generator(
            style_data->target_image,
            style_data->target_quadrants.at( i )
        );

#if DEBUG > 1
    for ( int i = 0; i < std::pow( 4, style_data->quadrant_depth ); i++ ) {
        cv::imshow("src_quad_roi", src_quad_rois[i]);
        cv::imshow("dst_quad_roi", dst_quad_rois[i]);
        cv::waitKey(100);
    }
#endif

        // average hue
        cv::Scalar src_mean_hue = cv::mean( src_planes.at( 0 ), src_quad_roi );
        cv::Scalar dst_mean_hue = cv::mean( dst_planes.at( 0 ), dst_quad_roi );
        // avg saturation
        cv::Scalar src_mean_sat = cv::mean( src_planes.at( 1 ), src_quad_roi );
        cv::Scalar dst_mean_sat = cv::mean( dst_planes.at( 1 ), dst_quad_roi );
        // average vintensity
        cv::Scalar src_mean_val = cv::mean( src_planes.at( 2 ), src_quad_roi );
        cv::Scalar dst_mean_val = cv::mean( dst_planes.at( 2 ), dst_quad_roi );
        src_quad_roi.release();

        // copy hue and saturation from src to dst
        // dst_planes[0].setTo( src_mean_hue, dst_marker_mask );
        dst_planes[0].setTo( (src_mean_hue.val[0] * 6 + dst_mean_hue.val[0] ) / 7, dst_quad_roi );
        // dst_planes[1].setTo( src_mean_sat, dst_marker_mask );
        dst_planes[1].setTo( (src_mean_sat.val[0] * 3 + dst_mean_sat.val[0] ) / 4, dst_quad_roi );
        // average vintensity of both weighing dst
        dst_planes[2].setTo( (src_mean_val.val[0] + dst_mean_val.val[0] * 3 ) / 4, dst_quad_roi );
        dst_quad_roi.release();
    }

    // release source planes
    for ( cv::Mat &img : src_planes ) {
        img.release();
    }

#if DEBUG
    clock_end = std::clock();
    std::printf( "\nStyle Transfer Time Elapsed: %.0f (ms)\n", (float)( clock_end - clock_begin ) / CLOCKS_PER_SEC * 1000 );
    clock_begin = std::clock();
#endif

    // merge dst_planes back to hsv image
    cv::merge( dst_planes, style_data->marked_up_image );
    for ( cv::Mat &img : dst_planes ) {
        img.release();
    }

    // and convert to bgr
    hsv_to_bgr( style_data->marked_up_image, &style_data->marked_up_image );

#if DEBUG
    clock_end = std::clock();
    std::printf( "\nMerge Time Elapsed: %.0f (ms)\n", (float)( clock_end - clock_begin ) / CLOCKS_PER_SEC * 1000 );
#endif
}


// apply output filters, show, save, and initialize mouse callback
void
postprocess_style_data(
    StyleTransferData* style_data,
    std::string out_dir,
    bool blur_output,
    bool equalize_output,
    bool sharpen_output
) {
    assert( style_data != NULL);
    assert( !style_data->marked_up_image.empty() );

    // blur the output if given 'b' flag
    if ( blur_output ) {
        cv::GaussianBlur( style_data->marked_up_image, style_data->marked_up_image, cv::Size( 3, 3 ), 3.5f );
    }

    if ( sharpen_output ) {
        cv::Mat tmp;
        cv::GaussianBlur( style_data->marked_up_image, tmp, cv::Size( 0, 0 ), 3 );
        cv::addWeighted( style_data->marked_up_image, 1.42, tmp, -0.42, 0, style_data->marked_up_image );
        tmp.release();
    }

    // equalize the output if given 'e' flag
    if ( equalize_output ) {
        equalize_image( &style_data->marked_up_image );
    }

    char metadata[50];
    std::sprintf( metadata, "q_%d.png",
        style_data->quadrant_depth
    );
    cv::imshow( style_data->window_name, style_data->marked_up_image );
    write_img_to_file( style_data->marked_up_image, "./out/style_output/" + out_dir, metadata );

    // initialize the mouse callback
    init_callback( style_data );
}
