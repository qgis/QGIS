from sextante.script.EditScriptDialog import EditScriptDialog
from sextante.gui.ToolboxAction import ToolboxAction
import os
from PyQt4 import QtGui

class CreateNewScriptAction(ToolboxAction):

    def __init__(self):
        self.name="Create new script"
        self.group="Tools"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/../images/script.png")

    def execute(self):
        dlg = EditScriptDialog(None)
        dlg.exec_()
        if dlg.update:
            self.toolbox.updateTree()
