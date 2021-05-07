// style_helper.cpp : This file contains the helper functions for the main
// Austin Hester CS642o may 2021
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>

#include "style_helper.hpp"

#include "affine.hpp"
#include "dir_func.hpp"
#include "equalize.hpp"
#include "hsv_convert.hpp"
#include "mouse_callback.hpp"
#include "quadrant.hpp"
#include "region_of_interest.hpp"
#include "registration.hpp"


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
    int quadrant_depth,
    int transfer_mode,
    float w_hue,
    float w_sat,
    float w_val
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

#if DEBUG
    cv::imshow( WINDOW_NAME + " Template Image", template_image );
    cv::imshow( WINDOW_NAME + " Target Image", target_image );
    cv::waitKey(1);
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
    style_data.transfer_mode = transfer_mode;
    style_data.w_hue = w_hue;
    style_data.w_sat = w_sat;
    style_data.w_val = w_val; // identity preservation

    // TEMPLATE
    // split images into equal amount of quadrants.
    style_data.template_quadrants = quadrant_split_recursive( style_data.template_image, quadrant_depth );

    // TARGET
    // split images into equal amount of quadrants.
    style_data.target_quadrants = quadrant_split_recursive( style_data.target_image, quadrant_depth );

#if DEBUG
    std::cout << std::endl << "Template Quadrants:\t" << style_data.template_quadrants.size() << std::endl;
    std::cout << "Target Quadrants:\t" << style_data.target_quadrants.size() << std::endl;
#endif

    return style_data;
}


void
process_style_data(StyleTransferData* style_data)
{
    float w_hue = style_data->w_hue;
    float w_sat = style_data->w_sat;
    float w_val = style_data->w_val;

    assert( style_data != NULL);
    assert( !style_data->template_image.empty() );
    assert( !style_data->target_image.empty() );
    assert( style_data->template_quadrants.size() > 0 );
    assert( style_data->template_quadrants.size() == style_data->target_quadrants.size() );
    assert( w_hue >= 0 && w_hue <= 1 );
    assert( w_sat >= 0 && w_sat <= 1 );
    assert( w_val >= 0 && w_val <= 1 );

    // register template to target
    cv::Mat m_template = register_images( style_data->template_image, style_data->target_image );
    cv::Mat m_target = style_data->target_image.clone();

#if DEBUG
    cv::imshow( "Registered Template", m_template );
    cv::waitKey( 0 );
    std::clock_t clock_begin;
    std::clock_t clock_end;
    std::printf( "\nBegin Style Transfer\n" );
    clock_begin = std::clock();
#endif


#if DEBUG
    std::printf( "\nBegin splitting planes\n" );
#endif

    // split hsv into planes hue[0], saturation[1], vintensity[2]
    std::vector<cv::Mat> src_planes;
    cv::split( m_template, src_planes );
    m_template.release();

    std::vector<cv::Mat> dst_planes;
    cv::split( m_target, dst_planes );
    m_target.release();

    // initialize 'canvas'
    cv::Mat output_tmp = cv::Mat::zeros( style_data->target_image.size(), style_data->target_image.type() );
    std::vector<cv::Mat> output_planes;
    cv::split( output_tmp, output_planes );
    output_tmp.release();

#if DEBUG
    std::printf( "\nBegin transfer loop\n" );

    // CLA `--mode=[Blend|mean]`
    switch ( style_data->transfer_mode ) {
    // mode = blend
    case blend:
        std::printf( "\nMode: \"blend\"\n" );
        break;
    case mean:
        std::printf( "\nMode: \"mean\"\n" );
        break;
    default:
        std::printf( "\nMode: \"none\"\n" );
    }
#endif

    // loop thru each superpixel to transfer style (mean)
    for ( int i = 0; i < std::pow( 4, style_data->quadrant_depth ); i++ ) {

        cv::Rect src_rect = style_data->template_quadrants.at( i );
        cv::Rect dst_rect = style_data->target_quadrants.at( i );

        if ( src_rect.area() == 0 || dst_rect.area() == 0 ) {
            continue;
        }

#if DEBUG > 1
    for ( int i = 0; i < std::pow( 4, style_data->quadrant_depth ); i++ ) {
        cv::imshow("src_quad_roi", src_quad_rois[i]);
        cv::imshow("dst_quad_roi", dst_quad_rois[i]);
        cv::waitKey(100);
    }
#endif

        // for blend mode
        cv::Mat src_hue;
        cv::Mat src_sat;
        cv::Mat src_val;
        cv::Mat dst_hue;
        cv::Mat dst_sat;
        cv::Mat dst_val;

        // for mean mode
        cv::Mat src_mask;
        cv::Mat dst_mask;
        cv::Scalar src_hue_s;
        cv::Scalar dst_hue_s;
        cv::Scalar src_sat_s;
        cv::Scalar dst_sat_s;
        cv::Scalar src_val_s;
        cv::Scalar dst_val_s;

        // CLA `--mode=[Blend|mean]`
        switch ( style_data->transfer_mode ) {
        // mode = blend
        case blend:

            // we resize each quadrant, as not to warp the template image out of affine warp_matrix
            // hue [0]
            src_hue = extract_roi_safe( src_planes.at( 0 ), src_rect );
            dst_hue = extract_roi_safe( dst_planes.at( 0 ), dst_rect );
            cv::resize( src_hue, src_hue, dst_rect.size(), 0, 0, cv::INTER_LINEAR );

            // saturation [1]
            src_sat = extract_roi_safe( src_planes.at( 1 ), src_rect );
            dst_sat = extract_roi_safe( dst_planes.at( 1 ), dst_rect );
            cv::resize( src_sat, src_sat, dst_rect.size(), 0, 0, cv::INTER_LINEAR );

            // vintensity [2]
            src_val = extract_roi_safe( src_planes.at( 2 ), src_rect );
            dst_val = extract_roi_safe( dst_planes.at( 2 ), dst_rect );
            cv::resize( src_val, src_val, dst_rect.size(), 0, 0, cv::INTER_LINEAR );

            // combine them based on weights
            // TODO give dst edges more weight
            dst_hue = ( src_hue * (1.0f - w_hue) ) + ( dst_hue * w_hue );
            dst_sat = ( src_sat * (1.0f - w_sat) ) + ( dst_sat * w_sat );
            dst_val = ( src_val * (1.0f - w_val) ) + ( dst_val * w_val );

            // // copy hue and saturation from src to dst
            dst_hue.copyTo( output_planes.at( 0 )( dst_rect ) );
            dst_sat.copyTo( output_planes.at( 1 )( dst_rect ) );
            dst_val.copyTo( output_planes.at( 2 )( dst_rect ) );

            src_hue.release();
            src_sat.release();
            src_val.release();

            dst_hue.release();
            dst_sat.release();
            dst_val.release();

            break;
        // mode = mean
        case 1:
            // much slower
            src_mask = quadrant_mask_generator( src_planes.at( 0 ), src_rect );
            dst_mask = quadrant_mask_generator( dst_planes.at( 0 ), dst_rect );

            // hue [0]
            src_hue_s = cv::mean( src_planes.at( 0 ), src_mask );
            dst_hue_s = cv::mean( dst_planes.at( 0 ), dst_mask );

            // saturation [1]
            src_sat_s = cv::mean( src_planes.at( 1 ), src_mask );
            dst_sat_s = cv::mean( dst_planes.at( 1 ), dst_mask );

            // vintensity [2]
            src_val_s = cv::mean( src_planes.at( 2 ), src_mask );
            dst_val_s = cv::mean( dst_planes.at( 2 ), dst_mask );

            // weigh mean hue and saturation against src to dst
            output_planes.at( 0 ).setTo(
                (src_hue_s.val[0] * (1.0f - w_hue) + dst_hue_s.val[0] * w_hue ),
                dst_mask
            );
            output_planes.at( 1 ).setTo(
                (src_sat_s.val[0] * (1.0f - w_sat) + dst_sat_s.val[0] * w_sat ),
                dst_mask
            );
            output_planes.at( 2 ).setTo(
                (src_val_s.val[0] * (1.0f - w_val) + dst_val_s.val[0] * w_val ),
                dst_mask
            );

            src_mask.release();
            dst_mask.release();

            break;
        // i guess do nothing
        default:
            output_planes.at( 0 ).setTo( dst_planes.at( 0 ) );
            output_planes.at( 1 ).setTo( dst_planes.at( 1 ) );
            output_planes.at( 2 ).setTo( dst_planes.at( 2 ) );
        }

    }

    // release source planes
    for ( cv::Mat &img : src_planes ) {
        img.release();
    }
    // release dst planes
    for ( cv::Mat &img : dst_planes ) {
        img.release();
    }

#if DEBUG
    clock_end = std::clock();
    std::printf( "\nStyle Transfer Time Elapsed: %.0f (ms)\n", (float)( clock_end - clock_begin ) / CLOCKS_PER_SEC * 1000 );
    clock_begin = std::clock();
#endif

    // merge output_planes back to hsv image
    cv::merge( output_planes, style_data->marked_up_image );

    // release out planes
    for ( cv::Mat &img : output_planes ) {
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
