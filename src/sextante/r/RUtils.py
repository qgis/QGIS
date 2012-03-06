from sextante.core.SextanteConfig import SextanteConfig
import os
from sextante.core.SextanteUtils import mkdir, SextanteUtils
import subprocess

class RUtils:

    RSCRIPTS_FOLDER = "R_SCRIPTS_FOLDER"
    R_FOLDER = "R_FOLDER"

    @staticmethod
    def RFolder():
        folder = SextanteConfig.getSetting(RUtils.R_FOLDER)
        if folder == None:
            folder =""

        return folder

    @staticmethod
    def RScriptsFolder():
        folder = SextanteConfig.getSetting(RUtils.RSCRIPTS_FOLDER)
        if folder == None:
            folder = os.path.join(os.path.dirname(__file__), "scripts")
        mkdir(folder)

        return folder

    @staticmethod
    def createRScriptFromRCommands(commands):
        scriptfile = open(RUtils.getRScriptFilename())
        for command in commands:
            scriptfile.write(command + "\n");
        scriptfile.close()


    @staticmethod
    def getRScriptFilename():
        return SextanteUtils.userFolder() + os.sep + "sextante_script.r"


    @staticmethod
    def getConsoleOutputFilename():
        return RUtils.getRScriptFilename()+".Rout"


    @staticmethod
    def executeRAlgorithm(alg):
        RUtils.consoleResults = []
        RUtils.verboseCommands = alg.getVerboseCommands();
        RUtils.addConsoleOutput = False;
        RUtils.createRScriptFromRCommands(alg.getFullSetOfRCommands())
        if SextanteUtils.isWindows():
            command = ["\"" + RUtils.RFolder() + os.sep + "bin" + os.sep + "R.exe\"", "CMD", "BATCH", "--vanilla",
                             "\"" + RUtils.getRScriptFilename() + "\""]
        else:#TODO***********
            pass

        proc = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stdin=subprocess.PIPE,stderr=subprocess.STDOUT, universal_newlines=True).stdout
        RUtils.createConsoleOutput()


    @staticmethod
    def  createConsoleOutput():
        lines = open(RUtils.getConsoleOutputFilename())
        line = lines.readline().strip("\n").strip(" ")
        while line != "":
            if line.startswith(">"):
                line = line[1:]
                if line in RUtils.verboseCommands:
                    add = True
                else:
                    add = False
            elif add:
                RUtils.consoleResults.append("<p>" + line + "</p>\n");
            line = lines.readline().strip("\n").strip(" ")


    @staticmethod
    def getConsoleOutput():
        s = "<font face=\"courier\">\n"
        s+="<h2> R Output</h2>\n"
        for line in RUtils.consoleResults:
            s+=line
        s+="</font>\n"

        return s


