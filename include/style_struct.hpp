// style_struct.hpp : Structures
// Austin Hester CS542o may 2021
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#ifndef STYLE_STRUCT_H
#define STYLE_STRUCT_H

#include <opencv2/core/core.hpp>

#include <vector>


typedef struct {
    std::string window_name;
    // input images
    cv::Mat template_image;
    cv::Mat target_image;

    // image quadrants, same size
    std::vector<cv::Rect> template_quadrants;
    std::vector<cv::Rect> target_quadrants;
    // quadrant depth, or how many times to recursively split into 4
    int quadrant_depth;
    int transfer_mode;

    float w_hue;
    float w_sat;
    float w_val; // identity preservation: have low w_val e.g. 0.05

    // output image
    cv::Mat marked_up_image;

} StyleTransferData;


#endif
