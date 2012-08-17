import os.path

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from qgis.gui import *

from sextante.core.SextanteConfig import SextanteConfig

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
        popupmenu = QMenu()
        saveToTemporaryFileAction = QAction("Save to a temporary file", self.pushButton)
        saveToTemporaryFileAction.triggered.connect(self.saveToTemporaryFile)
        popupmenu.addAction(saveToTemporaryFileAction )
        if (self.alg.provider.supportsNonFileBasedOutput()):
            saveToMemoryAction= QAction("Save to a memory layer...", self.pushButton)
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
            path = str(settings.value( "/SextanteQGIS/LastOutputPath", QVariant( "" ) ).toString())
        else:
            path = SextanteConfig.getSetting(SextanteConfig.OUTPUT_FOLDER)
        lastEncoding = settings.value("/SextanteQGIS/encoding", "System").toString()
        fileDialog = QgsEncodingFileDialog(self, "Save file", QString(path), filefilter, lastEncoding)
        fileDialog.setFileMode(QFileDialog.AnyFile)
        fileDialog.setAcceptMode(QFileDialog.AcceptSave)
        fileDialog.setConfirmOverwrite(True)
        if fileDialog.exec_() == QDialog.Accepted:
            filename = fileDialog.selectedFiles().first()
            encoding = fileDialog.encoding()
            self.output.encoding = encoding
            self.text.setText(str(filename))
            settings.setValue("/SextanteQGIS/LastOutputPath", os.path.dirname(str(filename)))
            settings.setValue("/SextanteQGIS/encoding", encoding)

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
