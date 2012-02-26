from PyQt4 import QtGui
import os
class ToolboxAction(object):

    def setData(self,toolbox):
        self.toolbox = toolbox

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/../images/alg.png")

