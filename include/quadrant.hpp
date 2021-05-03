// quandrant.hpp : Squandrants
// Austin Hester CS642o apr 2021
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#ifndef QUADRANT_H
#define QUADRANT_H

#include <opencv2/core/core.hpp>


// swap images
void swap_mat(cv::Mat* a, cv::Mat* b);

// swap quandrants
void swap_quadrants(cv::Mat* src);

// cut quadrants
std::vector<cv::Rect> quadrant_cut(cv::Rect src_rect);

std::vector<cv::Rect> quadrant_split_recursive(cv::Mat image, int depth = 2);

std::vector<cv::Rect> quadrant_split_recursive(cv::Mat image, cv::Rect src_rect, int depth = 2);

cv::Mat quadrant_mask_generator(cv::Mat image, cv::Rect quadrant);

void mask_quadrant(cv::Mat* mask, cv::Rect quadrant);

#endif
