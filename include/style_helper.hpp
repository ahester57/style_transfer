// style_helper.hpp : Helper Functions
// Austin Hester CS542o may 2021
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#ifndef STYLE_HELPER_H
#define STYLE_HELPER_H

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

    cv::Mat input_mask;
    cv::Mat region_of_interest;
    cv::Mat markers;
    cv::Mat marked_up_image;
    int algorithm;
    int region_size;
    float ruler;
    int connectivity;

} StyleTransferData;


StyleTransferData preprocess_style_data(
    std::string template_filename,
    std::string target_filename,
    float scale_image_value,
    bool pad_input,
    int region_size,
    float ruler,
    int connectivity,
    int quadrant_depth = 1
);

void process_style_data(StyleTransferData* style_data);

void postprocess_style_data(
    StyleTransferData* style_data,
    bool blur_output,
    bool equalize_output,
    bool sharpen_output
);

#endif
