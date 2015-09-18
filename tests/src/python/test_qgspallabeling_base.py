# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsPalLabeling: base suite setup

From build dir, run: ctest -R PyQgsPalLabelingBase -V

See <qgis-src-dir>/tests/testdata/labeling/README.rst for description.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = 'Larry Shaffer'
__date__ = '07/09/2013'
__copyright__ = 'Copyright 2013, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis
import os
import sys
import datetime
import glob
import shutil
import StringIO
import tempfile

from PyQt4.QtCore import QSize, qDebug
from PyQt4.QtGui import QFont, QColor

from qgis.core import (
    QgsCoordinateReferenceSystem,
    QgsDataSourceURI,
    QgsMapLayerRegistry,
    QgsMapSettings,
    QgsPalLabeling,
    QgsPalLayerSettings,
    QgsProviderRegistry,
    QgsVectorLayer,
    QgsMultiRenderChecker
)

from utilities import (
    getQgisTestApp,
    TestCase,
    unittest,
    unitTestDataPath,
    getTempfilePath,
    renderMapToImage,
    loadTestFonts,
    getTestFont,
    openInBrowserTab
)


QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()
FONTSLOADED = loadTestFonts()

PALREPORT = 'PAL_REPORT' in os.environ
PALREPORTS = {}


# noinspection PyPep8Naming,PyShadowingNames
class TestQgsPalLabeling(TestCase):

    _TestDataDir = unitTestDataPath()
    _PalDataDir = os.path.join(_TestDataDir, 'labeling')
    _PalFeaturesDb = os.path.join(_PalDataDir, 'pal_features_v3.sqlite')
    _TestFont = getTestFont()  # Roman at 12 pt
    """:type: QFont"""
    _MapRegistry = None
    """:type: QgsMapLayerRegistry"""
    _MapRenderer = None
    """:type: QgsMapRenderer"""
    _MapSettings = None
    """:type: QgsMapSettings"""
    _Canvas = None
    """:type: QgsMapCanvas"""
    _Pal = None
    """:type: QgsPalLabeling"""
    _PalEngine = None
    """:type: QgsLabelingEngineInterface"""
    _BaseSetup = False

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""

        # qgis instances
        cls._QgisApp, cls._Canvas, cls._Iface, cls._Parent = \
            QGISAPP, CANVAS, IFACE, PARENT

        # verify that spatialite provider is available
        msg = '\nSpatialite provider not found, SKIPPING TEST SUITE'
        # noinspection PyArgumentList
        res = 'spatialite' in QgsProviderRegistry.instance().providerList()
        assert res, msg

        cls._TestFunction = ''
        cls._TestGroup = ''
        cls._TestGroupPrefix = ''
        cls._TestGroupAbbr = ''
        cls._TestGroupCanvasAbbr = ''
        cls._TestImage = ''
        cls._TestMapSettings = None
        cls._Mismatch = 0
        cls._Mismatches = dict()
        cls._ColorTol = 0
        cls._ColorTols = dict()

        # initialize class MapRegistry, Canvas, MapRenderer, Map and PAL
        # noinspection PyArgumentList
        cls._MapRegistry = QgsMapLayerRegistry.instance()
        cls._MapRenderer = cls._Canvas.mapRenderer()

        cls._MapSettings = cls.getBaseMapSettings()
        osize = cls._MapSettings.outputSize()
        cls._Canvas.resize(QSize(osize.width(), osize.height()))  # necessary?
        # set color to match render test comparisons background
        cls._Canvas.setCanvasColor(cls._MapSettings.backgroundColor())

        cls.setDefaultEngineSettings()
        msg = ('\nCould not initialize PAL labeling engine, '
               'SKIPPING TEST SUITE')
        assert cls._PalEngine, msg

        cls._BaseSetup = True

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""

    def setUp(self):
        """Run before each test."""
        TestQgsPalLabeling.setDefaultEngineSettings()
        self.lyr = self.defaultLayerSettings()

    @classmethod
    def setDefaultEngineSettings(cls):
        """Restore default settings for pal labelling"""
        cls._Pal = QgsPalLabeling()
        cls._MapRenderer.setLabelingEngine(cls._Pal)
        cls._PalEngine = cls._MapRenderer.labelingEngine()

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
        if lyr_id in ms_layers:
            ms_layers.remove(lyr_id)
            cls._MapSettings.setLayers(ms_layers)

    @classmethod
    def getTestFont(cls):
        return QFont(cls._TestFont)

    @classmethod
    def loadFeatureLayer(cls, table, chk=False):
        if chk and cls._MapRegistry.mapLayersByName(table):
            return
        uri = QgsDataSourceURI()
        uri.setDatabase(cls._PalFeaturesDb)
        uri.setDataSource('', table, 'geometry')
        vlayer = QgsVectorLayer(uri.uri(), table, 'spatialite')
        # .qml should contain only style for symbology
        vlayer.loadNamedStyle(os.path.join(cls._PalDataDir,
                                           '{0}.qml'.format(table)))
        # qDebug('render_lyr = {0}'.format(repr(vlayer)))
        cls._MapRegistry.addMapLayer(vlayer)
        # place new layer on top of render stack
        render_lyrs = [vlayer.id()]
        render_lyrs.extend(cls._MapSettings.layers())
        # qDebug('render_lyrs = {0}'.format(repr(render_lyrs)))
        cls._MapSettings.setLayers(render_lyrs)

        # zoom to aoi
        cls._MapSettings.setExtent(cls.aoiExtent())
        cls._Canvas.zoomToFullExtent()
        return vlayer

    @classmethod
    def aoiExtent(cls):
        """Area of interest extent, which matches output aspect ratio"""
        uri = QgsDataSourceURI()
        uri.setDatabase(cls._PalFeaturesDb)
        uri.setDataSource('', 'aoi', 'geometry')
        aoilayer = QgsVectorLayer(uri.uri(), 'aoi', 'spatialite')
        return aoilayer.extent()

    @classmethod
    def getBaseMapSettings(cls):
        """
        :rtype: QgsMapSettings
        """
        ms = QgsMapSettings()
        crs = QgsCoordinateReferenceSystem()
        """:type: QgsCoordinateReferenceSystem"""
        # default for labeling test data: WGS 84 / UTM zone 13N
        crs.createFromSrid(32613)
        ms.setBackgroundColor(QColor(152, 219, 249))
        ms.setOutputSize(QSize(420, 280))
        ms.setOutputDpi(72)
        ms.setFlag(QgsMapSettings.Antialiasing, True)
        ms.setFlag(QgsMapSettings.UseAdvancedEffects, False)
        ms.setFlag(QgsMapSettings.ForceVectorOutput, False)  # no caching?
        ms.setDestinationCrs(crs)
        ms.setCrsTransformEnabled(False)
        ms.setMapUnits(crs.mapUnits())  # meters
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
        ms.setCrsTransformEnabled(oms.hasCrsTransformEnabled())
        ms.setMapUnits(oms.mapUnits())
        ms.setExtent(oms.extent())
        ms.setOutputImageFormat(oms.outputImageFormat())

        ms.setLayers(oms.layers())
        return ms

    def configTest(self, prefix, abbr):
        """Call in setUp() function of test subclass"""
        self._TestGroupPrefix = prefix
        self._TestGroupAbbr = abbr

        # insert test's Class.function marker into debug output stream
        # this helps visually track down the start of a test's debug output
        testid = self.id().split('.')
        self._TestGroup = testid[1]
        self._TestFunction = testid[2]
        testheader = '\n#####_____ {0}.{1} _____#####\n'.\
            format(self._TestGroup, self._TestFunction)
        qDebug(testheader)

        # define the shorthand name of the test (to minimize file name length)
        self._Test = '{0}_{1}'.format(self._TestGroupAbbr,
                                      self._TestFunction.replace('test_', ''))

    def defaultLayerSettings(self):
        lyr = QgsPalLayerSettings()
        lyr.enabled = True
        lyr.fieldName = 'text'  # default in test data sources
        font = self.getTestFont()
        font.setPointSize(32)
        lyr.textFont = font
        lyr.textNamedStyle = 'Roman'
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
                if not callable(value):
                    res[attr] = value
        return res

    def controlImagePath(self, grpprefix=''):
        if not grpprefix:
            grpprefix = self._TestGroupPrefix
        return os.path.join(self._TestDataDir, 'control_images',
                            'expected_' + grpprefix,
                            self._Test, self._Test + '.png')

    def saveControlImage(self, tmpimg=''):
        # don't save control images for RenderVsOtherOutput (Vs) tests, since
        # those control images belong to a different test result
        if ('PAL_CONTROL_IMAGE' not in os.environ
                or 'Vs' in self._TestGroup):
            return
        imgpath = self.controlImagePath()
        testdir = os.path.dirname(imgpath)
        if not os.path.exists(testdir):
            os.makedirs(testdir)
        imgbasepath = \
            os.path.join(testdir,
                         os.path.splitext(os.path.basename(imgpath))[0])
        # remove any existing control images
        for f in glob.glob(imgbasepath + '.*'):
            if os.path.exists(f):
                os.remove(f)
        qDebug('Control image for {0}.{1}'.format(self._TestGroup,
                                                  self._TestFunction))

        if not tmpimg:
            # TODO: this can be deprecated, when per-base-test-class rendering
            #       in checkTest() is verified OK for all classes
            qDebug('Rendering control to: {0}'.format(imgpath))
            ms = self._MapSettings  # class settings
            """:type: QgsMapSettings"""
            settings_type = 'Class'
            if self._TestMapSettings is not None:
                ms = self._TestMapSettings  # per test settings
                settings_type = 'Test'
            qDebug('MapSettings type: {0}'.format(settings_type))

            img = renderMapToImage(ms, parallel=False)
            """:type: QImage"""
            tmpimg = getTempfilePath('png')
            if not img.save(tmpimg, 'png'):
                os.unlink(tmpimg)
                raise OSError('Control not created for: {0}'.format(imgpath))

        if tmpimg and os.path.exists(tmpimg):
            qDebug('Copying control to: {0}'.format(imgpath))
            shutil.copyfile(tmpimg, imgpath)
        else:
            raise OSError('Control not copied to: {0}'.format(imgpath))

    def renderCheck(self, mismatch=0, colortol=0, imgpath='', grpprefix=''):
        """Check rendered map canvas or existing image against control image

        :mismatch: number of pixels different from control, and still valid
        :colortol: maximum difference for each color component including alpha
        :imgpath: existing image; if present, skips rendering canvas
        :grpprefix: compare test image/rendering against different test group
        """
        if not grpprefix:
            grpprefix = self._TestGroupPrefix
        chk = QgsMultiRenderChecker()

        chk.setControlPathPrefix('expected_' + grpprefix)

        chk.setControlName(self._Test)

        if imgpath:
            chk.setRenderedImage(imgpath)

        ms = self._MapSettings  # class settings
        if self._TestMapSettings is not None:
            ms = self._TestMapSettings  # per test settings
        chk.setMapSettings(ms)

        chk.setColorTolerance(colortol)
        # noinspection PyUnusedLocal
        res = chk.runTest(self._Test, mismatch)
        if PALREPORT and not res:  # don't report ok checks
            testname = self._TestGroup + ' . ' + self._Test
            PALREPORTS[testname] = chk.report()
        msg = '\nRender check failed for "{0}"'.format(self._Test)
        return res, msg

    def checkTest(self, **kwargs):
        """Intended to be overridden in subclasses"""
        pass


class TestPALConfig(TestQgsPalLabeling):

    @classmethod
    def setUpClass(cls):
        TestQgsPalLabeling.setUpClass()
        cls.layer = TestQgsPalLabeling.loadFeatureLayer('point')

    @classmethod
    def tearDownClass(cls):
        cls.removeMapLayer(cls.layer)

    def setUp(self):
        """Run before each test."""
        self.configTest('pal_base', 'base')

    def tearDown(self):
        """Run after each test."""
        pass

    def test_default_pal_disabled(self):
        # Verify PAL labeling is disabled for layer by default
        palset = self.layer.customProperty('labeling', '')
        msg = '\nExpected: Empty string\nGot: {0}'.format(palset)
        self.assertEqual(palset, '', msg)

    def test_settings_enable_pal(self):
        # Verify default PAL settings enable PAL labeling for layer
        lyr = QgsPalLayerSettings()
        lyr.writeToLayer(self.layer)
        palset = self.layer.customProperty('labeling', '')
        msg = '\nExpected: Empty string\nGot: {0}'.format(palset)
        self.assertEqual(palset, 'pal', msg)

    def test_layer_pal_activated(self):
        # Verify, via engine, that PAL labeling can be activated for layer
        lyr = self.defaultLayerSettings()
        lyr.writeToLayer(self.layer)
        msg = '\nLayer labeling not activated, as reported by labelingEngine'
        self.assertTrue(self._PalEngine.willUseLayer(self.layer), msg)

    def test_write_read_settings(self):
        # Verify written PAL settings are same when read from layer
        # load and write default test settings
        lyr1 = self.defaultLayerSettings()
        lyr1dict = self.settingsDict(lyr1)
        # print lyr1dict
        lyr1.writeToLayer(self.layer)

        # read settings
        lyr2 = QgsPalLayerSettings()
        lyr2.readFromLayer(self.layer)
        lyr2dict = self.settingsDict(lyr1)
        # print lyr2dict

        msg = '\nLayer settings read not same as settings written'
        self.assertDictEqual(lyr1dict, lyr2dict, msg)

    def test_default_partials_labels_enabled(self):
        # Verify ShowingPartialsLabels is enabled for PAL by default
        pal = QgsPalLabeling()
        self.assertTrue(pal.isShowingPartialsLabels())

    def test_partials_labels_activate(self):
        pal = QgsPalLabeling()
        # Enable partials labels
        pal.setShowingPartialsLabels(True)
        self.assertTrue(pal.isShowingPartialsLabels())

    def test_partials_labels_deactivate(self):
        pal = QgsPalLabeling()
        # Disable partials labels
        pal.setShowingPartialsLabels(False)
        self.assertFalse(pal.isShowingPartialsLabels())


# noinspection PyPep8Naming,PyShadowingNames
def runSuite(module, tests):
    """This allows for a list of test names to be selectively run.
    Also, ensures unittest verbose output comes at end, after debug output"""
    loader = unittest.defaultTestLoader
    if 'PAL_SUITE' in os.environ:
        if tests:
            suite = loader.loadTestsFromNames(tests, module)
        else:
            raise Exception(
                "\n\n####__ 'PAL_SUITE' set, but no tests specified __####\n")
    else:
        suite = loader.loadTestsFromModule(module)
    verb = 2 if 'PAL_VERBOSE' in os.environ else 0

    out = StringIO.StringIO()
    res = unittest.TextTestRunner(stream=out, verbosity=verb).run(suite)
    if verb:
        print '\nIndividual test summary:'
    print '\n' + out.getvalue()
    out.close()

    if PALREPORTS:
        teststamp = 'PAL Test Report: ' + \
                    datetime.datetime.now().strftime('%Y-%m-%d %X')
        report = '<html><head><title>{0}</title></head><body>'.format(teststamp)
        report += '\n<h2>Failed Tests: {0}</h2>'.format(len(PALREPORTS))
        for k, v in PALREPORTS.iteritems():
            report += '\n<h3>{0}</h3>\n{1}'.format(k, v)
        report += '</body></html>'

        tmp = tempfile.NamedTemporaryFile(suffix=".html", delete=False)
        tmp.write(report)
        tmp.close()
        openInBrowserTab('file://' + tmp.name)

    return res


if __name__ == '__main__':
    # NOTE: unless PAL_SUITE env var is set all test class methods will be run
    # ex: 'TestGroup(Point|Line|Curved|Polygon|Feature).test_method'
    suite = [
        'TestPALConfig.test_write_read_settings'
    ]
    res = runSuite(sys.modules[__name__], suite)
    sys.exit(not res.wasSuccessful())
