from sextante.script.ScriptAlgorithm import ScriptAlgorithm
from sextante.gui.ContextAction import ContextAction

class DeleteScriptAction(ContextAction):

    def __init__(self):
        self.name="Delete script"

    def isEnabled(self):
        return isinstance(self.alg, ScriptAlgorithm)

    def execute(self, alg):
        pass