import os
class SextanteUtils:

    @staticmethod
    def userFolder():
        userfolder = os.getenv('HOME') + os.sep + "sextante"
        mkdir(userfolder)

        return userfolder


    @staticmethod
    def isWindows():
        return True


    @staticmethod
    def setTempOutput(out):
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

