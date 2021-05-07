# Artistic Style Transfer
### Austin Hester
### CS 6420 - Computer Vision
### UMSL SP2021, Prof. Sanjiv Bhatia

----
## Purpose


----

## Task


----

#### Usage

```
Usage: style_transfer.exe [params] template_image target_image

        -b, --blur
                Output Image - Blur
        -e, --equalize
                Output Image - Equalize
        -h, --help (value:true)
                Show Help Message
        -m, --mode (value:blend)
                Transfer mode [ blend / mean ]
        -q, --quadrant_depth (value:0)
                Quadrant Depth. How many times to split. [0, 8]
        --sc, --scale (value:1.f)
                Scale input image size using Affine Transform. (0, 10)
        --sh, --sharpen
                Output Image - Sharpen

        template_image
                Template image. Provides theme.
        target_image
                Target image. Provides structure. Defaults to template image.

```
----

https://github.com/ahester57/style_transfer
