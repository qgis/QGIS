from sextante.outputs.Output import Output

class OutputHTML(Output):

    def getFileFilter(self,alg):
        return "HTML files(*.html)"

    def getDefaultFileExtension(self, alg):
        return "html"