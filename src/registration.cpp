// style_helper.cpp : This file contains the helper functions for the main
// Austin Hester CS642o may 2021
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/tracking.hpp>

#include "registration.hpp"

#define DEBUG 1

#if DEBUG
    #include <ctime>
#endif


cv::Mat
register_images(cv::Mat tmplate, cv::Mat target)
{
#if DEBUG
    std::clock_t clock_begin;
    std::clock_t clock_end;
    std::printf( "\nBegin Image Registration\n" );
    clock_begin = std::clock();
#endif

    int motion_type = cv::MOTION_EUCLIDEAN;
    float epsilon = 0.0000001;

    cv::Size warp_matrix_size = cv::Size( 3, motion_type != cv::MOTION_HOMOGRAPHY ? 2 : 3 );
    // create warp matrix
    cv::Mat warp_matrix = cv::Mat::eye( warp_matrix_size, CV_32F );;


    cv::Mat equalized_tmplate;
    // convert input image to grayscale
    cv::cvtColor( tmplate, equalized_tmplate, cv::COLOR_HSV2BGR );
    cv::cvtColor( target, target, cv::COLOR_HSV2BGR );
    cv::cvtColor( equalized_tmplate, equalized_tmplate, cv::COLOR_BGR2GRAY );
    cv::cvtColor( target, target, cv::COLOR_BGR2GRAY );
    // equalize grayscale input image
    //TODO perform histogram matching instead of equalize separately
    cv::equalizeHist( equalized_tmplate, equalized_tmplate );
    cv::equalizeHist( target, target );

#if DEBUG
    std::printf( "\nBegin findTransformECC\n" );
#endif

   double correlation_co
        = cv::findTransformECC(
            target,
            equalized_tmplate,
            warp_matrix,
            motion_type,
            cv::TermCriteria(
                cv::TermCriteria::COUNT+cv::TermCriteria::EPS,
                warp_matrix.size().area(),
                epsilon
            )
        );

#if DEBUG
    std::printf( "\nBegin warpAffine\n" );
#endif

    // warp original image using transformed warp matrix
    motion_type != cv::MOTION_HOMOGRAPHY ?
        cv::warpAffine(      tmplate, tmplate, warp_matrix, tmplate.size() ) :
        cv::warpPerspective( tmplate, tmplate, warp_matrix, tmplate.size() );

    equalized_tmplate.release();
    warp_matrix.release();

    // cv::cvtColor( tmplate, tmplate, cv::COLOR_BGR2HSV );

#if DEBUG
    clock_end = std::clock();
    std::printf( "\nRegistration Time Elapsed: %.0f (ms)\n", (float)( clock_end - clock_begin ) / CLOCKS_PER_SEC * 1000 );
#endif

    return tmplate;
}
