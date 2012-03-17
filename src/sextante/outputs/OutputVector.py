from sextante.outputs.Output import Output

class OutputVector(Output):

    def __init__(self, name="", description="", hidden=False):
        self.name = name
        self.description = description
        self.value = None
        self.hidden = hidden

    def getFileFilter(self,alg):
        exts = alg.provider.getSupportedOutputVectorLayerExtensions()
        for i in range(len(exts)):
            exts[i] = exts[i].upper() + " files(*." + exts[i].lower() + ")"
        return ";;".join(exts)

    def getDefaultFileExtension(self, alg):
        return alg.provider.getSupportedOutputVectorLayerExtensions()[0]