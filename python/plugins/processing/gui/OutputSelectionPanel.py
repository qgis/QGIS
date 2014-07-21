# -*- coding: utf-8 -*-

"""
***************************************************************************
    OutputSelectionPanel.py
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

import os.path
import re

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from qgis.gui import *

from processing.core.ProcessingConfig import ProcessingConfig
from processing.core.outputs import OutputVector
from processing.core.outputs import OutputDirectory

from processing.ui.ui_widgetOutputSelect import Ui_widgetOutputSelect

class OutputSelectionPanel(QWidget, Ui_widgetOutputSelect):

    SAVE_TO_TEMP_FILE = QCoreApplication.translate(
        'OutputSelectionPanel', '[Save to temporary file]')

    def __init__(self, output, alg):
        QWidget.__init__(self)
        self.setupUi(self)

        self.output = output
        self.alg = alg

        if hasattr(self.text, 'setPlaceholderText'):
            self.text.setPlaceholderText(self.SAVE_TO_TEMP_FILE)

        self.btnBrowse.clicked.connect(self.selectOutput)

    def selectOutput(self):
        if isinstance(self.output, OutputDirectory):
            self.selectDirectory()
        else:
            popupMenu = QMenu()

            actionSaveToTempFile = QAction(
                self.tr('Save to a temporary file'), self.btnBrowse)
            actionSaveToTempFile.triggered.connect(self.saveToTemporaryFile)
            popupMenu.addAction(actionSaveToTempFile)

            actionSaveToFile = QAction(
                self.tr('Save to file...'), self.btnBrowse)
            actionSaveToFile.triggered.connect(self.selectFile)
            popupMenu.addAction(actionSaveToFile)

            if isinstance(self.output, OutputVector) \
                    and self.alg.provider.supportsNonFileBasedOutput():
                actionSaveToMemory = QAction(
                    self.tr('Save to memory layer'), self.btnBrowse)
                actionSaveToMemory.triggered.connect(self.saveToMemory)
                popupMenu.addAction(actionSaveToMemory)

            popupMenu.exec_(QCursor.pos())

    def saveToTemporaryFile(self):
        self.text.setText('')

    def saveToMemory(self):
        self.text.setText('memory:')

    def selectFile(self):
        fileFilter = self.output.getFileFilter(self.alg)

        settings = QSettings()
        if settings.contains('/Processing/LastOutputPath'):
            path = settings.value('/Processing/LastOutputPath')
        else:
            path = ProcessingConfig.getSetting(ProcessingConfig.OUTPUT_FOLDER)

        encoding = settings.value('/Processing/encoding', 'System')
        fileDialog = QgsEncodingFileDialog(
            self, self.tr('Save file'), path, fileFilter, encoding)
        fileDialog.setFileMode(QFileDialog.AnyFile)
        fileDialog.setAcceptMode(QFileDialog.AcceptSave)
        fileDialog.setConfirmOverwrite(True)

        if fileDialog.exec_() == QDialog.Accepted:
            files = fileDialog.selectedFiles()
            encoding = unicode(fileDialog.encoding())
            self.output.encoding = encoding
            fileName = unicode(files[0])
            selectedFileFilter = unicode(fileDialog.selectedNameFilter())
            if not fileName.lower().endswith(
                    tuple(re.findall("\*(\.[a-z]{1,5})", fileFilter))):
                ext = re.search("\*(\.[a-z]{1,5})", selectedFileFilter)
                if ext:
                    fileName += ext.group(1)
            self.text.setText(fileName)
            settings.setValue('/Processing/LastOutputPath',
                              os.path.dirname(fileName))
            settings.setValue('/Processing/encoding', encoding)

    def selectDirectory(self):
        lastDir = ''

        dirName = QFileDialog.getExistingDirectory(self,
            self.tr('Select directory'), lastDir, QFileDialog.ShowDirsOnly)

        self.text.setText(dirName)

    def getValue(self):
        fileName = unicode(self.text.text())
        if fileName.strip() in ['', self.SAVE_TO_TEMP_FILE]:
            value = None
        elif fileName.startswith('memory:'):
            value = fileName
        elif not os.path.isabs(fileName):
            value = ProcessingConfig.getSetting(
                ProcessingConfig.OUTPUT_FOLDER) + os.sep + fileName
        else:
            value = fileName

        return value
