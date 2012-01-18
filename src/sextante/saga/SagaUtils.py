import os
from sextante.core.SextanteUtils import SextanteUtils
class SagaUtils:

    @staticmethod
    def sagaBatchJobFilename():

        if SextanteUtils.isWindows():
            filename = "saga_batch_job.bat";
        else:
            filename = "saga_batch_job.sh";

        batchfile = SextanteUtils.userFolder() + os.sep + filename;

        return batchfile

    @staticmethod
    def sagaPath():
        return SextanteUtils.softwareFolder() + os.sep + "saga"

    @staticmethod
    def sagaDescriptionPath():
        return SagaUtils.sagaPath() + os.sep + "description"

    @staticmethod
    def createSagaBatchJobFileFromSagaCommands(commands):

        fout = open(SagaUtils.getBatchJobFilename(), "w")
        if SextanteUtils.isWindows():
            fout.write("set SAGA=" + SagaUtils.sagaPath() + "\n");
            fout.write("set SAGA_MLB=" + SagaUtils.sagaPath()+ os.sep + "modules" + "\n");
            fout.write("PATH=PATH;%SAGA%;%SAGA_MLB%\n");
        else:
            fout.write("!#/bin/sh\n");
            fout.write("export SAGA_MLB=" + SagaUtils.sagaPath() + os.sep + "modules" + "\n");
            fout.write("PATH=$PATH:" + SagaUtils.sagaPath() + os.sep + "modules" + "\n");
            fout.write("export PATH");

        for command in commands:
            fout.write("saga_cmd " + command + "\n")

        fout.write("exit")
        fout.close()

