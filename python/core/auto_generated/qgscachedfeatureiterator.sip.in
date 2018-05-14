/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgscachedfeatureiterator.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsCachedFeatureIterator : QgsAbstractFeatureIterator
{
%Docstring
Delivers features from the cache
%End

%TypeHeaderCode
#include "qgscachedfeatureiterator.h"
%End
  public:

    QgsCachedFeatureIterator( QgsVectorLayerCache *vlCache, const QgsFeatureRequest &featureRequest );
%Docstring
This constructor creates a feature iterator, that delivers all cached features. No request is made to the backend.

:param vlCache:          The vector layer cache to use
:param featureRequest:   The feature request to answer
%End

    virtual bool rewind();

%Docstring
Rewind to the beginning of the iterator

:return: bool true if the operation was OK
%End

    virtual bool close();

%Docstring
Close this iterator. No further features will be available.

:return: true if successful
%End

  protected:

    virtual bool fetchFeature( QgsFeature &f );

%Docstring
Implementation for fetching a feature.

:param f:      Will write to this feature

:return: bool  true if the operation was OK

.. seealso:: :py:func:`bool`
%End

    virtual bool nextFeatureFilterFids( QgsFeature &f );
%Docstring
We have a local special iterator for FilterFids, no need to run the generic.

:param f:      Will write to this feature

:return: bool  true if the operation was OK
%End

};

class QgsCachedFeatureWriterIterator : QgsAbstractFeatureIterator
{
%Docstring
Uses another iterator as backend and writes features to the cache
%End

%TypeHeaderCode
#include "qgscachedfeatureiterator.h"
%End
  public:

    QgsCachedFeatureWriterIterator( QgsVectorLayerCache *vlCache, const QgsFeatureRequest &featureRequest );
%Docstring
This constructor creates a feature iterator, which queries the backend and caches retrieved features.

:param vlCache:          The vector layer cache to use
:param featureRequest:   The feature request to answer
%End

    virtual bool rewind();

%Docstring
Rewind to the beginning of the iterator

:return: bool true if the operation was OK
%End

    virtual bool close();

%Docstring
Close this iterator. No further features will be available.

:return: true if successful
%End

  protected:

    virtual bool fetchFeature( QgsFeature &f );

%Docstring
Implementation for fetching a feature.

:param f:      Will write to this feature

:return: bool  true if the operation was OK

.. seealso:: :py:func:`bool`
%End

};
/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgscachedfeatureiterator.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
