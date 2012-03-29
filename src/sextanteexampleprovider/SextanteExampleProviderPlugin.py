from qgis.core import *
import os, sys
import inspect
from sextante.core.Sextante import Sextante
from sextanteexampleprovider.ExampleAlgorithmProvider import ExampleAlgorithmProvider


cmd_folder = os.path.split(inspect.getfile( inspect.currentframe() ))[0]
if cmd_folder not in sys.path:
    sys.path.insert(0, cmd_folder)

class SextanteExampleProviderPlugin:

    def __init__(self):
        self.provider = ExampleAlgorithmProvider()
    def initGui(self):
        Sextante.addProvider(self.provider)

    def unload(self):
        #Uncomment to add provider
        Sextante.removeProvider(self.provider)

