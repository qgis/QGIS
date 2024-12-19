"""QGIS Unit tests for QgsRuleBasedRenderer

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import QgsMarkerSymbol, QgsFillSymbol, QgsRuleBasedRenderer
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


# ===========================================================
# Utility functions


def createMarkerSymbol():
    symbol = QgsMarkerSymbol.createSimple(
        {"color": "100,150,50", "name": "square", "size": "3.0"}
    )
    return symbol


def createFillSymbol():
    symbol = QgsFillSymbol.createSimple({"color": "100,150,50"})
    return symbol


class TestQgsRuleBasedSymbolRenderer(QgisTestCase):

    def test_to_sld(self):
        root_rule = QgsRuleBasedRenderer.Rule(None)
        symbol_a = createMarkerSymbol()
        root_rule.appendChild(
            QgsRuleBasedRenderer.Rule(
                symbol_a,
                filterExp='"something"=1',
                label="label a",
                description="rule a",
            )
        )
        symbol_b = createMarkerSymbol()
        root_rule.appendChild(
            QgsRuleBasedRenderer.Rule(
                symbol_b,
                filterExp='"something"=2',
                label="label b",
                description="rule b",
            )
        )

        # this rule should NOT be included in the SLD, as it would otherwise result
        # in an invalid se:rule with no symbolizer element
        symbol_which_is_empty_in_sld = createFillSymbol()
        symbol_which_is_empty_in_sld[0].setBrushStyle(Qt.BrushStyle.NoBrush)
        symbol_which_is_empty_in_sld[0].setStrokeStyle(Qt.PenStyle.NoPen)
        root_rule.appendChild(
            QgsRuleBasedRenderer.Rule(
                symbol_which_is_empty_in_sld,
                filterExp='"something"=3',
                label="label c",
                description="rule c",
            )
        )

        renderer = QgsRuleBasedRenderer(root_rule)

        dom = QDomDocument()
        root = dom.createElement("FakeRoot")
        dom.appendChild(root)
        renderer.toSld(dom, root, {})

        expected = """<FakeRoot>
 <se:Rule>
  <se:Name>label a</se:Name>
  <se:Description>
   <se:Title>label a</se:Title>
   <se:Abstract>rule a</se:Abstract>
  </se:Description>
  <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc">
   <ogc:PropertyIsEqualTo>
    <ogc:PropertyName>something</ogc:PropertyName>
    <ogc:Literal>1</ogc:Literal>
   </ogc:PropertyIsEqualTo>
  </ogc:Filter>
  <se:PointSymbolizer>
   <se:Graphic>
    <se:Mark>
     <se:WellKnownName>square</se:WellKnownName>
     <se:Fill>
      <se:SvgParameter name="fill">#649632</se:SvgParameter>
     </se:Fill>
     <se:Stroke>
      <se:SvgParameter name="stroke">#232323</se:SvgParameter>
      <se:SvgParameter name="stroke-width">0.5</se:SvgParameter>
     </se:Stroke>
    </se:Mark>
    <se:Size>11</se:Size>
   </se:Graphic>
  </se:PointSymbolizer>
 </se:Rule>
 <se:Rule>
  <se:Name>label b</se:Name>
  <se:Description>
   <se:Title>label b</se:Title>
   <se:Abstract>rule b</se:Abstract>
  </se:Description>
  <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc">
   <ogc:PropertyIsEqualTo>
    <ogc:PropertyName>something</ogc:PropertyName>
    <ogc:Literal>2</ogc:Literal>
   </ogc:PropertyIsEqualTo>
  </ogc:Filter>
  <se:PointSymbolizer>
   <se:Graphic>
    <se:Mark>
     <se:WellKnownName>square</se:WellKnownName>
     <se:Fill>
      <se:SvgParameter name="fill">#649632</se:SvgParameter>
     </se:Fill>
     <se:Stroke>
      <se:SvgParameter name="stroke">#232323</se:SvgParameter>
      <se:SvgParameter name="stroke-width">0.5</se:SvgParameter>
     </se:Stroke>
    </se:Mark>
    <se:Size>11</se:Size>
   </se:Graphic>
  </se:PointSymbolizer>
 </se:Rule>
</FakeRoot>
"""
        self.assertEqual(dom.toString(), expected)


if __name__ == "__main__":
    unittest.main()
