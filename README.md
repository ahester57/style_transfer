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

        -a, --algorithm (value:MSLIC)
                Name of SLIC algorithm variant
                         - SLIC segments image using a desired region size
                         - SLICO optimizes using an adaptive compactness factor
                         - MSLIC optimizes using manifold methods giving more context-sensitive superpixels
        -b, --blur (value:true)
                Output Image - Blur
        -c, --connectivity (value:99)
                The minimum element size in percents that should be absorbed into a bigger superpixel
        -e, --equalize (value:true)
                Output Image - Equalize
        -h, --help (value:true)
                Show Help Message
        -r, --ruler (value:10)
                Chooses the enforcement of superpixel smoothness
        --region_size, -s (value:8)
                Chooses an average superpixel size measured in pixels
        --sc, --scale (value:1.6)
                Scale input image size using Affine Transform
        --sh, --sharpen (value:true)
                Output Image - Sharpen

        template_image (value:images/tiger.jpg)
                Template image. Provides theme.
        target_image
                Target image. Provides structure. Defaults to template image.

```
----

https://github.com/ahester57/style_transfer
