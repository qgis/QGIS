/***************************************************************************
    qgsspatialindex.h  - wrapper class for spatial index library
    ----------------------
    begin                : December 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSPATIALINDEX_H
#define QGSSPATIALINDEX_H


#include "qgis_sip.h"

// forward declaration
namespace SpatialIndex SIP_SKIP
{
  class IStorageManager;
  class ISpatialIndex;
  class Region;
  class Point;

  namespace StorageManager
  {
    class IBuffer;
  }
}

class QgsFeedback;
class QgsFeature;
class QgsRectangle;
class QgsPointXY;

#include "qgis_core.h"
#include "qgsfeaturesink.h"
#include <QList>
#include <QSharedDataPointer>

#include "qgsfeature.h"

class QgsSpatialIndexData;
class QgsFeatureIterator;
class QgsFeatureSource;

/**
 * \ingroup core
 * \class QgsSpatialIndex
 *
 * A spatial index for QgsFeature objects.
 *
 * QgsSpatialIndex objects are implicitly shared and can be inexpensively copied.
 *
 * \note While the underlying libspatialindex is not thread safe on some platforms, the QgsSpatialIndex
 * class implements its own locks and accordingly, a single QgsSpatialIndex object can safely
 * be used across multiple threads.
 *
 * \see QgsSpatialIndexKDBush, which is an optimised non-mutable index for point geometries only.
 * \see QgsMeshSpatialIndex, which is for mesh faces
 */
class CORE_EXPORT QgsSpatialIndex : public QgsFeatureSink
{

  public:

    /* creation of spatial index */

    //! Flags controlling index behavior
    enum Flag
    {
      FlagStoreFeatureGeometries = 1 << 0, //!< Indicates that the spatial index should also store feature geometries. This requires more memory, but can speed up operations by avoiding additional requests to data providers to fetch matching feature geometries. Additionally, it is required for non-bounding box nearest neighbor searches.
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    /**
     * Constructor for QgsSpatialIndex. Creates an empty R-tree index.
     */
    QgsSpatialIndex( QgsSpatialIndex::Flags flags = nullptr );

    /**
     * Constructor - creates R-tree and bulk loads it with features from the iterator.
     * This is much faster approach than creating an empty index and then inserting features one by one.
     *
     * The optional \a feedback object can be used to allow cancellation of bulk feature loading. Ownership
     * of \a feedback is not transferred, and callers must take care that the lifetime of feedback exceeds
     * that of the spatial index construction.
     *
     * \since QGIS 2.8
     */
    explicit QgsSpatialIndex( const QgsFeatureIterator &fi, QgsFeedback *feedback = nullptr, QgsSpatialIndex::Flags flags = nullptr );

    /**
     * Constructor - creates R-tree and bulk loads it with features from the source.
     * This is much faster approach than creating an empty index and then inserting features one by one.
     *
     * The optional \a feedback object can be used to allow cancellation of bulk feature loading. Ownership
     * of \a feedback is not transferred, and callers must take care that the lifetime of feedback exceeds
     * that of the spatial index construction.

     *
     * \since QGIS 3.0
     */
    explicit QgsSpatialIndex( const QgsFeatureSource &source, QgsFeedback *feedback = nullptr, QgsSpatialIndex::Flags flags = nullptr );

    //! Copy constructor
    QgsSpatialIndex( const QgsSpatialIndex &other );

    //! Destructor finalizes work with spatial index
    ~QgsSpatialIndex() override;

    //! Implement assignment operator
    QgsSpatialIndex &operator=( const QgsSpatialIndex &other );

    /* operations */

    /**
     * Adds a \a feature to the index.
     * \deprecated Use addFeature() instead
     */
    Q_DECL_DEPRECATED bool insertFeature( const QgsFeature &feature ) SIP_DEPRECATED;

    /**
     * Adds a \a feature to the index.
     *
     * The \a flags argument is ignored.
     *
     * \since QGIS 3.4
     */
    bool addFeature( QgsFeature &feature, QgsFeatureSink::Flags flags = nullptr ) override;

    /**
     * Adds a list of \a features to the index.
     *
     * The \a flags argument is ignored.
     *
     * \see addFeature()
     */
    bool addFeatures( QgsFeatureList &features, QgsFeatureSink::Flags flags = nullptr ) override;

    /**
     * Add a feature \a id to the index with a specified bounding box.
     * \returns TRUE if feature was successfully added to index.
     * \deprecated Use addFeature() instead
    */
    Q_DECL_DEPRECATED bool insertFeature( QgsFeatureId id, const QgsRectangle &bounds ) SIP_DEPRECATED;

    /**
     * Add a feature \a id to the index with a specified bounding box.
     * \returns TRUE if feature was successfully added to index.
     * \since QGIS 3.4
    */
    bool addFeature( QgsFeatureId id, const QgsRectangle &bounds );

    /**
     * Removes a \a feature from the index.
     */
    bool deleteFeature( const QgsFeature &feature );


    /* queries */

    /**
     * Returns a list of features with a bounding box which intersects the specified \a rectangle.
     *
     * \note The intersection test is performed based on the feature bounding boxes only, so for non-point
     * geometry features it is necessary to manually test the returned features for exact geometry intersection
     * when required.
     */
    QList<QgsFeatureId> intersects( const QgsRectangle &rectangle ) const;

    /**
     * Returns nearest neighbors to a \a point. The number of neighbors returned is specified
     * by the \a neighbors argument.
     *
     * If the \a maxDistance argument is greater than 0, then only features within the specified
     * distance of \a point will be considered.
     *
     * Note that in some cases the number of returned features may differ from the requested
     * number of \a neighbors. E.g. if not enough features exist within the \a maxDistance of the
     * search point. If multiple features are equidistant from the search \a point then the
     * number of returned feature IDs may exceed \a neighbors.
     *
     * \warning If this QgsSpatialIndex object was not constructed with the FlagStoreFeatureGeometries flag,
     * then the nearest neighbor test is performed based on the feature bounding boxes ONLY, so for non-point
     * geometry features this method is not guaranteed to return the actual closest neighbors.
     */
    QList<QgsFeatureId> nearestNeighbor( const QgsPointXY &point, int neighbors = 1, double maxDistance = 0 ) const;

    /**
     * Returns nearest neighbors to a \a geometry. The number of neighbors returned is specified
     * by the \a neighbors argument.
     *
     * If the \a maxDistance argument is greater than 0, then only features within the specified
     * distance of \a point will be considered.
     *
     * Note that in some cases the number of returned features may differ from the requested
     * number of \a neighbors. E.g. if not enough features exist within the \a maxDistance of the
     * search point. If multiple features are equidistant from the search \a point then the
     * number of returned feature IDs may exceed \a neighbors.
     *
     * \warning If this QgsSpatialIndex object was not constructed with the FlagStoreFeatureGeometries flag,
     * then the nearest neighbor test is performed based on the feature bounding boxes ONLY, so for non-point
     * geometry features this method is not guaranteed to return the actual closest neighbors.
     *
     * \since QGIS 3.8
     */
    QList<QgsFeatureId> nearestNeighbor( const QgsGeometry &geometry, int neighbors = 1, double maxDistance = 0 ) const;

#ifndef SIP_RUN

    /**
     * Returns the stored geometry for the indexed feature with matching \a id.
     *
     * Geometry is only stored if the QgsSpatialIndex was created with the FlagStoreFeatureGeometries flag.
     *
     * \since QGIS 3.6
     */
    QgsGeometry geometry( QgsFeatureId id ) const;

#else

    /**
     * Returns the stored geometry for the indexed feature with matching \a id. A KeyError will be raised if no
     * geometry with the specified feature id exists in the index.
     *
     * Geometry is only stored if the QgsSpatialIndex was created with the FlagStoreFeatureGeometries flag.
     *
     * \since QGIS 3.6
     */
    SIP_PYOBJECT geometry( QgsFeatureId id ) const SIP_TYPEHINT( QgsGeometry );
    % MethodCode
    std::unique_ptr< QgsGeometry > g = qgis::make_unique< QgsGeometry >( sipCpp->geometry( a0 ) );
    if ( g->isNull() )
    {
      PyErr_SetString( PyExc_KeyError, QStringLiteral( "No geometry with feature id %1 exists in the index." ).arg( a0 ).toUtf8().constData() );
      sipIsErr = 1;
    }
    else
    {
      sipRes = sipConvertFromType( g.release(), sipType_QgsGeometry, Py_None );
    }
    % End
#endif

    /* debugging */

    //! Gets reference count - just for debugging!
    QAtomicInt SIP_PYALTERNATIVETYPE( int ) refs() const;

  private:

    static SpatialIndex::Region rectToRegion( const QgsRectangle &rect );

    /**
     * Calculates feature info to insert into index.
    * \param f input feature
    * \param r will be set to spatial index region
    * \param id will be set to feature's ID
    * \returns TRUE if feature info was successfully retrieved and the feature can be added to
    * the index
    */
    static bool featureInfo( const QgsFeature &f, SpatialIndex::Region &r, QgsFeatureId &id ) SIP_SKIP;

    /**
     * Calculates feature info to insert into index.
     * \param f input feature
     * \param rect will be set to feature's geometry bounding box
     * \param id will be set to feature's ID
     * \returns TRUE if feature info was successfully retrieved and the feature can be added to
     * the index
     * \since QGIS 3.0
     */
    static bool featureInfo( const QgsFeature &f, QgsRectangle &rect, QgsFeatureId &id );

    friend class QgsFeatureIteratorDataStream; // for access to featureInfo()

  private:

    QSharedDataPointer<QgsSpatialIndexData> d;

};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsSpatialIndex::Flags )

#endif //QGSSPATIALINDEX_H
