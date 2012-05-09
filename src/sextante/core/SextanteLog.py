import datetime
import os
from sextante.core.SextanteUtils import SextanteUtils
class SextanteLog():

    LOG_ERROR = "ERROR"
    LOG_INFO = "INFO"
    LOG_WARNING = "WARNING"
    LOG_ALGORITHM = "ALGORITHM"
    recentAlgs = []

    @staticmethod
    def startLogging():
        if os.path.isfile(SextanteLog.logFilename()):
            logfile = open(SextanteLog.logFilename(), "a")
        else:
            logfile = open(SextanteLog.logFilename(), "w")
        logfile.write("Started logging at " + datetime.datetime.now().strftime("%a %b %d %Y %H:%M:%S") + "\n")
        logfile.close()

    @staticmethod
    def logFilename():
        batchfile = SextanteUtils.userFolder() + os.sep + "sextante_qgis.log"
        return batchfile


    @staticmethod
    def addToLog(msgtype, msg):
        if isinstance(msg, list):
            text=""
            for i in range(0, len(msg)):
                text+=msg[i].strip("\n") + "|"
            text = text[:-1]
        else:
            text = str(msg).replace("\n", "|")
        line = msgtype + "|" + datetime.datetime.now().strftime("%a %b %d %Y %H:%M:%S") + "|" + text + "\n"
        logfile = open(SextanteLog.logFilename(), "a")
        logfile.write(line)
        logfile.close()
        if msgtype==SextanteLog.LOG_ALGORITHM:
            algname = text[len("Sextante.runalg(\""):]
            algname = algname[:algname.index("\"")]
            if algname not in SextanteLog.recentAlgs:
                SextanteLog.recentAlgs.append(algname)


    @staticmethod
    def getLogEntries():
        entries={}
        errors=[]
        algorithms=[]
        warnings=[]
        info=[]
        lines = open(SextanteLog.logFilename())
        line = lines.readline()
        while line != "":
            line = line.strip("\n").strip()
            tokens = line.split("|")
            text=""
            for i in range(2, len(tokens)):
                text+=tokens[i] + "|"
            if line.startswith(SextanteLog.LOG_ERROR):
                errors.append(LogEntry(tokens[1], text))
            elif line.startswith(SextanteLog.LOG_ALGORITHM):
                algorithms.append(LogEntry(tokens[1], tokens[2]))
            elif line.startswith(SextanteLog.LOG_WARNING):
                warnings.append(LogEntry(tokens[1], text))
            elif line.startswith(SextanteLog.LOG_INFO):
                info.append(LogEntry(tokens[1], text))
            line = lines.readline()
        lines.close()
        entries[SextanteLog.LOG_ERROR] = errors
        entries[SextanteLog.LOG_ALGORITHM] = algorithms
        entries[SextanteLog.LOG_INFO] = info
        entries[SextanteLog.LOG_WARNING] = warnings
        return entries


    @staticmethod
    def getRecentAlgorithms():
        return SextanteLog.recentAlgs


    @staticmethod
    def clearLog():
        os.unlink(SextanteLog.logFilename())
        SextanteLog.startLogging()



class LogEntry():
    def __init__(self, date, text):
        self.date = date
        self.text = text
