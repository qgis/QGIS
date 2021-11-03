# -*- coding: utf-8 -*-
'''
qgslayoutchecker.py - check rendering of QgsLayout against an expected image
 --------------------------------------
  Date                 : 31 Juli 2012
  Copyright            : (C) 2012 by Dr. Horst Düster / Dr. Marco Hugentobler
  email                : horst.duester@sourcepole.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
'''

import qgis  # NOQA

from qgis.PyQt.QtCore import QSize, QDir, QFileInfo
from qgis.PyQt.QtGui import QImage, QPainter
from qgis.core import QgsMultiRenderChecker, QgsLayoutExporter


class QgsLayoutChecker(QgsMultiRenderChecker):

    def __init__(self, test_name, layout):
        super(QgsLayoutChecker, self).__init__()
        self.layout = layout
        self.test_name = test_name
        self.dots_per_meter = 96 / 25.4 * 1000
        self.size = QSize(1122, 794)
        self.setColorTolerance(5)

    def testLayout(self, page=0, pixelDiff=0):
        if self.layout is None:
            myMessage = "Layout not valid"
            return False, myMessage

        # load expected image
        self.setControlName("expected_" + self.test_name)

        # get width/height, create image and render the composition to it
        outputImage = QImage(self.size, QImage.Format_RGB32)

        outputImage.setDotsPerMeterX(int(self.dots_per_meter))
        outputImage.setDotsPerMeterY(int(self.dots_per_meter))
        QgsMultiRenderChecker.drawBackground(outputImage)
        p = QPainter(outputImage)
        exporter = QgsLayoutExporter(self.layout)
        exporter.renderPage(p, page)
        p.end()

        renderedFilePath = QDir.tempPath() + QDir.separator() + QFileInfo(self.test_name).baseName() + "_rendered.png"
        outputImage.save(renderedFilePath, "PNG")

        self.setRenderedImage(renderedFilePath)

        testResult = self.runTest(self.test_name, pixelDiff)

        return testResult, self.report()
