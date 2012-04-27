from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.outputs.OutputVector import OutputVector
from sextante.parameters.ParameterVector import ParameterVector
from qgis.core import *
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from sextante.core.QGisLayers import QGisLayers
import os


class SaveSelectedFeatures(GeoAlgorithm):
    '''This is an example algorithm that takes a vector layer and creates
    a new one just with just those features of the input layer that are
    selected.
    It is meant to be used as an example of how to create your own SEXTANTE
    algorithms and explain methods and variables used to do it.
    An algorithm like this will be available in all SEXTANTE elements, and
    there is not need for additional work.

    All SEXTANTE algorithms should extend the GeoAlgorithm class'''

    #constants used to refer to parameters and outputs.
    #They will be used when calling the algorithm from another algorithm,
    #or when calling SEXTANTE from the QGIS console.
    OUTPUT_LAYER = "OUTPUT_LAYER"
    INPUT_LAYER = "INPUT_LAYER"

    def defineCharacteristics(self):
        '''Here we define the inputs and output of the algorithm, along
        with some other properties'''

        #the name that the user will see in the toolbox
        self.name = "Create new layer with selected features"

        #the branch of the toolbox under which the algorithm will appear
        self.group = "Algorithms for vector layers"

        #we add the input vector layer. It can have any kind of geometry
        #It is a mandatory (not optional) one, hence the False argument
        self.addParameter(ParameterVector(self.INPUT_LAYER, "Input layer", ParameterVector.VECTOR_TYPE_ANY, False))
        # we add a vector layer as output
        self.addOutput(OutputVector(self.OUTPUT_LAYER, "Output layer with selected features"))

    def getIcon(self):
        return QIcon(os.path.dirname(__file__) + "/../images/toolbox.png")

    def processAlgorithm(self, progress):
        '''Here is where the processing itself takes place'''

        #the first thing to do is retrieve the values of the parameters
        #entered by the user
        inputFilename = self.getParameterValue(self.INPUT_LAYER)
        output = self.getOutputValue(self.OUTPUT_LAYER)

        #input layers values are always a string with its location.
        #That string can be converted into a QGIS object (a QgsVectorLayer in this case))
        #using the Sextante.getObject() method
        vectorLayer = QGisLayers.getObjectFromUri(inputFilename)

        #And now we can process

        #First we create the output layer.
        #The output value entered by the user is a string containing a filename,
        #so we can use it directly
        settings = QSettings()
        systemEncoding = settings.value( "/UI/encoding", "System" ).toString()
        provider = vectorLayer.dataProvider()
        writer = QgsVectorFileWriter( output, systemEncoding, provider.fields(), provider.geometryType(), provider.crs() )

        #Now we take the selected features and add them to the output layer
        selection = vectorLayer.selectedFeatures()
        for feat in selection:
            writer.addFeature(feat)
        del writer

        #There is nothing more to do here. We do not have to open the layer that we have created.
        #SEXTANTE will take care of that, or will handle it if this algorithm is executed within
        #a complex model
