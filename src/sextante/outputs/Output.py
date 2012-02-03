class Output(object):


    def __str__(self):
        return self.name + " <" + self.__module__.split(".")[-1] +">"

    def getChannelAsCommandLineParameter(self):
        if self.channel == None:
            return str(None)
        else:
            return "\"" + str(self.channel) + "\""

    def setChannel(self, value):
        try:
            if value != None:
                value = value.strip()
            self.channel = value
            return True
        except:
            return False

