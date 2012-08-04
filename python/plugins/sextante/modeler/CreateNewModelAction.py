from sextante.gui.ToolboxAction import ToolboxAction
from sextante.modeler.ModelerDialog import ModelerDialog
import os
from PyQt4 import QtGui

class CreateNewModelAction(ToolboxAction):

    def __init__(self):
        self.name="Create new model"
        self.group="Tools"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/../images/model.png")

    def execute(self):
        dlg = ModelerDialog()
        dlg.exec_()
        if dlg.update:
            self.toolbox.updateTree()

