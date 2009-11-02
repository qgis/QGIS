"""
/***************************************************************************
        MapServerExport  - A QGIS plugin to export a saved project file
                            to a MapServer map file
                             -------------------
    begin                : 2008-01-07
    copyright            : (C) 2007 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
"""
from PyQt4 import QtCore, QtGui 
from ui_qgsmapserverexportbase import Ui_QgsMapserverExportBase
# create the dialog for mapserver export
class MapServerExportDialog(QtGui.QDialog): 
  def __init__(self): 
    QtGui.QDialog.__init__(self) 
    # Set up the user interface from Designer. 
    self.ui = Ui_QgsMapserverExportBase() 
    self.ui.setupUi(self) 

    for unit in ["dd", "feet", "meters", "miles", "inches", "kilometers"]:
        self.ui.cmbMapUnits.addItem( QtGui.QApplication.translate("QgsMapserverExportBase", unit, None, QtGui.QApplication.UnicodeUTF8), QtCore.QVariant(unit) )


