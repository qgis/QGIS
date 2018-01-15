/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsspatialindex.h                                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/









class QgsSpatialIndex
{

%TypeHeaderCode
#include "qgsspatialindex.h"
%End
  public:


    QgsSpatialIndex();
%Docstring
Constructor - creates R-tree
%End

    explicit QgsSpatialIndex( const QgsFeatureIterator &fi, QgsFeedback *feedback = 0 );
%Docstring
Constructor - creates R-tree and bulk loads it with features from the iterator.
This is much faster approach than creating an empty index and then inserting features one by one.

The optional ``feedback`` object can be used to allow cancelation of bulk feature loading. Ownership
of ``feedback`` is not transferred, and callers must take care that the lifetime of feedback exceeds
that of the spatial index construction.

.. versionadded:: 2.8
%End

    explicit QgsSpatialIndex( const QgsFeatureSource &source, QgsFeedback *feedback = 0 );
%Docstring
Constructor - creates R-tree and bulk loads it with features from the source.
This is much faster approach than creating an empty index and then inserting features one by one.

The optional ``feedback`` object can be used to allow cancelation of bulk feature loading. Ownership
of ``feedback`` is not transferred, and callers must take care that the lifetime of feedback exceeds
that of the spatial index construction.

.. versionadded:: 3.0
%End

    QgsSpatialIndex( const QgsSpatialIndex &other );
%Docstring
Copy constructor
%End

    ~QgsSpatialIndex();



    bool insertFeature( const QgsFeature &f );
%Docstring
Add feature to index
%End

    bool insertFeature( QgsFeatureId id, const QgsRectangle &bounds );
%Docstring
Add a feature ``id`` to the index with a specified bounding box.

:return: true if feature was successfully added to index.

.. versionadded:: 3.0
%End

    bool deleteFeature( const QgsFeature &f );
%Docstring
Remove feature from index
%End



    QList<QgsFeatureId> intersects( const QgsRectangle &rect ) const;
%Docstring
Returns features that intersect the specified rectangle
%End

    QList<QgsFeatureId> nearestNeighbor( const QgsPointXY &point, int neighbors ) const;
%Docstring
Returns nearest neighbors (their count is specified by second parameter)
%End


    int  refs() const;
%Docstring
get reference count - just for debugging!
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsspatialindex.h                                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
