# -*- coding: utf-8 -*-

"""
***************************************************************************
    optionsTable.py
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

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from ui_optionsTable import Ui_GdalToolsOptionsTable as Ui_OptionsTable

class GdalToolsOptionsTable(QWidget, Ui_OptionsTable):

  def __init__(self, parent = None):
      QWidget.__init__(self, parent)

      self.setupUi(self)

      self.connect(self.table, SIGNAL("cellChanged(int, int)"), SIGNAL("cellValueChanged(int, int)"))

      self.connect(self.table, SIGNAL("itemSelectionChanged()"), self.enableDeleteButton)
      self.connect(self.btnAdd, SIGNAL("clicked()"), self.addNewRow)
      self.connect(self.btnDel, SIGNAL("clicked()"), self.deleteRow)

      self.btnDel.setEnabled(False)

  def enableDeleteButton(self):
      self.btnDel.setEnabled(self.table.currentRow() >= 0)

  def addNewRow(self):
      self.table.insertRow(self.table.rowCount())
      # select the added row
      newRow = self.table.rowCount()-1;
      item = QTableWidgetItem()
      self.table.setItem(newRow, 0, item)
      self.table.setCurrentItem(item)
      self.emit(SIGNAL("rowAdded(int)"), newRow)

  def deleteRow(self):
      if self.table.currentRow() >= 0:
        self.table.removeRow(self.table.currentRow())
        # select the previous row or the next one if there is no previous row
        item = self.table.item(self.table.currentRow(), 0)
        self.table.setCurrentItem(item)
        self.emit(SIGNAL("rowRemoved()"))

  def options(self):
      options = []
      for row in range(0, self.table.rowCount()):
        name = self.table.item(row, 0)
        if not name:
          continue

        value = self.table.item(row, 1)
        if not value:
          continue

        options.append( name.text() + "=" + value.text())
      return options

