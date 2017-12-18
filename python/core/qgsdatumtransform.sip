/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsdatumtransform.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsDatumTransform
{
%Docstring
Contains methods and classes relating the datum transformations.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsdatumtransform.h"
%End
  public:

    struct TransformPair
    {

      TransformPair( int sourceTransformId = -1, int destinationTransformId = -1 );
%Docstring
Constructor for a TransformPair with the specified ``sourceTransformId``
and ``destinationTransformId`` transforms.
%End

      int sourceTransformId;
%Docstring
ID for the datum transform to use when projecting from the source CRS.

.. seealso:: :py:func:`QgsCoordinateTransform.datumTransformCrsInfo()`
%End

      int destinationTransformId;
%Docstring
ID for the datum transform to use when projecting to the destination CRS.

.. seealso:: :py:func:`QgsCoordinateTransform.datumTransformCrsInfo()`
%End

      bool operator==( const QgsDatumTransform::TransformPair &other ) const;

      bool operator!=( const QgsDatumTransform::TransformPair &other ) const;

    };

    struct TransformInfo
    {
      int datumTransformId;
%Docstring
Datum transform ID
%End

      int epsgCode;
%Docstring
EPSG code for the transform, or 0 if not found in EPSG database
%End

      QString sourceCrsAuthId;
%Docstring
Source CRS auth ID
%End

      QString destinationCrsAuthId;
%Docstring
Destination CRS auth ID
%End

      QString sourceCrsDescription;
%Docstring
Source CRS description
%End

      QString destinationCrsDescription;
%Docstring
Destination CRS description
%End

      QString remarks;
%Docstring
Transform remarks
%End

      QString scope;
%Docstring
Scope of transform
%End

      bool preferred;
%Docstring
True if transform is the preferred transform to use for the source/destination CRS combination
%End

      bool deprecated;
%Docstring
True if transform is deprecated
%End

    };
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsdatumtransform.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
