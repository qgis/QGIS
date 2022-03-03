/***************************************************************************
                         qgspointclouddataprovider.h
                         ---------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLOUDDATAPROVIDER_H
#define QGSPOINTCLOUDDATAPROVIDER_H

#include "qgis_core.h"
#include "qgsdataprovider.h"
#include "qgspointcloudattribute.h"
#include "qgsstatisticalsummary.h"
#include "qgspointcloudindex.h"
#include "qgspoint.h"
#include "qgsray3d.h"
#include <memory>

class IndexedPointCloudNode;
class QgsPointCloudIndex;
class QgsPointCloudRenderer;
class QgsGeometry;

/**
 * \ingroup core
 * \brief Base class for providing data for QgsPointCloudLayer
 *
 * Responsible for reading native point cloud data and returning the indexed data.
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsPointCloudDataProvider: public QgsDataProvider
{
    Q_OBJECT
  public:

    /**
     * Capabilities that providers may implement.
     */
    enum Capability
    {
      NoCapabilities = 0,       //!< Provider has no capabilities
      ReadLayerMetadata = 1 << 0, //!< Provider can read layer metadata from data store.
      WriteLayerMetadata = 1 << 1, //!< Provider can write layer metadata to the data store. See QgsDataProvider::writeLayerMetadata()
      CreateRenderer = 1 << 2, //!< Provider can create 2D renderers using backend-specific formatting information. See QgsPointCloudDataProvider::createRenderer().
    };

    Q_DECLARE_FLAGS( Capabilities, Capability )

    /**
     * Point cloud index state
     */
    enum PointCloudIndexGenerationState
    {
      NotIndexed = 0, //!< Provider has no index available
      Indexing = 1 << 0, //!< Provider try to index the source data
      Indexed = 1 << 1 //!< The index is ready to be used
    };

    //! Ctor
    QgsPointCloudDataProvider( const QString &uri,
                               const QgsDataProvider::ProviderOptions &providerOptions,
                               QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() );

    ~QgsPointCloudDataProvider() override;

#ifndef SIP_RUN

    /**
     * Returns the list of points of the point cloud according to a zoom level
     * defined by \a maxError (in layer coordinates), an extent \a geometry in the 2D plane
     * and a range \a extentZRange for z values. The function will try to limit
     * the number of points returned to \a pointsLimit points
     *
     * \a maxErrorPixels : maximum accepted error factor in pixels
     *
     * \note this function does not handle elevation properties and you need to
     * change elevation coordinates yourself after returning from the function
     */
    QVector<QVariantMap> identify( double maxError, const QgsGeometry &extentGeometry, const QgsDoubleRange &extentZRange = QgsDoubleRange(), int pointsLimit = 1000 );
#else

    /**
     * Returns the list of points of the point cloud according to a zoom level
     * defined by \a maxError (in layer coordinates), an extent \a geometry in the 2D plane
     * and a range \a extentZRange for z values. The function will try to limit
     * the number of points returned to \a pointsLimit points
     *
     * \a maxErrorPixels : maximum accepted error factor in pixels
     *
     * \note this function does not handle elevation properties and you need to
     * change elevation coordinates yourself after returning from the function
     */
    SIP_PYLIST identify( float maxErrorInMapCoords, QgsGeometry extentGeometry, const QgsDoubleRange extentZRange = QgsDoubleRange(), int pointsLimit = 1000 );
    % MethodCode
    {
      QVector<QMap<QString, QVariant>> res = sipCpp->identify( a0, *a1, *a2, a3 );
      sipRes = PyList_New( res.size() );
      for ( int i = 0; i < res.size(); ++i )
      {
        PyObject *dict = PyDict_New();
        for ( QString key : res[i].keys() )
        {
          PyObject *keyObj = sipConvertFromNewType( new QString( key ), sipType_QString, Py_None );
          PyObject *valObj = sipConvertFromNewType( new QVariant( res[i][key] ), sipType_QVariant, Py_None );
          PyDict_SetItem( dict, keyObj, valObj );
        }
        PyList_SET_ITEM( sipRes, i, dict );
      }
    }
    % End
#endif

    /**
     * Returns flags containing the supported capabilities for the data provider.
     */
    virtual QgsPointCloudDataProvider::Capabilities capabilities() const;

    /**
     * Returns the attributes available from this data provider.
     * May return empty collection until pointCloudIndexLoaded() is emitted
     */
    virtual QgsPointCloudAttributeCollection attributes() const = 0;

    /**
     * Triggers loading of the point cloud index
     *
     * \sa index()
     */
    virtual void loadIndex( ) = 0;

    /**
     * Triggers generation of the point cloud index
     *
     * emits indexGenerationStateChanged()
     *
     * \sa index()
     */
    virtual void generateIndex( ) = 0;


    /**
     * Gets the current index generation state
     */
    virtual PointCloudIndexGenerationState indexingState( ) = 0;

    /**
     * Returns the point cloud index associated with the provider.
     *
     * Can be nullptr (e.g. the index is being created)
     *
     * \note Not available in Python bindings
     */
    virtual QgsPointCloudIndex *index() const SIP_SKIP {return nullptr;}

    /**
     * Returns whether provider has index which is valid
     */
    bool hasValidIndex() const;

    /**
     * Returns the total number of points available in the dataset.
     */
    virtual qint64 pointCount() const = 0;

    /**
     * Returns the polygon bounds of the layer. The CRS of the returned geometry will match the provider's crs().
     *
     * This method will return the best approximation for the actual bounds of points contained in the
     * dataset available from the provider's metadata. This may match the bounding box rectangle returned
     * by extent(), or for some datasets a "convex hull" style polygon representing a more precise bounds
     * will be returned.
     *
     * This method will not attempt to calculate the data bounds, rather it will return only whatever precomputed bounds
     * are included in the data source's metadata.
     */
    virtual QgsGeometry polygonBounds() const;

    /**
     * Returns a representation of the original metadata included in a point cloud dataset.
     *
     * This is a free-form dictionary of values, the contents and structure of which will vary by provider and
     * dataset.
     */
    virtual QVariantMap originalMetadata() const;

    /**
     * Creates a new 2D point cloud renderer, using provider backend specific information.
     *
     * The \a configuration map can be used to pass provider-specific configuration maps to the provider to
     * allow customization of the returned renderer. Support and format of \a configuration varies by provider.
     *
     * When called with an empty \a configuration map the provider's default renderer will be returned.
     *
     * This method returns a new renderer and the caller takes ownership of the returned object.
     *
     * Only providers which report the CreateRenderer capability will return a 2D renderer. Other
     * providers will return NULLPTR.
     */
    virtual QgsPointCloudRenderer *createRenderer( const QVariantMap &configuration = QVariantMap() ) const SIP_FACTORY;
#ifndef SIP_RUN

    /**
     * Returns a statistic for the specified \a attribute, taken only from the metadata of the point cloud
     * data source.
     *
     * This method will not perform any statistical calculations, rather it will return only precomputed attribute
     * statistics which are included in the data source's metadata. Not all data sources include this information
     * in the metadata, and even for sources with statistical metadata only some \a statistic values may be available.
     *
     * If no matching precalculated statistic is available then an invalid variant will be returned.
     */
    virtual QVariant metadataStatistic( const QString &attribute, QgsStatisticalSummary::Statistic statistic ) const;
#else

    /**
     * Returns a statistic for the specified \a attribute, taken only from the metadata of the point cloud
     * data source.
     *
     * This method will not perform any statistical calculations, rather it will return only precomputed attribute
     * statistics which are included in the data source's metadata. Not all data sources include this information
     * in the metadata, and even for sources with statistical metadata only some \a statistic values may be available.
     *
     * \throws ValueError if no matching precalculated statistic is available for the attribute.
     */
    SIP_PYOBJECT metadataStatistic( const QString &attribute, QgsStatisticalSummary::Statistic statistic ) const;
    % MethodCode
    {
      const QVariant res = sipCpp->metadataStatistic( *a0, a1 );
      if ( !res.isValid() )
      {
        PyErr_SetString( PyExc_ValueError, QStringLiteral( "Statistic is not available" ).toUtf8().constData() );
        sipIsErr = 1;
      }
      else
      {
        QVariant *v = new QVariant( res );
        sipRes = sipConvertFromNewType( v, sipType_QVariant, Py_None );
      }
    }
    % End
#endif

    /**
     * Returns a list of existing classes which are present for the specified \a attribute, taken only from the
     * metadata of the point cloud data source.
     *
     * This method will not perform any classification or scan for available classes, rather it will return only
     * precomputed classes which are included in the data source's metadata. Not all data sources include this information
     * in the metadata.
     */
    virtual QVariantList metadataClasses( const QString &attribute ) const;


#ifndef SIP_RUN

    /**
     * Returns a statistic for one class \a value from the specified \a attribute, taken only from the metadata of the point cloud
     * data source.
     *
     * This method will not perform any statistical calculations, rather it will return only precomputed class
     * statistics which are included in the data source's metadata. Not all data sources include this information
     * in the metadata, and even for sources with statistical metadata only some \a statistic values may be available.
     *
     * If no matching precalculated statistic is available then an invalid variant will be returned.
     */
    virtual QVariant metadataClassStatistic( const QString &attribute, const QVariant &value, QgsStatisticalSummary::Statistic statistic ) const;

#else

    /**
     * Returns a statistic for one class \a value from the specified \a attribute, taken only from the metadata of the point cloud
     * data source.
     * This method will not perform any statistical calculations, rather it will return only precomputed class
     * statistics which are included in the data source's metadata. Not all data sources include this information
     * in the metadata, and even for sources with statistical metadata only some \a statistic values may be available.
     *
     * \throws ValueError if no matching precalculated statistic is available for the attribute.
     */
    SIP_PYOBJECT metadataClassStatistic( const QString &attribute, const QVariant &value, QgsStatisticalSummary::Statistic statistic ) const;
    % MethodCode
    {
      const QVariant res = sipCpp->metadataClassStatistic( *a0, *a1, a2 );
      if ( !res.isValid() )
      {
        PyErr_SetString( PyExc_ValueError, QStringLiteral( "Statistic is not available" ).toUtf8().constData() );
        sipIsErr = 1;
      }
      else
      {
        QVariant *v = new QVariant( res );
        sipRes = sipConvertFromNewType( v, sipType_QVariant, Py_None );
      }
    }
    % End
#endif

    bool supportsSubsetString() const override { return true; }
    QString subsetString() const override;
    bool setSubsetString( const QString &subset, bool updateFeatureCount = false ) override;

    /**
     * Returns the map of LAS classification code to untranslated string value, corresponding to the ASPRS Standard
     * Lidar Point Classes.
     *
     * \see translatedLasClassificationCodes()
     */
    static QMap< int, QString > lasClassificationCodes();

    /**
     * Returns the map of LAS classification code to translated string value, corresponding to the ASPRS Standard
     * Lidar Point Classes.
     *
     * \see lasClassificationCodes()
     */
    static QMap< int, QString > translatedLasClassificationCodes();

    /**
     * Returns the map of LAS data format ID to untranslated string value.
     *
     * \see translatedDataFormatIds()
     */
    static QMap< int, QString > dataFormatIds();

    /**
     * Returns the map of LAS data format ID to translated string value.
     *
     * \see dataFormatIds()
     */
    static QMap< int, QString > translatedDataFormatIds();

  signals:

    /**
     * Emitted when point cloud generation state is changed
     */
    void indexGenerationStateChanged( PointCloudIndexGenerationState state );

  private:
    QVector<IndexedPointCloudNode> traverseTree( const QgsPointCloudIndex *pc, IndexedPointCloudNode n, double maxError, double nodeError, const QgsGeometry &extentGeometry, const QgsDoubleRange &extentZRange );

    //! String used to define a subset of the layer
    QString mSubsetString;
};

#endif // QGSMESHDATAPROVIDER_H
