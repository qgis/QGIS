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

.. seealso:: :py:class:`QgsCoordinateTransformContext`

.. seealso:: :py:class:`QgsCoordinateTransform`

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

      int destinationTransformId;

      bool operator==( const QgsDatumTransform::TransformPair &other ) const;

      bool operator!=( const QgsDatumTransform::TransformPair &other ) const;

    };

    struct TransformInfo
    {
      int datumTransformId;

      int epsgCode;

      QString sourceCrsAuthId;

      QString destinationCrsAuthId;

      QString sourceCrsDescription;

      QString destinationCrsDescription;

      QString remarks;

      QString scope;

      bool preferred;

      bool deprecated;

    };

    static QList< QgsDatumTransform::TransformPair > datumTransformations( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &destination );
%Docstring
Returns a list of datum transformations which are available for the given ``source`` and ``destination`` CRS.

.. seealso:: :py:func:`datumTransformToProj`

.. seealso:: :py:func:`datumTransformInfo`
%End

    static QString datumTransformToProj( int datumTransformId );
%Docstring
Returns a proj string representing the specified ``datumTransformId`` datum transform ID.

.. seealso:: :py:func:`datumTransformations`

.. seealso:: :py:func:`datumTransformInfo`

.. seealso:: :py:func:`projStringToDatumTransformId`
%End

    static int projStringToDatumTransformId( const QString &string );
%Docstring
Returns the datum transform ID corresponding to a specified proj ``string``.
Returns -1 if matching datum ID was not found.

.. seealso:: :py:func:`datumTransformToProj`
%End

    static QgsDatumTransform::TransformInfo datumTransformInfo( int datumTransformId );
%Docstring
Returns detailed information about the specified ``datumTransformId``.
If ``datumTransformId`` was not a valid transform ID, a TransformInfo with TransformInfo.datumTransformId of
-1 will be returned.

.. seealso:: :py:func:`datumTransformations`

.. seealso:: :py:func:`datumTransformToProj`
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsdatumtransform.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
