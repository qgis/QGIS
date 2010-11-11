# -*- coding: utf-8 -*-

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *

from ui_dialogSettings import Ui_GdalToolsSettingsDialog as Ui_Dialog
import GdalTools_utils as Utils

class GdalToolsSettingsDialog( QDialog, Ui_Dialog ):
  def __init__( self, iface ):
    QDialog.__init__( self, iface.mainWindow() )
    self.iface = iface
    self.setupUi( self )

    self.leGdalBinPath.setText( Utils.getGdalPath() )
    self.leGdalHelpPath.setText( Utils.getHelpPath() )

    QObject.connect( self.btnSetBinPath, SIGNAL( "clicked()" ), self.setBinPath )
    QObject.connect( self.btnSetHelpPath, SIGNAL( "clicked()" ), self.setHelpPath )

  def setBinPath( self ):
    inputDir = Utils.FileDialog.getExistingDirectory( self, self.tr( "Select directory with GDAL executables" ) )
    if inputDir.isEmpty():
      return

    self.leGdalBinPath.setText( inputDir )

  def setHelpPath( self ):
    inputDir = Utils.FileDialog.getExistingDirectory( self, self.tr( "Select directory with the GDAL documentation" ) )
    if inputDir.isEmpty():
      return

    self.leGdalHelpPath.setText( inputDir )


  def accept( self ):
    Utils.setGdalPath( self.leGdalBinPath.text() )
    Utils.setHelpPath( self.leGdalHelpPath.text() )
    QDialog.accept( self )
