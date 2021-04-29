// quandrant.cpp : Squandrants
// Austin Hester CS642o apr 2021
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#include <opencv2/core/core.hpp>

#include "quadrant.hpp"


// swap images
void
swap_mat(cv::Mat* a, cv::Mat* b)
{
    cv::Mat tmp;
    a->copyTo(tmp);
    b->copyTo(*a);
    tmp.copyTo(*b);
    tmp.release();
}

// swap quandrants
void
swap_quadrants(cv::Mat* src)
{
    // assert cols and rows are even
    assert( !( src->cols & 1 || src->rows & 1) );

    std::vector<cv::Mat> q = quadrant_cut( src );

    // swap quandrants
    swap_mat( &q[0], &q[3] );
    swap_mat( &q[1], &q[2] );

    for (cv::Mat &img : q) {
        img.release();
    }
}

std::vector<cv::Mat>
quadrant_cut(cv::Mat* src)
{
    assert( !( src->cols & 1 || src->rows & 1) );
    int c_x = src->cols / 2;
    int c_y = src->rows / 2;

    std::vector<cv::Mat> quandrants;
    quandrants.push_back(cv::Mat( *src, cv::Rect( 0, 0, c_x, c_y ))); // top_left
    quandrants.push_back(cv::Mat( *src, cv::Rect( c_x, 0, c_x, c_y ))); // top_right
    quandrants.push_back(cv::Mat( *src, cv::Rect( 0, c_y, c_x, c_y ))); // bottom_left
    quandrants.push_back(cv::Mat( *src, cv::Rect( c_x, c_y, c_x, c_y )));// bottom_right
    return quandrants;
}
