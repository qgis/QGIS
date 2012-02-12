class Parameter:

    def __init__(self, name, description):
        self.name = name
        self.description = description

    def setValue(self, obj):
        self.value = str(obj)
        return True

    def __str__(self):
        return self.name + " <" + self.__module__.split(".")[-1] +">"

    def serialize(self):
        return self.__module__.split(".")[-1] + " " + self.name + " " + self.description

    def getValueAsCommandLineParameter(self):
        return str(self.value)



