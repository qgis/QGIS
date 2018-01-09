/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsvectorlayercache.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







class QgsVectorLayerCache : QObject
{
%Docstring
This class caches features of a given QgsVectorLayer.

The cached features can be indexed by QgsAbstractCacheIndex.

Proper indexing for a given use-case may speed up performance substantially.
%End

%TypeHeaderCode
#include "qgsvectorlayercache.h"
%End
      public:
  public:
    QgsVectorLayerCache( QgsVectorLayer *layer, int cacheSize, QObject *parent /TransferThis/ = 0 );
    ~QgsVectorLayerCache();

    void setCacheSize( int cacheSize );
%Docstring
Sets the maximum number of features to keep in the cache. Some features will be removed from
the cache if the number is smaller than the previous size of the cache.

:param cacheSize: indicates the maximum number of features to keep in the cache
%End

    int cacheSize();
%Docstring
Returns the maximum number of features this cache will hold.
In case full caching is enabled, this number can change, as new features get added.

:return: int
%End

    void setCacheGeometry( bool cacheGeometry );
%Docstring
Enable or disable the caching of geometries

:param cacheGeometry:    Enable or disable the caching of geometries

.. seealso:: :py:func:`cacheGeometry`
%End

    bool cacheGeometry() const;
%Docstring
Returns true if the cache will fetch and cache feature geometries.

.. versionadded:: 3.0

.. seealso:: :py:func:`setCacheGeometry`
%End

    void setCacheSubsetOfAttributes( const QgsAttributeList &attributes );
%Docstring
Set the subset of attributes to be cached

:param attributes:   The attributes to be cached
%End

    void setCacheAddedAttributes( bool cacheAddedAttributes );
%Docstring
If this is enabled, the subset of cached attributes will automatically be extended
to also include newly added attributes.

:param cacheAddedAttributes:   Automatically cache new attributes
%End

    void setFullCache( bool fullCache );
%Docstring
This enables or disables full caching.
If enabled, all features will be held in the cache. The cache size will incrementally
be increased to offer space for all features.
When enabled, all features will be read into cache. As this feature will most likely
be used for slow data sources, be aware, that the call to this method might take a long time.

:param fullCache:   True: enable full caching, False: disable full caching

.. note::

   when a cache is invalidated() (e.g. by adding an attribute to a layer) this setting
is reset. A full cache rebuild must be performed by calling setFullCache( true ) again.

.. seealso:: :py:func:`hasFullCache`
%End

    bool hasFullCache() const;
%Docstring
Returns true if the cache is complete, ie it contains all features. This may happen as
a result of a call to setFullCache() or by through a feature request which resulted in
all available features being cached.

.. seealso:: :py:func:`setFullCache`

.. versionadded:: 3.0
%End

    void addCacheIndex( QgsAbstractCacheIndex *cacheIndex /Transfer/ );
%Docstring
Adds a QgsAbstractCacheIndex to this cache. Cache indices know about features present
in this cache and decide, if enough information is present in the cache to respond to a :py:class:`QgsFeatureRequest`.
The layer cache will take ownership of the index.

:param cacheIndex:  The cache index to add.
%End

    QgsFeatureIterator getFeatures( const QgsFeatureRequest &featureRequest = QgsFeatureRequest() );
%Docstring
Query this VectorLayerCache for features.
If the VectorLayerCache (and moreover any of its indices) is able to satisfy
the request, the returned QgsFeatureIterator will iterate over cached features.
If it's not possible to fully satisfy the request from the cache, part or all of the features
will be requested from the data provider.

:param featureRequest:  The request specifying filter and required data.

:return: An iterator over the requested data.
%End

    QgsFeatureIterator getFeatures( const QString &expression );
%Docstring
Query the layer for features matching a given expression.
%End

    QgsFeature getFeature( QgsFeatureId fid );
%Docstring
Query the layer for the feature with the given id.
If there is no such feature, the returned feature will be invalid.
%End

    QgsFeatureIterator getFeatures( const QgsFeatureIds &fids );
%Docstring
Query the layer for the features with the given ids.
%End

    QgsFeatureIterator getFeatures( const QgsRectangle &rectangle );
%Docstring
Query the layer for the features which intersect the specified rectangle.
%End

    bool isFidCached( const QgsFeatureId fid ) const;
%Docstring
Check if a certain feature id is cached.

\param  fid The feature id to look for

:return: True if this id is in the cache

.. seealso:: :py:func:`cachedFeatureIds`
%End

    QgsFeatureIds cachedFeatureIds() const;
%Docstring
Returns the set of feature IDs for features which are cached.

.. versionadded:: 3.0

.. seealso:: :py:func:`isFidCached`
%End

    bool featureAtId( QgsFeatureId featureId, QgsFeature &feature, bool skipCache = false );
%Docstring
Gets the feature at the given feature id. Considers the changed, added, deleted and permanent features

:param featureId: The id of the feature to query
:param feature:   The result of the operation will be written to this feature
:param skipCache: Will query the layer regardless if the feature is in the cache already

:return: true in case of success
%End

    bool removeCachedFeature( QgsFeatureId fid );
%Docstring
Removes the feature identified by fid from the cache if present.

:param fid: The id of the feature to delete

:return: true if the feature was removed, false if the feature id was not found in the cache
%End

    QgsVectorLayer *layer();
%Docstring
Returns the layer to which this cache belongs
%End

    QgsCoordinateReferenceSystem sourceCrs() const;
%Docstring
Returns the coordinate reference system for features in the cache.
%End

    QgsFields fields() const;
%Docstring
Returns the fields associated with features in the cache.
%End

    QgsWkbTypes::Type wkbType() const;
%Docstring
Returns the geometry type for features in the cache.
%End


    int __len__() const;
%Docstring
Returns the number of features contained in the source, or -1
if the feature count is unknown.
%End
%MethodCode
    sipRes = sipCpp->featureCount();
%End

    long featureCount() const;
%Docstring
Returns the number of features contained in the source, or -1
if the feature count is unknown.
%End

  protected:

    void requestCompleted( const QgsFeatureRequest &featureRequest, const QgsFeatureIds &fids );
%Docstring
Gets called, whenever the full list of feature ids for a certain request is known.
Broadcasts this information to indices, so they can update their tables.

:param featureRequest:  The feature request that was answered
:param fids:            The feature ids that have been returned
%End

    void featureRemoved( QgsFeatureId fid );
%Docstring
Gets called, whenever a feature has been removed.
Broadcasts this information to indices, so they can invalidate their cache if required.

:param fid:             The feature id of the removed feature.
%End

    bool checkInformationCovered( const QgsFeatureRequest &featureRequest );
%Docstring
Checks if the information required to complete the request is cached.
i.e. If all attributes required and the geometry is held in the cache.
Please note, that this does not check, if the requested features are cached.

:param featureRequest:  The QgsFeatureRequest to be answered

:return:                True if the information is being cached, false if not
%End


  signals:


    void finished();
%Docstring
When filling the cache, this signal gets emitted once the cache is fully initialized.
%End

    void cachedLayerDeleted();
%Docstring
 Is emitted when the cached layer is deleted. Is emitted when the cached layers layerDelete()
signal is being emitted, but before the local reference to it has been set to NULL. So call to
layer() will still return a valid pointer for cleanup purpose.
%End

    void attributeValueChanged( QgsFeatureId fid, int field, const QVariant &value );
%Docstring
 Is emitted when an attribute is changed. Is re-emitted after the layer itself emits this signal.
You should connect to this signal, to be sure, to not get a cached value if querying the cache.
%End

    void featureAdded( QgsFeatureId fid );
%Docstring
Is emitted, when a new feature has been added to the layer and this cache.
You should connect to this signal instead of the layers', if you want to be sure
that this cache has updated information for the new feature

:param fid: The featureid of the changed feature
%End

    void invalidated();
%Docstring
The cache has been invalidated and cleared. Note that when a cache is invalidated
the fullCache() setting will be cleared, and a full cache rebuild via setFullCache( true )
will need to be performed.
%End

};
/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsvectorlayercache.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
