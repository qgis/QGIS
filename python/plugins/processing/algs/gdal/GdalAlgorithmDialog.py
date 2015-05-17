
from PyQt4.QtGui import *
from processing.gui.AlgorithmDialog import AlgorithmDialog
from processing.gui.AlgorithmDialogBase import AlgorithmDialogBase
from processing.gui.ParametersPanel import ParametersPanel

class GdalAlgorithmDialog(AlgorithmDialog):

    def __init__(self, alg):
        AlgorithmDialogBase.__init__(self, alg)

        self.alg = alg

        self.mainWidget = GdalParametersPanel(self, alg)
        self.setMainWidget()
        self.mainWidget.parametersHaveChanged()

class GdalParametersPanel(ParametersPanel):

    def __init__(self, parent, alg):
        ParametersPanel.__init__(self, parent, alg)

        w = QWidget()
        layout = QVBoxLayout()
        layout.setMargin(9)
        layout.setSpacing(6)
        label = QLabel()
        label.setText("GDAL/OGR console call")
        layout.addWidget(label)
        self.text = QPlainTextEdit()
        self.text.setEnabled(False)
        layout.addWidget(self.text)
        w.setLayout(layout)
        self.layout().addWidget(w)

        self.connectParameterSignals()
        self.parametersHaveChanged()

    def connectParameterSignals(self):
        for w in self.widgets.values():
            if isinstance(w, QLineEdit):
                w.textChanged.connect(self.parametersHaveChanged)
            elif isinstance(w, QComboBox):
                w.currentIndexChanged.connect(self.parametersHaveChanged)
            elif isinstance(w, QCheckBox):
                w.stateChanged.connect(self.parametersHaveChanged)

    def parametersHaveChanged(self):
        try:
            self.parent.setParamValues()
            for output in self.alg.outputs:
                if output.value is None:
                    output.value = "[temporary file]"
            commands = self.alg.getConsoleCommands()
            commands = [c for c in commands if c not in ['cmd.exe', '/C ']]
            self.text.setPlainText(" ".join(commands))
        except:
            self.text.setPlainText("")