// registration.hpp : Image Registration
// Austin Hester CS642o may 2021
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#ifndef REGISTRATION_H
#define REGISTRATION_H

#include <opencv2/core/core.hpp>


cv::Mat register_images(cv::Mat tmplate, cv::Mat target);

#endif
