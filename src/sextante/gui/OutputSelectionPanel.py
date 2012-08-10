from PyQt4 import QtGui, QtCore
from PyQt4.QtCore import *
from PyQt4.QtGui import *
import os.path
from sextante.core.SextanteConfig import SextanteConfig


class OutputSelectionPanel(QtGui.QWidget):

    lastOutputFolder = None
    SAVE_TO_TEMP_FILE = "[Save to temporary file]"

    def __init__(self, output, alg):
        self.output = output
        self.alg = alg
        super(OutputSelectionPanel, self).__init__(None)
        self.horizontalLayout = QtGui.QHBoxLayout(self)
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.text = QtGui.QLineEdit()
        if hasattr(self.text, 'setPlaceholderText'):
            self.text.setPlaceholderText(OutputSelectionPanel.SAVE_TO_TEMP_FILE)
        self.text.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)
        self.horizontalLayout.addWidget(self.text)
        self.pushButton = QtGui.QPushButton()
        self.pushButton.setText("...")
        self.pushButton.clicked.connect(self.buttonPushed)
        self.horizontalLayout.addWidget(self.pushButton)
        self.setLayout(self.horizontalLayout)

    def buttonPushed(self):
        popupmenu = QMenu()
        saveToTemporaryFileAction = QtGui.QAction("Save to a temporary file", self.pushButton)
        saveToTemporaryFileAction.triggered.connect(self.saveToTemporaryFile)
        popupmenu.addAction(saveToTemporaryFileAction )
        if (self.alg.provider.supportsNonFileBasedOutput()):
            saveToMemoryAction= QtGui.QAction("Save to a memory layer...", self.pushButton)
            saveToMemoryAction.triggered.connect(self.saveToMemory)
            popupmenu.addAction(saveToMemoryAction)
        saveToFileAction = QtGui.QAction("Save to file...", self.pushButton)
        saveToFileAction.triggered.connect(self.saveToFile)
        popupmenu.addAction(saveToFileAction)

        popupmenu.exec_(QtGui.QCursor.pos())

    def saveToTemporaryFile(self):
        self.text.setText("")

    def saveToMemory(self):
        self.text.setText("memory:")

    def saveToFile(self):
        filefilter = self.output.getFileFilter(self.alg)
        settings = QtCore.QSettings()
        if settings.contains("/SextanteQGIS/LastOutputPath"):
            path = str(settings.value( "/SextanteQGIS/LastOutputPath", QtCore.QVariant( "" ) ).toString())
        else:
            path = SextanteConfig.getSetting(SextanteConfig.OUTPUT_FOLDER)
        filename = QtGui.QFileDialog.getSaveFileName(self, "Save file", QtCore.QString(path), filefilter)
        if filename:
            self.text.setText(str(filename))
            settings.setValue("/SextanteQGIS/LastOutputPath", os.path.dirname(str(filename)))

    def getValue(self):
        filename = str(self.text.text())
        if filename.strip() == "" or filename == OutputSelectionPanel.SAVE_TO_TEMP_FILE:
            return None
        if filename.startswith("memory:"):
            return filename
        else:
            if not os.path.isabs(filename):
                filename = SextanteConfig.getSetting(SextanteConfig.OUTPUT_FOLDER) + os.sep + filename
            return filename
