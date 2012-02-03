from sextante.script.EditScriptDialog import EditScriptDialog
from sextante.gui.ToolboxAction import ToolboxAction

class CreateNewScriptAction(ToolboxAction):

    def __init__(self):
        self.name="Create new script"
        self.group="Tools"

    def execute(self):
        dlg = EditScriptDialog(None)
        dlg.exec_()
        if dlg.update:
            self.toolbox.updateTree()
