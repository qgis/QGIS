# -*- coding: utf-8 -*-

"""
***************************************************************************
    MultipleInputPanel.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
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
from processing.gui.MultipleInputDialog import MultipleInputDialog

class MultipleInputPanel(QtGui.QWidget):

    def __init__(self, options, datatype = None, parent = None):
        super(MultipleInputPanel, self).__init__(parent)
        self.options = options
        self.datatype = datatype
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
        #=======================================================================
        # #If there is a datatype, we use it to create the list of options
        # if self.datatype is not None:
        #    if self.datatype == ParameterMultipleInput.TYPE_RASTER:
        #        options = dataobjects.getRasterLayers()
        #    elif self.datatype == ParameterMultipleInput.TYPE_VECTOR_ANY:
        #        options = dataobjects.getVectorLayers()
        #    else:
        #        options = dataobjects.getVectorLayers(self.datatype)
        #    opts = []
        #    for opt in options:
        #        opts.append(opt.name())
        #    self.options = opts
        #=======================================================================
        dlg = MultipleInputDialog(self.options, self.selectedoptions)
        dlg.exec_()
        if dlg.selectedoptions != None:
            self.selectedoptions = dlg.selectedoptions
            self.label.setText(str(len(self.selectedoptions)) + " elements selected")
