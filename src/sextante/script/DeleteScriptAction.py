from sextante.script.ScriptAlgorithm import ScriptAlgorithm
from sextante.gui.ContextAction import ContextAction
import os

class DeleteScriptAction(ContextAction):

    def __init__(self):
        self.name="Delete script"

    def isEnabled(self):
        return isinstance(self.alg, ScriptAlgorithm)

    def execute(self, alg):
        os.remove(self.alg.descriptionFile)
        self.toolbox.updateTree()