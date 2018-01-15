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
                       QgsFillSymbol,
                       QgsSingleSymbolRenderer,
                       QgsMapThemeCollection,
                       QgsProject,
                       QgsApplication)
from qgis.gui import (QgsMapCanvas)

from qgis.PyQt.QtCore import (Qt,
                              QDir)
from qgis.PyQt.QtXml import (QDomDocument, QDomElement)
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

    def testGettersSetters(self):
        canvas = QgsMapCanvas()

        # should be disabled by default
        self.assertFalse(canvas.previewJobsEnabled())
        canvas.setPreviewJobsEnabled(True)
        self.assertTrue(canvas.previewJobsEnabled())

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
        canvas.waitWhileRendering()
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
        canvas.waitWhileRendering()

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
        canvas.waitWhileRendering()
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

    def testMapTheme(self):
        canvas = QgsMapCanvas()
        canvas.setDestinationCrs(QgsCoordinateReferenceSystem(4326))
        canvas.setFrameStyle(0)
        canvas.resize(600, 400)
        self.assertEqual(canvas.width(), 600)
        self.assertEqual(canvas.height(), 400)

        layer = QgsVectorLayer("Polygon?crs=epsg:4326&field=fldtxt:string",
                               "layer", "memory")
        # add a polygon to layer
        f = QgsFeature()
        f.setGeometry(QgsGeometry.fromRect(QgsRectangle(5, 25, 25, 45)))
        self.assertTrue(layer.dataProvider().addFeatures([f]))

        # create a style
        sym1 = QgsFillSymbol.createSimple({'color': '#ffb200'})
        renderer = QgsSingleSymbolRenderer(sym1)
        layer.setRenderer(renderer)

        canvas.setLayers([layer])
        canvas.setExtent(QgsRectangle(10, 30, 20, 35))
        canvas.show()

        # need to wait until first redraw can occur (note that we first need to wait till drawing starts!)
        while not canvas.isDrawing():
            app.processEvents()
        canvas.waitWhileRendering()
        self.assertTrue(self.canvasImageCheck('theme1', 'theme1', canvas))

        # add some styles
        layer.styleManager().addStyleFromLayer('style1')
        sym2 = QgsFillSymbol.createSimple({'color': '#00b2ff'})
        renderer2 = QgsSingleSymbolRenderer(sym2)
        layer.setRenderer(renderer2)
        layer.styleManager().addStyleFromLayer('style2')

        canvas.refresh()
        canvas.waitWhileRendering()
        self.assertTrue(self.canvasImageCheck('theme2', 'theme2', canvas))

        layer.styleManager().setCurrentStyle('style1')
        canvas.refresh()
        canvas.waitWhileRendering()
        self.assertTrue(self.canvasImageCheck('theme1', 'theme1', canvas))

        # OK, so all good with setting/rendering map styles
        # try setting canvas to a particular theme

        # make some themes...
        theme1 = QgsMapThemeCollection.MapThemeRecord()
        record1 = QgsMapThemeCollection.MapThemeLayerRecord(layer)
        record1.currentStyle = 'style1'
        record1.usingCurrentStyle = True
        theme1.setLayerRecords([record1])

        theme2 = QgsMapThemeCollection.MapThemeRecord()
        record2 = QgsMapThemeCollection.MapThemeLayerRecord(layer)
        record2.currentStyle = 'style2'
        record2.usingCurrentStyle = True
        theme2.setLayerRecords([record2])

        QgsProject.instance().mapThemeCollection().insert('theme1', theme1)
        QgsProject.instance().mapThemeCollection().insert('theme2', theme2)

        canvas.setTheme('theme2')
        canvas.refresh()
        canvas.waitWhileRendering()
        self.assertTrue(self.canvasImageCheck('theme2', 'theme2', canvas))

        canvas.setTheme('theme1')
        canvas.refresh()
        canvas.waitWhileRendering()
        self.assertTrue(self.canvasImageCheck('theme1', 'theme1', canvas))

        # add another layer
        layer2 = QgsVectorLayer("Polygon?crs=epsg:4326&field=fldtxt:string",
                                "layer2", "memory")
        f = QgsFeature()
        f.setGeometry(QgsGeometry.fromRect(QgsRectangle(5, 25, 25, 45)))
        self.assertTrue(layer2.dataProvider().addFeatures([f]))

        # create a style
        sym1 = QgsFillSymbol.createSimple({'color': '#b2ff00'})
        renderer = QgsSingleSymbolRenderer(sym1)
        layer2.setRenderer(renderer)

        # rerender canvas - should NOT show new layer
        canvas.refresh()
        canvas.waitWhileRendering()
        self.assertTrue(self.canvasImageCheck('theme1', 'theme1', canvas))
        # test again - this time refresh all layers
        canvas.refreshAllLayers()
        canvas.waitWhileRendering()
        self.assertTrue(self.canvasImageCheck('theme1', 'theme1', canvas))

        # add layer 2 to theme1
        record3 = QgsMapThemeCollection.MapThemeLayerRecord(layer2)
        theme1.setLayerRecords([record3])
        QgsProject.instance().mapThemeCollection().update('theme1', theme1)

        canvas.refresh()
        canvas.waitWhileRendering()
        self.assertTrue(self.canvasImageCheck('theme3', 'theme3', canvas))

        # change the appearance of an active style
        layer2.styleManager().addStyleFromLayer('original')
        layer2.styleManager().addStyleFromLayer('style4')
        record3.currentStyle = 'style4'
        record3.usingCurrentStyle = True
        theme1.setLayerRecords([record3])
        QgsProject.instance().mapThemeCollection().update('theme1', theme1)

        canvas.refresh()
        canvas.waitWhileRendering()
        self.assertTrue(self.canvasImageCheck('theme3', 'theme3', canvas))

        layer2.styleManager().setCurrentStyle('style4')
        sym3 = QgsFillSymbol.createSimple({'color': '#b200b2'})
        layer2.renderer().setSymbol(sym3)
        canvas.refresh()
        canvas.waitWhileRendering()
        self.assertTrue(self.canvasImageCheck('theme4', 'theme4', canvas))

        # try setting layers while a theme is in place
        canvas.setLayers([layer])
        canvas.refresh()

        # should be no change... setLayers should be ignored if canvas is following a theme!
        canvas.waitWhileRendering()
        self.assertTrue(self.canvasImageCheck('theme4', 'theme4', canvas))

        # setLayerStyleOverrides while theme is in place
        canvas.setLayerStyleOverrides({layer2.id(): 'original'})
        # should be no change... setLayerStyleOverrides should be ignored if canvas is following a theme!
        canvas.refresh()
        canvas.waitWhileRendering()
        self.assertTrue(self.canvasImageCheck('theme4', 'theme4', canvas))

        # clear theme
        canvas.setTheme('')
        canvas.refresh()
        canvas.waitWhileRendering()
        # should be different - we should now render project layers
        self.assertFalse(self.canvasImageCheck('theme4', 'theme4', canvas))

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

    def testSaveMultipleCanvasesToProject(self):
        # test saving/restoring canvas state to project with multiple canvases
        c1 = QgsMapCanvas()
        c1.setObjectName('c1')
        c1.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:3111'))
        c1.setRotation(45)
        c2 = QgsMapCanvas()
        c2.setObjectName('c2')
        c2.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        c2.setRotation(65)

        doc = QDomDocument("testdoc")
        elem = doc.createElement("qgis")
        doc.appendChild(elem)
        c1.writeProject(doc)
        c2.writeProject(doc)

        c3 = QgsMapCanvas()
        c3.setObjectName('c1')
        c4 = QgsMapCanvas()
        c4.setObjectName('c2')
        c3.readProject(doc)
        c4.readProject(doc)

        self.assertEqual(c3.mapSettings().destinationCrs().authid(), 'EPSG:3111')
        self.assertEqual(c3.rotation(), 45)
        self.assertEqual(c4.mapSettings().destinationCrs().authid(), 'EPSG:4326')
        self.assertEqual(c4.rotation(), 65)


if __name__ == '__main__':
    unittest.main()
