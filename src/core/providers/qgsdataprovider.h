/***************************************************************************
                qgsdataprovider.h - DataProvider Interface class
                     --------------------------------------
    Date                 : 09-Sep-2003
    Copyright            : (C) 2003 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QQGSDATAPROVIDER_H
#define QQGSDATAPROVIDER_H

#include "qgis_core.h"
#include <QDateTime>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QMutex>

#include "qgsdatasourceuri.h"
#include "qgscoordinatetransformcontext.h"
#include "qgslayermetadata.h"
#include "qgserror.h"

class QgsRectangle;
class QgsCoordinateReferenceSystem;
class QgsDataProviderTemporalCapabilities;


/**
 * \ingroup core
 * \brief Abstract base class for spatial data provider implementations.
 */
class CORE_EXPORT QgsDataProvider : public QObject
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( qobject_cast<QgsVectorDataProvider *>( sipCpp ) )
    {
      sipType = sipType_QgsVectorDataProvider;
    }
    else if ( qobject_cast<QgsRasterDataProvider *>( sipCpp ) )
    {
      sipType = sipType_QgsRasterDataProvider;
    }
    else if ( qobject_cast<QgsMeshDataProvider *>( sipCpp ) )
    {
      sipType = sipType_QgsMeshDataProvider;
    }
    else if ( qobject_cast<QgsPointCloudDataProvider *>( sipCpp ) )
    {
      sipType = sipType_QgsPointCloudDataProvider;
    }
    else
    {
      sipType = 0;
    }
    SIP_END
#endif
    Q_OBJECT

  public:

    // TODO QGIS 4: (re)move DataCapability as this enum is really meant for data items rather than data providers

    /**
     * Used in browser model to understand which items for which providers should be populated
     */
    enum DataCapability
    {
      NoDataCapabilities  = 0,
      File                = 1,
      Dir                 = 1 << 1,
      Database            = 1 << 2,
      Net                 = 1 << 3  // Internet source
    };
    Q_DECLARE_FLAGS( DataCapabilities, DataCapability )

    /**
     * Properties are used to pass custom configuration options into data providers.
     * This enum defines a list of custom properties which can be used on different
     * providers. It depends on the provider, which properties are supported.
     * In addition to these default properties, providers can add their custom properties
     * starting from CustomData.
     */
    enum ProviderProperty
    {
      EvaluateDefaultValues,       //!< Evaluate default values on provider side when calling QgsVectorDataProvider::defaultValue( int index ) rather than on commit.
      CustomData   = 3000          //!< Custom properties for 3rd party providers or very provider-specific properties which are not expected to be of interest for other providers can be added starting from this value up.
    };


    /**
     * Setting options for creating vector data providers.
     *
     * \note coordinateTransformContext was added in QGIS 3.8
     *
     * \since QGIS 3.2
     */
    struct ProviderOptions
    {

      /**
       * Coordinate transform context
       */
      QgsCoordinateTransformContext transformContext;

    };

    /**
     * Flags which control dataprovider construction.
     * \since QGIS 3.16
     */
    enum ReadFlag
    {
      FlagTrustDataSource = 1 << 0, //!< Trust datasource config (primary key unicity, geometry type and srid, etc). Improves provider load time by skipping expensive checks like primary key unicity, geometry type and srid and by using estimated metadata on data load. Since QGIS 3.16
      SkipFeatureCount = 1 << 1, //!< Make featureCount() return -1 to indicate unknown, and subLayers() to return a unknown feature count as well. Since QGIS 3.18. Only implemented by OGR provider at time of writing.
      FlagLoadDefaultStyle = 1 << 2, //!< Reset the layer's style to the default for the datasource
      SkipGetExtent = 1 << 3, //!< Skip the extent from provider
      SkipFullScan = 1 << 4, //!< Skip expensive full scan on files (i.e. on delimited text) (since QGIS 3.24)
    };
    Q_DECLARE_FLAGS( ReadFlags, ReadFlag )

    /**
     * Create a new dataprovider with the specified in the \a uri.
     *
     * Additional creation options are specified within the \a options value and since QGIS 3.16 creation flags are specified within the \a flags value.
     */
    QgsDataProvider( const QString &uri = QString(),
                     const QgsDataProvider::ProviderOptions &providerOptions = QgsDataProvider::ProviderOptions(),
                     QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() );

    /**
     * Returns the coordinate system for the data source.
     * If the provider isn't capable of returning its projection then an invalid
     * QgsCoordinateReferenceSystem will be returned.
     */
    virtual QgsCoordinateReferenceSystem crs() const = 0;

    /**
     * Set the data source specification. This may be a path or database
     * connection string
     * \param uri source specification
     */
    virtual void setDataSourceUri( const QString &uri )
    {
      mDataSourceURI = uri;
    }

    /**
     * Gets the data source specification. This may be a path or database
     * connection string
     * \param expandAuthConfig Whether to expand any assigned authentication configuration
     * \returns data source specification
     * \note The default authentication configuration expansion is FALSE. This keeps credentials
     * out of layer data source URIs and project files. Expansion should be specifically done
     * only when needed within a provider
     */
    virtual QString dataSourceUri( bool expandAuthConfig = false ) const
    {
      if ( expandAuthConfig && mDataSourceURI.contains( QLatin1String( "authcfg" ) ) )
      {
        const QgsDataSourceUri uri( mDataSourceURI );
        return uri.uri( expandAuthConfig );
      }
      else
      {
        return mDataSourceURI;
      }
    }

    /**
     * Returns a short comment for the data that this provider is
     * providing access to (e.g. the comment for postgres table).
     *
     * \note The default implementation returns an empty string.
     * \since QGIS 3.14
     */
    virtual QString dataComment() const { return QString(); };


    /**
     * Set the data source specification.
     *
     * \since QGIS 3.0
     */
    void setUri( const QgsDataSourceUri &uri )
    {
      mDataSourceURI = uri.uri( true );
    }

    /**
     * Gets the data source specification.
     *
     * \since QGIS 3.0
     */
    QgsDataSourceUri uri() const
    {
      return QgsDataSourceUri( mDataSourceURI );
    }

    /**
     * Returns the generic data provider flags.
     *
     * \since QGIS 3.26
     */
    virtual Qgis::DataProviderFlags flags() const;

    /**
     * Returns the provider's temporal capabilities.
     *
     * This may be NULLPTR, depending on the data provider.
     *
     * \since QGIS 3.14
     */
    virtual QgsDataProviderTemporalCapabilities *temporalCapabilities();

    /**
     * Returns the provider's temporal capabilities.
     *
     * This may be NULLPTR, depending on the data provider.
     *
     * \since QGIS 3.14
     */
    virtual const QgsDataProviderTemporalCapabilities *temporalCapabilities() const SIP_SKIP;

    /**
     * Returns the extent of the layer
     * \returns QgsRectangle containing the extent of the layer
     */
    virtual QgsRectangle extent() const = 0;


    /**
     * Returns TRUE if this is a valid layer. It is up to individual providers
     * to determine what constitutes a valid layer.
     */
    virtual bool isValid() const = 0;

    /**
     * Update the extents of the layer. Not implemented by default.
     */
    virtual void updateExtents()
    {
      // NOP by default
    }


    /**
     * Set the subset string used to create a subset of features in
     * the layer. This may be a sql where clause or any other string
     * that can be used by the data provider to create a subset.
     * Must be implemented in the dataprovider.
     */
    virtual bool setSubsetString( const QString &subset, bool updateFeatureCount = true )
    {
      // NOP by default
      Q_UNUSED( subset )
      Q_UNUSED( updateFeatureCount )
      return false;
    }


    /**
     * Returns TRUE if the provider supports setting of subset strings.
    */
    virtual bool supportsSubsetString() const { return false; }

    /**
     * Returns the subset definition string (typically sql) currently in
     * use by the layer and used by the provider to limit the feature set.
     * Must be overridden in the dataprovider, otherwise returns a null
     * QString.
     */
    virtual QString subsetString() const
    {
      return QString();
    }


    /**
     * Sub-layers handled by this provider, in order from bottom to top
     *
     * Sub-layers are used when the provider's source can combine layers
     * it knows about in some way before it hands them off to the provider.
     *
     * \see SUBLAYER_SEPARATOR
     */
    virtual QStringList subLayers() const
    {
      return QStringList();  // Empty
    }

    /**
     * Sub-layer styles for each sub-layer handled by this provider,
     * in order from bottom to top
     *
     * Sub-layer styles are used to abstract the way the provider's source can symbolise
     * layers in some way at the server, before it serves them to the provider.
     */
    virtual QStringList subLayerStyles() const
    {
      return QStringList();  // Empty
    }


    /**
     * Returns the number of layers for the current data source
     */
    virtual uint subLayerCount() const
    {
      return 0;
    }


    /**
     * Reorder the list of layer names to be rendered by this provider
     * (in order from bottom to top)
     * \note   layers must have been previously added.
     */
    virtual void setLayerOrder( const QStringList &layers )
    {
      //prevent unused var warnings
      if ( layers.count() < 1 )
      {
        return;
      }
      // NOOP
    }


    /**
     * Set the visibility of the given sublayer name
     */
    virtual void setSubLayerVisibility( const QString &name, bool vis )
    {
      //prevent unused var warnings
      if ( name.isEmpty() || !vis )
      {
        return;
      }
      // NOOP
    }


    /**
     * Returns a provider name
     *
     * Essentially just returns the provider key.  Should be used to build file
     * dialogs so that providers can be shown with their supported types. Thus
     * if more than one provider supports a given format, the user is able to
     * select a specific provider to open that file.
     *
     * \note
     *
     * Instead of being pure virtual, might be better to generalize this
     * behavior and presume that none of the sub-classes are going to do
     * anything strange with regards to their name or description?
     *
     */
    virtual QString name() const = 0;


    /**
     * Returns description
     *
     * Returns a terse string describing what the provider is.
     *
     * \note
     *
     * Instead of being pure virtual, might be better to generalize this
     * behavior and presume that none of the sub-classes are going to do
     * anything strange with regards to their name or description?
     *
     */
    virtual QString description() const = 0;


    /**
     * Returns vector file filter string
     *
     * Returns a string suitable for a QFileDialog of vector file formats
     * supported by the data provider.  Naturally this will be an empty string
     * for those data providers that do not deal with plain files, such as
     * databases and servers.
     *
     * \note It'd be nice to eventually be raster/vector neutral.
     */
    virtual QString fileVectorFilters() const
    {
      return QString();
    }


    /**
     * Returns raster file filter string
     *
     * Returns a string suitable for a QFileDialog of raster file formats
     * supported by the data provider.  Naturally this will be an empty string
     * for those data providers that do not deal with plain files, such as
     * databases and servers.
     *
     * \note It'd be nice to eventually be raster/vector neutral.
     */
    virtual QString fileRasterFilters() const
    {
      return QString();
    }

    // TODO QGIS 4 -> Make `reloadData()` non virtual. This should be implemented in `reloadProviderData()`.

    /**
     * Reloads the data from the source for providers with data caches to synchronize,
     * changes in the data source, feature counts and other specific actions.
     * Emits the `dataChanged` signal
     *
     * \note only available for providers which implement the reloadProviderData() method.
     */
    virtual void reloadData();

    //! Time stamp of data source in the moment when data/metadata were loaded by provider
    virtual QDateTime timestamp() const { return mTimestamp; }

    //! Current time stamp of data source
    virtual QDateTime dataTimestamp() const { return QDateTime(); }

    /**
     * Gets current status error. This error describes some principal problem
     *  for which provider cannot work and thus is not valid. It is not last error
     *  after accessing data by block(), identify() etc.
     */
    virtual QgsError error() const { return mError; }

    /**
     * Invalidate connections corresponding to specified name
     * \since QGIS 2.16
     */
    virtual void invalidateConnections( const QString &connection ) { Q_UNUSED( connection ) }

    /**
     * Enter update mode.
     *
     * This is aimed at providers that can open differently the connection to
     * the datasource, according it to be in update mode or in read-only mode.
     * A call to this method shall be balanced with a call to leaveUpdateMode(),
     * if this method returns TRUE.
     *
     * Most providers will have an empty implementation for that method.
     *
     * For backward compatibility, providers that implement enterUpdateMode() should
     * still make sure to allow editing operations to work even if enterUpdateMode()
     * is not explicitly called.
     *
     * Several successive calls to enterUpdateMode() can be done. So there is
     * a concept of stack of calls that must be handled by the provider. Only the first
     * call to enterUpdateMode() will really turn update mode on.
     *
     * \returns TRUE in case of success (or no-op implementation), FALSE in case of failure.
     *
     * \since QGIS 2.16
     */
    virtual bool enterUpdateMode() { return true; }

    /**
     * Leave update mode.
     *
     * This is aimed at providers that can open differently the connection to
     * the datasource, according it to be in update mode or in read-only mode.
     * This method shall be balanced with a successful call to enterUpdateMode().
     *
     * Most providers will have an empty implementation for that method.
     *
     * Several successive calls to enterUpdateMode() can be done. So there is
     * a concept of stack of calls that must be handled by the provider. Only the last
     * call to leaveUpdateMode() will really turn update mode off.
     *
     * \returns TRUE in case of success (or no-op implementation), FALSE in case of failure.
     *
     * \since QGIS 2.16
     */
    virtual bool leaveUpdateMode() { return true; }

    /**
     * Allows setting arbitrary properties on the provider.
     * It depends on the provider which properties are supported.
     *
     * \since QGIS 2.16
     */
    void setProviderProperty( ProviderProperty property, const QVariant &value );

    /**
     * Allows setting arbitrary properties on the provider.
     * It depends on the provider which properties are supported.
     *
     * \since QGIS 2.16
     */
    void setProviderProperty( int property, const QVariant &value ); // SIP_SKIP

    /**
     * Gets the current value of a certain provider property.
     * It depends on the provider which properties are supported.
     *
     * \since QGIS 2.16
     */
    QVariant providerProperty( ProviderProperty property, const QVariant &defaultValue = QVariant() ) const;

    /**
     * Gets the current value of a certain provider property.
     * It depends on the provider which properties are supported.
     *
     * \since QGIS 2.16
     */
    QVariant providerProperty( int property, const QVariant &defaultValue ) const; // SIP_SKIP

    /**
     * Set whether the provider will listen to datasource notifications
     * If set, the provider will issue notify signals.
     *
     * The default implementation does nothing.
     *
     * \see notify
     *
     * \since QGIS 3.0
     */
    virtual void setListening( bool isListening );

#ifndef SIP_RUN

    /**
     * Stores settings related to the context in which a preview job runs.
     * \note Not available in Python bindings
     * \since QGIS 3.0
     */
    struct PreviewContext
    {
      //! Previous rendering time for the layer, in ms
      double lastRenderingTimeMs = -1;

      //! Default maximum allowable render time, in ms
      double maxRenderingTimeMs = MAXIMUM_LAYER_PREVIEW_TIME_MS;
    };
#endif

    /**
     * Returns whether the layer must be rendered in preview jobs.
     *
     * The \a context argument gives useful information which can be used
     * to determine whether the layer should be rendered or not.
     *
     * The base implementation returns TRUE if lastRenderingTimeMs <= maxRenderingTimeMs.
     *
     *
     * \note not available in Python bindings
     * \since QGIS 3.0
     */
    virtual bool renderInPreview( const QgsDataProvider::PreviewContext &context ); // SIP_SKIP

    /**
     * Returns layer metadata collected from the provider's source.
     *
     * Individual data providers must implement this method if they support collecting metadata.
     *
     * \see writeLayerMetadata()
     * \since QGIS 3.0
    */
    virtual QgsLayerMetadata layerMetadata() const { return QgsLayerMetadata(); }

    /**
     * Writes layer \a metadata to the underlying provider source. Support depends
     * on individual provider capabilities.
     *
     * Returns TRUE if metadata was successfully written to the data provider.
     * \see layerMetadata()
     * \since QGIS 3.0
    */
    virtual bool writeLayerMetadata( const QgsLayerMetadata &metadata ) { Q_UNUSED( metadata ) return false; }

    /**
     * Returns data provider coordinate transform context
     *
     * \see setTransformContext()
     * \note not available in Python bindings
     * \since QGIS 3.8
     */
    QgsCoordinateTransformContext transformContext() const SIP_SKIP;

    /**
     * Sets data coordinate transform context to \a transformContext
     *
     * The default implementation is a simple setter, subclasses may override to perform
     * additional actions required by a change of coordinate transform context.
     *
     * \see transformContext()
     * \note not available in Python bindings
     * \since QGIS 3.8
     */
    virtual void setTransformContext( const QgsCoordinateTransformContext &transformContext ) SIP_SKIP;

    /**
     * String sequence used for separating components of sublayers strings.
     * \note Replaces the static const SUBLAYER_SEPARATOR
     * \see subLayers()
     * \since QGIS 3.12
     */
    static QString sublayerSeparator();

  signals:

    /**
     * Emitted whenever a deferred extent calculation is completed by the provider.
     *
     * Layers should connect to this signal and update their cached extents whenever
     * it is emitted.
     */
    void fullExtentCalculated();

    /**
     * Emitted whenever a change is made to the data provider which may have
     * caused changes in the provider's data OUTSIDE of QGIS.
     *
     * When emitted from a QgsVectorDataProvider, any cached information such as
     * feature ids should be invalidated.
     *
     * \warning This signal is NOT emitted when changes are made to a provider
     * from INSIDE QGIS -- e.g. when adding features to a vector layer, deleting features
     * or modifying existing features. Instead, the specific QgsVectorLayer signals
     * should be used to detect these operations.
     */
    void dataChanged();

    /**
     * Emitted when the datasource issues a notification.
     *
     * \see setListening
     *
     * \since QGIS 3.0
     */
    void notify( const QString &msg );


  protected:

    /**
     * Timestamp of data in the moment when the data were loaded by provider.
     */
    QDateTime mTimestamp;

    //! \brief Error
    QgsError mError;

    //! Add error message
    void appendError( const QgsErrorMessage &message ) { mError.append( message ); }

    //! Sets error message
    void setError( const QgsError &error ) { mError = error;}

    //! Read flags. It's up to the subclass to respect these when needed
    QgsDataProvider::ReadFlags mReadFlags = QgsDataProvider::ReadFlags();

  private:

    /**
     * Universal Resource Identifier for source data.
     * This could be a file, database, or server address.
     */
    QString mDataSourceURI;

    QgsDataProvider::ProviderOptions mOptions;

    QMap< int, QVariant > mProviderProperties;

    /**
     * Protects options from being accessed concurrently
     */
    mutable QMutex mOptionsMutex;

    /**
     * Reloads the data according to the provider
     * \since QGIS 3.12
    */
    virtual void reloadProviderData() {}
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsDataProvider::ReadFlags )

#endif
