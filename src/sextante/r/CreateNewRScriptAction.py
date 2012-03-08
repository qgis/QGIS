from sextante.script.EditScriptDialog import EditScriptDialog
from sextante.gui.ToolboxAction import ToolboxAction
import os
from PyQt4 import QtGui
from sextante.r.EditRScriptDialog import EditRScriptDialog

class CreateNewRScriptAction(ToolboxAction):

    def __init__(self):
        self.name="Create new R script"
        self.group="Tools"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/../images/r.png")

    def execute(self):
        dlg = EditRScriptDialog(None)
        dlg.exec_()
        if dlg.update:
            self.toolbox.updateTree()
