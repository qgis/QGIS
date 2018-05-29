/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsfeaturefilterprovider.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/








class QgsFeatureFilterProvider
{
%Docstring
Abstract interface for use by classes that filter the features of a layer.
A QgsFeatureFilterProvider provides a method for modifying a QgsFeatureRequest in place to apply
additional filters to the request.

.. versionadded:: 2.14
%End

%TypeHeaderCode
#include "qgsfeaturefilterprovider.h"
%End
  public:


    virtual void filterFeatures( const QgsVectorLayer *layer, QgsFeatureRequest &featureRequest ) const = 0;
%Docstring
Add additional filters to the feature request to further restrict the features returned by the request.
Derived classes must implement this method.

:param layer: the layer to filter
:param featureRequest: the feature request to update
%End

    virtual QgsFeatureFilterProvider *clone() const = 0 /Factory/;
%Docstring
Create a clone of the feature filter provider

:return: a new clone
%End
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsfeaturefilterprovider.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
