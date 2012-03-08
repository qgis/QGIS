from sextante.gui.ContextAction import ContextAction
from sextante.r.RAlgorithm import RAlgorithm
from sextante.r.EditRScriptDialog import EditRScriptDialog

class EditRScriptAction(ContextAction):

    def __init__(self):
        self.name="Edit R script"

    def isEnabled(self):
        return isinstance(self.alg, RAlgorithm)

    def execute(self):
        dlg = EditRScriptDialog(self.alg)
        dlg.exec_()
        if dlg.update:
            self.toolbox.updateTree()