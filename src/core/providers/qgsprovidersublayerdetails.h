/***************************************************************************
                             qgsprovidersublayerdetails.h
                             ----------------------------
    begin                : May 2021
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPROVIDERSUBLAYERDETAILS_H
#define QGSPROVIDERSUBLAYERDETAILS_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgswkbtypes.h"
#include "qgscoordinatetransformcontext.h"
#include "qgsmimedatautils.h"

#include <QString>
#include <QStringList>

/**
 * \class QgsProviderSublayerDetails
 * \ingroup core
 *
 * \brief Contains details about a sub layer available from a dataset.
 *
 * This class encapsulates the properties of a single sublayer available from a dataset,
 * according to a specific data provider.
 *
 * \note It is possible that multiple data providers may be able to utilize the same underlying
 * sub layer in a single dataset, yet will interpret this layer differently. For instance, a layer
 * in a GeoPDF document can be interpreted as either a vector layer by the OGR data provider or a raster
 * layer by the GDAL provider. The providerKey() property can be used to determine the data provider
 * associated with a QgsProviderSubLayerDetails instance.
 *
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsProviderSublayerDetails
{
  public:

    /**
     * Returns the associated data provider key.
     *
     * \see setProviderKey()
     */
    QString providerKey() const { return mProviderKey; }

    /**
     * Sets the associated data provider key.
     *
     * \see providerKey()
     */
    void setProviderKey( const QString &key ) { mProviderKey = key; }

    /**
     * Returns the layer type.
     *
     * \see setType()
     */
    QgsMapLayerType type() const { return mType; }

    /**
     * Sets the layer \a type.
     *
     * \see type()
     */
    void setType( QgsMapLayerType type ) { mType = type; }

    /**
     * Returns the layer's URI.
     *
     * \see setUri()
     */
    QString uri() const { return mUri; }

    /**
     * Sets the layer's \a uri.
     *
     * \see uri()
     */
    void setUri( const QString &uri ) { mUri = uri; }

    /**
     * Setting options for loading layers.
     */
    struct LayerOptions
    {

      /**
       * Constructor for LayerOptions with \a transformContext.
       */
      explicit LayerOptions( const QgsCoordinateTransformContext &transformContext )
        : transformContext( transformContext )
      {}

      //! Coordinate transform context
      QgsCoordinateTransformContext transformContext;

      //! Set to TRUE if the default layer style should be loaded
      bool loadDefaultStyle = true;

      /**
       * Controls whether the stored styles will be all loaded.
       *
       * If TRUE and the layer's provider supports style stored in the
       * data source all the available styles will be loaded in addition
       * to the default one.
       *
       * If FALSE (the default), the layer's provider will only load
       * the default style.
       *
       * \since QGIS 3.30
       */
      bool loadAllStoredStyle = false;
    };

    /**
     * Creates a new QgsMapLayer object associated with the sublayer.
     *
     * Caller takes ownership of the returned layer.
     */
    QgsMapLayer *toLayer( const LayerOptions &options ) const SIP_FACTORY;

    /**
     * Returns the layer's name.
     *
     * \see setName()
     */
    QString name() const { return mName; }

    /**
     * Sets the layer's \a name.
     *
     * \see name()
     */
    void setName( const QString &name ) { mName = name; }

    /**
     * Returns the layer's description.
     *
     * \see setDescription()
     */
    QString description() const { return mDescription; }

    /**
     * Sets the layer's \a description.
     *
     * \see description()
     */
    void setDescription( const QString &description ) { mDescription = description; }

    /**
     * Returns the layer's flags, which indicate properties of the layer.
     *
     * \see setFlags()
     */
    Qgis::SublayerFlags flags() const { return mFlags; }

    /**
     * Sets the layer's \a flags, which indicate properties of the layer.
     *
     * \see flags()
     */
    void setFlags( Qgis::SublayerFlags newFlags ) { mFlags = newFlags; }

    /**
     * Returns the layer's driver name.
     *
     * This is supported only for providers which handle multiple drivers.
     *
     * \see setDriverName()
     */
    QString driverName() const { return mDriverName; }

    /**
     * Sets the layer's \a driver name.
     *
     * \see driverName()
     */
    void setDriverName( const QString &driver ) { mDriverName = driver; }

    /**
     * Returns the path to the sublayer.
     *
     * This is an internal path, relative to the dataset itself. It can be used to encapsulate
     * the hierarchy of a dataset which organises layers in schemas or in an internal folder structure.
     *
     * \see setPath()
     */
    QStringList path() const { return mPath; }

    /**
     * Sets the \a path to the sublayer.
     *
     * This is an internal path, relative to the dataset itself. It can be used to encapsulate
     * the hierarchy of a dataset which organises layers in schemas or in an internal folder structure.
     *
     * \see path()
     */
    void setPath( const QStringList &path ) { mPath = path; }

    /**
     * Returns the layer's feature count.
     *
     * Will return Qgis::FeatureCountState::UnknownCount or Qgis::FeatureCountState::Uncounted if no feature count is applicable or available.
     *
     * \see setFeatureCount()
     */
    long long featureCount() const { return mFeatureCount; }

    /**
     * Sets the layer's feature \a count.
     *
     * Set to Qgis::FeatureCountState::UnknownCount or Qgis::FeatureCountState::Uncounted if no feature count is applicable or available.
     *
     * \see featureCount()
     */
    void setFeatureCount( long long count ) { mFeatureCount = count; }

    /**
     * Returns the layer's WKB type, or QgsWkbTypes::Unknown if the WKB type is not application or unknown.
     *
     * \see setWkbType()
     */
    QgsWkbTypes::Type wkbType() const { return mWkbType; }

    /**
     * Set the layer's WKB \a type.
     *
     * Set to QgsWkbTypes::Unknown if the WKB type is not application or unknown.
     *
     * \see wkbType()
     */
    void setWkbType( QgsWkbTypes::Type type ) { mWkbType = type; }

    /**
     * Returns the layer's geometry column name, or an empty string if not applicable.
     *
     * \see setGeometryColumnName()
     */
    QString geometryColumnName() const { return mGeometryColumnName; }

    /**
     * Sets the layer's geometry column \a name.
     *
     * Set to an empty string if not applicable.
     *
     * \see geometryColumnName()
     */
    void setGeometryColumnName( const QString &name ) { mGeometryColumnName = name; }

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsProviderSublayerDetails: %1 - %2>" ).arg( sipCpp->providerKey(), sipCpp->name() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    /**
     * Returns the associated layer number, for providers which order sublayers.
     *
     * \see setLayerNumber()
     */
    int layerNumber() const { return mLayerNumber; }

    /**
     * Sets the associated layer \a number, for providers which order sublayers.
     *
     * \see layerNumber()
     */
    void setLayerNumber( int number ) { mLayerNumber = number; }

    /**
     * Set to TRUE if the layer is a potential dataset container and an in-depth scan
     * of its contents was skipped.
     *
     * \see skippedContainerScan();
     */
    void setSkippedContainerScan( bool skipped ) { mSkippedContainerScan = skipped; }

    /**
     * Returns TRUE if the layer is a potential dataset container and an in-depth scan
     * of its contents was skipped.
     *
     * \see setSkippedContainerScan();
     */
    bool skippedContainerScan() const { return mSkippedContainerScan; }

    /**
     * Converts the sublayer details to a QgsMimeDataUtils::Uri representing the sublayer.
     *
     * \since QGIS 3.28
     */
    QgsMimeDataUtils::Uri toMimeUri() const;

    // TODO c++20 - replace with = default
    bool operator==( const QgsProviderSublayerDetails &other ) const;
    bool operator!=( const QgsProviderSublayerDetails &other ) const;

  private:

    QString mProviderKey;
    QgsMapLayerType mType = QgsMapLayerType::VectorLayer;
    QString mUri;
    int mLayerNumber = 0;
    QString mName;
    QString mDescription;
    long long mFeatureCount = static_cast< long >( Qgis::FeatureCountState::UnknownCount );
    QString mGeometryColumnName;
    QStringList mPath;
    QgsWkbTypes::Type mWkbType = QgsWkbTypes::Unknown;
    QString mDriverName;
    bool mSkippedContainerScan = false;
    Qgis::SublayerFlags mFlags = Qgis::SublayerFlags();

};

#endif //QGSPROVIDERSUBLAYERDETAILS_H



