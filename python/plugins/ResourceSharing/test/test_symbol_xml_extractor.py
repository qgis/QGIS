# coding=utf-8
from qgis.testing import start_app, unittest
import os
import sys

# append plugin folder to path
sys.path.append(os.path.join(os.path.dirname(__file__), os.path.pardir))

# Note that the instance returned in the import test is
# QGIS 2.x: QgsRandomColorRamp
# QGIS 3.x is QgsLimitedRandomColorRamp

try:  # QGIS 2.x
    from qgis.core import (
        QgsVectorColorBrewerColorRampV2 as QgsColorBrewerColorRamp,
        QgsVectorGradientColorRampV2 as QgsGradientColorRamp,
        QgsVectorRandomColorRampV2 as random_color_ramp,  # <-- !!!!
        QgsFillSymbolV2 as QgsFillSymbol,
        QgsLineSymbolV2 as QgsLineSymbol,
        QgsMarkerSymbolV2 as QgsMarkerSymbol)
except ImportError:  # QGIS 3.x
    from qgis.core import (
        QgsColorBrewerColorRamp,
        QgsLimitedRandomColorRamp as random_color_ramp,  # <-- !!!!
        QgsGradientColorRamp,
        QgsFillSymbol,
        QgsLineSymbol,
        QgsMarkerSymbol)

from resource_sharing.symbol_xml_extractor import SymbolXMLExtractor
from utilities import test_data_path


class TestSymbolXMLExtractor(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        start_app()

    def test_parse_xml(self):
        """Test parsing the xml works correctly."""
        xml_path = test_data_path(
            'collections', 'test_collection', 'symbol', 'various_symbols.xml')
        extractor = SymbolXMLExtractor(xml_path)
        # There are 9 symbols and 3 colorramps there
        expected_symbols = {
            'fill_raster': QgsFillSymbol,
            'fill_svg': QgsFillSymbol,
            'fill_svg_line': QgsFillSymbol,
            'line_arrow': QgsLineSymbol,
            'line_svg_marker': QgsLineSymbol,
            'marker_ellipse': QgsMarkerSymbol,
            'marker_font': QgsMarkerSymbol,
            'marker_simple': QgsMarkerSymbol,
            'marker_svg': QgsMarkerSymbol
        }
        self.assertEqual(len(extractor.symbols), len(expected_symbols))
        for symbol in extractor.symbols:
            self.assertTrue(
                isinstance(symbol['symbol'], expected_symbols[symbol['name']])
            )
        expected_colorramps = {
            'cr_colorbrewer': QgsColorBrewerColorRamp,
            'cr_gradient': QgsGradientColorRamp,
            'cr_random': random_color_ramp # QGIS 2.x is QgsRandomColorRamp QGIS 3.x is QgsLimitedRandomColorRamp
        }
        self.assertEqual(len(extractor.colorramps), len(expected_colorramps))
        for colorramp in extractor.colorramps:
            self.assertTrue(
                isinstance(colorramp['colorramp'],
                           expected_colorramps[colorramp['name']])
            )


if __name__ == "__main__":
    unittest.main()
