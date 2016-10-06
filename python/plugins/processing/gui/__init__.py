from qgis.PyQt import uic
import logging

uic.properties.logger.setLevel(logging.WARNING)
uic.uiparser.logger.setLevel(logging.WARNING)
uic.Compiler.qobjectcreator.logger.setLevel(logging.WARNING)
