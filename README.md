# Artistic Style Transfer
### Austin Hester
### CS 6420 - Computer Vision
### UMSL SP2021, Prof. Sanjiv Bhatia

----
## Purpose

Computer-assisted artistic style transfer gives animators a tool to produce rotoscope-like videos with relative ease. Automatic artistic style transfer can transform a simple home movie into something straight out of the artist's imagination, and they must only provide one example style frame, provided the subject does not change dramatically. Artistic style transfer softwares allow ideas to flow more freely in a short-film medium by providing the tools to apply an artistic style and background to the entire frame sequence, given only a single example frame [6 Fišer]. Attractive constraints of the style transfer can range from a total makeover of the entire frame to simply an addition of mustaches to every person in the picture. The focus of style transfer shifts to facial animations, such as the "filters" seen on many popular social media apps. The use of technology for fun became more commonplace in 2020, and tools to help people utilize their imagination are increasingly valuable.

----


## Usage

```
Usage: style_transfer.exe [params] template_image target_image

        -b, --blur
                Output Image - Blur
        -e, --equalize
                Output Image - Equalize
        -h, --help (value:true)
                Show Help Message
        -m, --transfer_mode (value:blend)
                Transfer Mode [ blend / mean ]
        -q, --quadrant_depth (value:0)
                Quadrant Depth. How many times to split. [0, 8]
        --sc, --scale (value:1.f)
                Scale input image size using Affine Transform. (0, 10)
        --sh, --sharpen
                Output Image - Sharpen
        --w_hue, --wh (value:0.2f)
                Hue Weight (of template)
        --w_sat, --ws (value:0.1f)
                Saturation Weight (of template)
        --w_val, --wv (value:0.85f)
                Value Weight (of template)

        template_image
                Template image. Provides theme.
        target_image
                Target image. Provides structure. Defaults to template image.

```
----

## Mean Mode Results

[Mean Mode](doc/MEAN_RESULTS.md)

## Blend Mode Results

[Blend Mode](doc/BLEND_RESULTS.md)


https://github.com/ahester57/style_transfer
