/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsfeaturesource.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsFeatureSource
{
%Docstring
An interface for objects which provide features via a getFeatures method.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsfeaturesource.h"
%End
  public:

    virtual ~QgsFeatureSource();

    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest &request = QgsFeatureRequest() ) const = 0;
%Docstring
Returns an iterator for the features in the source.
An optional ``request`` can be used to optimise the returned
iterator, eg by restricting the returned attributes or geometry.
%End

    virtual QString sourceName() const = 0;
%Docstring
Returns a friendly display name for the source. The returned value can be an empty string.
%End

    virtual QgsCoordinateReferenceSystem sourceCrs() const = 0;
%Docstring
Returns the coordinate reference system for features in the source.
%End

    virtual QgsFields fields() const = 0;
%Docstring
Returns the fields associated with features in the source.
%End

    virtual QgsWkbTypes::Type wkbType() const = 0;
%Docstring
Returns the geometry type for features returned by this source.
%End


    int __len__() const;
%Docstring
Returns the number of features contained in the source, or -1
if the feature count is unknown.
%End
%MethodCode
    sipRes = sipCpp->featureCount();
%End

    virtual long featureCount() const = 0;
%Docstring
Returns the number of features contained in the source, or -1
if the feature count is unknown.
%End

    virtual QSet<QVariant> uniqueValues( int fieldIndex, int limit = -1 ) const;
%Docstring
Returns the set of unique values contained within the specified ``fieldIndex`` from this source.
If specified, the ``limit`` option can be used to limit the number of returned values.
The base class implementation uses a non-optimised approach of looping through
all features in the source.

.. seealso:: :py:func:`minimumValue`

.. seealso:: :py:func:`maximumValue`
%End

    virtual QVariant minimumValue( int fieldIndex ) const;
%Docstring
Returns the minimum value for an attribute column or an invalid variant in case of error.
The base class implementation uses a non-optimised approach of looping through
all features in the source.

.. seealso:: :py:func:`maximumValue`

.. seealso:: :py:func:`uniqueValues`
%End

    virtual QVariant maximumValue( int fieldIndex ) const;
%Docstring
Returns the maximum value for an attribute column or an invalid variant in case of error.
The base class implementation uses a non-optimised approach of looping through
all features in the source.

.. seealso:: :py:func:`minimumValue`

.. seealso:: :py:func:`uniqueValues`
%End

    virtual QgsRectangle sourceExtent() const;
%Docstring
Returns the extent of all geometries from the source.
The base class implementation uses a non-optimised approach of looping through
all features in the source.
%End

    virtual QgsFeatureIds allFeatureIds() const;
%Docstring
Returns a list of all feature IDs for features present in the source.
%End

    QgsVectorLayer *materialize( const QgsFeatureRequest &request,
                                 QgsFeedback *feedback = 0 ) /Factory/;
%Docstring
Materializes a ``request`` (query) made against this feature source, by running
it over the source and returning a new memory based vector layer containing
the result. All settings from feature ``request`` will be honored.

If a subset of attributes has been set for the request, then only
those selected fields will be present in the output layer.

The CRS for the output layer will match the input layer, unless
QgsFeatureRequest.setDestinationCrs() has been called with a valid :py:class:`QgsCoordinateReferenceSystem`.
In this case the output layer will match the QgsFeatureRequest.destinationCrs() CRS.

The returned layer WKB type will match wkbType(), unless the QgsFeatureRequest.NoGeometry flag is set
on the ``request``. In that case the returned layer will not be a spatial layer.

An optional ``feedback`` argument can be used to cancel the materialization
before it has fully completed.

The returned value is a new instance and the caller takes responsibility
for its ownership.

.. versionadded:: 3.0
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsfeaturesource.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
