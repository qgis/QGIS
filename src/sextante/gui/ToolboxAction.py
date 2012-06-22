from PyQt4 import QtGui
import os
class ToolboxAction(object):

    def __init__(self):
        #this should be true if the action should be shown even if there are no algorithms
        #in the provider (for instance, when it is deactivated
        self.showAlways = False

    def setData(self,toolbox):
        self.toolbox = toolbox

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/../images/alg.png")

