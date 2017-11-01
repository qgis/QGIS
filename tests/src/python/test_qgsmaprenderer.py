# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsMapRenderer.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '1/02/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.core import (QgsMapRendererCache,
                       QgsMapRendererParallelJob,
                       QgsMapRendererSequentialJob,
                       QgsMapRendererCustomPainterJob,
                       QgsPalLayerSettings,
                       QgsRectangle,
                       QgsTextFormat,
                       QgsVectorLayer,
                       QgsVectorLayerSimpleLabeling,
                       QgsFeature,
                       QgsGeometry,
                       QgsMapSettings,
                       QgsPointXY)
from qgis.testing import start_app, unittest
from qgis.PyQt.QtCore import QSize, QThreadPool
from qgis.PyQt.QtGui import QPainter, QImage
from qgis.PyQt.QtTest import QSignalSpy
from random import uniform


app = start_app()


class TestQgsMapRenderer(unittest.TestCase):

    def setUp(self):
        pass

    def tearDown(self):
        # avoid crash on finish, probably related to https://bugreports.qt.io/browse/QTBUG-35760
        QThreadPool.globalInstance().waitForDone()

    def checkRendererUseCachedLabels(self, job_type):
        layer = QgsVectorLayer("Point?field=fldtxt:string",
                               "layer1", "memory")

        settings = QgsMapSettings()
        settings.setExtent(QgsRectangle(5, 25, 25, 45))
        settings.setOutputSize(QSize(600, 400))
        settings.setLayers([layer])

        # no cache
        job = job_type(settings)
        job.start()
        job.waitForFinished()
        self.assertFalse(job.usedCachedLabels())
        self.assertTrue(job.takeLabelingResults())

        # with cache - first run should populate cache
        cache = QgsMapRendererCache()
        job = job_type(settings)
        job.setCache(cache)
        job.start()
        job.waitForFinished()
        self.assertFalse(job.usedCachedLabels())
        self.assertTrue(cache.hasCacheImage('_labels_'))
        self.assertTrue(job.takeLabelingResults())

        # second job should use label cache
        job = job_type(settings)
        job.setCache(cache)
        job.start()
        job.waitForFinished()
        self.assertTrue(job.usedCachedLabels())
        self.assertTrue(cache.hasCacheImage('_labels_'))
        self.assertTrue(job.takeLabelingResults())

        # one last run - no cache
        job = job_type(settings)
        job.start()
        job.waitForFinished()
        self.assertFalse(job.usedCachedLabels())
        self.assertTrue(job.takeLabelingResults())

    def checkRepaintNonLabeledLayerDoesNotInvalidateLabelCache(self, job_type):
        layer = QgsVectorLayer("Point?field=fldtxt:string",
                               "layer1", "memory")
        settings = QgsMapSettings()
        settings.setExtent(QgsRectangle(5, 25, 25, 45))
        settings.setOutputSize(QSize(600, 400))
        settings.setLayers([layer])

        # with cache - first run should populate cache
        cache = QgsMapRendererCache()
        job = job_type(settings)
        job.setCache(cache)
        job.start()
        job.waitForFinished()
        self.assertFalse(job.usedCachedLabels())
        self.assertTrue(cache.hasCacheImage('_labels_'))
        self.assertTrue(cache.hasCacheImage(layer.id()))
        self.assertEqual(cache.dependentLayers('_labels_'), [])

        # trigger repaint on layer - should not invalidate label cache because layer is not labeled
        layer.triggerRepaint()
        self.assertTrue(cache.hasCacheImage('_labels_'))
        self.assertFalse(cache.hasCacheImage(layer.id()))
        self.assertTrue(job.takeLabelingResults())

        # second job should still use label cache
        job = job_type(settings)
        job.setCache(cache)
        job.start()
        job.waitForFinished()
        self.assertTrue(job.usedCachedLabels())
        self.assertTrue(cache.hasCacheImage('_labels_'))
        self.assertTrue(job.takeLabelingResults())

    def checkRepaintLabeledLayerInvalidatesLabelCache(self, job_type):
        layer = QgsVectorLayer("Point?field=fldtxt:string",
                               "layer1", "memory")

        labelSettings = QgsPalLayerSettings()
        labelSettings.fieldName = "fldtxt"
        layer.setLabeling(QgsVectorLayerSimpleLabeling(labelSettings))

        settings = QgsMapSettings()
        settings.setExtent(QgsRectangle(5, 25, 25, 45))
        settings.setOutputSize(QSize(600, 400))
        settings.setLayers([layer])

        # with cache - first run should populate cache
        cache = QgsMapRendererCache()
        job = job_type(settings)
        job.setCache(cache)
        job.start()
        job.waitForFinished()
        self.assertFalse(job.usedCachedLabels())
        self.assertTrue(cache.hasCacheImage('_labels_'))
        self.assertTrue(job.takeLabelingResults())

        self.assertEqual(cache.dependentLayers('_labels_'), [layer])

        # trigger repaint on layer - should invalidate cache and block use of cached labels
        layer.triggerRepaint()
        self.assertFalse(cache.hasCacheImage('_labels_'))

        # second job should not use label cache, since layer was repainted
        job = job_type(settings)
        job.setCache(cache)
        job.start()
        job.waitForFinished()
        # shouldn't use cache
        self.assertFalse(job.usedCachedLabels())
        # but results should have been cached
        self.assertTrue(cache.hasCacheImage('_labels_'))
        self.assertTrue(job.takeLabelingResults())

    def checkAddingNewLabeledLayerInvalidatesLabelCache(self, job_type):
        """ adding a new labeled layer should invalidate any previous label caches"""
        layer = QgsVectorLayer("Point?field=fldtxt:string",
                               "layer1", "memory")

        labelSettings = QgsPalLayerSettings()
        labelSettings.fieldName = "fldtxt"
        layer.setLabeling(QgsVectorLayerSimpleLabeling(labelSettings))

        settings = QgsMapSettings()
        settings.setExtent(QgsRectangle(5, 25, 25, 45))
        settings.setOutputSize(QSize(600, 400))
        settings.setLayers([layer])

        # with cache - first run should populate cache
        cache = QgsMapRendererCache()
        job = job_type(settings)
        job.setCache(cache)
        job.start()
        job.waitForFinished()
        self.assertFalse(job.usedCachedLabels())
        self.assertTrue(cache.hasCacheImage('_labels_'))
        self.assertTrue(job.takeLabelingResults())

        self.assertEqual(cache.dependentLayers('_labels_'), [layer])

        # add another labeled layer
        layer2 = QgsVectorLayer("Point?field=fldtxt:string",
                                "layer2", "memory")
        layer2.setLabeling(QgsVectorLayerSimpleLabeling(labelSettings))
        settings.setLayers([layer, layer2])

        # second job should not be able to use label cache, since a new layer was added
        job = job_type(settings)
        job.setCache(cache)
        job.start()
        job.waitForFinished()
        # shouldn't use cache
        self.assertFalse(job.usedCachedLabels())
        # but results should have been cached
        self.assertTrue(cache.hasCacheImage('_labels_'))
        self.assertEqual(set(cache.dependentLayers('_labels_')), {layer, layer2})
        self.assertTrue(job.takeLabelingResults())

    def checkAddingNewNonLabeledLayerKeepsLabelCache(self, job_type):
        """ adding a new non-labeled layer should keep any previous label caches"""
        layer = QgsVectorLayer("Point?field=fldtxt:string",
                               "layer1", "memory")

        labelSettings = QgsPalLayerSettings()
        labelSettings.fieldName = "fldtxt"
        layer.setLabeling(QgsVectorLayerSimpleLabeling(labelSettings))

        settings = QgsMapSettings()
        settings.setExtent(QgsRectangle(5, 25, 25, 45))
        settings.setOutputSize(QSize(600, 400))
        settings.setLayers([layer])

        # with cache - first run should populate cache
        cache = QgsMapRendererCache()
        job = job_type(settings)
        job.setCache(cache)
        job.start()
        job.waitForFinished()
        self.assertFalse(job.usedCachedLabels())
        self.assertTrue(cache.hasCacheImage('_labels_'))
        self.assertTrue(job.takeLabelingResults())

        self.assertEqual(cache.dependentLayers('_labels_'), [layer])

        # add another, non-labeled layer
        layer2 = QgsVectorLayer("Point?field=fldtxt:string",
                                "layer2", "memory")
        settings.setLayers([layer, layer2])

        # second job should use label cache, since new layer was not labeled
        job = job_type(settings)
        job.setCache(cache)
        job.start()
        job.waitForFinished()
        # should use cache
        self.assertTrue(job.usedCachedLabels())
        # results should have been cached
        self.assertTrue(cache.hasCacheImage('_labels_'))
        self.assertEqual(set(cache.dependentLayers('_labels_')), {layer})
        self.assertTrue(job.takeLabelingResults())

    def checkRemovingLabeledLayerInvalidatesLabelCache(self, job_type):
        """ removing a previously labeled layer should invalidate any previous label caches"""
        layer = QgsVectorLayer("Point?field=fldtxt:string",
                               "layer1", "memory")

        labelSettings = QgsPalLayerSettings()
        labelSettings.fieldName = "fldtxt"
        layer.setLabeling(QgsVectorLayerSimpleLabeling(labelSettings))

        layer2 = QgsVectorLayer("Point?field=fldtxt:string",
                                "layer2", "memory")
        layer2.setLabeling(QgsVectorLayerSimpleLabeling(labelSettings))

        settings = QgsMapSettings()
        settings.setExtent(QgsRectangle(5, 25, 25, 45))
        settings.setOutputSize(QSize(600, 400))
        settings.setLayers([layer, layer2])

        # with cache - first run should populate cache
        cache = QgsMapRendererCache()
        job = job_type(settings)
        job.setCache(cache)
        job.start()
        job.waitForFinished()
        self.assertFalse(job.usedCachedLabels())
        self.assertTrue(cache.hasCacheImage('_labels_'))
        self.assertTrue(job.takeLabelingResults())

        self.assertEqual(set(cache.dependentLayers('_labels_')), {layer, layer2})

        # remove a previously labeled layer
        settings.setLayers([layer2])

        # second job should not be able to use label cache, since a labeled layer was removed
        job = job_type(settings)
        job.setCache(cache)
        job.start()
        job.waitForFinished()
        # shouldn't use cache
        self.assertFalse(job.usedCachedLabels())
        # but results should have been cached
        self.assertTrue(cache.hasCacheImage('_labels_'))
        self.assertEqual(set(cache.dependentLayers('_labels_')), {layer2})
        self.assertTrue(job.takeLabelingResults())

    def checkRemovingNonLabeledLayerKeepsLabelCache(self, job_type):
        """ removing a previously used non-labeled layer should keep any previous label caches"""
        layer = QgsVectorLayer("Point?field=fldtxt:string",
                               "layer1", "memory")

        labelSettings = QgsPalLayerSettings()
        labelSettings.fieldName = "fldtxt"
        layer.setLabeling(QgsVectorLayerSimpleLabeling(labelSettings))

        layer2 = QgsVectorLayer("Point?field=fldtxt:string",
                                "layer2", "memory")

        settings = QgsMapSettings()
        settings.setExtent(QgsRectangle(5, 25, 25, 45))
        settings.setOutputSize(QSize(600, 400))
        settings.setLayers([layer, layer2])

        # with cache - first run should populate cache
        cache = QgsMapRendererCache()
        job = job_type(settings)
        job.setCache(cache)
        job.start()
        job.waitForFinished()
        self.assertFalse(job.usedCachedLabels())
        self.assertTrue(cache.hasCacheImage('_labels_'))
        self.assertTrue(job.takeLabelingResults())

        self.assertEqual(set(cache.dependentLayers('_labels_')), {layer})

        # remove a previously labeled layer
        settings.setLayers([layer])

        # second job should be able to use label cache, since only a non-labeled layer was removed
        job = job_type(settings)
        job.setCache(cache)
        job.start()
        job.waitForFinished()
        # should use cache
        self.assertTrue(job.usedCachedLabels())
        # results should have been cached
        self.assertTrue(cache.hasCacheImage('_labels_'))
        self.assertEqual(set(cache.dependentLayers('_labels_')), {layer})
        self.assertTrue(job.takeLabelingResults())

    def checkLabeledLayerWithBlendModesCannotBeCached(self, job_type):
        """ any labeled layer utilising blending modes cannot be cached"""
        layer = QgsVectorLayer("Point?field=fldtxt:string",
                               "layer1", "memory")

        labelSettings = QgsPalLayerSettings()
        labelSettings.fieldName = "fldtxt"
        layer.setLabeling(QgsVectorLayerSimpleLabeling(labelSettings))

        layer2 = QgsVectorLayer("Point?field=fldtxt:string",
                                "layer2", "memory")
        labelSettings2 = QgsPalLayerSettings()
        labelSettings2.fieldName = "fldtxt"
        format2 = QgsTextFormat()
        format2.setBlendMode(QPainter.CompositionMode_SourceIn)
        labelSettings2.setFormat(format2)
        layer2.setLabeling(QgsVectorLayerSimpleLabeling(labelSettings2))

        settings = QgsMapSettings()
        settings.setExtent(QgsRectangle(5, 25, 25, 45))
        settings.setOutputSize(QSize(600, 400))
        settings.setLayers([layer, layer2])

        # with cache - cache should not be populated!
        cache = QgsMapRendererCache()
        job = job_type(settings)
        job.setCache(cache)
        job.start()
        job.waitForFinished()
        self.assertFalse(job.usedCachedLabels())
        self.assertFalse(cache.hasCacheImage('_labels_'))
        self.assertTrue(job.takeLabelingResults())

        # second job should also not be able to use label cache
        job = job_type(settings)
        job.setCache(cache)
        job.start()
        job.waitForFinished()
        # shouldn't use cache
        self.assertFalse(job.usedCachedLabels())
        # and results should not have been cached
        self.assertFalse(cache.hasCacheImage('_labels_'))
        self.assertTrue(job.takeLabelingResults())

    def checkCancel(self, job_type):
        """test canceling a render job"""
        layer = QgsVectorLayer("Point?field=fldtxt:string",
                               "layer1", "memory")

        # add a ton of random points
        for i in range(2000):
            x = uniform(5, 25)
            y = uniform(25, 45)
            g = QgsGeometry.fromPointXY(QgsPointXY(x, y))
            f = QgsFeature()
            f.setGeometry(g)
            f.initAttributes(1)
            layer.dataProvider().addFeatures([f])

        settings = QgsMapSettings()
        settings.setExtent(QgsRectangle(5, 25, 25, 45))
        settings.setOutputSize(QSize(600, 400))
        settings.setLayers([layer])

        # first try non-blocking cancelWithoutBlocking() call
        job = job_type(settings)
        finished_spy = QSignalSpy(job.finished)
        job.start()

        # insta cancel!
        job.cancelWithoutBlocking()
        # should still be active immediately after
        self.assertTrue(job.isActive())

        while job.isActive():
            app.processEvents()
        self.assertEqual(len(finished_spy), 1)

        # try blocking cancel() call
        job = job_type(settings)
        finished_spy = QSignalSpy(job.finished)
        job.start()

        # insta cancel!
        job.cancel()
        # should not be active anymore
        self.assertFalse(job.isActive())
        self.assertEqual(len(finished_spy), 1)

    def runRendererChecks(self, renderer):
        """ runs all checks on the specified renderer """
        self.checkRendererUseCachedLabels(renderer)
        self.checkRepaintNonLabeledLayerDoesNotInvalidateLabelCache(renderer)
        self.checkRepaintLabeledLayerInvalidatesLabelCache(renderer)
        self.checkAddingNewLabeledLayerInvalidatesLabelCache(renderer)
        self.checkRemovingLabeledLayerInvalidatesLabelCache(renderer)
        self.checkAddingNewNonLabeledLayerKeepsLabelCache(renderer)
        self.checkRemovingNonLabeledLayerKeepsLabelCache(renderer)
        self.checkLabeledLayerWithBlendModesCannotBeCached(renderer)
        self.checkCancel(renderer)

    def testParallelRenderer(self):
        """ run test suite on QgsMapRendererParallelJob"""
        self.runRendererChecks(QgsMapRendererParallelJob)

    def testSequentialRenderer(self):
        """ run test suite on QgsMapRendererSequentialJob"""
        self.runRendererChecks(QgsMapRendererSequentialJob)

    def testCustomPainterRenderer(self):
        """ run test suite on QgsMapRendererCustomPainterJob"""
        im = QImage(200, 200, QImage.Format_RGB32)
        p = QPainter(im)

        def create_job(settings):
            return QgsMapRendererCustomPainterJob(settings, p)

        self.runRendererChecks(create_job)
        p.end()


if __name__ == '__main__':
    unittest.main()
