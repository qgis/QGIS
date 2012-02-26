from sextante.gui.ToolboxAction import ToolboxAction
from sextante.modeler.ModelerDialog import ModelerDialog

class CreateNewModelAction(ToolboxAction):

    def __init__(self):
        self.name="Create new model"
        self.group="Tools"

    def execute(self):
        dlg = ModelerDialog()
        dlg.exec_()
        if dlg.update:
            self.toolbox.updateTree()

