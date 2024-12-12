"""QGIS Unit tests for QgsPalLabeling: base suite setup

From build dir, run: ctest -R PyQgsPalLabelingBase -V

See <qgis-src-dir>/tests/testdata/labeling/README.rst for description.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Larry Shaffer"
__date__ = "07/09/2013"
__copyright__ = "Copyright 2013, The QGIS Project"

import os
import sys
from collections.abc import Callable

from qgis.PyQt.QtCore import QSize, Qt, qDebug
from qgis.PyQt.QtGui import QColor, QFont
from qgis.core import (
    QgsCoordinateReferenceSystem,
    QgsCoordinateTransform,
    QgsGeometry,
    QgsLabelingEngineSettings,
    QgsMapSettings,
    QgsPalLabeling,
    QgsPalLayerSettings,
    QgsProject,
    QgsStringReplacementCollection,
    QgsTextFormat,
    QgsUnitTypes,
    QgsVectorLayer,
    QgsVectorLayerSimpleLabeling,
    QgsVectorTileBasicLabeling,
    QgsVectorTileBasicLabelingStyle,
    QgsVectorTileLayer,
    QgsWkbTypes,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import (
    getTestFont,
    loadTestFonts,
    unitTestDataPath,
)

start_app(
    sys.platform != "darwin"
)  # No cleanup on mac os x, it crashes the pallabelingcanvas test on exit
FONTSLOADED = loadTestFonts()


# noinspection PyPep8Naming,PyShadowingNames
class TestQgsPalLabeling(QgisTestCase):

    _PalDataDir = os.path.join(unitTestDataPath(), "labeling")
    _TestFont = getTestFont()  # Roman at 12 pt
    """:type: QFont"""
    _MapRegistry = None
    """:type: QgsProject"""
    _MapSettings = None
    """:type: QgsMapSettings"""
    _BaseSetup = False

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()

        cls._TestFunction = ""
        cls._TestGroup = ""
        cls._TestGroupPrefix = ""
        cls._TestGroupAbbr = ""
        cls._TestGroupCanvasAbbr = ""
        cls._TestImage = ""
        cls._TestMapSettings = None
        cls._Mismatch = 0
        cls._Mismatches = dict()
        cls._ColorTol = 0
        cls._ColorTols = dict()

        # initialize class MapRegistry, Canvas, MapRenderer, Map and PAL
        # noinspection PyArgumentList
        cls._MapRegistry = QgsProject.instance()

        cls._MapSettings = cls.getBaseMapSettings()

        cls.setDefaultEngineSettings()

        cls._BaseSetup = True

    def setUp(self):
        """Run before each test."""
        TestQgsPalLabeling.setDefaultEngineSettings()
        self.lyr = self.defaultLayerSettings()

    @classmethod
    def setDefaultEngineSettings(cls):
        """Restore default settings for pal labeling"""
        settings = QgsLabelingEngineSettings()
        settings.setPlacementVersion(
            QgsLabelingEngineSettings.PlacementEngineVersion.PlacementEngineVersion2
        )
        cls._MapSettings.setLabelingEngineSettings(settings)

    @classmethod
    def removeAllLayers(cls):
        cls._MapSettings.setLayers([])
        cls._MapRegistry.removeAllMapLayers()

    @classmethod
    def removeMapLayer(cls, layer):
        if layer is None:
            return
        lyr_id = layer.id()
        cls._MapRegistry.removeMapLayer(lyr_id)
        ms_layers = cls._MapSettings.layers()
        if layer in ms_layers:
            ms_layers.remove(layer)
            cls._MapSettings.setLayers(ms_layers)

    @classmethod
    def getTestFont(cls):
        return QFont(cls._TestFont)

    @classmethod
    def loadFeatureLayer(cls, table, chk=False):
        if chk and cls._MapRegistry.mapLayersByName(table):
            return

        options = QgsVectorLayer.LayerOptions()
        options.forceReadOnly = True
        vlayer = QgsVectorLayer(
            f"{cls._PalDataDir}/{table}.geojson", table, "ogr", options
        )
        assert vlayer.isValid()
        # .qml should contain only style for symbology
        vlayer.loadNamedStyle(os.path.join(cls._PalDataDir, f"{table}.qml"))
        # qDebug('render_lyr = {0}'.format(repr(vlayer)))
        cls._MapRegistry.addMapLayer(vlayer)
        # place new layer on top of render stack
        render_lyrs = [vlayer]
        render_lyrs.extend(cls._MapSettings.layers())
        # qDebug('render_lyrs = {0}'.format(repr(render_lyrs)))
        cls._MapSettings.setLayers(render_lyrs)

        # zoom to aoi
        cls._MapSettings.setExtent(cls.aoiExtent())
        return vlayer

    @classmethod
    def aoiExtent(cls):
        """Area of interest extent, which matches output aspect ratio"""
        options = QgsVectorLayer.LayerOptions()
        options.forceReadOnly = True
        aoilayer = QgsVectorLayer(
            f"{cls._PalDataDir}/aoi.geojson", "aoi", "ogr", options
        )
        assert aoilayer.isValid()
        return aoilayer.extent()

    @classmethod
    def getBaseMapSettings(cls):
        """
        :rtype: QgsMapSettings
        """
        ms = QgsMapSettings()
        # default for labeling test data: WGS 84 / UTM zone 13N
        crs = QgsCoordinateReferenceSystem("epsg:32613")
        ms.setBackgroundColor(QColor(152, 219, 249))
        ms.setOutputSize(QSize(420, 280))
        ms.setOutputDpi(72)
        ms.setFlag(QgsMapSettings.Flag.Antialiasing, True)
        ms.setFlag(QgsMapSettings.Flag.UseAdvancedEffects, False)
        ms.setFlag(QgsMapSettings.Flag.ForceVectorOutput, False)  # no caching?
        ms.setDestinationCrs(crs)
        ms.setExtent(cls.aoiExtent())
        return ms

    def cloneMapSettings(self, oms):
        """
        :param QgsMapSettings oms: Other QgsMapSettings
        :rtype: QgsMapSettings
        """
        ms = QgsMapSettings()
        ms.setBackgroundColor(oms.backgroundColor())
        ms.setOutputSize(oms.outputSize())
        ms.setOutputDpi(oms.outputDpi())
        ms.setFlags(oms.flags())
        ms.setDestinationCrs(oms.destinationCrs())
        ms.setExtent(oms.extent())
        ms.setOutputImageFormat(oms.outputImageFormat())
        ms.setLabelingEngineSettings(oms.labelingEngineSettings())

        ms.setLayers(oms.layers())
        return ms

    def configTest(self, prefix, abbr):
        """Call in setUp() function of test subclass"""
        self._TestGroupPrefix = prefix
        self._TestGroupAbbr = abbr

        # insert test's Class.function marker into debug output stream
        # this helps visually track down the start of a test's debug output
        testid = self.id().split(".")
        self._TestGroup = testid[1]
        self._TestFunction = testid[2]

        # define the shorthand name of the test (to minimize file name length)
        self._Test = f"{self._TestGroupAbbr}_{self._TestFunction.replace('test_', '')}"

    def defaultLayerSettings(self):
        lyr = QgsPalLayerSettings()
        lyr.fieldName = "text"  # default in test data sources
        font = self.getTestFont()
        font.setPointSize(32)
        format = QgsTextFormat()
        format.setFont(font)
        format.setColor(QColor(0, 0, 0))
        format.setNamedStyle("Roman")
        format.setSize(32)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.buffer().setJoinStyle(Qt.PenJoinStyle.BevelJoin)
        lyr.setFormat(format)
        return lyr

    @staticmethod
    def settingsDict(lyr):
        """Return a dict of layer-level labeling settings

        .. note:: QgsPalLayerSettings is not a QObject, so we can not collect
        current object properties, and the public properties of the C++ obj
        can't be listed with __dict__ or vars(). So, we sniff them out relative
        to their naming convention (camelCase), as reported by dir().
        """
        res = {}
        for attr in dir(lyr):
            if attr[0].islower() and not attr.startswith("__"):
                value = getattr(lyr, attr)
                if isinstance(
                    value,
                    (
                        QgsGeometry,
                        QgsStringReplacementCollection,
                        QgsCoordinateTransform,
                    ),
                ):
                    continue  # ignore these objects
                if not isinstance(value, Callable):
                    res[attr] = value
        return res

    def checkTest(self, **kwargs):
        """Intended to be overridden in subclasses"""
        pass

    def testSplitToLines(self):
        self.assertEqual(QgsPalLabeling.splitToLines("", ""), [""])
        self.assertEqual(QgsPalLabeling.splitToLines("abc def", ""), ["abc def"])
        self.assertEqual(QgsPalLabeling.splitToLines("abc def", " "), ["abc", "def"])
        self.assertEqual(QgsPalLabeling.splitToLines("abc\ndef", " "), ["abc", "def"])


class TestPALConfig(TestQgsPalLabeling):

    @classmethod
    def setUpClass(cls):
        TestQgsPalLabeling.setUpClass()

        cls.layer = TestQgsPalLabeling.loadFeatureLayer("point")

    @classmethod
    def tearDownClass(cls):
        cls.removeMapLayer(cls.layer)
        super().tearDownClass()

    def setUp(self):
        """Run before each test."""
        self.configTest("pal_base", "base")

    def tearDown(self):
        """Run after each test."""
        pass

    def test_default_pal_disabled(self):
        # Verify PAL labeling is disabled for layer by default
        palset = self.layer.customProperty("labeling", "")
        msg = f"\nExpected: Empty string\nGot: {palset}"
        self.assertEqual(palset, "", msg)

    def test_settings_no_labeling(self):
        self.layer.setLabeling(None)
        self.assertEqual(None, self.layer.labeling())

    def test_layer_pal_activated(self):
        # Verify, via engine, that PAL labeling can be activated for layer
        lyr = self.defaultLayerSettings()
        self.layer.setLabeling(QgsVectorLayerSimpleLabeling(lyr))
        msg = "\nLayer labeling not activated, as reported by labelingEngine"
        self.assertTrue(QgsPalLabeling.staticWillUseLayer(self.layer), msg)

        # also test for vector tile layer
        tile_layer = QgsVectorTileLayer("x", "y")
        self.assertFalse(QgsPalLabeling.staticWillUseLayer(tile_layer))

        st = QgsVectorTileBasicLabelingStyle()
        st.setStyleName("st1")
        st.setLayerName("place")
        st.setFilterExpression("rank = 1 AND class = 'country'")
        st.setGeometryType(QgsWkbTypes.GeometryType.PointGeometry)
        labeling = QgsVectorTileBasicLabeling()
        labeling.setStyles([st])
        tile_layer.setLabeling(labeling)
        self.assertTrue(QgsPalLabeling.staticWillUseLayer(tile_layer))

    def test_write_read_settings(self):
        # Verify written PAL settings are same when read from layer
        # load and write default test settings
        lyr1 = self.defaultLayerSettings()
        lyr1dict = self.settingsDict(lyr1)
        # print(lyr1dict)
        self.layer.setLabeling(QgsVectorLayerSimpleLabeling(lyr1))

        # read settings
        lyr2 = self.layer.labeling().settings()
        lyr2dict = self.settingsDict(lyr2)
        # print(lyr2dict)

        msg = "\nLayer settings read not same as settings written"
        self.assertDictEqual(lyr1dict, lyr2dict, msg)

    def test_default_partials_labels_enabled(self):
        # Verify ShowingPartialsLabels is enabled for PAL by default
        engine_settings = QgsLabelingEngineSettings()
        self.assertTrue(
            engine_settings.testFlag(
                QgsLabelingEngineSettings.Flag.UsePartialCandidates
            )
        )

    def test_partials_labels_activate(self):
        engine_settings = QgsLabelingEngineSettings()
        # Enable partials labels
        engine_settings.setFlag(QgsLabelingEngineSettings.Flag.UsePartialCandidates)
        self.assertTrue(
            engine_settings.testFlag(
                QgsLabelingEngineSettings.Flag.UsePartialCandidates
            )
        )

    def test_partials_labels_deactivate(self):
        engine_settings = QgsLabelingEngineSettings()
        # Disable partials labels
        engine_settings.setFlag(
            QgsLabelingEngineSettings.Flag.UsePartialCandidates, False
        )
        self.assertFalse(
            engine_settings.testFlag(
                QgsLabelingEngineSettings.Flag.UsePartialCandidates
            )
        )


# noinspection PyPep8Naming,PyShadowingNames
def runSuite(module, tests):
    """This allows for a list of test names to be selectively run.
    Also, ensures unittest verbose output comes at end, after debug output"""
    loader = unittest.defaultTestLoader
    if "PAL_SUITE" in os.environ:
        if tests:
            suite = loader.loadTestsFromNames(tests, module)
        else:
            raise Exception(
                "\n\n####__ 'PAL_SUITE' set, but no tests specified __####\n"
            )
    else:
        suite = loader.loadTestsFromModule(module)
    verb = 2 if "PAL_VERBOSE" in os.environ else 0

    res = unittest.TextTestRunner(verbosity=verb).run(suite)

    return res


if __name__ == "__main__":
    # NOTE: unless PAL_SUITE env var is set all test class methods will be run
    # ex: 'TestGroup(Point|Line|Curved|Polygon|Feature).test_method'
    suite = ["TestPALConfig.test_write_read_settings"]
    res = runSuite(sys.modules[__name__], suite)
    sys.exit(not res.wasSuccessful())
