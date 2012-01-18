from sextante.script.ScriptAlgorithm import ScriptAlgorithm

class EditScriptAction:

    def isEnabled(self, alg):
        return isinstance(alg, ScriptAlgorithm)
