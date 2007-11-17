from PyQt4.QtCore import *
from PyQt4.QtGui import *

from qgis.core import QgsContextHelp

from gui import Ui_Dialog
import resources

class InstallerPluginGui(QDialog, Ui_Dialog):
  def __init__(self, parent, fl):
    QDialog.__init__(self, parent, fl)
    
    self.setupUi(self)
    
  def on_buttonBrowse_released(self):
    self.emit(SIGNAL("retrieveList(QString )"),"test" )
    print "browse"
 
  def on_pbnOK_released(self):
    #self.hide()
    self.emit(SIGNAL("installPlugin(QString )"), self.linePlugin.text() )
    return
    #self.done(1)

  def on_pbnCancel_clicked(self):
    self.close()
