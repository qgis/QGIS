from sextante.outputs.Output import Output

class OutputRaster(Output):

    def getFileFilter(self, alg):
        exts = alg.provider.getSupportedOutputRasterLayerExtensions()
        for i in range(len(exts)):
            exts[i] = exts[i].upper() + " files(*." + exts[i].lower() + ")"
        return ";;".join(exts)

    def getDefaultFileExtension(self, alg):
        return alg.provider.getSupportedOutputRasterLayerExtensions()[0]
