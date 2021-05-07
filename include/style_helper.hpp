// style_helper.hpp : Helper Functions
// Austin Hester CS542o may 2021
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#ifndef STYLE_HELPER_H
#define STYLE_HELPER_H

#include <opencv2/core/core.hpp>

#include <vector>

#include "style_enum.hpp"
#include "style_struct.hpp"


StyleTransferData preprocess_style_data(
    std::string template_filename,
    std::string target_filename,
    float scale_image_value,
    int quadrant_depth = 1,
    int transfer_mode = blend,
    float w_hue = 0.2,
    float w_sat = 0.1,
    float w_val = 0.85
);

void process_style_data(StyleTransferData* style_data);

void postprocess_style_data(
    StyleTransferData* style_data,
    std::string out_dir,
    bool blur_output,
    bool equalize_output,
    bool sharpen_output
);

#endif
