# -*- coding: utf-8 -*-

"""
***************************************************************************
    BatchOutputSelectionPanel.py
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

import os

from PyQt4.QtGui import *
from PyQt4.QtCore import *

from processing.gui.AutofillDialog import AutofillDialog
from processing.core.parameters import ParameterMultipleInput
from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterTable
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterFixedTable


class BatchOutputSelectionPanel(QWidget):

    def __init__(self, output, alg, row, col, panel):
        super(BatchOutputSelectionPanel, self).__init__(None)
        self.alg = alg
        self.row = row
        self.col = col
        self.output = output
        self.panel = panel
        self.table = self.panel.tblParameters
        self.horizontalLayout = QHBoxLayout(self)
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.text = QLineEdit()
        self.text.setText('')
        self.text.setSizePolicy(QSizePolicy.Expanding,
                                QSizePolicy.Expanding)
        self.horizontalLayout.addWidget(self.text)
        self.pushButton = QPushButton()
        self.pushButton.setText('...')
        self.pushButton.clicked.connect(self.showSelectionDialog)
        self.horizontalLayout.addWidget(self.pushButton)
        self.setLayout(self.horizontalLayout)

    def showSelectionDialog(self):
        filefilter = self.output.getFileFilter(self.alg)
        settings = QSettings()
        if settings.contains('/Processing/LastBatchOutputPath'):
            path = unicode(settings.value('/Processing/LastBatchOutputPath'))
        else:
            path = ''
        filename = QFileDialog.getSaveFileName(self, self.tr('Save file'), path,
                filefilter)
        if filename:
            filename = unicode(filename)
            settings.setValue('/Processing/LastBatchOutputPath', os.path.dirname(filename))
            dlg = AutofillDialog(self.alg)
            dlg.exec_()
            if dlg.mode is not None:
                try:
                    if dlg.mode == AutofillDialog.DO_NOT_AUTOFILL:
                        self.table.cellWidget(self.row,
                                self.col).setValue(filename)
                    elif dlg.mode == AutofillDialog.FILL_WITH_NUMBERS:
                        n = self.table.rowCount() - self.row
                        for i in range(n):
                            name = filename[:filename.rfind('.')] \
                                + str(i + 1) + filename[filename.rfind('.'):]
                            self.table.cellWidget(i + self.row,
                                    self.col).setValue(name)
                    elif dlg.mode == AutofillDialog.FILL_WITH_PARAMETER:
                        n = self.table.rowCount() - self.row
                        for i in range(n):
                            widget = self.table.cellWidget(i + self.row,
                                    dlg.param)
                            param = self.alg.parameters[dlg.param]
                            if isinstance(param, (ParameterRaster,
                                    ParameterVector, ParameterTable,
                                    ParameterMultipleInput)):
                                s = unicode(widget.getText())
                                s = os.path.basename(s)
                                s = os.path.splitext(s)[0]
                            elif isinstance(param, ParameterBoolean):
                                s = str(widget.currentIndex() == 0)
                            elif isinstance(param, ParameterSelection):
                                s = unicode(widget.currentText())
                            elif isinstance(param, ParameterFixedTable):
                                s = unicode(widget.table)
                            else:
                                s = unicode(widget.text())
                            name = filename[:filename.rfind('.')] + s \
                                + filename[filename.rfind('.'):]
                            self.table.cellWidget(i + self.row,
                                    self.col).setValue(name)
                except:
                    pass

    def setValue(self, text):
        return self.text.setText(text)

    def getValue(self):
        return unicode(self.text.text())
