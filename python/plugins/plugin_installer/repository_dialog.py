"""
Copyright (C) 2008 Matthew Perry
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
"""
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
