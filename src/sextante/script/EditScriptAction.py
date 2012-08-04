from sextante.script.ScriptAlgorithm import ScriptAlgorithm
from sextante.gui.ContextAction import ContextAction
from sextante.script.EditScriptDialog import EditScriptDialog

class EditScriptAction(ContextAction):

    def __init__(self):
        self.name="Edit script"

    def isEnabled(self):
        return isinstance(self.alg, ScriptAlgorithm)

    def execute(self):
        dlg = EditScriptDialog(self.alg)
        dlg.exec_()
        if dlg.update:
            self.toolbox.updateTree()