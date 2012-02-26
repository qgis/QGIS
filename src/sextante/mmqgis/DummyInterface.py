from sextante.core.Sextante import Sextante
class DummyInterface:

    def addVector(self,p1,p2,p3):
        pass

    def mainWindow(self):
        return Sextante.getInterface().mainWindow()