// mouse_callback.cpp : Mouse Callback Stuff
// Austin Hester CS642o april 2021
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "mouse_callback.hpp"

#include "dir_func.hpp"
#include "style_struct.hpp"

#define DEBUG 1

#if DEBUG
    #include <iostream>
#endif


// draw zeros on image
void
mouse_callback_draw_zeros(int event, int x, int y, int d, void* userdata)
{
    StyleTransferData* style_data = (StyleTransferData*) userdata;

    switch ( event ) {

    // RIGHT MOUSE BUTTON
        case cv::EVENT_RBUTTONUP:
            // zero-out region of interest
            style_data->marked_up_image = cv::Mat::zeros( style_data->template_image.size(), style_data->template_image.type() );

            // right click blanks atm.

            // show marked_up_image
            cv::imshow( style_data->window_name, style_data->marked_up_image );
            break;

    // LEFT MOUSE BUTTON
        case cv::EVENT_LBUTTONUP:
            // check bounds (needed if double ROI is larger than input image
            if ( x > style_data->markers.size().width || y > style_data->markers.size().height ) {
#if DEBUG
                std::cout << "OOB" << std::endl;
#endif
                break;
            }

            // find the marker at that point
            int marker_value = style_data->markers.at<int>( y, x );

#if DEBUG
            std::cout << "Marker Value:\t\t" << marker_value << std::endl;
#endif
            // check marker exists
            if ( marker_value < 0 || marker_value >= std::pow( 4, style_data->quadrant_depth ) ) {
#if DEBUG
                std::cout << "Marker Out of Range." << std::endl;
#endif
                break;
            }

            // show marked_up_image
            cv::imshow( style_data->window_name, style_data->marked_up_image );

            // save marked_up_image
            char metadata[50];
            std::sprintf( metadata, "regions/out_q_%dm_%d.png",
                style_data->quadrant_depth,
                marker_value
            );
            write_img_to_file(
                style_data->marked_up_image,
                "./out",
                metadata
            );
            break;

    }
}


// assign mouse callbacks
void
init_callback(StyleTransferData* style_data)
{
    cv::setMouseCallback( style_data->window_name, mouse_callback_draw_zeros, style_data );
}

void
init_callback(StyleTransferData* style_data, std::string window_name)
{
    cv::setMouseCallback( window_name, mouse_callback_draw_zeros, style_data );
}
