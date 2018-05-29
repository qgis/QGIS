# -*- coding: utf-8 -*-

"""
***************************************************************************
    processing.py
    ---------------------
    Date                 : May 2018
    Copyright            : (C) 2018 by Denis Rouzaud
    Email                : denis@opengis.ch
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""


# add some __repr__ methods to processing classes
def processing_source_repr(self):
    return "<QgsProcessingFeatureSourceDefinition {{'source':{}, 'selectedFeaturesOnly': {}}}>".format(
        self.source.staticValue(), self.selectedFeaturesOnly)


def processing_output_layer_repr(self):
    return "<QgsProcessingOutputLayerDefinition {{'sink':{}, 'createOptions': {}}}>".format(self.sink.staticValue(),
                                                                                            self.createOptions)
