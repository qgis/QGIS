
from PyQt4.QtCore import *
from PyQt4.QtGui import *

from repository_ui import Ui_RepositoryDetailsDialog

class RepositoryDialog(QDialog, Ui_RepositoryDetailsDialog):
  def __init__(self, parent=None):
    QDialog.__init__(self, parent)
    
    self.setupUi(self)
    
    self.connect(self.editName, SIGNAL("textChanged(const QString &)"), self.changed)
    self.connect(self.editURL,  SIGNAL("textChanged(const QString &)"), self.changed)
    
    self.changed(None)
    
  def changed(self, string):
    enable = (self.editName.text().count() > 0 and self.editURL.text().count() > 0)
    self.buttonBox.button(QDialogButtonBox.Ok).setEnabled(enable)
