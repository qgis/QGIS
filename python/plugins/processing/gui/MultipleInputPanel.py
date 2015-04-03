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

from PyQt4.QtGui import QWidget

from processing.gui.MultipleInputDialog import MultipleInputDialog
from processing.gui.MultipleFileInputDialog import MultipleFileInputDialog

from processing.ui.ui_widgetBaseSelector import Ui_Form

class MultipleInputPanel(QWidget, Ui_Form):

    def __init__(self, options=None, datatype=None):
        QWidget.__init__(self)
        self.setupUi(self)

        self.leText.setEnabled(False)
        self.leText.setText(self.tr('0 elements selected'))

        self.btnSelect.clicked.connect(self.showSelectionDialog)

        self.options = options
        self.datatype = datatype
        self.selectedoptions = []

    def setSelectedItems(self, selected):
        # No checking is performed!
        self.selectedoptions = selected
        self.leText.setText(
            self.tr('%d elements selected') % len(self.selectedoptions))

    def showSelectionDialog(self):
        if self.datatype is None:
            dlg = MultipleInputDialog(self.options, self.selectedoptions)
        else:
            dlg = MultipleFileInputDialog(self.selectedoptions)
        dlg.exec_()
        if dlg.selectedoptions is not None:
            self.selectedoptions = dlg.selectedoptions
            self.leText.setText(
                self.tr('%d elements selected') % len(self.selectedoptions))
