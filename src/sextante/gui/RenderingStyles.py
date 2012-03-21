from sextante.core.SextanteUtils import SextanteUtils
import os.path

class RenderingStyles():

    styles = {}

    @staticmethod
    def addAlgStylesAndSave(algname, styles):
        RenderingStyles.styles[algname] = styles
        RenderingStyles.saveSettings()


    @staticmethod
    def configFile():
        return os.path.join(SextanteUtils.userFolder(), "sextante_qgis_styles.conf")

    @staticmethod
    def loadStyles():
        if not os.path.isfile(RenderingStyles.configFile()):
            return
        lines = open(RenderingStyles.configFile())
        line = lines.readline().strip("\n")
        while line != "":
            tokens = line.split("|")
            if tokens[0] in RenderingStyles.styles.keys():
                RenderingStyles.styles[tokens[0]][tokens[1]] = tokens[2]
            else:
                alg = {}
                alg[tokens[1]]=tokens[2]
                RenderingStyles.styles[tokens[0]] = alg
            line = lines.readline().strip("\n")
        lines.close()

    @staticmethod
    def saveSettings():
        fout = open(RenderingStyles.configFile(), "w")
        for alg in RenderingStyles.styles.keys():
            for out in RenderingStyles.styles[alg].keys():
                fout.write(alg + "|" + out + "|" + RenderingStyles.styles[alg][out] + "\n")
        fout.close()

    @staticmethod
    def getStyle(algname, outputname):
        if algname in RenderingStyles.styles:
            if outputname in RenderingStyles.styles[algname]:
                return RenderingStyles.styles[algname][outputname]
        return None

