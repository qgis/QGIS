# -*- coding: utf-8 -*-
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *

class GdalToolsSRSDialog(QDialog):
  def __init__(self, title, parent):
      QDialog.__init__(self, parent)
      self.setWindowTitle( title )

      layout = QVBoxLayout()
      self.selector = QgsProjectionSelector(self)
      buttonBox = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Close)

      layout.addWidget(self.selector)
      layout.addWidget(buttonBox)
      self.setLayout(layout)

      self.connect(buttonBox, SIGNAL("accepted()"), self.accept)
      self.connect(buttonBox, SIGNAL("rejected()"), self.reject)

  def epsg(self):
      return "EPSG:" + str(self.selector.selectedEpsg())

  def proj4string(self):
      return self.selector.selectedProj4String()

  def getProjection(self):
      if self.selector.selectedEpsg() != 0:
        return self.epsg()

      if not self.selector.selectedProj4String().isEmpty():
        return self.proj4string()

      return QString()

