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

            // show marked_up_image
            cv::imshow( style_data->window_name, style_data->marked_up_image );
            break;

    // LEFT MOUSE BUTTON
        case cv::EVENT_LBUTTONUP:

            // show marked_up_image
            cv::imshow( style_data->window_name, style_data->marked_up_image );

            // save marked_up_image
            char metadata[50];
            std::sprintf( metadata, "regions/out_q_%d.png",
                style_data->quadrant_depth
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
