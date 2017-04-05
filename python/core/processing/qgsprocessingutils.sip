/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/processing/qgsprocessingutils.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







class QgsProcessingUtils
{
%Docstring
 Utility functions for use with processing classes.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingutils.h"
%End

  public:

    static QList< QgsRasterLayer * > compatibleRasterLayers( QgsProject *project, bool sort = true );
%Docstring
 Returns a list of raster layers from a project which are compatible with the processing
 framework.

 If the sort argument is true then the layers will be sorted by their QgsMapLayer.name()
 value.
 \see compatibleVectorLayers()
 \see compatibleLayers()
%End

    static QList< QgsVectorLayer * > compatibleVectorLayers( QgsProject *project,
        const QList< QgsWkbTypes::GeometryType > &geometryTypes = QList< QgsWkbTypes::GeometryType >(),
        bool sort = true );
%Docstring
 Returns a list of vector layers from a project which are compatible with the processing
 framework.

 If the geometryTypes list is non-empty then the layers will be sorted so that only
 layers with geometry types included in the list will be returned. Leaving the geometryTypes
 list empty will cause all vector layers, regardless of their geometry type, to be returned.

 If the sort argument is true then the layers will be sorted by their QgsMapLayer.name()
 value.
 \see compatibleRasterLayers()
 \see compatibleLayers()
%End

    static QList< QgsMapLayer * > compatibleLayers( QgsProject *project, bool sort = true );
%Docstring
 Returns a list of map layers from a project which are compatible with the processing
 framework.

 If the sort argument is true then the layers will be sorted by their QgsMapLayer.name()
 value.
 \see compatibleRasterLayers()
 \see compatibleVectorLayers()
%End

};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/processing/qgsprocessingutils.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
