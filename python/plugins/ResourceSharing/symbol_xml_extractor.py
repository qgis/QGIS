# coding=utf-8
import os

from qgis.PyQt.QtXml import QDomDocument
from qgis.PyQt.QtCore import QFile, QIODevice
try:
    from qgis.core import QgsSymbolLayerV2Utils as QgsSymbolLayerUtils
except ImportError:
    from qgis.core import QgsSymbolLayerUtils


class SymbolXMLExtractor(object):
    """Parses the given file and returns the symbols and colorramps"""

    def __init__(self, xml_path):
        """Constructor of the class.

        :param xml_path: The path to the symbol xml
        :type xml_path: str
        """
        self._xml_path = xml_path
        self._symbols = []
        self._colorramps = []
        # Parse the xml to get the symbols and colorramps
        self.parse_xml()

    def parse_xml(self):
        """Parse the xml file. Returns false if there is failure."""
        xml_file = QFile(self._xml_path)
        if not xml_file.open(QIODevice.ReadOnly):
            return False

        document = QDomDocument()
        if not document.setContent(xml_file):
            return False

        xml_file.close()

        document_element = document.documentElement()
        if document_element.tagName() != 'qgis_style':
            return False

        # Get all the symbols
        self._symbols = []
        symbols_element = document_element.firstChildElement('symbols')
        symbol_element = symbols_element.firstChildElement()
        while not symbol_element.isNull():
            if symbol_element.tagName() == 'symbol':
                symbol = QgsSymbolLayerUtils.loadSymbol(symbol_element)
                if symbol:
                    self._symbols.append({
                        'name': symbol_element.attribute('name'),
                        'symbol': symbol
                    })
            symbol_element = symbol_element.nextSiblingElement()

        # Get all the colorramps
        self._colorramps = []
        ramps_element = document_element.firstChildElement('colorramps')
        ramp_element = ramps_element.firstChildElement()
        while not ramp_element.isNull():
            if ramp_element.tagName() == 'colorramp':
                colorramp = QgsSymbolLayerUtils.loadColorRamp(ramp_element)
                if colorramp:
                    self._colorramps.append({
                        'name': ramp_element.attribute('name'),
                        'colorramp': colorramp
                    })

            ramp_element = ramp_element.nextSiblingElement()

        return True

    @property
    def symbols(self):
        """Return list of symbols in the xml.

        The structure of the property:
        symbols = [
            {
                'name': str
                'symbol': QgsSymbolV2
            }
        ]
        """
        return self._symbols

    @property
    def colorramps(self):
        """Return list of colorramps in the xml.

        The structure of the property:
        colorramps = [
            {
                'name': str
                'colorramp': QgsVectorColorRampV2
            }
        ]
        """
        return self._colorramps
