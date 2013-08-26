# -*- coding: utf-8 -*-

"""
***************************************************************************
    SaveSelectedFeatures.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.outputs.OutputVector import OutputVector
from processing.parameters.ParameterVector import ParameterVector
from qgis.core import *
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from processing.core.QGisLayers import QGisLayers


class SaveSelectedFeatures(GeoAlgorithm):
    '''This is an example algorithm that takes a vector layer and creates
    a new one just with just those features of the input layer that are
    selected.
    It is meant to be used as an example of how to create your own SEXTANTE
    algorithms and explain methods and variables used to do it.
    An algorithm like this will be available in all SEXTANTE elements, and
    there is not need for additional work.

    All geoprocessingalgorithms should extend the GeoAlgorithm class'''

    #constants used to refer to parameters and outputs.
    #They will be used when calling the algorithm from another algorithm,
    #or when calling from the QGIS console.
    OUTPUT_LAYER = "OUTPUT_LAYER"
    INPUT_LAYER = "INPUT_LAYER"

    def defineCharacteristics(self):
        '''Here we define the inputs and output of the algorithm, along
        with some other properties'''

        #the name that the user will see in the toolbox
        self.name = "Save selected features"

        #the branch of the toolbox under which the algorithm will appear
        self.group = "Vector general tools"

        #we add the input vector layer. It can have any kind of geometry
        #It is a mandatory (not optional) one, hence the False argument
        self.addParameter(ParameterVector(self.INPUT_LAYER, "Input layer", [ParameterVector.VECTOR_TYPE_ANY], False))
        # we add a vector layer as output
        self.addOutput(OutputVector(self.OUTPUT_LAYER, "Output layer with selected features"))


    def processAlgorithm(self, progress):
        '''Here is where the processing itself takes place'''

        #the first thing to do is retrieve the values of the parameters
        #entered by the user
        inputFilename = self.getParameterValue(self.INPUT_LAYER)
        output = self.getOutputFromName(self.OUTPUT_LAYER)

        #input layers values are always a string with its location.
        #That string can be converted into a QGIS object (a QgsVectorLayer in this case))
        #using the Processing.getObject() method
        vectorLayer = QGisLayers.getObjectFromUri(inputFilename)

        #And now we can process

        #First we create the output layer.
        #To do so, we call the getVectorWriter method in the Output object.
        #That will give as a ProcessingVectorWriter, that we can later use to add features.
        provider = vectorLayer.dataProvider()
        writer = output.getVectorWriter( provider.fields(), provider.geometryType(), vectorLayer.crs() )

        #Now we take the selected features and add them to the output layer
        features = QGisLayers.features(vectorLayer)
        total = len(features)
        i = 0
        for feat in features:
            writer.addFeature(feat)
            progress.setPercentage(100 * i / float(total))
            i += 1
        del writer

        #There is nothing more to do here. We do not have to open the layer that we have created.
        #The processing framework will take care of that, or will handle it if this algorithm is executed within
        #a complex model
