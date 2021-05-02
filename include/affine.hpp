// affine.hpp : Affine Transformations
// Austin Hester CS642o april 2021
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#ifndef AFFINE_H
#define AFFINE_H

#include <opencv2/core/core.hpp>

std::vector<cv::Point2f> make_right_triangle(float x, float y);

cv::Mat resize_affine(cv::Mat src, float scale);

#endif
