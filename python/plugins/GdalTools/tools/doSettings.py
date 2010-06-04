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

    QObject.connect( self.btnSetBinPath, SIGNAL( "clicked()" ), self.setBinPath )

  def setBinPath( self ):
    inputDir = Utils.FileDialog.getExistingDirectory( self, self.tr( "Select directory with GDAL executables" ) )
    if inputDir.isEmpty():
      return

    self.leGdalBinPath.setText( inputDir )

  def accept( self ):
    Utils.setGdalPath( self.leGdalBinPath.text() )
    QDialog.accept( self )
