class Parameter:
    '''Base class for all parameters that a geoalgorithm might take as input'''

    def __init__(self, name="", description=""):
        self.name = name
        self.description = description
        self.value = None

        self.isAdvanced = False

        #a hidden parameter can be used to set a hard-coded value.
        #It can be used as any other parameter, but it will not be shown to the user
        self.hidden = False


    def setValue(self, obj):
        '''sets the value of the parameter.
        Returns true if the value passed is correct for the type of parameter'''
        self.value = str(obj)
        return True

    def __str__(self):
        return self.name + " <" + self.__module__.split(".")[-1] +">"

    def serialize(self):
        return self.__module__.split(".")[-1] + "|" + self.name + "|" + self.description

    def getValueAsCommandLineParameter(self):
        '''returns the value of this parameter as it should have been entered in the console
        if calling an algorithm using the Sextante.runalg() method'''
        return str(self.value)

    def parameterName(self):
        return self.__module__.split(".")[-1]