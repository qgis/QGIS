import os
from sextante.core.SextanteUtils import SextanteUtils, mkdir

class ScriptUtils:

    @staticmethod
    def scriptsFolder():
        folder = SextanteUtils.userFolder() + os.sep + "scripts"
        mkdir(folder)
        return folder



