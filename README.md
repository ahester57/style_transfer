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

https://github.com/ahester57/style_transfer
