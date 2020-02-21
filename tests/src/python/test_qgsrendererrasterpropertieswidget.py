import pathlib
from qgis.testing import start_app, unittest, TestCase
from qgis.testing.mocked import get_iface
from qgis.core import QgsRasterLayer, QgsProject, QgsMultiBandColorRenderer
from qgis.gui import QgsRendererRasterPropertiesWidget, QgsMapCanvas, QgsMultiBandColorRendererWidget, QgsRasterRendererWidget
from utilities import unitTestDataPath


class QgsRendererRasterPropertiesTestCases(TestCase):

    def setUp(self):
        self.iface = get_iface()
        QgsProject.instance().removeAllMapLayers()

    def test_syncToLayer(self):

        path = pathlib.Path(unitTestDataPath(), 'landsat_4326.tif')
        self.assertTrue(path.is_file())

        lyr = QgsRasterLayer(path.as_posix())
        c = QgsMapCanvas()
        c.setDestinationCrs(lyr.crs())
        c.setExtent(lyr.extent())
        c.setLayers([lyr])

        renderer = lyr.renderer()
        if isinstance(renderer, QgsMultiBandColorRenderer):
            renderer.setRedBand(1)
            w = QgsRendererRasterPropertiesWidget(lyr, c, None)
            cw = w.currentRenderWidget()
            self.assertIsInstance(cw, QgsRasterRendererWidget)
            self.assertEquals(cw.renderer().redBand(), 1)

            b = lyr.bandCount()

            renderer.setRedBand(b)
            lyr.styleChanged.emit()
            w.syncToLayer(lyr)

            cw = w.currentRenderWidget()
            self.assertIsInstance(cw, QgsRasterRendererWidget)
            self.assertEquals(cw.renderer().redBand(), b)


if __name__ == '__main__':
    unittest.main()
