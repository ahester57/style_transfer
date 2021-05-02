// quandrant.cpp : Squandrants
// Austin Hester CS642o apr 2021
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#include <opencv2/core/core.hpp>

#include "quadrant.hpp"

#include "region_of_interest.hpp"


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

    // cut with full quadrant
    std::vector<cv::Rect> quadrants = quadrant_cut( cv::Rect( 0, 0, src->cols, src->rows ) );
    std::vector<cv::Mat> mat_quads;

    // get source image @ quadrants
    for (cv::Rect& q : quadrants) {
        mat_quads.push_back( cv::Mat( *src, q ) );
    }

    // swap quandrants
    swap_mat( &mat_quads[0], &mat_quads[3] );
    swap_mat( &mat_quads[1], &mat_quads[2] );

    for (cv::Mat &img : mat_quads) {
        img.release();
    }
}

std::vector<cv::Rect>
quadrant_cut(cv::Rect src_rect)
{
    assert( !( src_rect.x & 1 || src_rect.y & 1) );
    int c_x = src_rect.x / 2;
    int c_y = src_rect.y / 2;

    std::vector<cv::Rect> quandrants;
    quandrants.push_back( cv::Rect( 0, 0, c_x, c_y ) ); // top_left
    quandrants.push_back( cv::Rect( c_x, 0, c_x, c_y ) ); // top_right
    quandrants.push_back( cv::Rect( 0, c_y, c_x, c_y ) ); // bottom_left
    quandrants.push_back( cv::Rect( c_x, c_y, c_x, c_y ) );// bottom_right
    return quandrants;
}

std::vector<cv::Rect>
quadrant_split_recursive(cv::Rect src_rect, int depth)
{
    if (depth == 0) {
        std::vector<cv::Rect> depth_limit_rect;
        depth_limit_rect.push_back( src_rect );
        return depth_limit_rect;
    }
    // compute quadrants of src_rect
    std::vector<cv::Rect> quadrants = quadrant_cut( src_rect );
    std::vector<cv::Rect> recursive_quads;
    // recursively do all quadrants
    for (cv::Rect& q : quadrants) {
        std::vector<cv::Rect> q_quads = quadrant_split_recursive( q, depth - 1 );
        recursive_quads.insert( recursive_quads.begin(), q_quads.begin(), q_quads.end() );
    }
    return recursive_quads;
}

// return list of ROIs given image and rects
std::vector<cv::Mat>
quadrant_selector(cv::Mat image, std::vector<cv::Rect> quadrants)
{
    std::vector<cv::Mat> quad_rois;

    // TEMPLATE
    // loop thru each rect to create mask lookup
    for (cv::Rect& q : quadrants) {
        // find the mask for given quadrant
        cv::Mat quad_roi = extract_roi_safe( image, q );
        // add to list
        quad_rois.push_back( quad_roi );

#if DEBUG > 1
        cv::imshow("quad_roi", quad_roi);
        cv::waitKey(1);
#endif

        quad_roi.release();
    }

    return quad_rois;
}
