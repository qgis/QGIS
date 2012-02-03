import os
import time
from sextante.outputs.OutputTable import OutputTable
from sextante.outputs.OutputVector import OutputVector
from sextante.outputs.OutputRaster import OutputRaster
import datetime
class SextanteUtils:

    NUM_EXPORTED = 1
    LOG_ERROR = "ERROR"
    LOG_INFO = "INFO"
    LOG_WARNING = "WARNING"
    LOG_ALGORITHM = "ALGORITHM"

    @staticmethod
    def userFolder():
        userfolder = os.getenv('HOME') + os.sep + "sextante"
        mkdir(userfolder)

        return userfolder

    @staticmethod
    def softwareFolder():
        path = os.path.join(os.path.dirname(__file__),"..","soft")
        return path

    @staticmethod
    def isWindows():
        return os.name =="nt"

    @staticmethod
    def tempFolder():
        tempfolder = os.getenv('HOME') + os.sep + "sextante" + os.sep + "tempdata"
        mkdir(tempfolder)

        return tempfolder

    @staticmethod
    def setTempOutput(out):
        seconds = str(time.time())
        if isinstance(out, OutputRaster):
            ext = ".tif"
        elif isinstance(out, OutputVector):
            ext = ".shp"
        elif isinstance(out, OutputTable):
            ext = ".dbf"
        else:
            ext =""

        filename = SextanteUtils.tempFolder() + os.sep + seconds + str(SextanteUtils.NUM_EXPORTED) + ext
        out.channel = filename
        SextanteUtils.NUM_EXPORTED += 1


    @staticmethod
    def logFilename():
        batchfile = SextanteUtils.userFolder() + os.sep + "sextante_qgis.log"
        return batchfile


    @staticmethod
    def addToLog(msgtype, msg):

        if isinstance(msg, list):
            text=""
            for i in range(0, len(msg)):
                text+=msg[i] + "|"
            text = text[:-1]
        else:
            text = str(msg)
        line = msgtype + "|" +str(datetime.datetime.now()) + "|" + text + "\n"
        logfile = open(SextanteUtils.logFilename(), "a")
        logfile.write(line)
        logfile.close()

    @staticmethod
    def getLogEntries():
        entries={}
        errors=[]
        algorithms=[]
        warnings=[]
        info=[]
        lines = open(SextanteUtils.logFilename())
        line = lines.readline()
        while line != "":
            line = line.strip("\n").strip()
            tokens = line.split("|")
            text=""
            for i in range(2, len(tokens)):
                text+=tokens[i] + "\n"
            if line.startswith(SextanteUtils.LOG_ERROR):
                errors.append(LogElement(tokens[1], text))
            elif line.startswith(SextanteUtils.LOG_ALGORITHM):
                algorithms.append(LogElement(tokens[1], text))
            elif line.startswith(SextanteUtils.LOG_WARNING):
                warnings.append(LogElement(tokens[1], text))
            elif line.startswith(SextanteUtils.LOG_INFO):
                info.append(LogElement(tokens[1], text))
        lines.close()
        entries[SextanteUtils.LOG_ERROR] = errors
        entries[SextanteUtils.LOG_ALGORITHM] = algorithms
        return entries

class LogElement():

    def __init__(self, date, text):
        self.date = date
        self.text = text



def mkdir(newdir):
    if os.path.isdir(newdir):
        pass
    else:
        head, tail = os.path.split(newdir)
        if head and not os.path.isdir(head):
            mkdir(head)
        if tail:
            os.mkdir(newdir)

