"""
***************************************************************************
    AlgorithmsTest.py
    ---------------------
    Date                 : January 2016
    Copyright            : (C) 2016 by Matthias Kuhn
    Email                : matthias@opengis.ch
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Matthias Kuhn"
__date__ = "January 2016"
__copyright__ = "(C) 2016, Matthias Kuhn"


import glob
import hashlib
import math
import os
import re
import shutil
import struct
import tempfile
from copy import deepcopy

import nose2
import processing
import yaml
from numpy import nan_to_num
from osgeo import gdal
from osgeo.gdalconst import GA_ReadOnly
from qgis.analysis import QgsNativeAlgorithms
from qgis.core import (
    Qgis,
    QgsApplication,
    QgsCoordinateReferenceSystem,
    QgsFeatureRequest,
    QgsMapLayer,
    QgsMeshLayer,
    QgsProcessingContext,
    QgsProcessingFeedback,
    QgsProcessingUtils,
    QgsProject,
    QgsProperty,
    QgsRasterLayer,
    QgsVectorLayer,
)
from qgis.PyQt.QtCore import QT_VERSION
from qgis.testing import QgisTestCase, _UnexpectedSuccess, start_app
from utilities import unitTestDataPath

gdal.UseExceptions()

REGENERATE_REFERENCE_RASTERS = False


def GDAL_COMPUTE_VERSION(maj, min, rev):
    return (maj) * 1000000 + (min) * 10000 + (rev) * 100


def processingTestDataPath():
    return os.path.join(os.path.dirname(__file__), "testdata")


class GenericAlgorithmsTest(QgisTestCase):
    """
    General (non-provider specific) algorithm tests
    """

    @classmethod
    def setUpClass(cls):
        start_app()
        from processing.core.Processing import Processing

        Processing.initialize()
        cls.cleanup_paths = []

    @classmethod
    def tearDownClass(cls):
        from processing.core.Processing import Processing

        Processing.deinitialize()
        for path in cls.cleanup_paths:
            shutil.rmtree(path)

    def testAlgorithmCompliance(self):
        for p in QgsApplication.processingRegistry().providers():
            print(f"testing provider {p.id()}")
            for a in p.algorithms():
                print(f"testing algorithm {a.id()}")
                self.check_algorithm(a)

    def check_algorithm(self, alg):
        # check that calling helpUrl() works without error
        alg.helpUrl()

        if alg.provider().id() in ("qgis", "native", "3d", "pdal"):
            if alg.id() not in (
                "native:exportmeshedges",
                "native:exportmeshfaces",
                "native:exportmeshongrid",
                "native:exportmeshvertices",
                "native:intersection",
                "native:meshcontours",
                "native:meshexportcrosssection",
                "native:meshexporttimeseries",
                "native:meshrasterize",
                "native:surfacetopolygon",
                "qgis:advancedpythonfieldcalculator",
                "qgis:barplot",
                "qgis:boxplot",
                "qgis:distancetonearesthublinetohub",
                "qgis:distancetonearesthubpoints",
                "qgis:eliminateselectedpolygons",
                "qgis:generatepointspixelcentroidsalongline",
                "qgis:knearestconcavehull",
                "qgis:meanandstandarddeviationplot",
                "qgis:pointsdisplacement",
                "qgis:polarplot",
                "qgis:randompointsalongline",
                "qgis:randompointsinlayerbounds",
                "qgis:randompointsinsidepolygons",
                "qgis:randomselection",
                "qgis:randomselectionwithinsubsets",
                "qgis:rastercalculator",
                "qgis:rasterlayerhistogram",
                "qgis:rectanglesovalsdiamondsvariable",
                "qgis:regularpoints",
                "qgis:relief",
                "qgis:scatter3dplot",
                "qgis:setstyleforrasterlayer",
                "qgis:setstyleforvectorlayer",
                "qgis:texttofloat",
                "qgis:variabledistancebuffer",
                "qgis:vectorlayerhistogram",
                "qgis:vectorlayerscatterplot",
            ):
                self.assertTrue(
                    alg.tags(),
                    f"Algorithm {alg.id()} has no tags!",
                )

            if alg.id() not in ("qgis:rectanglesovalsdiamondsvariable",):
                self.assertTrue(
                    alg.shortHelpString(),
                    f"Algorithm {alg.id()} has no shortHelpString!",
                )

            if alg.id() not in (
                "native:createspatialindex",
                "native:tilesxyzdirectory",
                "native:tilesxyzmbtiles",
                "pdal:assignprojection",
                "pdal:boundary",
                "pdal:clip",
                "pdal:convertformat",
                "pdal:createcopc",
                "pdal:density",
                "pdal:exportraster",
                "pdal:exportrastertin",
                "pdal:exportvector",
                "pdal:filter",
                "pdal:info",
                "pdal:merge",
                "pdal:reproject",
                "pdal:thinbydecimate",
                "pdal:thinbyradius",
                "pdal:tile",
                "pdal:virtualpointcloud",
                "qgis:advancedpythonfieldcalculator",
                "qgis:distancetonearesthublinetohub",
                "qgis:distancetonearesthubpoints",
                "qgis:eliminateselectedpolygons",
                "qgis:generatepointspixelcentroidsalongline",
                "qgis:linestopolygons",
                "qgis:pointsdisplacement",
                "qgis:randompointsalongline",
                "qgis:randompointsinlayerbounds",
                "qgis:randompointsinsidepolygons",
                "qgis:rastercalculator",
                "qgis:rectanglesovalsdiamondsvariable",
                "qgis:regularpoints",
                "qgis:relief",
                "qgis:setstyleforrasterlayer",
                "qgis:setstyleforvectorlayer",
                "qgis:statisticsbycategories",
                "qgis:variabledistancebuffer",
            ):
                self.assertTrue(
                    alg.shortDescription(),
                    f"Algorithm {alg.id()} has not shortDescription!",
                )
            if alg.shortDescription():
                self.assertTrue(
                    alg.shortDescription()[0].isupper(),
                    f'Algorithm {alg.id()} shortDescription does not start with capital! "{alg.shortDescription()}"',
                )
                self.assertEqual(
                    alg.shortDescription()[-1],
                    ".",
                    f'Algorithm {alg.id()} shortDescription does not end with full stop "{alg.shortDescription()}"',
                )
                self.assertFalse(
                    alg.shortDescription().lower().startswith("this algorithm"),
                    f'Algorithm {alg.id()} shortDescription should NOT start with eg "This algorithm computes...", just use "Computes..." instead: "{alg.shortDescription()}"',
                )
                first_word = alg.shortDescription().split(" ")[0].lower()
                if first_word not in ("randomly",):
                    self.assertEqual(
                        first_word[-1],
                        "s",
                        f'Algorithm {alg.id()} shortDescription should start with a verb ending in s, eg "Combines", "Creates",... "{alg.shortDescription()}"',
                    )
                self.assertFalse(
                    "</" in alg.shortDescription(),
                    f'Algorithm {alg.id()} shortDescription should not contain any HTML formatting "{alg.shortDescription()}"',
                )

        # enable when all native algorithms have QGS_MARK_ALGORITHM_SOURCE
        if False:
            if alg.provider().id() in ("native",):
                self.assertTrue(
                    alg.implementationSourceUri(),
                    f"Algorithm {alg.id()} has no QGS_MARK_ALGORITHM_SOURCE macro inserted!",
                )


if __name__ == "__main__":
    nose2.main()
