import os
class AlgNames():

    names = {}

    @staticmethod
    def getName(shortname):
        if not AlgNames.names:
            f = open(os.path.dirname(__file__) + "/algnames.txt")
            lines = f.readlines()
            for line in lines:
                tokens = line.split(":")
                AlgNames.names[tokens[0].strip()] = tokens[1].strip()
                f.close()
        return AlgNames.names[shortname]