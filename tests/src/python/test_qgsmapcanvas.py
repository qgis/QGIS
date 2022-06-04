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

import qgis  # NOQA

from qgis.PyQt.QtCore import (
    QDate,
    QTime,
    QDateTime,
    QDir
)

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
                       QgsProject, QgsAnnotationPolygonItem,
                       QgsPolygon,
                       QgsLineString,
                       QgsPoint,
                       QgsPointXY,
                       QgsApplication,
                       QgsAnnotationLayer,
                       QgsAnnotationLineItem,
                       QgsAnnotationMarkerItem,
                       QgsTemporalController,
                       QgsTemporalNavigationObject,
                       QgsDateTimeRange,
                       QgsInterval
                       )
from qgis.gui import (QgsMapCanvas)

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
        canvas.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
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
        canvas.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
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
        canvas.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
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
        canvas.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
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

        # set canvas to theme1
        canvas.setTheme('theme1')
        canvas.refresh()
        canvas.waitWhileRendering()
        self.assertEqual(canvas.theme(), 'theme1')
        themeLayers = theme1.layerRecords()
        # rename the active theme
        QgsProject.instance().mapThemeCollection().renameMapTheme('theme1', 'theme5')
        # canvas theme should now be set to theme5
        canvas.refresh()
        canvas.waitWhileRendering()
        self.assertEqual(canvas.theme(), 'theme5')
        # theme5 should render as theme1
        theme5 = QgsProject.instance().mapThemeCollection().mapThemeState('theme5')
        theme5Layers = theme5.layerRecords()
        self.assertEqual(themeLayers, theme5Layers, 'themes are different')
        # self.assertTrue(self.canvasImageCheck('theme5', 'theme5', canvas))

    def testMainAnnotationLayerRendered(self):
        """ test that main annotation layer is rendered above all other layers """
        canvas = QgsMapCanvas()
        canvas.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        canvas.setFrameStyle(0)
        canvas.resize(600, 400)
        self.assertEqual(canvas.width(), 600)
        self.assertEqual(canvas.height(), 400)

        layer = QgsVectorLayer("Polygon?crs=epsg:4326&field=fldtxt:string",
                               "layer", "memory")
        sym3 = QgsFillSymbol.createSimple({'color': '#b200b2'})
        layer.renderer().setSymbol(sym3)

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

        # refresh canvas
        canvas.refresh()
        canvas.waitWhileRendering()

        # no annotation yet...
        self.assertFalse(self.canvasImageCheck('main_annotation_layer', 'main_annotation_layer', canvas))

        annotation_layer = QgsProject.instance().mainAnnotationLayer()
        annotation_layer.setCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        annotation_geom = QgsGeometry.fromRect(QgsRectangle(12, 30, 18, 33))
        annotation = QgsAnnotationPolygonItem(annotation_geom.constGet().clone())
        sym3 = QgsFillSymbol.createSimple({'color': '#ff0000', 'outline_style': 'no'})
        annotation.setSymbol(sym3)
        annotation_layer.addItem(annotation)

        # refresh canvas
        canvas.refresh()
        canvas.waitWhileRendering()

        # annotation must be rendered over other layers
        self.assertTrue(self.canvasImageCheck('main_annotation_layer', 'main_annotation_layer', canvas))
        annotation_layer.clear()

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

    def testSaveCanvasVariablesToProject(self):
        """
        Ensure that temporary canvas atlas variables are not written to project
        """
        c1 = QgsMapCanvas()
        c1.setObjectName('c1')
        c1.expressionContextScope().setVariable('atlas_featurenumber', 1111)
        c1.expressionContextScope().setVariable('atlas_pagename', 'bb')
        c1.expressionContextScope().setVariable('atlas_feature', QgsFeature(1))
        c1.expressionContextScope().setVariable('atlas_featureid', 22)
        c1.expressionContextScope().setVariable('atlas_geometry', QgsGeometry.fromWkt('Point( 1 2 )'))
        c1.expressionContextScope().setVariable('vara', 1111)
        c1.expressionContextScope().setVariable('varb', 'bb')

        doc = QDomDocument("testdoc")
        elem = doc.createElement("qgis")
        doc.appendChild(elem)
        c1.writeProject(doc)

        c2 = QgsMapCanvas()
        c2.setObjectName('c1')
        c2.readProject(doc)

        self.assertCountEqual(c2.expressionContextScope().variableNames(), ['vara', 'varb'])
        self.assertEqual(c2.expressionContextScope().variable('vara'), 1111)
        self.assertEqual(c2.expressionContextScope().variable('varb'), 'bb')

    def testSaveMultipleCanvasesToProject(self):
        # test saving/restoring canvas state to project with multiple canvases
        c1 = QgsMapCanvas()
        c1.setObjectName('c1')
        c1.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:3111'))
        c1.setRotation(45)
        c1.expressionContextScope().setVariable('vara', 1111)
        c1.expressionContextScope().setVariable('varb', 'bb')
        c2 = QgsMapCanvas()
        c2.setObjectName('c2')
        c2.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        c2.setRotation(65)
        c2.expressionContextScope().setVariable('vara', 2222)
        c2.expressionContextScope().setVariable('varc', 'cc')

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
        self.assertEqual(set(c3.expressionContextScope().variableNames()), {'vara', 'varb'})
        self.assertEqual(c3.expressionContextScope().variable('vara'), 1111)
        self.assertEqual(c3.expressionContextScope().variable('varb'), 'bb')
        self.assertEqual(c4.mapSettings().destinationCrs().authid(), 'EPSG:4326')
        self.assertEqual(c4.rotation(), 65)
        self.assertEqual(set(c4.expressionContextScope().variableNames()), {'vara', 'varc'})
        self.assertEqual(c4.expressionContextScope().variable('vara'), 2222)
        self.assertEqual(c4.expressionContextScope().variable('varc'), 'cc')

    def testLockedScale(self):
        """Test zoom/pan/center operations when scale lock is on"""

        c = QgsMapCanvas()
        dpr = c.mapSettings().devicePixelRatio()
        self.assertEqual(c.size().width(), 640)
        self.assertEqual(c.size().height(), 480)

        c.setExtent(QgsRectangle(5, 45, 9, 47))
        self.assertEqual(round(c.scale() / 100000), 13 * dpr)
        c.zoomScale(2500000)
        c.setScaleLocked(True)
        self.assertEqual(round(c.magnificationFactor(), 1), 1)

        # Test setExtent
        c.setExtent(QgsRectangle(6, 45.5, 8, 46), True)
        self.assertEqual(round(c.scale()), 2500000)
        self.assertEqual(c.center().x(), 7.0)
        self.assertEqual(c.center().y(), 45.75)
        self.assertEqual(round(c.magnificationFactor()), 4 / dpr)

        # Test setCenter
        c.setCenter(QgsPointXY(6, 46))
        self.assertEqual(c.center().x(), 6)
        self.assertEqual(c.center().y(), 46)
        self.assertEqual(round(c.scale()), 2500000)

        # Test zoom
        c.zoomByFactor(0.5, QgsPointXY(6.5, 46.5), False)
        self.assertEqual(c.center().x(), 6.5)
        self.assertEqual(c.center().y(), 46.5)
        self.assertTrue(c.magnificationFactor() > 7 / dpr)
        self.assertEqual(round(c.scale()), 2500000)

        # Test zoom with center
        # default zoom factor is 2, x and y are pixel coordinates, default size is 640x480
        c.zoomWithCenter(300, 200, True)
        self.assertEqual(round(c.center().x(), 1), 6.5)
        self.assertEqual(round(c.center().y(), 1), 46.6)
        self.assertEqual(round(c.scale()), 2500000)
        self.assertTrue(c.magnificationFactor() > (14 / dpr) and c.magnificationFactor() < (16 / dpr))
        # out ...
        c.zoomWithCenter(300, 200, False)
        self.assertEqual(round(c.center().x(), 1), 6.5)
        self.assertEqual(round(c.center().y(), 1), 46.6)
        self.assertEqual(round(c.scale()), 2500000)
        self.assertTrue(c.magnificationFactor() > 7 / dpr)

        # Test setExtent with different ratio
        c2 = QgsMapCanvas()
        c2.setExtent(QgsRectangle(5, 45, 9, 47))
        c2.zoomScale(2500000)
        c2.setScaleLocked(True)

        c2.setExtent(QgsRectangle(3, 45, 11, 45.5), True)
        self.assertEqual(round(c2.scale()), 2500000)
        self.assertEqual(c2.center().x(), 7.0)
        self.assertEqual(c2.center().y(), 45.25)
        self.assertAlmostEqual(c2.magnificationFactor(), 1 / dpr, 0)

        # Restore original
        c2.setExtent(QgsRectangle(5, 45, 9, 47), True)
        self.assertEqual(round(c2.scale()), 2500000)
        self.assertEqual(c2.center().x(), 7.0)
        self.assertEqual(c2.center().y(), 46.0)
        self.assertAlmostEqual(c2.magnificationFactor(), 2 / dpr, 0)

        c2.setExtent(QgsRectangle(7, 46, 11, 46.5), True)
        self.assertEqual(round(c2.scale()), 2500000)
        self.assertEqual(c2.center().x(), 9.0)
        self.assertEqual(c2.center().y(), 46.25)
        self.assertAlmostEqual(c2.magnificationFactor(), 2 / dpr, 0)

        c2.setExtent(QgsRectangle(7, 46, 9, 46.5), True)
        self.assertEqual(round(c2.scale()), 2500000)
        self.assertEqual(c2.center().x(), 8.0)
        self.assertEqual(c2.center().y(), 46.25)
        self.assertAlmostEqual(c2.magnificationFactor(), 4 / dpr, 0)

    def test_rendered_items(self):
        canvas = QgsMapCanvas()
        canvas.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        canvas.setFrameStyle(0)
        canvas.resize(600, 400)
        canvas.setCachingEnabled(True)
        self.assertEqual(canvas.width(), 600)
        self.assertEqual(canvas.height(), 400)

        layer = QgsAnnotationLayer('test', QgsAnnotationLayer.LayerOptions(QgsProject.instance().transformContext()))
        self.assertTrue(layer.isValid())
        layer2 = QgsAnnotationLayer('test', QgsAnnotationLayer.LayerOptions(QgsProject.instance().transformContext()))
        self.assertTrue(layer2.isValid())

        item = QgsAnnotationPolygonItem(
            QgsPolygon(QgsLineString([QgsPoint(11.5, 13), QgsPoint(12, 13), QgsPoint(12, 13.5), QgsPoint(11.5, 13)])))
        item.setSymbol(
            QgsFillSymbol.createSimple({'color': '200,100,100', 'outline_color': 'black', 'outline_width': '2'}))
        item.setZIndex(1)
        i1_id = layer.addItem(item)

        item = QgsAnnotationLineItem(QgsLineString([QgsPoint(11, 13), QgsPoint(12, 13), QgsPoint(12, 15)]))
        item.setZIndex(2)
        i2_id = layer.addItem(item)

        item = QgsAnnotationMarkerItem(QgsPoint(12, 13))
        item.setZIndex(3)
        i3_id = layer2.addItem(item)

        layer.setCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        layer2.setCrs(QgsCoordinateReferenceSystem('EPSG:4326'))

        canvas.setLayers([layer, layer2])
        canvas.setExtent(QgsRectangle(10, 10, 18, 18))
        canvas.show()

        # need to wait until first redraw can occur (note that we first need to wait till drawing starts!)
        while not canvas.isDrawing():
            app.processEvents()
        canvas.waitWhileRendering()

        results = canvas.renderedItemResults()
        self.assertCountEqual([i.itemId() for i in results.renderedItems()], [i1_id, i2_id, i3_id])

        # turn off a layer -- the other layer will be rendered direct from the cached version
        canvas.setLayers([layer2])
        while not canvas.isDrawing():
            app.processEvents()
        canvas.waitWhileRendering()

        results = canvas.renderedItemResults()
        # only layer2 items should be present in results -- but these MUST be present while layer2 is visible in the canvas,
        # even though the most recent canvas redraw used a cached version of layer2 and didn't actually have to redraw the layer
        self.assertEqual([i.itemId() for i in results.renderedItems()], [i3_id])

        # turn layer 1 back on
        canvas.setLayers([layer, layer2])
        while not canvas.isDrawing():
            app.processEvents()
        canvas.waitWhileRendering()

        results = canvas.renderedItemResults()
        # both layer1 and layer2 items should be present in results -- even though NEITHER of these layers were re-rendered,
        # and instead we used precached renders of both layers
        self.assertCountEqual([i.itemId() for i in results.renderedItems()], [i1_id, i2_id, i3_id])

    def test_rendered_item_results_remove_outdated(self):
        """
        Test that outdated results are removed from rendered item result caches
        """
        canvas = QgsMapCanvas()
        canvas.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        canvas.setFrameStyle(0)
        canvas.resize(600, 400)
        canvas.setCachingEnabled(True)
        self.assertEqual(canvas.width(), 600)
        self.assertEqual(canvas.height(), 400)

        layer = QgsAnnotationLayer('test', QgsAnnotationLayer.LayerOptions(QgsProject.instance().transformContext()))
        self.assertTrue(layer.isValid())
        layer2 = QgsAnnotationLayer('test', QgsAnnotationLayer.LayerOptions(QgsProject.instance().transformContext()))
        self.assertTrue(layer2.isValid())

        item = QgsAnnotationPolygonItem(
            QgsPolygon(QgsLineString([QgsPoint(11.5, 13), QgsPoint(12, 13), QgsPoint(12, 13.5), QgsPoint(11.5, 13)])))
        item.setSymbol(
            QgsFillSymbol.createSimple({'color': '200,100,100', 'outline_color': 'black', 'outline_width': '2'}))
        item.setZIndex(1)
        i1_id = layer.addItem(item)

        item = QgsAnnotationLineItem(QgsLineString([QgsPoint(11, 13), QgsPoint(12, 13), QgsPoint(12, 15)]))
        item.setZIndex(2)
        i2_id = layer.addItem(item)

        item = QgsAnnotationMarkerItem(QgsPoint(12, 13))
        item.setZIndex(3)
        i3_id = layer2.addItem(item)

        layer.setCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        layer2.setCrs(QgsCoordinateReferenceSystem('EPSG:4326'))

        canvas.setLayers([layer, layer2])
        canvas.setExtent(QgsRectangle(10, 10, 18, 18))
        canvas.show()

        # need to wait until first redraw can occur (note that we first need to wait till drawing starts!)
        while not canvas.isDrawing():
            app.processEvents()
        canvas.waitWhileRendering()

        results = canvas.renderedItemResults()
        self.assertCountEqual([i.itemId() for i in results.renderedItems()], [i1_id, i2_id, i3_id])

        # now try modifying an annotation in the layer -- it will redraw, and we don't want to reuse any previously
        # cached rendered item results for this layer!

        item = QgsAnnotationPolygonItem(
            QgsPolygon(QgsLineString([QgsPoint(11.5, 13), QgsPoint(12.5, 13), QgsPoint(12.5, 13.5), QgsPoint(11.5, 13)])))
        item.setZIndex(1)
        layer.replaceItem(i1_id, item)
        while not canvas.isDrawing():
            app.processEvents()
        canvas.waitWhileRendering()

        item = QgsAnnotationMarkerItem(QgsPoint(17, 18))
        item.setZIndex(3)
        layer2.replaceItem(i3_id, item)
        while not canvas.isDrawing():
            app.processEvents()
        canvas.waitWhileRendering()

        results = canvas.renderedItemResults()
        items_in_bounds = results.renderedAnnotationItemsInBounds(QgsRectangle(10, 10, 15, 15))
        self.assertCountEqual([i.itemId() for i in items_in_bounds], [i1_id, i2_id])

        items_in_bounds = results.renderedAnnotationItemsInBounds(QgsRectangle(15, 15, 20, 20))
        self.assertCountEqual([i.itemId() for i in items_in_bounds], [i3_id])

    def test_temporal_animation(self):
        """
        Test temporal animation logic
        """
        canvas = QgsMapCanvas()
        self.assertEqual(canvas.mapSettings().frameRate(), -1)
        self.assertEqual(canvas.mapSettings().currentFrame(), -1)

        controller = QgsTemporalController()
        canvas.setTemporalController(controller)
        controller.updateTemporalRange.emit(QgsDateTimeRange(QDateTime(QDate(2020, 1, 2), QTime(1, 2, 3)),
                                                             QDateTime(QDate(2020, 1, 4), QTime(1, 2, 3))))
        # should be no change
        self.assertEqual(canvas.mapSettings().frameRate(), -1)
        self.assertEqual(canvas.mapSettings().currentFrame(), -1)

        temporal_no = QgsTemporalNavigationObject()
        temporal_no.setTemporalExtents(QgsDateTimeRange(QDateTime(QDate(2020, 1, 2), QTime(1, 2, 3)),
                                                        QDateTime(QDate(2020, 1, 4), QTime(1, 2, 3))))
        temporal_no.setFrameDuration(QgsInterval(0, 0, 0, 0, 1, 0, 0))

        canvas.setTemporalController(temporal_no)
        controller.updateTemporalRange.emit(QgsDateTimeRange(QDateTime(QDate(2020, 1, 2), QTime(1, 2, 3)),
                                                             QDateTime(QDate(2020, 1, 4), QTime(1, 2, 3))))
        # should be no change
        self.assertEqual(canvas.mapSettings().frameRate(), -1)
        self.assertEqual(canvas.mapSettings().currentFrame(), -1)

        temporal_no.setFramesPerSecond(30)
        temporal_no.pause()
        temporal_no.setCurrentFrameNumber(6)
        canvas.refresh()

        # should be no change - temporal controller is not in animation mode
        self.assertEqual(canvas.mapSettings().frameRate(), -1)
        self.assertEqual(canvas.mapSettings().currentFrame(), -1)

        temporal_no.setNavigationMode(QgsTemporalNavigationObject.Animated)
        self.assertEqual(canvas.mapSettings().frameRate(), 30)
        self.assertEqual(canvas.mapSettings().currentFrame(), 6)

        temporal_no.setCurrentFrameNumber(7)
        self.assertEqual(canvas.mapSettings().frameRate(), 30)
        self.assertEqual(canvas.mapSettings().currentFrame(), 6)

        # switch off animation mode
        temporal_no.setNavigationMode(QgsTemporalNavigationObject.FixedRange)
        self.assertEqual(canvas.mapSettings().frameRate(), -1)
        self.assertEqual(canvas.mapSettings().currentFrame(), -1)

        temporal_no.setNavigationMode(QgsTemporalNavigationObject.Animated)
        self.assertEqual(canvas.mapSettings().frameRate(), 30)
        self.assertEqual(canvas.mapSettings().currentFrame(), 7)

        temporal_no.setNavigationMode(QgsTemporalNavigationObject.NavigationOff)
        self.assertEqual(canvas.mapSettings().frameRate(), -1)
        self.assertEqual(canvas.mapSettings().currentFrame(), -1)


if __name__ == '__main__':
    unittest.main()
