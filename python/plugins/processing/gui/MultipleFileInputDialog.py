# -*- coding: utf-8 -*-

"""
***************************************************************************
    MultipleExternalInputDialog.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
                           (C) 2013 by CS Systemes d'information (CS SI)
    Email                : volayaf at gmail dot com
                           otb at c-s dot fr (CS SI)
    Contributors         : Victor Olaya  - basis from MultipleInputDialog
                           Alexia Mondot (CS SI) - new parameter
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from PyQt4 import QtCore, QtGui
import os

class MultipleFileInputDialog(QtGui.QDialog):

    def __init__(self, selectedoptions):
        self.selectedoptions = selectedoptions
        self.options=selectedoptions
        QtGui.QDialog.__init__(self)
        self.setModal(True)
        self.setupUi()
        self.selectedoptions = None

    def setupUi(self):
        self.resize(381, 320)
        self.setWindowTitle("Multiple selection")
        self.horizontalLayout = QtGui.QHBoxLayout(self)
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.buttonBox = QtGui.QDialogButtonBox()
        self.buttonBox.setOrientation(QtCore.Qt.Vertical)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
        self.buttonPlus = QtGui.QPushButton("+")
        self.buttonBox.addButton(self.buttonPlus, QtGui.QDialogButtonBox.ActionRole)
        self.buttonMoins = QtGui.QPushButton("-")
        self.buttonBox.addButton(self.buttonMoins, QtGui.QDialogButtonBox.ActionRole)
        self.table = QtGui.QTableWidget()
        self.table.setColumnCount(1)
        self.table.setColumnWidth(0,270)
        self.table.verticalHeader().setVisible(False)
        self.table.horizontalHeader().setVisible(False)
        self.table.horizontalHeader().setResizeMode(QtGui.QHeaderView.Stretch)
        self.horizontalLayout.addWidget(self.table)
        self.horizontalLayout.addWidget(self.buttonBox)
        self.setLayout(self.horizontalLayout)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL("accepted()"), self.okPressed)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL("rejected()"), self.cancelPressed)
        QtCore.QObject.connect(self.buttonPlus, QtCore.SIGNAL("clicked()"), self.addFile)
        QtCore.QObject.connect(self.buttonMoins, QtCore.SIGNAL("clicked()"), self.removeFile)
        QtCore.QMetaObject.connectSlotsByName(self)

    def setTableContent(self):
        self.table.setRowCount(len(self.options))
        for i in range(len(self.options)):
            item = QtGui.QLabel()
            item.setText(self.options[i])
            self.table.setCellWidget(i,0, item)

    def okPressed(self):
        self.selectedoptions = []
        self.selectedoptions = self.options
        self.close()

    def cancelPressed(self):
        self.selectedoptions = None
        self.close()

    def addFile(self):
        settings = QtCore.QSettings()
        lastfolder = settings.value("processingFilesLastFolder")
        if lastfolder :
            path = lastfolder
        else :
            path = QtCore.QDir.currentPath()

        filesOpened = QtGui.QFileDialog.getOpenFileNames( None, "Select the file(s) to use", path, "All files (*.*)" )

        lastfile = ""
        for item in filesOpened:
            self.options.append( str(item) )
            lastfile=item

        self.setTableContent()
        folder = os.path.dirname( str( lastfile ) )
        settings.setValue("processingFilesLastFolder", folder)

    def removeFile(self):
        indexRow = self.table.currentRow()
        itemToRemove = self.options[indexRow]
        self.options.remove(itemToRemove)
        self.setTableContent()
