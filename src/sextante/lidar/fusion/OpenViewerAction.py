from sextante.gui.ToolboxAction import ToolboxAction
import os
from PyQt4 import QtGui
import subprocess
from sextante.lidar.fusion.FusionUtils import FusionUtils

class OpenViewerAction(ToolboxAction):

    def __init__(self):
        self.name="Open Fusion LAS viewer"
        self.group="Visualization"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/../images/tool.png")

    def execute(self):
        f = os.path.join(FusionUtils.FusionPath(), "pdq.exe")
        if os.path.exists(f):
            subprocess.Popen(f)
        else:
            QtGui.QMessageBox.critical(None, "Unable to open viewer", "The current Fusion folder does not contain the viewer executable.\nPlease check the configuration in the SEXTANTE settings dialog.")
