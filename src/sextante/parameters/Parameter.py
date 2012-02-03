class Parameter:

    def setValue(self, obj):
        self.value = str(obj)
        return True

    def __str__(self):
        return self.name + " <" + self.__module__.split(".")[-1] +">"

    def getValueAsCommandLineParameter(self):
        return str(self.value)



