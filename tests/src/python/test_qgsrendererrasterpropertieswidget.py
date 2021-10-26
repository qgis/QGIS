import pathlib
from qgis.testing import start_app, unittest, TestCase
from qgis.testing.mocked import get_iface
from qgis.core import QgsRasterLayer, QgsProject, QgsMultiBandColorRenderer, QgsRasterRenderer, QgsSingleBandGrayRenderer
from qgis.gui import QgsRendererRasterPropertiesWidget, QgsMapCanvas, QgsMultiBandColorRendererWidget, QgsRasterRendererWidget


class QgsRendererRasterPropertiesTestCases(TestCase):

    def setUp(self):
        self.iface = get_iface()

    def multibandRasterLayer(self) -> QgsRasterLayer:

        try:
            from utilities import unitTestDataPath
            path = pathlib.Path(unitTestDataPath()) / 'landsat_4326.tif'
        except ModuleNotFoundError:
            path = pathlib.Path(__file__).parent / 'landsat_4326.tif'

        assert isinstance(path, pathlib.Path) and path.is_file()
        lyr = QgsRasterLayer(path.as_posix())
        lyr.setName(path.name)
        self.assertIsInstance(lyr, QgsRasterLayer)
        self.assertTrue(lyr.isValid())
        self.assertTrue(lyr.bandCount() > 1)

        return lyr

    def test_syncToLayer_SingleBandGray(self):

        lyr = self.multibandRasterLayer()
        lyr.setRenderer(QgsSingleBandGrayRenderer(lyr.dataProvider(), 2))
        c = QgsMapCanvas()
        w = QgsRendererRasterPropertiesWidget(lyr, c)
        assert isinstance(w.currentRenderWidget().renderer(), QgsSingleBandGrayRenderer)
        assert w.currentRenderWidget().renderer().grayBand() == 2
        lyr.renderer().setGrayBand(1)
        w.syncToLayer(lyr)
        assert w.currentRenderWidget().renderer().grayBand() == 1

    def test_syncToLayer_MultiBand(self):

        lyr = self.multibandRasterLayer()
        assert isinstance(lyr.renderer(), QgsMultiBandColorRenderer)
        lyr.renderer().setRedBand(1)
        lyr.renderer().setGreenBand(2)
        lyr.renderer().setBlueBand(3)

        c = QgsMapCanvas()
        w = QgsRendererRasterPropertiesWidget(lyr, c)
        assert isinstance(w.currentRenderWidget().renderer(), QgsMultiBandColorRenderer)
        r = w.currentRenderWidget().renderer()
        assert isinstance(r, QgsMultiBandColorRenderer)
        assert r.usesBands() == [1, 2, 3]

        lyr.renderer().setRedBand(3)
        lyr.renderer().setGreenBand(1)
        lyr.renderer().setBlueBand(2)

        w.syncToLayer(lyr)

        r = w.currentRenderWidget().renderer()
        assert isinstance(r, QgsMultiBandColorRenderer)
        assert r.usesBands() == [3, 1, 2]


if __name__ == '__main__':
    unittest.main()
