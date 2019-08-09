#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
****************************3***********************************************
    parse_dash_results.py
    ---------------------
    Date                 : October 2016
    Copyright            : (C) 2016 by Nyall Dawson
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
from __future__ import print_function
from future import standard_library
standard_library.install_aliases()
from builtins import range

__author__ = 'Nyall Dawson'
__date__ = 'October 2016'
__copyright__ = '(C) 2016, Nyall Dawson'

import os
import sys
import argparse
import urllib.request
import urllib.parse
import urllib.error
import re
from bs4 import BeautifulSoup
from PyQt5.QtGui import (
    QImage, QColor, qRed, qBlue, qGreen, qAlpha, qRgb, QPixmap)
from PyQt5.QtWidgets import (QDialog,
                             QApplication,
                             QLabel,
                             QVBoxLayout,
                             QHBoxLayout,
                             QGridLayout,
                             QPushButton,
                             QDoubleSpinBox,
                             QMessageBox,
                             QWidget,
                             QScrollArea)
import struct
import glob

dash_url = 'https://cdash.orfeo-toolbox.org'


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
    print(path)
    if (path[:8] == 'https://' or path[:7] == 'file://'):
        # fetch remote image
        print('fetching remote!')
        data = urllib.request.urlopen(path).read()
        image = QImage()
        image.loadFromData(data)
    else:
        print('using local!')
        image = QImage(path)
    return image


class ResultHandler(QDialog):

    def __init__(self, parent=None):
        super(ResultHandler, self).__init__()
        self.setWindowTitle('Dash results')
        self.control_label = QLabel()
        self.rendered_label = QLabel()
        self.diff_label = QLabel()

        self.mask_label = QLabel()
        self.new_mask_label = QLabel()

        self.scrollArea = QScrollArea()
        self.widget = QWidget()

        self.test_name_label = QLabel()
        grid = QGridLayout()
        grid.addWidget(self.test_name_label, 0, 0)
        grid.addWidget(QLabel('Control'), 1, 0)
        grid.addWidget(QLabel('Rendered'), 1, 1)
        grid.addWidget(QLabel('Difference'), 1, 2)
        grid.addWidget(self.control_label, 2, 0)
        grid.addWidget(self.rendered_label, 2, 1)
        grid.addWidget(self.diff_label, 2, 2)
        grid.addWidget(QLabel('Current Mask'), 3, 0)
        grid.addWidget(QLabel('New Mask'), 3, 1)
        grid.addWidget(self.mask_label, 4, 0)
        grid.addWidget(self.new_mask_label, 4, 1)

        self.widget.setLayout(grid)
        self.scrollArea.setWidget(self.widget)
        v_layout = QVBoxLayout()
        v_layout.addWidget(self.scrollArea, 1)

        next_image_button = QPushButton()
        next_image_button.setText('Skip')
        next_image_button.pressed.connect(self.load_next)

        self.overload_spin = QDoubleSpinBox()
        self.overload_spin.setMinimum(1)
        self.overload_spin.setMaximum(255)
        self.overload_spin.setValue(1)
        self.overload_spin.valueChanged.connect(lambda: save_mask_button.setEnabled(False))

        preview_mask_button = QPushButton()
        preview_mask_button.setText('Preview New Mask')
        preview_mask_button.pressed.connect(self.preview_mask)
        preview_mask_button.pressed.connect(lambda: save_mask_button.setEnabled(True))

        save_mask_button = QPushButton()
        save_mask_button.setText('Save New Mask')
        save_mask_button.pressed.connect(self.save_mask)

        button_layout = QHBoxLayout()
        button_layout.addWidget(next_image_button)
        button_layout.addWidget(QLabel('Mask diff multiplier:'))
        button_layout.addWidget(self.overload_spin)
        button_layout.addWidget(preview_mask_button)
        button_layout.addWidget(save_mask_button)
        button_layout.addStretch()
        v_layout.addLayout(button_layout)
        self.setLayout(v_layout)

    def closeEvent(self, event):
        self.reject()

    def parse_url(self, url):
        print('Fetching dash results from: {}'.format(url))
        page = urllib.request.urlopen(url)
        soup = BeautifulSoup(page, "lxml")

        # build up list of rendered images
        measurement_img = [img for img in soup.find_all('img') if
                           img.get('alt') and img.get('alt').startswith('Rendered Image')]

        images = {}
        for img in measurement_img:
            m = re.search('Rendered Image (.*?)(\s|$)', img.get('alt'))
            test_name = m.group(1)
            rendered_image = img.get('src')
            images[test_name] = '{}/{}'.format(dash_url, rendered_image)

        if images:
            print('found images:\n{}'.format(images))
        else:
            print('no images found\n')
        self.images = images
        self.load_next()

    def load_next(self):
        if not self.images:
            # all done
            self.accept()
            exit(0)

        test_name, rendered_image = self.images.popitem()
        self.test_name_label.setText(test_name)
        control_image = self.get_control_image_path(test_name)
        if not control_image:
            self.load_next()
            return

        self.mask_image_path = control_image[:-4] + '_mask.png'
        self.load_images(control_image, rendered_image, self.mask_image_path)

    def load_images(self, control_image_path, rendered_image_path, mask_image_path):
        self.control_image = imageFromPath(control_image_path)
        if not self.control_image:
            error('Could not read control image {}'.format(control_image_path))

        self.rendered_image = imageFromPath(rendered_image_path)
        if not self.rendered_image:
            error(
                'Could not read rendered image {}'.format(rendered_image_path))
        if not self.rendered_image.width() == self.control_image.width() or not self.rendered_image.height() == self.control_image.height():
            print(
                'Size mismatch - control image is {}x{}, rendered image is {}x{}'.format(self.control_image.width(),
                                                                                         self.control_image.height(
                ),
                    self.rendered_image.width(
                ),
                    self.rendered_image.height()))

        max_width = min(
            self.rendered_image.width(), self.control_image.width())
        max_height = min(
            self.rendered_image.height(), self.control_image.height())

        # read current mask, if it exist
        self.mask_image = imageFromPath(mask_image_path)
        if self.mask_image.isNull():
            print(
                'Mask image does not exist, creating {}'.format(mask_image_path))
            self.mask_image = QImage(
                self.control_image.width(), self.control_image.height(), QImage.Format_ARGB32)
            self.mask_image.fill(QColor(0, 0, 0))

        self.diff_image = self.create_diff_image(
            self.control_image, self.rendered_image, self.mask_image)
        if not self.diff_image:
            self.load_next()
            return

        self.control_label.setPixmap(QPixmap.fromImage(self.control_image))
        self.rendered_label.setPixmap(QPixmap.fromImage(self.rendered_image))
        self.mask_label.setPixmap(QPixmap.fromImage(self.mask_image))
        self.diff_label.setPixmap(QPixmap.fromImage(self.diff_image))
        self.preview_mask()

    def preview_mask(self):
        self.new_mask_image = self.create_mask(
            self.control_image, self.rendered_image, self.mask_image, self.overload_spin.value())
        self.new_mask_label.setPixmap(QPixmap.fromImage(self.new_mask_image))

    def save_mask(self):
        self.new_mask_image.save(self.mask_image_path, "png")
        self.load_next()

    def create_mask(self, control_image, rendered_image, mask_image, overload=1):
        max_width = min(rendered_image.width(), control_image.width())
        max_height = min(rendered_image.height(), control_image.height())

        new_mask_image = QImage(
            control_image.width(), control_image.height(), QImage.Format_ARGB32)
        new_mask_image.fill(QColor(0, 0, 0))

        # loop through pixels in rendered image and compare
        mismatch_count = 0
        linebytes = max_width * 4
        for y in range(max_height):
            control_scanline = control_image.constScanLine(
                y).asstring(linebytes)
            rendered_scanline = rendered_image.constScanLine(
                y).asstring(linebytes)
            mask_scanline = mask_image.scanLine(y).asstring(linebytes)

            for x in range(max_width):
                currentTolerance = qRed(
                    struct.unpack('I', mask_scanline[x * 4:x * 4 + 4])[0])

                if currentTolerance == 255:
                    # ignore pixel
                    new_mask_image.setPixel(
                        x, y, qRgb(currentTolerance, currentTolerance, currentTolerance))
                    continue

                expected_rgb = struct.unpack(
                    'I', control_scanline[x * 4:x * 4 + 4])[0]
                rendered_rgb = struct.unpack(
                    'I', rendered_scanline[x * 4:x * 4 + 4])[0]
                difference = min(
                    255, colorDiff(expected_rgb, rendered_rgb) * overload)

                if difference > currentTolerance:
                    # update mask image
                    new_mask_image.setPixel(
                        x, y, qRgb(difference, difference, difference))
                    mismatch_count += 1
                else:
                    new_mask_image.setPixel(
                        x, y, qRgb(currentTolerance, currentTolerance, currentTolerance))
        return new_mask_image

    def get_control_image_path(self, test_name):
        if os.path.isfile(test_name):
            return path

        # else try and find matching test image
        script_folder = os.path.dirname(os.path.realpath(sys.argv[0]))
        control_images_folder = os.path.join(
            script_folder, '../tests/testdata/control_images')

        matching_control_images = [x[0]
                                   for x in os.walk(control_images_folder) if test_name in x[0]]
        if len(matching_control_images) > 1:
            QMessageBox.warning(
                self, 'Result', 'Found multiple matching control images for {}'.format(test_name))
            return None
        elif len(matching_control_images) == 0:
            QMessageBox.warning(
                self, 'Result', 'No matching control images found for {}'.format(test_name))
            return None

        found_control_image_path = matching_control_images[0]

        # check for a single matching expected image
        images = glob.glob(os.path.join(found_control_image_path, '*.png'))
        filtered_images = [i for i in images if not i[-9:] == '_mask.png']
        if len(filtered_images) > 1:
            error(
                'Found multiple matching control images for {}'.format(test_name))
        elif len(filtered_images) == 0:
            error('No matching control images found for {}'.format(test_name))

        found_image = filtered_images[0]
        print('Found matching control image: {}'.format(found_image))
        return found_image

    def create_diff_image(self, control_image, rendered_image, mask_image):
        # loop through pixels in rendered image and compare
        mismatch_count = 0
        max_width = min(rendered_image.width(), control_image.width())
        max_height = min(rendered_image.height(), control_image.height())
        linebytes = max_width * 4

        diff_image = QImage(
            control_image.width(), control_image.height(), QImage.Format_ARGB32)
        diff_image.fill(QColor(152, 219, 249))

        for y in range(max_height):
            control_scanline = control_image.constScanLine(
                y).asstring(linebytes)
            rendered_scanline = rendered_image.constScanLine(
                y).asstring(linebytes)
            mask_scanline = mask_image.scanLine(y).asstring(linebytes)

            for x in range(max_width):
                currentTolerance = qRed(
                    struct.unpack('I', mask_scanline[x * 4:x * 4 + 4])[0])

                if currentTolerance == 255:
                    # ignore pixel
                    continue

                expected_rgb = struct.unpack(
                    'I', control_scanline[x * 4:x * 4 + 4])[0]
                rendered_rgb = struct.unpack(
                    'I', rendered_scanline[x * 4:x * 4 + 4])[0]
                difference = colorDiff(expected_rgb, rendered_rgb)

                if difference > currentTolerance:
                    # update mask image
                    diff_image.setPixel(x, y, qRgb(255, 0, 0))
                    mismatch_count += 1

        if mismatch_count:
            return diff_image
        else:
            print('No mismatches')
            return None


def main():
    app = QApplication(sys.argv)

    parser = argparse.ArgumentParser()
    parser.add_argument('dash_url')
    args = parser.parse_args()

    w = ResultHandler()
    w.parse_url(args.dash_url)
    w.exec_()


if __name__ == '__main__':
    main()
