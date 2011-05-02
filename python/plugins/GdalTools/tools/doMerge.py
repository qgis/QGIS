# -*- coding: utf-8 -*-
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *

from ui_widgetMerge import Ui_GdalToolsWidget as Ui_Widget
from widgetPluginBase import GdalToolsBasePluginWidget as BasePluginWidget
import GdalTools_utils as Utils

class GdalToolsDialog(QWidget, Ui_Widget, BasePluginWidget):

  def __init__(self, iface):
      QWidget.__init__(self)
      self.iface = iface

      self.setupUi(self)
      BasePluginWidget.__init__(self, self.iface, "gdal_merge.py")

      self.inSelector.setType( self.inSelector.FILE )
      self.outSelector.setType( self.outSelector.FILE )

      self.outputFormat = Utils.fillRasterOutputFormat()
      self.extent = None

      self.setParamsStatus(
        [
          (self.inSelector, SIGNAL("filenameChanged()")), 
          (self.outSelector, SIGNAL("filenameChanged()")), 
          (self.noDataSpin, SIGNAL("valueChanged(int)"), self.noDataCheck),
	      ( self.separateCheck, SIGNAL( "stateChanged( int )" ) ),
          ( self.pctCheck, SIGNAL( "stateChanged( int )" ) ),
          ( self.intersectCheck, SIGNAL( "stateChanged( int )" ) ),
          (self.creationOptionsTable, [SIGNAL("cellValueChanged(int, int)"), SIGNAL("rowRemoved()")], self.creationGroupBox)
        ]
      )

      self.connect(self.inSelector, SIGNAL("selectClicked()"), self.fillInputFilesEdit)
      self.connect(self.outSelector, SIGNAL("selectClicked()"), self.fillOutputFileEdit)
      self.connect(self.intersectCheck, SIGNAL("stateChanged(int)"), self.refreshExtent)

  def fillInputFilesEdit(self):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      files = Utils.FileDialog.getOpenFileNames(self, self.tr( "Select the files to Merge" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter )
      if files.isEmpty():
        return
      Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

      self.inSelector.setFilename(files.join(","))
      self.intersectCheck.setEnabled( files.count() > 1 )
      self.refreshExtent()

  def refreshExtent(self):
      files = self.getInputFileName()
      if files.count() < 2:
        self.intersectCheck.setChecked( False )
        self.extent = None
        return

      self.extent = self.getExtent()
      self.someValueChanged()

      if not self.intersectCheck.isChecked():
        return

      if self.extent == None:
        QMessageBox.warning( self, self.tr( "Error retrieving the extent" ), self.tr( 'GDAL was unable to retrieve the extent from any file. \nThe "Use intersected extent" option will be unchecked.' ) )
        self.intersectCheck.setChecked( False )
      elif self.extent.isEmpty():
        QMessageBox.warning( self, self.tr( "Empty extent" ), self.tr( 'The computed extent is empty. \nDisable the "Use intersected extent" option to have a nonempty output.' ) )
 
  def fillOutputFileEdit(self):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      outputFile = Utils.FileDialog.getSaveFileName(self, self.tr( "Select where to save the Merge output" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter )
      if outputFile.isEmpty():
        return
      Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

      self.outputFormat = Utils.fillRasterOutputFormat( lastUsedFilter, outputFile )
      self.outSelector.setFilename( outputFile )

  def getArguments(self):
      arguments = QStringList()
      if self.intersectCheck.isChecked():
        if self.extent != None:
          arguments << "-ul_lr"
          arguments << str( self.extent.xMinimum() )
          arguments << str( self.extent.yMaximum() )
          arguments << str( self.extent.xMaximum() )
          arguments << str( self.extent.yMinimum() )
      if self.noDataCheck.isChecked():
        arguments << "-n"
        arguments << str(self.noDataSpin.value())
      if self.separateCheck.isChecked():
        arguments << "-separate"
      if self.pctCheck.isChecked():
        arguments << "-pct"
      if self.creationGroupBox.isChecked():
        for opt in self.creationOptionsTable.options():
          arguments << "-co"
          arguments << opt
      outputFn = self.getOutputFileName()
      if not outputFn.isEmpty():
        arguments << "-of"
        arguments << self.outputFormat
        arguments << "-o"
        arguments << outputFn
      arguments << self.getInputFileName()
      return arguments

  def getOutputFileName(self):
      return self.outSelector.filename()

  def getInputFileName(self):
      return self.inSelector.filename().split( "," )

  def addLayerIntoCanvas(self, fileInfo):
      self.iface.addRasterLayer(fileInfo.filePath())

  def getExtent( self ):
    files = self.getInputFileName()

    i = 0
    res = rect2 = None
    for fileName in files:
      if res == None:
        res = Utils.getRasterExtent( self, fileName )
        continue
      rect2 = Utils.getRasterExtent( self, fileName )
      if rect2 == None:
        continue
      res = res.intersect( rect2 )

    return res

