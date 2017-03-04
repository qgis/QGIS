# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsMapCanvas

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '24/1/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.core import (QgsMapSettings,
                       QgsCoordinateReferenceSystem,
                       QgsRectangle,
                       QgsVectorLayer,
                       QgsFeature,
                       QgsGeometry,
                       QgsMultiRenderChecker,
                       QgsApplication)
from qgis.gui import (QgsMapCanvas)

from qgis.PyQt.QtCore import (Qt,
                              QDir)
import time
from qgis.testing import start_app, unittest

app = start_app()


class TestQgsMapCanvas(unittest.TestCase):

    def setUp(self):
        self.report = "<h1>Python QgsMapCanvas Tests</h1>\n"

    def tearDown(self):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(self.report)

    def testDeferredUpdate(self):
        """ test that map canvas doesn't auto refresh on deferred layer update """
        canvas = QgsMapCanvas()
        canvas.setDestinationCrs(QgsCoordinateReferenceSystem(4326))
        canvas.setFrameStyle(0)
        canvas.resize(600, 400)
        self.assertEqual(canvas.width(), 600)
        self.assertEqual(canvas.height(), 400)

        layer = QgsVectorLayer("Polygon?crs=epsg:4326&field=fldtxt:string",
                               "layer", "memory")

        canvas.setLayers([layer])
        canvas.setExtent(QgsRectangle(10, 30, 20, 35))
        canvas.show()

        # need to wait until first redraw can occur (note that we first need to wait till drawing starts!)
        while not canvas.isDrawing():
            app.processEvents()
        while canvas.isDrawing():
            app.processEvents()

        self.assertTrue(self.canvasImageCheck('empty_canvas', 'empty_canvas', canvas))

        # add polygon to layer
        f = QgsFeature()
        f.setGeometry(QgsGeometry.fromRect(QgsRectangle(5, 25, 25, 45)))
        self.assertTrue(layer.dataProvider().addFeatures([f]))

        # deferred update - so expect that canvas will not been refreshed
        layer.triggerRepaint(True)
        timeout = time.time() + 0.1
        while time.time() < timeout:
            # messy, but only way to check that canvas redraw doesn't occur
            self.assertFalse(canvas.isDrawing())
        # canvas should still be empty
        self.assertTrue(self.canvasImageCheck('empty_canvas', 'empty_canvas', canvas))

        # refresh canvas
        canvas.refresh()
        while not canvas.isDrawing():
            app.processEvents()
        while canvas.isDrawing():
            app.processEvents()

        # now we expect the canvas check to fail (since they'll be a new polygon rendered over it)
        self.assertFalse(self.canvasImageCheck('empty_canvas', 'empty_canvas', canvas))

    def testRefreshOnTimer(self):
        """ test that map canvas refreshes with auto refreshing layers """
        canvas = QgsMapCanvas()
        canvas.setDestinationCrs(QgsCoordinateReferenceSystem(4326))
        canvas.setFrameStyle(0)
        canvas.resize(600, 400)
        self.assertEqual(canvas.width(), 600)
        self.assertEqual(canvas.height(), 400)

        layer = QgsVectorLayer("Polygon?crs=epsg:4326&field=fldtxt:string",
                               "layer", "memory")

        canvas.setLayers([layer])
        canvas.setExtent(QgsRectangle(10, 30, 20, 35))
        canvas.show()

        # need to wait until first redraw can occur (note that we first need to wait till drawing starts!)
        while not canvas.isDrawing():
            app.processEvents()
        while canvas.isDrawing():
            app.processEvents()

        self.assertTrue(self.canvasImageCheck('empty_canvas', 'empty_canvas', canvas))

        # add polygon to layer
        f = QgsFeature()
        f.setGeometry(QgsGeometry.fromRect(QgsRectangle(5, 25, 25, 45)))
        self.assertTrue(layer.dataProvider().addFeatures([f]))

        # set auto refresh on layer
        layer.setAutoRefreshInterval(100)
        layer.setAutoRefreshEnabled(True)

        timeout = time.time() + 1
        # expect canvas to auto refresh...
        while not canvas.isDrawing():
            app.processEvents()
            self.assertTrue(time.time() < timeout)
        while canvas.isDrawing():
            app.processEvents()
            self.assertTrue(time.time() < timeout)

        # add a polygon to layer
        f = QgsFeature()
        f.setGeometry(QgsGeometry.fromRect(QgsRectangle(5, 25, 25, 45)))
        self.assertTrue(layer.dataProvider().addFeatures([f]))
        # wait for canvas auto refresh
        while not canvas.isDrawing():
            app.processEvents()
            self.assertTrue(time.time() < timeout)
        while canvas.isDrawing():
            app.processEvents()
            self.assertTrue(time.time() < timeout)

        # now canvas should look different...
        self.assertFalse(self.canvasImageCheck('empty_canvas', 'empty_canvas', canvas))

        # switch off auto refresh
        layer.setAutoRefreshEnabled(False)
        timeout = time.time() + 0.5
        while time.time() < timeout:
            # messy, but only way to check that canvas redraw doesn't occur
            self.assertFalse(canvas.isDrawing())

    def testCancelAndDestroy(self):
        """ test that nothing goes wrong if we destroy a canvas while a job is canceling """
        canvas = QgsMapCanvas()
        canvas.setDestinationCrs(QgsCoordinateReferenceSystem(4326))
        canvas.setFrameStyle(0)
        canvas.resize(600, 400)

        layer = QgsVectorLayer("Polygon?crs=epsg:4326&field=fldtxt:string",
                               "layer", "memory")

        # add a ton of features
        for i in range(5000):
            f = QgsFeature()
            f.setGeometry(QgsGeometry.fromRect(QgsRectangle(5, 25, 25, 45)))
            self.assertTrue(layer.dataProvider().addFeatures([f]))

        canvas.setLayers([layer])
        canvas.setExtent(QgsRectangle(10, 30, 20, 35))
        canvas.show()

        # need to wait until first redraw can occur (note that we first need to wait till drawing starts!)
        while not canvas.isDrawing():
            app.processEvents()
        self.assertTrue(canvas.isDrawing())

        canvas.stopRendering()
        del canvas

    def canvasImageCheck(self, name, reference_image, canvas):
        self.report += "<h2>Render {}</h2>\n".format(name)
        temp_dir = QDir.tempPath() + '/'
        file_name = temp_dir + 'mapcanvas_' + name + ".png"
        print(file_name)
        canvas.saveAsImage(file_name)
        checker = QgsMultiRenderChecker()
        checker.setControlPathPrefix("mapcanvas")
        checker.setControlName("expected_" + reference_image)
        checker.setRenderedImage(file_name)
        checker.setColorTolerance(2)
        result = checker.runTest(name, 20)
        self.report += checker.report()
        print((self.report))
        return result


if __name__ == '__main__':
    unittest.main()
