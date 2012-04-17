from sextante.gui.ContextAction import ContextAction
from sextante.modeler.ModelerAlgorithm import ModelerAlgorithm
import os

class DeleteModelAction(ContextAction):

    def __init__(self):
        self.name="Delete model"

    def isEnabled(self):
        return isinstance(self.alg, ModelerAlgorithm)

    def execute(self):
        os.remove(self.alg.descriptionFile)
        self.toolbox.updateTree()