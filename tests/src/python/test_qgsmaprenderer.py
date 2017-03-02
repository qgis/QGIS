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
                       QgsRectangle,
                       QgsVectorLayer,
                       QgsProject,
                       QgsFeature,
                       QgsGeometry,
                       QgsMapSettings,
                       QgsPoint)
from qgis.testing import start_app, unittest
from qgis.PyQt.QtCore import QSize, QThreadPool
from qgis.PyQt.QtGui import QPainter, QImage
from random import uniform


app = start_app()


class TestQgsMapRenderer(unittest.TestCase):

    def setUp(self):
        pass

    def tearDown(self):
        # avoid crash on finish, probably related to https://bugreports.qt.io/browse/QTBUG-35760
        QThreadPool.globalInstance().waitForDone()

    def checkCancel(self, job_type):
        """test canceling a render job"""
        layer = QgsVectorLayer("Point?field=fldtxt:string",
                               "layer1", "memory")

        # add a ton of random points
        for i in range(2000):
            x = uniform(5, 25)
            y = uniform(25, 45)
            g = QgsGeometry.fromPoint(QgsPoint(x, y))
            f = QgsFeature()
            f.setGeometry(g)
            f.initAttributes(1)
            layer.dataProvider().addFeatures([f])

        settings = QgsMapSettings()
        settings.setExtent(QgsRectangle(5, 25, 25, 45))
        settings.setOutputSize(QSize(600, 400))
        settings.setLayers([layer.id()])

        # first try non-blocking cancelWithoutBlocking() call
        job = job_type(settings)
        job.start()

        # insta cancel!
        job.cancelWithoutBlocking()
        # should still be active immediately after
        self.assertTrue(job.isActive())

        while job.isActive():
            app.processEvents()

        # try blocking cancel() call
        job = job_type(settings)
        job.start()

        # insta cancel!
        job.cancel()
        # should not be active anymore
        self.assertFalse(job.isActive())

    def runRendererChecks(self, renderer):
        """ runs all checks on the specified renderer """
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
