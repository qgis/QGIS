from sextante.outputs.Output import Output

class OutputFile(Output):

    def getFileFilter(self,alg):
        return "All files(*.*)"

    def getDefaultFileExtension(self, alg):
        return "file"