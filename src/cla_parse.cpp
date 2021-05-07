// cla_parse.cpp : Parse given command line arguments.
// Austin Hester CS542o nov 2020
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#include <opencv2/core/core.hpp>

#include <map>

#include "cla_parse.hpp"

#include "style_enum.hpp"


// parse command line arguments
int
parse_arguments(
    int argc,
    const char** argv,
    std::string* template_image_filename,
    std::string* target_image_filename,
    float* scale_image_value,
    int* quadrant_depth,
    int* transfer_mode,
    bool* blur_output,
    bool* equalize_output,
    bool* sharpen_output
) {
    cv::String keys =
        "{@template_image |      | Template image. Provides theme.}"
        "{@target_image   |      | Target image. Provides structure. Defaults to template image.}"
        // "{grayscale g     |      | Read Input As Grayscale}"
        "{scale sc        |1.f   | Scale input image size using Affine Transform. (0, 10)}"
        "{quadrant_depth q|0     | Quadrant Depth. How many times to split. [0, 8]}"
        "{mode m          |blend | Transfer mode [ blend / mean ]}"
        "{equalize e      |      | Output Image - Equalize}"
        "{blur b          |      | Output Image - Blur}"
        "{sharpen sh      |      | Output Image - Sharpen}"
        "{help h          |      | Show Help Message}";

    cv::CommandLineParser parser =  cv::CommandLineParser(argc, argv, keys);

    parser.printMessage();

    if ( parser.has( "h" ) ) {
        return 0;
    }

    if ( !parser.check() ) {
        parser.printErrors();
        return -1;
    }

    try {
        *template_image_filename = (std::string) parser.get<std::string>( 0 ).c_str();
        assert( template_image_filename->size() > 0 );
    } catch (...) {
        std::cerr << "Failed to parse template_image argument!:" << std::endl;
        return -1;
    }

    try {
        *target_image_filename = (std::string) parser.get<std::string>( 1 ).c_str();
        if ( target_image_filename->size() == 0 ) {
            *target_image_filename = *template_image_filename;
        }
        assert( target_image_filename->size() > 0 );
    } catch (...) {
        std::cerr << "Failed to parse target_image argument!:" << std::endl;
        return -1;
    }

    try {
        *scale_image_value = (float) parser.get<float>( "sc" );
        assert( *scale_image_value > 0.f && *scale_image_value < 10.f );
    } catch (...) {
        std::cerr << "Failed to parse scale argument!:" << std::endl;
        return -1;
    }

    try {
        *quadrant_depth = parser.get<int>( "q" );
        assert( *quadrant_depth >= 0 && *quadrant_depth <= 8 );
    } catch (...) {
        std::cerr << "Failed to parse quadrant_depth argument!:" << std::endl;
        return -1;
    }

    try {
        std::string transfer_mode_str = (std::string) parser.get<std::string>( "m" ).c_str();
        assert( transfer_mode_str.size() > 0 );

        std::map<std::string, int> transfer_mode_str_map = {
            { "blend",  blend  },
            { "mean", mean }
        };
        *transfer_mode = transfer_mode_str_map[transfer_mode_str];
    } catch (...) {
        std::cerr << "Failed to parse transfer_mode argument!:" << std::endl;
        return -1;
    }

    try {
        *blur_output = parser.has( "b" );
    } catch (...) {
        std::cerr << "Failed to parse blur argument!:" << std::endl;
        return -1;
    }

    try {
        *equalize_output = parser.has( "e" );
    } catch (...) {
        std::cerr << "Failed to parse equalize argument!:" << std::endl;
        return -1;
    }

    try {
        *sharpen_output = parser.has( "sh" );
    } catch (...) {
        std::cerr << "Failed to parse sharpen argument!:" << std::endl;
        return -1;
    }

    std::cout << std::endl << "Shortcuts:" << std::endl << "\tq\t- quit" << std::endl << std::endl;

    return 1;
}
