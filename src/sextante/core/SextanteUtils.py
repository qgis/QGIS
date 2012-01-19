import os
import time
from sextante.outputs.OutputTable import OutputTable
from sextante.outputs.OutputVector import OutputVector
from sextante.outputs.OutputRaster import OutputRaster
class SextanteUtils:

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
        return os.path =="nt"

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

        filename = SextanteUtils.tempFolder() + os.sep + seconds + os.sep + ext
        out.channel = filename


    @staticmethod
    def addToLog(msg):
        pass





def mkdir(newdir):
    if os.path.isdir(newdir):
        pass
    else:
        head, tail = os.path.split(newdir)
        if head and not os.path.isdir(head):
            mkdir(head)
        if tail:
            os.mkdir(newdir)

