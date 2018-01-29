/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgscacheindex.h                                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsAbstractCacheIndex
{
%Docstring
Abstract base class for cache indices
%End

%TypeHeaderCode
#include "qgscacheindex.h"
%End
  public:

    QgsAbstractCacheIndex();
%Docstring
Constructor for QgsAbstractCacheIndex.
%End
    virtual ~QgsAbstractCacheIndex();

    virtual void flushFeature( const QgsFeatureId fid ) = 0;
%Docstring
Is called, whenever a feature is removed from the cache. You should update your indexes, so
they become invalid in case this feature was required to successfully answer a request.
%End

    virtual void flush() = 0;
%Docstring
Sometimes, the whole cache changes its state and its easier to just withdraw everything.
In this case, this method is issued. Be sure to clear all cache information in here.
%End

    virtual void requestCompleted( const QgsFeatureRequest &featureRequest, const QgsFeatureIds &fids );
%Docstring
Implement this method to update the the indices, in case you need information contained by the request
to properly index. (E.g. spatial index)
Does nothing by default

:param featureRequest:  The feature request that was answered
:param fids:            The feature ids that have been returned
%End

    virtual bool getCacheIterator( QgsFeatureIterator &featureIterator, const QgsFeatureRequest &featureRequest ) = 0;
%Docstring
Is called, when a feature request is issued on a cached layer.
If this cache index is able to completely answer the feature request, it will return true
and set the iterator to a valid iterator over the cached features. If it is not able
it will return false.

:param featureIterator:  A reference to a :py:class:`QgsFeatureIterator`. A valid featureIterator will
be assigned in case this index is able to answer the request and the return
value is true.
:param featureRequest:   The feature request, for which this index is queried.

:return:   True, if this index holds the information to answer the request.
%End
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgscacheindex.h                                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
