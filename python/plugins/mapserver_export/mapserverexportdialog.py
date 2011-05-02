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
from ms_export import defaults
from ui_qgsmapserverexportbase import Ui_QgsMapserverExportBase
# create the dialog for mapserver export
class MapServerExportDialog(QtGui.QDialog): 
  def __init__(self):
    QtGui.QDialog.__init__(self) 
    # Set up the user interface from Designer. 
    self.ui = Ui_QgsMapserverExportBase() 
    self.ui.setupUi(self) 

    units =  ["meters", "dd", "feet", "miles", "inches", "kilometers"]
    # make them able to be translated
    tr_units = [ QtGui.QApplication.translate("QgsMapserverExportBase", "meters", None, QtGui.QApplication.UnicodeUTF8), QtGui.QApplication.translate("QgsMapserverExportBase", "dd", None, QtGui.QApplication.UnicodeUTF8), QtGui.QApplication.translate("QgsMapserverExportBase", "feet", None, QtGui.QApplication.UnicodeUTF8), QtGui.QApplication.translate("QgsMapserverExportBase", "miles", None, QtGui.QApplication.UnicodeUTF8), QtGui.QApplication.translate("QgsMapserverExportBase", "inches", None, QtGui.QApplication.UnicodeUTF8), QtGui.QApplication.translate("QgsMapserverExportBase", "kilometers", None, QtGui.QApplication.UnicodeUTF8) ]
    for unit in units:
        self.ui.cmbMapUnits.addItem( QtGui.QApplication.translate("QgsMapserverExportBase", unit, None, QtGui.QApplication.UnicodeUTF8), QtCore.QVariant(unit) )
    
    # TODO: set default unit. Is now the first value entered in the unit-list above

    # Set defaults from ms_export.py:
    self.ui.txtMapServerUrl.setText(defaults.mapServerUrl)
    self.ui.txtFontsetPath.setText(defaults.fontsPath)
    self.ui.txtSymbolsetPath.setText(defaults.symbolsPath)
    self.ui.checkBoxAntiAlias.setChecked(defaults.antialias)
    self.ui.checkBoxDump.setChecked(defaults.dump)
    self.ui.checkBoxForce.setChecked(defaults.force)
    self.ui.checkBoxPartials.setChecked(defaults.partials)
    self.ui.txtMapWidth.setText(defaults.width)
    self.ui.txtMapHeight.setText(defaults.height)



