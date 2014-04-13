# -*- coding: utf-8 -*-

"""
***************************************************************************
    MultipleFileInputPanel.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
                           (C) 2013 by CS Systemes d'information (CS SI)
    Email                : volayaf at gmail dot com
                           otb at c-s dot fr (CS SI)
    Contributors         : Victor Olaya  - basis from MultipleInputPanel
                           Alexia Mondot (CS SI) - adapt for a new parameter
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

from PyQt4 import  QtGui
from processing.gui.MultipleFileInputDialog import MultipleFileInputDialog

class MultipleFileInputPanel(QtGui.QWidget):

    def __init__(self, parent = None):
        super(MultipleFileInputPanel, self).__init__(parent)
        self.selectedoptions = []
        self.horizontalLayout = QtGui.QHBoxLayout(self)
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.label = QtGui.QLabel()
        self.label.setText("0 elements selected")
        self.label.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)
        self.horizontalLayout.addWidget(self.label)
        self.pushButton = QtGui.QPushButton()
        self.pushButton.setText("...")
        self.pushButton.clicked.connect(self.showSelectionDialog)
        self.horizontalLayout.addWidget(self.pushButton)
        self.setLayout(self.horizontalLayout)

    def setSelectedItems(self, selected):
        #no checking is performed!
        self.selectedoptions = selected
        self.label.setText(str(len(self.selectedoptions)) + " elements selected")

    def showSelectionDialog(self):
        dlg = MultipleFileInputDialog(self.selectedoptions)
        dlg.exec_()
        if dlg.selectedoptions != None:
            self.selectedoptions = dlg.selectedoptions
            self.label.setText(str(len(self.selectedoptions)) + " elements selected")
