from sextante.gui.ContextAction import ContextAction
from sextante.modeler.ModelerAlgorithm import ModelerAlgorithm
from sextante.modeler.ModelerDialog import ModelerDialog

class EditModelAction(ContextAction):

    def __init__(self):
        self.name="Edit model"

    def isEnabled(self):
        return isinstance(self.alg, ModelerAlgorithm)

    def execute(self):
        dlg = ModelerDialog(self.alg)
        dlg.exec_()
        if dlg.update:
            self.toolbox.updateTree()