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
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.gui import *
from sextante.core.SextanteConfig import SextanteConfig
from sextante.outputs.OutputVector import OutputVector
from sextante.outputs.OutputDirectory import OutputDirectory

class OutputSelectionPanel(QWidget):

    lastOutputFolder = None
    SAVE_TO_TEMP_FILE = "[Save to temporary file]"

    def __init__(self, output, alg):
        self.output = output
        self.alg = alg
        super(OutputSelectionPanel, self).__init__(None)
        self.horizontalLayout = QHBoxLayout(self)
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.text = QLineEdit()
        if isinstance(self.output, OutputDirectory):
            settings = QSettings()
            if settings.contains("/SextanteQGIS/LastOutputPath"):
                currentPath = settings.value( "/SextanteQGIS/LastOutputPath")
            else:
                currentPath = SextanteConfig.getSetting(SextanteConfig.OUTPUT_FOLDER)
            self.text.setText(currentPath)
        else:
            if hasattr(self.text, 'setPlaceholderText'):
                self.text.setPlaceholderText(OutputSelectionPanel.SAVE_TO_TEMP_FILE)
        self.text.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        self.horizontalLayout.addWidget(self.text)
        self.pushButton = QPushButton()
        self.pushButton.setText("...")
        self.pushButton.clicked.connect(self.buttonPushed)
        self.horizontalLayout.addWidget(self.pushButton)
        self.setLayout(self.horizontalLayout)

    def buttonPushed(self):
        if isinstance(self.output, OutputDirectory):
            self.saveToDirectory()
        else:
            popupmenu = QMenu()
            saveToTemporaryFileAction = QAction("Save to a temporary file", self.pushButton)
            saveToTemporaryFileAction.triggered.connect(self.saveToTemporaryFile)
            popupmenu.addAction(saveToTemporaryFileAction )
            if (isinstance(self.output, OutputVector) and self.alg.provider.supportsNonFileBasedOutput()):
                saveToMemoryAction= QAction("Save to a memory layer", self.pushButton)
                saveToMemoryAction.triggered.connect(self.saveToMemory)
                popupmenu.addAction(saveToMemoryAction)
            saveToFileAction = QAction("Save to file...", self.pushButton)
            saveToFileAction.triggered.connect(self.saveToFile)
            popupmenu.addAction(saveToFileAction)

            popupmenu.exec_(QCursor.pos())

    def saveToTemporaryFile(self):
        self.text.setText("")

    def saveToMemory(self):
        self.text.setText("memory:")

    def saveToFile(self):
        filefilter = self.output.getFileFilter(self.alg)
        settings = QSettings()
        if settings.contains("/SextanteQGIS/LastOutputPath"):
            path = settings.value( "/SextanteQGIS/LastOutputPath")
        else:
            path = SextanteConfig.getSetting(SextanteConfig.OUTPUT_FOLDER)
        lastEncoding = settings.value("/SextanteQGIS/encoding", "System")
        fileDialog = QgsEncodingFileDialog(self, "Save file", path, filefilter, lastEncoding)
        fileDialog.setFileMode(QFileDialog.AnyFile)
        fileDialog.setAcceptMode(QFileDialog.AcceptSave)
        fileDialog.setConfirmOverwrite(True)
        if fileDialog.exec_() == QDialog.Accepted:
            files = fileDialog.selectedFiles()
            encoding = unicode(fileDialog.encoding())
            self.output.encoding = encoding
            filename = unicode(files[0])
            self.text.setText(filename)
            settings.setValue("/SextanteQGIS/LastOutputPath", os.path.dirname(filename))
            settings.setValue("/SextanteQGIS/encoding", encoding)

    def saveToDirectory(self):
        '''
        Show a dialog for choosing an output directory.
        '''

        settings = QSettings()
        if settings.contains("/SextanteQGIS/LastOutputPath"):
            currentPath = settings.value( "/SextanteQGIS/LastOutputPath")
        else:
            currentPath = SextanteConfig.getSetting(SextanteConfig.OUTPUT_FOLDER)
        lastEncoding = settings.value("/SextanteQGIS/encoding", "System")
        dirDialog = QgsEncodingFileDialog(self, "Save Directory", currentPath,
                                          "", lastEncoding)
        dirDialog.setFileMode(QFileDialog.Directory)
        dirDialog.setOption(QFileDialog.ShowDirsOnly)
        if dirDialog.exec_() == QDialog.Accepted:
            directories = dirDialog.selectedFiles()
            encoding = unicode(dirDialog.encoding())
            self.output.encoding = encoding
            directory = unicode(directories[0])
            self.text.setText(directory)
            settings.setValue("/SextanteQGIS/LastOutputPath", directory)
            settings.setValue("/SextanteQGIS/encoding", encoding)

    def getValue(self):
        filepath = unicode(self.text.text())
        if filepath.strip() == "" or filepath == OutputSelectionPanel.SAVE_TO_TEMP_FILE:
            value = None
        elif filepath.startswith("memory:"):
            value = filepath
        elif not os.path.isabs(filepath):
            value = SextanteConfig.getSetting(SextanteConfig.OUTPUT_FOLDER) + os.sep + filepath
        else:
            value = filepath
        return value
