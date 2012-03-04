import os
from sextante.core.SextanteUtils import SextanteUtils
import subprocess
from sextante.core.SextanteConfig import SextanteConfig
class SagaUtils:

    SAGA_FOLDER = "SAGA_FOLDER"
    ACTIVATE_SAGA = "ACTIVATE_SAGA"

    @staticmethod
    def sagaBatchJobFilename():

        if SextanteUtils.isWindows():
            filename = "saga_batch_job.bat";
        else:
            filename = "saga_batch_job.sh";

        batchfile = SextanteUtils.userFolder() + os.sep + filename

        return batchfile

    @staticmethod
    def sagaPath():
        folder = SextanteConfig.getSetting(SagaUtils.SAGA_FOLDER)
        if folder == None:
            folder =""

        return folder

    @staticmethod
    def sagaDescriptionPath():
        return os.path.join(os.path.dirname(__file__),"description")

    @staticmethod
    def createSagaBatchJobFileFromSagaCommands(commands):

        fout = open(SagaUtils.sagaBatchJobFilename(), "w")
        if SextanteUtils.isWindows():
            fout.write("set SAGA=" + SagaUtils.sagaPath() + "\n");
            fout.write("set SAGA_MLB=" + SagaUtils.sagaPath()+ os.sep + "modules" + "\n");
            fout.write("PATH=PATH;%SAGA%;%SAGA_MLB%\n");
        else:
            fout.write("!#/bin/sh\n");
            fout.write("export SAGA_MLB=" + SagaUtils.sagaPath() + os.sep + "modules" + "\n");
            fout.write("PATH=$PATH:" + SagaUtils.sagaPath() + os.sep + "modules" + "\n");
            fout.write("export PATH\n");

        for command in commands:
            fout.write("saga_cmd " + command + "\n")

        fout.write("exit")
        fout.close()

    @staticmethod
    def executeSaga(progress):
        if SextanteUtils.isWindows():
            command = ["cmd.exe", "/C ", SagaUtils.sagaBatchJobFilename()]
        else:
            #TODO linux
            pass

        proc = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stdin=subprocess.PIPE,stderr=subprocess.STDOUT, universal_newlines=True).stdout
        for line in iter(proc.readline, ""):
            if "%" in line:
                s = "".join([x for x in line if x.isdigit()])
                progress.setPercentage(int(s))





