import os
from sextante.core.SextanteUtils import mkdir
from sextante.core.SextanteConfig import SextanteConfig

class ScriptUtils:

    SCRIPTS_FOLDER = "SCRIPTS_FOLDER"
    ACTIVATE_SCRIPTS = "ACTIVATE_SCRIPTS"

    @staticmethod
    def scriptsFolder():
        folder = SextanteConfig.getSetting(ScriptUtils.SCRIPTS_FOLDER)
        if folder == None:
            folder = os.path.join(os.path.dirname(__file__), "scripts")
            #folder = SextanteUtils.userFolder() + os.sep + "scripts"
        mkdir(folder)

        return folder



