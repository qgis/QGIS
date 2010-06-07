# -*- coding: utf-8 -*-

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *

from ui_dialogAbout import Ui_GdalToolsAboutDialog as Ui_Dialog
from GdalTools import version

class GdalToolsAboutDialog(QDialog, Ui_Dialog):

  def __init__(self, iface):
      QDialog.__init__(self, iface.mainWindow())
      self.iface = iface
      self.setupUi(self)

      QObject.connect(self.btnWeb, SIGNAL("clicked()"), self.openWebsite)

      self.lblVersion.setText( version() )
      self.textEdit.setText(self.getText())

  def getText(self):
    return self.tr("""GDAL Tools (AKA Raster Tools) is a plugin for QuantumGIS aiming at making life simpler for users of GDAL Utilities, providing a simplified graphical interface for most commonly used programs.

The plugin is being developed by Faunalia (http://faunalia.it) with help from GIS-lab (http://gis-lab.info).
Icons by Robert Szczepanek.
Sponsorship by Silvio Grosso was much appreciated.

Please help us by testing the tools, reporting eventual issues, improving the code, or providing financial support.

DEVELOPERS:
  Faunalia
    Paolo Cavallini
    Giuseppe Sucameli
    Lorenzo Masini
  GIS-lab
    Maxim Dubinin
    Alexander Bruy
icons by Robert Szepanek

HOMEPAGE:
http://trac.faunalia.it/GdalTools-plugin""")

  def openWebsite(self):
      url = QUrl("http://trac.faunalia.it/GdalTools-plugin")
      QDesktopServices.openUrl(url)


