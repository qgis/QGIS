import os
from sextante.core.SextanteUtils import SextanteUtils
import subprocess
class SagaUtils:

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
        return SextanteUtils.softwareFolder() + os.sep + "saga"

    @staticmethod
    def sagaDescriptionPath():
        return SagaUtils.sagaPath() + os.sep + "description"

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
    def executeSaga(alg, progress):
        if SextanteUtils.isWindows():
            command = ["cmd.exe", "/C ", SagaUtils.sagaBatchJobFilename()]
        else:
            #TODO linux
            pass

        proc = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stdin=subprocess.PIPE,stderr=subprocess.STDOUT).stdout
        loglines=[]
        loglines.append("SAGA Execution log message")
        while 1:
            line = proc.readline()
            if not line:
                break
            if "%" in line:
                pass
            else:
               loglines.append(line[0:-2])
               progress.addText(line)
        SextanteUtils.addToLog(SextanteUtils.LOG_INFO, loglines)
           #====================================================================
           # try:
           #    n = int(line.replace("%","").replace("\n","").replace(" ",""))
           #    progress.addText(str(n))
           # except:
           #    progress.addText(line.replace("%","").replace("\n","").replace(" ",""))
           #====================================================================






