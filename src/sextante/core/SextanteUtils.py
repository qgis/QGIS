import os
import time

class SextanteUtils:

    NUM_EXPORTED = 1

    @staticmethod
    def userFolder():
        userfolder = os.path.expanduser("~") + os.sep + "sextante"
        mkdir(userfolder)

        return userfolder

    @staticmethod
    def isWindows():
        return os.name =="nt"

    @staticmethod
    def tempFolder():
        tempfolder = os.path.expanduser("~") + os.sep + "sextante" + os.sep + "tempdata"
        mkdir(tempfolder)

        return tempfolder

    @staticmethod
    def setTempOutput(out, alg):
        seconds = str(time.time())
        ext = out.getDefaultFileExtension(alg)
        filename = SextanteUtils.tempFolder() + os.sep + seconds + str(SextanteUtils.NUM_EXPORTED) + "." + ext
        out.value = filename
        SextanteUtils.NUM_EXPORTED += 1


def mkdir(newdir):
    if os.path.isdir(newdir):
        pass
    else:
        head, tail = os.path.split(newdir)
        if head and not os.path.isdir(head):
            mkdir(head)
        if tail:
            os.mkdir(newdir)


