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

//#include "qgsdataitem.h"
#include "qgsdatasourceuri.h"
#include "qgserror.h"

typedef int dataCapabilities_t(); // SIP_SKIP

class QgsRectangle;
class QgsCoordinateReferenceSystem;


/**
 * \ingroup core
 * Abstract base class for spatial data provider implementations.
 *
 * This object needs to inherit from QObject to enable event
 * processing in the Postgres/PostGIS provider (QgsPostgresProvider).
 * It is called *here* so that this vtable and the vtable for
 * QgsPostgresProvider don't get misaligned -
 * the QgsVectorLayer class factory (which refers
 * to generic QgsVectorDataProvider's) depends on it.
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
    else
    {
      sipType = 0;
    }
    SIP_END
#endif
    Q_OBJECT

  public:

    enum DataCapability
    {
      NoDataCapabilities  = 0,
      File                = 1,
      Dir                 = 1 << 1,
      Database            = 1 << 2,
      Net                 = 1 << 3  // Internet source
    };
    Q_ENUM( DataCapability );

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
     * Create a new dataprovider with the specified in the \a uri.
     */
    QgsDataProvider( const QString &uri = QString() )
      : mDataSourceURI( uri )
    {}

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
     * Get the data source specification. This may be a path or database
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
        QgsDataSourceUri uri( mDataSourceURI );
        return uri.uri( expandAuthConfig );
      }
      else
      {
        return mDataSourceURI;
      }
    }

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
     * Get the data source specification.
     *
     * \since QGIS 3.0
     */
    QgsDataSourceUri uri() const
    {
      return QgsDataSourceUri( mDataSourceURI );
    }

    /**
     * Returns the extent of the layer
     * \returns QgsRectangle containing the extent of the layer
     */
    virtual QgsRectangle extent() const = 0;


    /**
     * Returns true if this is a valid layer. It is up to individual providers
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
      Q_UNUSED( subset );
      Q_UNUSED( updateFeatureCount );
      return false;
    }


    /**
     * Returns true if the provider supports setting of subset strings.
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
     * return the number of layers for the current data source
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
     * Return a provider name
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
     * Return description
     *
     * Return a terse string describing what the provider is.
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
     * Return vector file filter string
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
      return QLatin1String( "" );
    }


    /**
     * Return raster file filter string
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
      return QLatin1String( "" );
    }

    /**
     * Reloads the data from the source. Needs to be implemented by providers with data caches to
     * synchronize with changes in the data source
     */
    virtual void reloadData() {}

    //! Time stamp of data source in the moment when data/metadata were loaded by provider
    virtual QDateTime timestamp() const { return mTimestamp; }

    //! Current time stamp of data source
    virtual QDateTime dataTimestamp() const { return QDateTime(); }

    /**
     * Get current status error. This error describes some principal problem
     *  for which provider cannot work and thus is not valid. It is not last error
     *  after accessing data by block(), identify() etc.
     */
    virtual QgsError error() const { return mError; }

    /**
     * Invalidate connections corresponding to specified name
     * \since QGIS 2.16
     */
    virtual void invalidateConnections( const QString &connection ) { Q_UNUSED( connection ); }

    /**
     * Enter update mode.
     *
     * This is aimed at providers that can open differently the connection to
     * the datasource, according it to be in update mode or in read-only mode.
     * A call to this method shall be balanced with a call to leaveUpdateMode(),
     * if this method returns true.
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
     * \returns true in case of success (or no-op implementation), false in case of failure.
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
     * \returns true in case of success (or no-op implementation), false in case of failure.
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
     * Get the current value of a certain provider property.
     * It depends on the provider which properties are supported.
     *
     * \since QGIS 2.16
     */
    QVariant providerProperty( ProviderProperty property, const QVariant &defaultValue = QVariant() ) const;

    /**
     * Get the current value of a certain provider property.
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

  signals:

    /**
     *   This is emitted whenever the worker thread has fully calculated the
     *   PostGIS extents for this layer, and its event has been received by this
     *   provider.
     */
    void fullExtentCalculated();

    /**
     *   This is emitted whenever an asynchronous operation has finished
     *   and the data should be redrawn
     *
     *   When emitted from a QgsVectorDataProvider, any cached information such as
     *   feature ids should be invalidated.
     */
    void dataChanged();

    /**
     * Emitted when datasource issues a notification
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

    //! Set error message
    void setError( const QgsError &error ) { mError = error;}

  private:

    /**
     * Universal Resource Identifier for source data.
     * This could be a file, database, or server address.
     */
    QString mDataSourceURI;

    QMap< int, QVariant > mProviderProperties;
};


#endif
