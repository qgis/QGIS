from sextante.core.GeoAlgorithm import GeoAlgorithm
import os
from PyQt4 import QtGui
from sextante.parameters.ParameterString import ParameterString
from sextante.lidar.fusion.FusionUtils import FusionUtils

class FusionAlgorithm(GeoAlgorithm):

    ADVANCED_MODIFIERS = "ADVANCED_MODIFIERS"

    def getIcon(self):
        filepath = os.path.dirname(__file__) + "/../images/tool.png"
        return QtGui.QIcon(filepath)

    def checkBeforeOpeningParametersDialog(self):
            path = FusionUtils.FusionPath()
            if path == "":
                return "Fusion folder is not configured.\nPlease configure it before running Fusion algorithms."

    def addAdvancedModifiers(self):
        param = ParameterString(self.ADVANCED_MODIFIERS, "Additional modifiers", "")
        param.isAdvanced = True
        self.addParameter(param)

    def addAdvancedModifiersToCommand(self, commands):
        s = str(self.getParameterValue(self.ADVANCED_MODIFIERS)).strip()
        if s != "":
            commands.append(s)
