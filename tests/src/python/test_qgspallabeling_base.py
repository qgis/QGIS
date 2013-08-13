# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsPalLabeling: base suite setup

From build dir: ctest -R PyQgsPalLabelingBase -V
Set the following env variables when manually running tests:
  PAL_SUITE to run specific tests (define in __main__)
  PAL_VERBOSE to output individual test summary
  PAL_CONTROL_IMAGE to trigger building of new control images
  PAL_REPORT to open any failed image check reports in web browser

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

import os
import sys
import datetime
import glob
import StringIO
import subprocess
import tempfile
from PyQt4.QtCore import *
from PyQt4.QtGui import *

from qgis.core import (
    QGis,
    QgsCoordinateReferenceSystem,
    QgsDataSourceURI,
    QgsMapLayerRegistry,
    QgsMapRenderer,
    QgsPalLabeling,
    QgsPalLayerSettings,
    QgsProviderRegistry,
    QgsVectorLayer,
    QgsRenderChecker
)

from utilities import (
    getQgisTestApp,
    TestCase,
    unittest,
    expectedFailure,
    unitTestDataPath
)

QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()

PALREPORT = 'PAL_REPORT' in os.environ
PALREPORTS = {}


class TestQgsPalLabeling(TestCase):

    _TestDataDir = unitTestDataPath()
    _PalDataDir = os.path.join(_TestDataDir, 'labeling')
    _PalFeaturesDb = os.path.join(_PalDataDir, 'pal_features_v3.sqlite')
    _TestFontID = -1
    _MapRegistry = None
    _MapRenderer = None
    _Canvas = None

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""

        # qgis instances
        cls._QgisApp, cls._Canvas, cls._Iface, cls._Parent = \
            QGISAPP, CANVAS, IFACE, PARENT

        # verify that spatialite provider is available
        msg = ('\nSpatialite provider not found, '
               'SKIPPING TEST SUITE')
        res = 'spatialite' in QgsProviderRegistry.instance().providerList()
        assert res, msg

        # load the FreeSansQGIS labeling test font
        fontdb = QFontDatabase()
        cls._TestFontID = fontdb.addApplicationFont(
            os.path.join(cls._TestDataDir, 'font', 'FreeSansQGIS.ttf'))
        msg = ('\nCould not store test font in font database, '
               'SKIPPING TEST SUITE')
        assert cls._TestFontID != -1, msg

        cls._TestFont = fontdb.font('FreeSansQGIS', 'Medium', 48)
        appfont = QApplication.font()
        msg = ('\nCould not load test font from font database, '
               'SKIPPING TEST SUITE')
        assert cls._TestFont.toString() != appfont.toString(), msg

        cls._TestFunction = ''
        cls._TestGroup = ''
        cls._TestGroupPrefix = ''
        cls._TestGroupAbbr = ''

        # initialize class MapRegistry, Canvas, MapRenderer, Map and PAL
        cls._MapRegistry = QgsMapLayerRegistry.instance()
        # set color to match render test comparisons background
        cls._Canvas.setCanvasColor(QColor(152, 219, 249))
        cls._Map = cls._Canvas.map()
        cls._Map.resize(QSize(600, 400))
        cls._MapRenderer = cls._Canvas.mapRenderer()
        crs = QgsCoordinateReferenceSystem()
        # default for labeling test data sources: WGS 84 / UTM zone 13N
        crs.createFromSrid(32613)
        cls._MapRenderer.setDestinationCrs(crs)
        # use platform's native logical output dpi for QgsMapRenderer on launch

        cls._Pal = QgsPalLabeling()
        cls._MapRenderer.setLabelingEngine(cls._Pal)
        cls._PalEngine = cls._MapRenderer.labelingEngine()
        msg = ('\nCould not initialize PAL labeling engine, '
               'SKIPPING TEST SUITE')
        assert cls._PalEngine, msg

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        cls.removeAllLayers()

    @classmethod
    def removeAllLayers(cls):
        cls._MapRegistry.removeAllMapLayers()

    @classmethod
    def loadFeatureLayer(cls, table):
        uri = QgsDataSourceURI()
        uri.setDatabase(cls._PalFeaturesDb)
        uri.setDataSource('', table, 'geometry')
        vlayer = QgsVectorLayer(uri.uri(), table, 'spatialite')
        # .qml should contain only style for symbology
        vlayer.loadNamedStyle(os.path.join(cls._PalDataDir,
                                           '{0}.qml'.format(table)))
        cls._MapRegistry.addMapLayer(vlayer)
        cls._MapRenderer.setLayerSet([vlayer.id()])

        # zoom to area of interest, which matches output aspect ratio
        uri.setDataSource('', 'aoi', 'geometry')
        aoilayer = QgsVectorLayer(uri.uri(), table, 'spatialite')
        cls._MapRenderer.setExtent(aoilayer.extent())
        cls._Canvas.zoomToFullExtent()
        return vlayer

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

    def defaultSettings(self):
        lyr = QgsPalLayerSettings()
        lyr.enabled = True
        lyr.fieldName = 'text'  # default in data sources
        lyr.textFont = self._TestFont
        lyr.textNamedStyle = 'Medium'
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

    def saveContolImage(self):
        if 'PAL_CONTROL_IMAGE' not in os.environ:
            return
        testgrpdir = 'expected_' + self._TestGroupPrefix
        testdir = os.path.join(self._TestDataDir, 'control_images',
                               testgrpdir, self._Test)
        if not os.path.exists(testdir):
            os.makedirs(testdir)
        imgbasepath = os.path.join(testdir, self._Test)
        imgpath = imgbasepath + '.png'
        for f in glob.glob(imgbasepath + '.*'):
            if os.path.exists(f):
                os.remove(f)
        self._Map.render()
        self._Canvas.saveAsImage(imgpath)

    def renderCheck(self, mismatch=0):
        chk = QgsRenderChecker()
        chk.setControlPathPrefix('expected_' + self._TestGroupPrefix)
        chk.setControlName(self._Test)
        chk.setMapRenderer(self._MapRenderer)
        res = chk.runTest(self._Test, mismatch)
        if PALREPORT and not res:  # don't report ok checks
            testname = self._TestGroup + ' . ' + self._Test
            PALREPORTS[testname] = str(chk.report().toLocal8Bit())
        msg = '\nRender check failed for "{0}"'.format(self._Test)
        return res, msg


class TestQgsPalLabelingBase(TestQgsPalLabeling):

    @classmethod
    def setUpClass(cls):
        TestQgsPalLabeling.setUpClass()
        cls.layer = TestQgsPalLabeling.loadFeatureLayer('point')

    def setUp(self):
        """Run before each test."""
        self.configTest('pal_base', 'base')

    def tearDown(self):
        """Run after each test."""
        pass

    def test_default_pal_disabled(self):
        # Verify PAL labeling is disabled for layer by default
        palset = self.layer.customProperty('labeling', '').toString()
        msg = '\nExpected: Empty string\nGot: {0}'.format(palset)
        self.assertEqual(palset, '', msg)

    def test_settings_enable_pal(self):
        # Verify default PAL settings enable PAL labeling for layer
        lyr = QgsPalLayerSettings()
        lyr.writeToLayer(self.layer)
        palset = self.layer.customProperty('labeling', '').toString()
        msg = '\nExpected: Empty string\nGot: {0}'.format(palset)
        self.assertEqual(palset, 'pal', msg)

    def test_layer_pal_activated(self):
        # Verify, via engine, that PAL labeling can be activated for layer
        lyr = self.defaultSettings()
        lyr.writeToLayer(self.layer)
        msg = '\nLayer labeling not activated, as reported by labelingEngine'
        self.assertTrue(self._PalEngine.willUseLayer(self.layer), msg)

    def test_write_read_settings(self):
        # Verify written PAL settings are same when read from layer
        # load and write default test settings
        lyr1 = self.defaultSettings()
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


def runSuite(module, tests):
    """This allows for a list of test names to be selectively run.
    Also, ensures unittest verbose output comes at end, after debug output"""
    loader = unittest.defaultTestLoader
    if 'PAL_SUITE' in os.environ and tests:
        suite = loader.loadTestsFromNames(tests, module)
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
        palreport = tmp.name
        tmp.write(report)
        tmp.close()

        if sys.platform[:3] in ('win', 'dar'):
            import webbrowser
            webbrowser.open_new_tab("file://{0}".format(palreport))
        else:
            # some Linux OS pause execution on webbrowser open, so background it
            cmd = 'import webbrowser;' \
                  'webbrowser.open_new_tab("file://{0}")'.format(palreport)
            p = subprocess.Popen([sys.executable, "-c", cmd],
                                 stdout=subprocess.PIPE,
                                 stderr=subprocess.STDOUT).pid

    return res


if __name__ == '__main__':
    # NOTE: unless PAL_SUITE env var is set
    # all test class methods will be run
    b = 'TestQgsPalLabelingBase.'
    tests = [b + 'test_write_read_settings']
    res = runSuite(sys.modules[__name__], tests)
    sys.exit(not res.wasSuccessful())
