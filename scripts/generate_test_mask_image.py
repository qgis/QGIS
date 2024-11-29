#!/usr/bin/env python3

"""
***************************************************************************
    generate_test_mask_image.py
    ---------------------
    Date                 : February 2015
    Copyright            : (C) 2015 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Nyall Dawson"
__date__ = "February 2015"
__copyright__ = "(C) 2015, Nyall Dawson"

# Generates (or updates) a unit test image mask, which is used to specify whether
# a pixel in the control image should be checked (black pixel in mask) or not (white
# pixel in mask). For non black or white pixels, the pixels lightness is used to
# specify a maximum delta for each color component

import argparse
import glob
import os
import struct
import sys
import urllib.error
import urllib.parse
import urllib.request

from PyQt5.QtGui import QColor, QImage, qAlpha, qBlue, qGreen, qRed, qRgb


def error(msg):
    print(msg)
    sys.exit(1)


def colorDiff(c1, c2):
    redDiff = abs(qRed(c1) - qRed(c2))
    greenDiff = abs(qGreen(c1) - qGreen(c2))
    blueDiff = abs(qBlue(c1) - qBlue(c2))
    alphaDiff = abs(qAlpha(c1) - qAlpha(c2))
    return max(redDiff, greenDiff, blueDiff, alphaDiff)


def imageFromPath(path):
    if path[:7] == "http://" or path[:7] == "file://" or path[:8] == "https://":
        # fetch remote image
        data = urllib.request.urlopen(path).read()
        image = QImage()
        image.loadFromData(data)
    else:
        image = QImage(path)
    return image


def getControlImagePath(path):
    if os.path.isfile(path):
        return path

    # else try and find matching test image
    script_folder = os.path.dirname(os.path.realpath(sys.argv[0]))
    control_images_folder = os.path.join(
        script_folder, "../tests/testdata/control_images"
    )

    matching_control_images = [
        x[0] for x in os.walk(control_images_folder) if path in x[0]
    ]
    if len(matching_control_images) > 1:
        error(f"Found multiple matching control images for {path}")
    elif len(matching_control_images) == 0:
        error(f"No matching control images found for {path}")

    found_control_image_path = matching_control_images[0]

    # check for a single matching expected image
    images = glob.glob(os.path.join(found_control_image_path, "*.png"))
    filtered_images = [i for i in images if not i[-9:] == "_mask.png"]
    if len(filtered_images) > 1:
        error(f"Found multiple matching control images for {path}")
    elif len(filtered_images) == 0:
        error(f"No matching control images found for {path}")

    found_image = filtered_images[0]
    print(f"Found matching control image: {found_image}")
    return found_image


def updateMask(control_image_path, rendered_image_path, mask_image_path):
    control_image = imageFromPath(control_image_path)
    if not control_image:
        error(f"Could not read control image {control_image_path}")

    rendered_image = imageFromPath(rendered_image_path)
    if not rendered_image:
        error(f"Could not read rendered image {rendered_image_path}")
    if (
        not rendered_image.width() == control_image.width()
        or not rendered_image.height() == control_image.height()
    ):
        print(
            "Size mismatch - control image is {}x{}, rendered image is {}x{}".format(
                control_image.width(),
                control_image.height(),
                rendered_image.width(),
                rendered_image.height(),
            )
        )

    max_width = min(rendered_image.width(), control_image.width())
    max_height = min(rendered_image.height(), control_image.height())

    # read current mask, if it exist
    mask_image = imageFromPath(mask_image_path)
    if mask_image.isNull():
        print(f"Mask image does not exist, creating {mask_image_path}")
        mask_image = QImage(
            control_image.width(), control_image.height(), QImage.Format.Format_ARGB32
        )
        mask_image.fill(QColor(0, 0, 0))

    # loop through pixels in rendered image and compare
    mismatch_count = 0
    linebytes = max_width * 4
    for y in range(max_height):
        control_scanline = control_image.constScanLine(y).asstring(linebytes)
        rendered_scanline = rendered_image.constScanLine(y).asstring(linebytes)
        mask_scanline = mask_image.scanLine(y).asstring(linebytes)

        for x in range(max_width):
            currentTolerance = qRed(
                struct.unpack("I", mask_scanline[x * 4 : x * 4 + 4])[0]
            )

            if currentTolerance == 255:
                # ignore pixel
                continue

            expected_rgb = struct.unpack("I", control_scanline[x * 4 : x * 4 + 4])[0]
            rendered_rgb = struct.unpack("I", rendered_scanline[x * 4 : x * 4 + 4])[0]
            difference = colorDiff(expected_rgb, rendered_rgb)

            if difference > currentTolerance:
                # update mask image
                mask_image.setPixel(x, y, qRgb(difference, difference, difference))
                mismatch_count += 1

    if mismatch_count:
        # update mask
        mask_image.save(mask_image_path, "png")
        print(f"Updated {mismatch_count} pixels in {mask_image_path}")
    else:
        print(f"No mismatches in {mask_image_path}")


parser = (
    argparse.ArgumentParser()
)  # OptionParser("usage: %prog control_image rendered_image mask_image")
parser.add_argument("control_image")
parser.add_argument("rendered_image")
parser.add_argument("mask_image", nargs="?", default=None)
args = parser.parse_args()

args.control_image = getControlImagePath(args.control_image)

if not args.mask_image:
    args.mask_image = args.control_image[:-4] + "_mask.png"

updateMask(args.control_image, args.rendered_image, args.mask_image)
