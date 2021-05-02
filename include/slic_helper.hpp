// slic_helper.hpp : Helper Functions
// Austin Hester CS542o apr 2021
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#ifndef SLIC_HELPER_H
#define SLIC_HELPER_H

#include <opencv2/core/core.hpp>

#include <vector>


typedef struct {
    std::string window_name;
    cv::Mat template_image;
    cv::Mat input_mask;
    cv::Mat region_of_interest;
    cv::Mat markers;
    cv::Mat marked_up_image;
    int algorithm;
    int region_size;
    float ruler;
    int connectivity;
    int num_superpixels;
    std::vector<cv::Rect> input_quadrants;
    int quadrant_depth;
} StyleTransferData;


StyleTransferData preprocess_slic(
    std::string input_image_filename,
    float scale_image_value,
    bool pad_input,
    std::string algorithm_string,
    int region_size,
    float ruler,
    int connectivity,
    int num_superpixels = 0,
    int quadrant_depth = 0
);

void process_slic(StyleTransferData* image_data);

void postprocess_slic(
    StyleTransferData* image_data,
    bool blur_output,
    bool equalize_output,
    bool sharpen_output
);

void superpixel_slic(StyleTransferData* image_data);

int slic_string_to_int(std::string algorithm_string);

void select_region(StyleTransferData* map_data, int marker_value);

cv::Mat paint_map_atop_region(StyleTransferData* map_data, int marker_value, cv::Mat drawn_contour);

void draw_on_original(StyleTransferData* map_data, int marker_value = -1);

#endif
