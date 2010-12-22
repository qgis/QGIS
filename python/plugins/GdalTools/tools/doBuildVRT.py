# -*- coding: utf-8 -*-
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *

from ui_widgetBuildVRT import Ui_GdalToolsWidget as Ui_Widget
from widgetPluginBase import GdalToolsBasePluginWidget as BasePluginWidget
import GdalTools_utils as Utils

class GdalToolsDialog(QWidget, Ui_Widget, BasePluginWidget):

  def __init__(self, iface):
      QWidget.__init__(self)
      self.iface = iface
      self.resolutions = ("highest", "average", "lowest")

      self.setupUi(self)
      BasePluginWidget.__init__(self, self.iface, "gdalbuildvrt")

      self.recurseCheck.hide()

      self.setParamsStatus(
        [
          (self.inputFilesEdit, SIGNAL("textChanged(const QString &)")), 
          (self.outputFileEdit, SIGNAL("textChanged(const QString &)")), 
          (self.resolutionComboBox, SIGNAL("currentIndexChanged(int)"), self.resolutionCheck),
          (self.srcNoDataSpin, SIGNAL("valueChanged(int)"), self.srcNoDataCheck, "1.7.0"),
          (self.separateCheck, SIGNAL("stateChanged(int)"), None, "1.7.0"),
          (self.inputDirCheck, SIGNAL("stateChanged(int)")),
          (self.recurseCheck, SIGNAL("stateChanged(int)"), self.inputDirCheck)
        ]
      )

      self.connect(self.selectInputFilesButton, SIGNAL("clicked()"), self.fillInputFilesEdit)
      self.connect(self.selectOutputFileButton, SIGNAL("clicked()"), self.fillOutputFileEdit)
      self.connect( self.inputDirCheck, SIGNAL( "stateChanged( int )" ), self.switchToolMode )

  def switchToolMode(self):
      self.inputFilesEdit.clear()

      if self.inputDirCheck.isChecked():
        self.inFileLabel = self.label.text()
        self.label.setText( QCoreApplication.translate( "GdalTools", "&Input directory" ) )

        self.recurseCheck.show()

        QObject.disconnect( self.selectInputFilesButton, SIGNAL( "clicked()" ), self.fillInputFilesEdit )
        QObject.connect( self.selectInputFilesButton, SIGNAL( "clicked()" ), self.fillInputDir )
      else:
        self.label.setText( self.inFileLabel )

        self.recurseCheck.hide()

        QObject.connect( self.selectInputFilesButton, SIGNAL( "clicked()" ), self.fillInputFilesEdit )
        QObject.disconnect( self.selectInputFilesButton, SIGNAL( "clicked()" ), self.fillInputDir )

  def fillInputFilesEdit(self):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      files = Utils.FileDialog.getOpenFileNames(self, self.tr( "Select the files for VRT" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter)
      if files.isEmpty():
        return
      Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

      self.inputFilesEdit.setText(files.join(","))

  def fillOutputFileEdit(self):
      outputFile = Utils.FileDialog.getSaveFileName(self, self.tr( "Select where to save the VRT" ), self.tr( "VRT (*.vrt)" ))
      if outputFile.isEmpty():
        return

      self.outputFileEdit.setText(outputFile)

  def fillInputDir( self ):
      inputDir = Utils.FileDialog.getExistingDirectory( self, self.tr( "Select the input directory with files for VRT" ))
      if inputDir.isEmpty():
        return

      self.inputFilesEdit.setText( inputDir )

  def getArguments(self):
      arguments = QStringList()
      if self.resolutionCheck.isChecked() and self.resolutionComboBox.currentIndex() >= 0:
        arguments << "-resolution"
        arguments << self.resolutions[self.resolutionComboBox.currentIndex()]
      if self.separateCheck.isChecked():
        arguments << "-separate"
      if self.srcNoDataCheck.isChecked():
        arguments << "-srcnodata"
        arguments << str(self.srcNoDataSpin.value())
      arguments << self.outputFileEdit.text()
      if self.inputDirCheck.isChecked():
        arguments << Utils.getRasterFiles( self.inputFilesEdit.text(), self.recurseCheck.isChecked() )
      else:
        arguments << self.inputFilesEdit.text().split(",")
      return arguments

  def getOutputFileName(self):
      return self.outputFileEdit.text()

  def addLayerIntoCanvas(self, fileInfo):
      self.iface.addRasterLayer(fileInfo.filePath())
