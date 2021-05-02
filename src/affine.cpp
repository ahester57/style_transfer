// affine.cpp : Affine Transformations
// Austin Hester CS642o april 2021
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "affine.hpp"

/*
    Make a right triangle
*/
std::vector<cv::Point2f>
make_right_triangle(float x, float y)
{
    std::vector<cv::Point2f> points;
    points.push_back( cv::Point2f( 0.f, 0.f ) );
    points.push_back( cv::Point2f( x, 0.f ) );
    points.push_back( cv::Point2f( 0.f, y ) );
    return points;
}


cv::Mat
resize_affine(cv::Mat src, float scale)
{
    assert( (src.rows * scale <= 3840) && (src.cols * scale <= 3840) );

    // save these to a variable so we can delete them later, thanks C
    std::vector<cv::Point2f> src_points = make_right_triangle( src.cols - 1.f, src.rows - 1.f );
    std::vector<cv::Point2f> dst_points = make_right_triangle( (src.cols - 1.f) * scale, (src.rows - 1.f) * scale );

    cv::Mat warp_mat = cv::getAffineTransform( src_points, dst_points );

    cv::Mat warp_dst = cv::Mat::zeros( src.rows * scale, src.cols * scale, src.type() );

    cv::warpAffine( src, warp_dst, warp_mat, warp_dst.size() );

    warp_mat.release();

    return warp_dst;
}
