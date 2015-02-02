# -*- coding: utf-8 -*-

"""
***************************************************************************
    doOverview.py
    ---------------------
    Date                 : June 2010
    Copyright            : (C) 2010 by Giuseppe Sucameli
    Email                : brush dot tyler at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Giuseppe Sucameli'
__date__ = 'June 2010'
__copyright__ = '(C) 2010, Giuseppe Sucameli'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from PyQt4.QtCore import QObject, SIGNAL, QCoreApplication
from PyQt4.QtGui import QWidget
from qgis.core import QgsRaster

from ui_widgetOverview import Ui_GdalToolsWidget as Ui_Widget
from widgetBatchBase import GdalToolsBaseBatchWidget as BaseBatchWidget
import GdalTools_utils as Utils

class GdalToolsDialog( QWidget, Ui_Widget, BaseBatchWidget ):

  def __init__( self, iface ):
      QWidget.__init__( self )
      self.iface = iface
      self.resampling_method = ('nearest', 'average', 'gauss', 'cubic', 'average_mp', 'average_magphase', 'mode')

      self.setupUi( self )
      BaseBatchWidget.__init__( self, self.iface, "gdaladdo" )

      # set the default QSpinBoxes and QProgressBar value
      self.progressBar.setValue(0)

      self.progressBar.hide()
      # we don't need load to canvas functionality
      self.base.loadCheckBox.hide()

      self.setParamsStatus([
          ( self.inSelector, SIGNAL("filenameChanged()")),
          ( self.cleanCheck, SIGNAL( "stateChanged(int)" ), None, 1700 ),
          ( self.mPyramidOptionsWidget, SIGNAL( "overviewListChanged()" )),
          ( self.mPyramidOptionsWidget, SIGNAL( "someValueChanged()" ))
      ])

      self.connect( self.inSelector, SIGNAL( "selectClicked()" ), self.fillInputFile )
      self.connect( self.batchCheck, SIGNAL( "stateChanged( int )" ), self.switchToolMode )

      self.init = False  # workaround bug that pyramid options widgets are not initialized at first

  # make sure we get a command line when dialog appears
  def show_(self):
      BaseBatchWidget.show_(self)
      self.someValueChanged()

  # switch to batch or normal mode
  def switchToolMode( self ):
      self.setCommandViewerEnabled( not self.batchCheck.isChecked() )
      self.progressBar.setVisible( self.batchCheck.isChecked() )

      self.inSelector.setType( self.inSelector.FILE if self.batchCheck.isChecked() else self.inSelector.FILE_LAYER )

      if self.batchCheck.isChecked():
        self.inFileLabel = self.label.text()
        self.label.setText( QCoreApplication.translate( "GdalTools", "&Input directory" ) )

        QObject.disconnect( self.inSelector, SIGNAL( "selectClicked()" ), self.fillInputFile )
        QObject.connect( self.inSelector, SIGNAL( "selectClicked()" ), self.fillInputDir )
      else:
        self.label.setText( self.inFileLabel )

        QObject.disconnect( self.inSelector, SIGNAL( "selectClicked()" ), self.fillInputDir )
        QObject.connect( self.inSelector, SIGNAL( "selectClicked()" ), self.fillInputFile )

  def onLayersChanged(self):
      self.inSelector.setLayers( Utils.LayerRegistry.instance().getRasterLayers() )

  def fillInputFile( self ):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      inputFile = Utils.FileDialog.getOpenFileName( self, self.tr( "Select the input file" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter )
      if inputFile == '':
        return
      Utils.FileFilter.setLastUsedRasterFilter( lastUsedFilter )

      self.inSelector.setFilename( inputFile )

      self.mPyramidOptionsWidget.setRasterLayer(None)

  def fillInputDir( self ):
      inputDir = Utils.FileDialog.getExistingDirectory( self, self.tr( "Select the input directory with files" ))
      if inputDir == '':
        return

      self.inSelector.setFilename( inputDir )

  def getArguments( self ):
      arguments = []

      arguments.append("-r")
      arguments.append(self.mPyramidOptionsWidget.resamplingMethod())

      format = self.mPyramidOptionsWidget.pyramidsFormat()
      if format == QgsRaster.PyramidsGTiff:
        arguments.append("-ro")
      elif format == QgsRaster.PyramidsErdas:
        arguments.append("--config")
        arguments.append("USE_RRD")
        arguments.append("YES")

      for option in self.mPyramidOptionsWidget.configOptions():
        (k,v) = option.split("=")
        arguments.append("--config")
        arguments.append(str(k))
        arguments.append(str(v))

      if self.cleanCheck.isChecked():
          arguments.append("-clean")

      # TODO fix batch enabled, argument order is wrong, levels not at end
      if self.isBatchEnabled():
        return arguments

      arguments.append(self.getInputFileName())

      if len(self.mPyramidOptionsWidget.overviewList()) > 0:
        for level in self.mPyramidOptionsWidget.overviewList():
          arguments.append(str(level))

      # set creation options filename/layer for validation
      if self.init:
        if self.isBatchEnabled():
          self.mPyramidOptionsWidget.setRasterLayer(None)
        elif self.inSelector.layer():
          self.mPyramidOptionsWidget.setRasterLayer(self.inSelector.layer())
        else:
          self.mPyramidOptionsWidget.setRasterFileName(self.getInputFileName())
      else:
        self.init = True

      return arguments

  def getInputFileName( self ):
      return self.inSelector.filename()

  def getOutputFileName( self ):
      return self.inSelector.filename()

  def addLayerIntoCanvas(self, fileInfo):
      self.iface.addRasterLayer(fileInfo.filePath())

  def getBatchArguments(self, inFile, outFile=None):
      arguments = self.getArguments()
      arguments.append(inFile)
      if len(self.mPyramidOptionsWidget.overviewList()) == 0:
        arguments.extend(["2", "4", "8", "16", "32"])
      else:
        for level in self.mPyramidOptionsWidget.overviewList():
          arguments.append(str(level))
      return arguments

  def isBatchEnabled(self):
      return self.batchCheck.isChecked()

  def onFinished(self, exitCode, status):
      if not self.isBatchEnabled():
        from widgetPluginBase import GdalToolsBasePluginWidget as BasePluginWidget
        BasePluginWidget.onFinished(self, exitCode, status)
        return

      msg = str( self.base.process.readAllStandardError() )
      if msg != '':
        self.errors.append( ">> " + self.inFiles[self.batchIndex] + "<br>" + msg.replace( "\n", "<br>" ) )

      self.base.process.close()
      self.batchIndex += 1
      self.runItem( self.batchIndex, self.batchTotal )

  def setProgressRange(self, maximum):
      self.progressBar.setRange(0, maximum)

  def updateProgress(self, index, total):
      if index < total:
        self.progressBar.setValue( index + 1 )
      else:
        self.progressBar.setValue( 0 )
