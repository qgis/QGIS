from sextante.script.ScriptAlgorithm import ScriptAlgorithm
from sextante.gui.ContextAction import ContextAction
import os
from PyQt4 import QtGui

class DeleteScriptAction(ContextAction):

    def __init__(self):
        self.name="Delete script"

    def isEnabled(self):
        return isinstance(self.alg, ScriptAlgorithm)

    def execute(self, alg):
        reply = QtGui.QMessageBox.question(None, 'Confirmation',
                            "Are you sure you want to delete this script?", QtGui.QMessageBox.Yes |
                            QtGui.QMessageBox.No, QtGui.QMessageBox.No)
        if reply == QtGui.QMessageBox.Yes:
            os.remove(self.alg.descriptionFile)
            self.toolbox.updateTree()