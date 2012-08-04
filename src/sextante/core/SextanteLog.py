import datetime
import os
from sextante.core.SextanteUtils import SextanteUtils
import codecs
from PyQt4 import QtGui
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
        try: #it seems that this fails sometimes depending on the msg added:
            #To avoid it stopping the normal functioning of the algorithm,
            #we catch all errors, assuming that is better to miss some log info
            #that breaking the algorithm.
            if isinstance(msg, list):
                a = "|".join(m.strip("\n")  for m in msg)
                text = a
            else:
                text = msg.replace("\n", "|")
            line = msgtype + "|" + datetime.datetime.now().strftime("%a %b %d %Y %H:%M:%S") + "|" + text + "\n"
            logfile = open(SextanteLog.logFilename(), "a")
            #logfile = codecs.open(SextanteLog.logFilename(), "a", encoding='utf-8')
            logfile.write(line)
            logfile.close()
            if msgtype==SextanteLog.LOG_ALGORITHM:
               algname = text[len("Sextante.runalg(\""):]
               algname = algname[:algname.index("\"")]
               if algname not in SextanteLog.recentAlgs:
                   SextanteLog.recentAlgs.append(algname)
        except:
            pass


    @staticmethod
    def getLogEntries():
        entries={}
        errors=[]
        algorithms=[]
        warnings=[]
        info=[]
        #lines = codecs.open(SextanteLog.logFilename(), encoding='utf-8')
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
