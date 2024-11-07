# -*- coding: utf-8 -*-

"""
***************************************************************************
    KeepMetadata.py
    ---------------------
    Date                 : November 2024
    Copyright            : (C) 2024 by Matjaž Mori
    Email                : matjaz.mori at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Matjaž Mori'
__date__ = '2024-11-01'
__copyright__ = '(C) 2024 by Matjaž Mori'

from qgis.core import (QgsProcessing,
                       QgsProcessingParameterVectorLayer,
                       QgsProcessingParameterBoolean
                       )
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
import datetime
import tempfile
import os

class KeepMetadata(QgisAlgorithm):
    TARGET = 'TARGET'
    SOURCE = 'SOURCE'

    def __init__(self):
        super().__init__()
       
    def group(self):
        return self.tr('Layer tools')

    def groupId(self):
        return 'layertools'

    def name(self):
        return 'keepmetadata'

    def displayName(self):
        return self.tr('Keep Metadata')
   
    def shortHelpString(self):
        string = """This tool accepts two vector layers, source and target. It copies metadata from source to target layer.
        Metadata includes history, abstract, title, keywords, contact, data quality...
        Optionally, It also copies symbology from source to target layer.
        """
        return self.tr(string)
    
    def initAlgorithm(self, config=None):
        self.addParameter(
            QgsProcessingParameterVectorLayer(
                self.SOURCE,
                self.tr('Source layer'),
                [QgsProcessing.TypeVector]
            )
        )

        self.addParameter(
            QgsProcessingParameterVectorLayer(
                self.TARGET,
                self.tr('Target layer'),
                [QgsProcessing.TypeVector]
            )
        )

        self.addParameter(QgsProcessingParameterBoolean(
            'keep_metadata', 
            self.tr('Keep metadata'), 
            defaultValue=True
            ))
        
        self.addParameter(QgsProcessingParameterBoolean(
            'keep_symbology', 
            self.tr('Keep symbology'), 
            defaultValue=False
            ))
     

    def processAlgorithm(self, parameters, context, feedback):

        source_layer = self.parameterAsVectorLayer(parameters, self.SOURCE, context)
        target_layer = self.parameterAsVectorLayer(parameters, self.TARGET, context)
  
        keep_symbology = self.parameterAsBool(parameters, 'keep_symbology', context)
        keep_metadata = self.parameterAsBool(parameters, 'keep_metadata', context)  
           
        history = {
            'timestamp': str(datetime.datetime.now()),
            'process': 'qgis:keepmetadata',
            'source': source_layer.publicSource()
        }


        def transfer_metadata(source_layer, target_layer):
            original_metadata = source_layer.metadata() 
            target_layer.setMetadata(original_metadata)
            target_layer.saveDefaultMetadata() 

        #update history
        def update_history(target_layer, history):
            metadata = target_layer.metadata()
            metadata.addHistoryItem(str(history))
            target_layer.setMetadata(metadata)

            target_layer.saveDefaultMetadata() 
        

        def transfer_symbology(source_layer, target_layer):
            # Create a temporary file to hold the QML
            temp_file = tempfile.NamedTemporaryFile(delete=False, suffix='.qml')
            temp_file_path = temp_file.name
            temp_file.close()

            # Export the layer's current style to the temporary QML file
            source_layer.saveNamedStyle(temp_file_path)
            target_layer.loadNamedStyle(temp_file_path, True)


        if keep_metadata:
            try:   
                transfer_metadata(source_layer, target_layer)
                update_history(target_layer, history)
            except Exception as e:
                feedback.reportError(f'Could not transfer metadata: {str(e)}')


        if keep_symbology:
            try:
                transfer_symbology(source_layer, target_layer) 
            except Exception as e:
                feedback.reportError(f'Could not transfer symbology: {str(e)}')

        return {}
                





