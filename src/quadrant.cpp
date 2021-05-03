// quandrant.cpp : Squandrants
// Austin Hester CS642o apr 2021
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "quadrant.hpp"

#include "region_of_interest.hpp"

#define DEBUG 0

#if DEBUG
    #include <opencv2/highgui/highgui.hpp>
    #include <iostream>
#endif


// swap images
void
swap_mat(cv::Mat* a, cv::Mat* b)
{
    cv::Mat tmp;
    a->copyTo( tmp );
    b->copyTo( *a );
    tmp.copyTo( *b );
    tmp.release();
}

// swap quandrants
void
swap_quadrants(cv::Mat* src)
{
    // assert cols and rows are even
    assert( !(src->cols & 1 || src->rows & 1) );

    // cut with full quadrant
    std::vector<cv::Rect> quadrants = quadrant_cut( cv::Rect( 0, 0, src->cols, src->rows ) );
    std::vector<cv::Mat> mat_quads;

    // get source image @ quadrants
    for ( cv::Rect& q : quadrants ) {
        mat_quads.push_back( cv::Mat( *src, q ) );
    }

    // swap quandrants
    swap_mat( &mat_quads[0], &mat_quads[3] );
    swap_mat( &mat_quads[1], &mat_quads[2] );

    for ( cv::Mat &img : mat_quads ) {
        img.release();
    }
}

// split rect into 4 equal size quadrants
std::vector<cv::Rect>
quadrant_cut(cv::Rect src_rect)
{
    int c_h = src_rect.height / 2;
    int c_w = src_rect.width / 2;

    //TODO still, not all data preserved
    std::vector<cv::Rect> quandrants;
    quandrants.push_back( cv::Rect( src_rect.x, src_rect.y                  , (c_h + 1) ^ 2, (c_w + 1) ^ 2 ) ); // top_left
    quandrants.push_back( cv::Rect( src_rect.x, (src_rect.y + c_w)          , (c_h + 1) ^ 2, (c_w + 1) ^ 2 ) ); // top_right
    quandrants.push_back( cv::Rect( (src_rect.x + c_h), src_rect.y          , (c_h + 1) ^ 2, (c_w + 1) ^ 2 ) ); // bottom_left
    quandrants.push_back( cv::Rect( (src_rect.x + c_h), (src_rect.y + c_w)  , (c_h + 1) ^ 2, (c_w + 1) ^ 2 ) ); // bottom_right
    return quandrants;
}

// split a rect into 4 quadrants `depth` times
std::vector<cv::Rect>
quadrant_split_recursive(cv::Rect src_rect, int depth)
{
#if DEBUG
        std::cout << "depth: " << depth << std::endl;
#endif
    if ( depth == 0 ) {
        // return given rect when depth limit hit
        std::vector<cv::Rect> depth_limit_rect;
        depth_limit_rect.push_back( src_rect );
#if DEBUG
        std::cout << "depth limit hit" << std::endl;
        std::cout << src_rect.x << ", " << src_rect.y << ", " << src_rect.height << ", " << src_rect.width << std::endl;
#endif
        return depth_limit_rect;
    }
    // compute quadrants of src_rect
    std::vector<cv::Rect> quadrants = quadrant_cut( src_rect );

    std::vector<cv::Rect> recursive_quads;
    // recursively do all quadrants
    for ( cv::Rect& q : quadrants ) {
        std::vector<cv::Rect> q_quads = quadrant_split_recursive( q, depth - 1 );
        recursive_quads.insert( recursive_quads.end(), q_quads.begin(), q_quads.end() );
    }

#if DEBUG
    std::cout << std::endl << "Quadrants:\t" << recursive_quads.size() << std::endl;
    for ( cv::Rect& q : recursive_quads ) {
        std::cout << q.x << ", " << q.y << ", " << q.height << ", " << q.width << std::endl;
    }
#endif

    return recursive_quads;
}

cv::Mat
quadrant_mask_generator(cv::Mat image, cv::Rect quadrant)
{
    // find the mask for given quadrant
    cv::Mat quad_roi = cv::Mat::zeros( image.size(), CV_8U );
    mask_quadrant( &quad_roi, quadrant );

#if DEBUG > 1
        cv::imshow("quad_roi", quad_roi);
        cv::waitKey(1);
#endif

    return quad_roi;
}

void
mask_quadrant(cv::Mat* mask, cv::Rect quadrant)
{
    cv::Rect safe_q = extract_roi_rect_safe( *mask, quadrant );
    (*mask)( safe_q ) = cv::Scalar( 255 );
}
