/***************************************************************************
                    qgsprovidermetadata.h  -  Metadata class for
                    describing a data provider.
                             -------------------
    begin                : Sat Jan 10 2004
    copyright            : (C) 2004 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROVIDERMETADATA_H
#define QGSPROVIDERMETADATA_H


#include <QString>
#include <QVariantMap>
#include <QMap>
#include <QList>
#include <memory>
#include <QPair>

#include "qgis_sip.h"
#include "qgsdataprovider.h"
#include "qgis_core.h"
#include <functional>
#include "qgsvectorlayerexporter.h"
#include "qgsabstractproviderconnection.h"
#include "qgsabstractdatabaseproviderconnection.h"
#include "qgsfields.h"
#include "qgsexception.h"

class QgsDataItem;
class QgsDataItemProvider;
class QgsTransaction;

class QgsRasterDataProvider;


/**
 * \ingroup core
 * Holds data provider key, description, and associated shared library file or function pointer information.
 *
 * Provider metadata refers either to providers which are loaded via libraries or
 * which are native providers that are included in the core QGIS installation
 * and accessed through function pointers.
 *
 * For library based providers, the metadata class is used in a lazy load
 * implementation in QgsProviderRegistry.  To save memory, data providers
 * are only actually loaded via QLibrary calls if they're to be used.  (Though they're all
 * iteratively loaded once to get their metadata information, and then
 * unloaded when the QgsProviderRegistry is created.)  QgsProviderMetadata
 * supplies enough information to be able to later load the associated shared
 * library object.
 *
 */
class CORE_EXPORT QgsProviderMetadata
{
  public:

    /**
     * Typedef for data provider creation function.
     * \since QGIS 3.0
     */
    SIP_SKIP typedef std::function < QgsDataProvider*( const QString &, const QgsDataProvider::ProviderOptions & ) > CreateDataProviderFunction;

    /**
     * Constructor for provider metadata
     * \param key provider key
     * \param description provider description
     * \param library plugin library file name (empty if the provider is not loaded from a library)
     */
    QgsProviderMetadata( const QString &key, const QString &description, const QString &library = QString() );

    /**
     * Metadata for provider with direct provider creation function pointer, where
     * no library is involved.
     * \since QGIS 3.0
     * \deprecated QGIS 3.10
     */
    SIP_SKIP Q_DECL_DEPRECATED QgsProviderMetadata( const QString &key, const QString &description, const QgsProviderMetadata::CreateDataProviderFunction &createFunc );

    //! dtor
    virtual ~QgsProviderMetadata();

    /**
     * This returns the unique key associated with the provider
     *
     * This key string is used for the associative container in QgsProviderRegistry
     */
    QString key() const;

    /**
     * This returns descriptive text for the provider
     *
     * This is used to provide a descriptive list of available data providers.
     */
    QString description() const;

    /**
     * This returns the library file name
     *
     * This is used to QLibrary calls to load the data provider (only for dynamically loaded libraries)
     *
     * \deprecated QGIS 3.10 - providers may not need to be loaded from a library (empty string returned)
     */
    Q_DECL_DEPRECATED QString library() const SIP_DEPRECATED;

    /**
     * Returns a pointer to the direct provider creation function, if supported
     * by the provider.
     * \note not available in Python bindings
     * \since QGIS 3.0
     * \deprecated QGIS 3.10
     */
    SIP_SKIP Q_DECL_DEPRECATED CreateDataProviderFunction createFunction() const;

    /**
      * Initialize the provider
      * \since QGIS 3.10
      */
    virtual void initProvider();

    /**
     * Cleanup the provider
     * \since QGIS 3.10
     */
    virtual void cleanupProvider();

    /**
     * Type of file filters
     * \since QGIS 3.10
     */
    enum class FilterType
    {
      FilterVector = 1,
      FilterRaster,
      FilterMesh,
      FilterMeshDataset
    };

    /**
     * Builds the list of file filter strings (supported formats)
     *
     * Suitable for use in a QFileDialog::getOpenFileNames() call.
     *
     * \since QGIS 3.10
     */
    virtual QString filters( FilterType type );

    /**
     * Class factory to return a pointer to a newly created QgsDataProvider object
     * \since QGIS 3.10
     */
    virtual QgsDataProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options ) SIP_FACTORY;

#ifndef SIP_RUN

    /**
     * Creates new empty vector layer
     * \note not available in Python bindings
     * \since QGIS 3.10
     */
    virtual QgsVectorLayerExporter::ExportError createEmptyLayer( const QString &uri,
        const QgsFields &fields,
        QgsWkbTypes::Type wkbType,
        const QgsCoordinateReferenceSystem &srs,
        bool overwrite,
        QMap<int, int> &oldToNewAttrIdxMap,
        QString &errorMessage,
        const QMap<QString, QVariant> *options );
#endif

    /**
     * Creates a new instance of the raster data provider.
     * \since QGIS 3.10
     */
    virtual QgsRasterDataProvider *createRasterDataProvider(
      const QString &uri,
      const QString &format,
      int nBands,
      Qgis::DataType type,
      int width,
      int height,
      double *geoTransform,
      const QgsCoordinateReferenceSystem &crs,
      const QStringList &createOptions = QStringList() ) SIP_FACTORY;

    /**
     * Returns pyramid resampling methods available for provider
     * \since QGIS 3.10
     */
    virtual QList<QPair<QString, QString> > pyramidResamplingMethods();

    /**
     * Breaks a provider data source URI into its component paths (e.g. file path, layer name).
     * \param uri uri string
     * \returns map containing components. Standard components include "path", "layerName", "url".
     * \note this function may not be supported by all providers, an empty map will be returned in such case
     * \since QGIS 3.10
     */
    virtual QVariantMap decodeUri( const QString &uri );

    /**
     * Returns data item providers. Caller is responsible for ownership of the item providers
     * \see QgsProviderGuiMetadata::dataItemGuiProviders()
     * \note Ownership of created data item providers is passed to the caller.
     * \since QGIS 3.10
     */
    virtual QList< QgsDataItemProvider * > dataItemProviders() const SIP_FACTORY;

    /**
     * Lists stored layer styles in the provider defined by \a uri
     * \returns -1 if not implemented by provider, otherwise number of styles stored
     * \since QGIS 3.10
     */
    virtual int listStyles( const QString &uri, QStringList &ids, QStringList &names,
                            QStringList &descriptions, QString &errCause );

    /**
     * Gets a layer style defined by \a uri
     * \since QGIS 3.10
     */
    virtual QString getStyleById( const QString &uri, QString styleId, QString &errCause );

    /**
     * Deletes a layer style defined by \a styleId
     * \since QGIS 3.10
     */
    virtual bool deleteStyleById( const QString &uri, QString styleId, QString &errCause );

    /**
     * Saves a layer style to provider
     * \since QGIS 3.10
     */
    virtual bool saveStyle( const QString &uri, const QString &qmlStyle, const QString &sldStyle,
                            const QString &styleName, const QString &styleDescription,
                            const QString &uiFileContent, bool useAsDefault, QString &errCause );

    /**
     * Loads a layer style defined by \a uri
     * \since QGIS 3.10
     */
    virtual QString loadStyle( const QString &uri, QString &errCause );

    /**
     * Creates database by the provider on the path
     * \since QGIS 3.10
     */
    virtual bool createDb( const QString &dbPath, QString &errCause );

    /**
     * Returns new instance of transaction. Ownership is transferred to the caller
     * \since QGIS 3.10
     */
    virtual QgsTransaction *createTransaction( const QString &connString ) SIP_FACTORY;

    /**
     * Returns a dictionary of stored provider connections,
     * the dictionary key is the connection identifier.
     * Ownership is not transferred.
     * Raises a QgsProviderConnectionException if any errors are encountered.
     * \param cached if FALSE connections will be re-read from the settings
     * \throws QgsProviderConnectionException
     * \since QGIS 3.10
     */
    virtual QMap<QString, QgsAbstractProviderConnection *> connections( bool cached = true ) SIP_THROW( QgsProviderConnectionException );

    /**
     * Returns a dictionary of database provider connections,
     * the dictionary key is the connection identifier.
     * Ownership is not transferred.
     * Raises a QgsProviderConnectionException if any errors are encountered.
     * \param cached if FALSE connections will be re-read from the settings
     * \throws QgsProviderConnectionException
     * \since QGIS 3.10
     */
    QMap<QString, QgsAbstractDatabaseProviderConnection *> dbConnections( bool cached = true ) SIP_THROW( QgsProviderConnectionException );

    /**
     * Searches and returns a (possibly NULL) connection from the stored provider connections.
     * Ownership is not transferred.
     * Raises a QgsProviderConnectionException if any errors are encountered.
     * \param name the connection name
     * \param cached if FALSE connections will be re-read from the settings
     * \throws QgsProviderConnectionException
     * \since QGIS 3.10
     */
    QgsAbstractProviderConnection *findConnection( const QString &name, bool cached = true ) SIP_THROW( QgsProviderConnectionException );

#ifndef SIP_RUN

    /**
     * Returns a dictionary of provider connections of the specified type T,
     * the dictionary key is the connection identifier.
     * \param cached if FALSE connections will be re-read from the settings
     * \note not available in Python bindings
     * \since QGIS 3.10
     */
    template <typename T> QMap<QString, T *>connections( bool cached = true );


#endif

    /**
     * Creates a new connection from \a uri and \a configuration,
     * the newly created connection is not automatically stored in the settings, call
     * saveConnection() to save it.
     * Ownership is transferred to the caller.
     * \see saveConnection()
     * \since QGIS 3.10
     */
    virtual QgsAbstractProviderConnection *createConnection( const QString &uri, const QVariantMap &configuration ) SIP_FACTORY;

    /**
     * Creates a new connection by loading the connection with the given \a name from the settings.
     * Ownership is transferred to the caller.
     * \see findConnection()
     */
    virtual QgsAbstractProviderConnection *createConnection( const QString &name );

    /**
     * Removes the connection with the given \a name from the settings.
     * Raises a QgsProviderConnectionException if any errors are encountered.
     * \throws QgsProviderConnectionException
     * \since QGIS 3.10
     */
    virtual void deleteConnection( const QString &name ) SIP_THROW( QgsProviderConnectionException );

    /**
     * Stores the connection in the settings
     * \param connection the connection to be stored in the settings
     * \param name the name under which the connection will be stored
     * \since QGIS 3.10
     */
    virtual void saveConnection( const QgsAbstractProviderConnection *connection, const QString &name );

  protected:

#ifndef SIP_RUN
///@cond PRIVATE

    // Common functionality for connections management, to be moved into the class
    // when all the providers are ready
    // T_provider_conn: subclass of QgsAbstractProviderConnection,
    // T_conn: provider connection class (such as QgsOgrDbConnection or QgsPostgresConn)
    // TODO QGIS4: remove all old provider conn classes and move functionality into QgsAbstractProviderConnection subclasses
    template <class T_provider_conn, class T_conn> QMap<QString, QgsAbstractProviderConnection *> connectionsProtected( bool cached = true )
    {
      if ( ! cached || mProviderConnections.isEmpty() )
      {
        qDeleteAll( mProviderConnections );
        mProviderConnections.clear();
        const auto connNames { T_conn::connectionList() };
        for ( const auto &cname : connNames )
        {
          mProviderConnections.insert( cname, new T_provider_conn( cname ) );
        }
      }
      return mProviderConnections;
    }

    template <class T_provider_conn> void deleteConnectionProtected( const QString &name )
    {
      T_provider_conn conn( name );
      conn.remove( name );
      mProviderConnections.clear();
    }
    virtual void saveConnectionProtected( const QgsAbstractProviderConnection *connection, const QString &name );
    //! Provider connections cache
    QMap<QString, QgsAbstractProviderConnection *> mProviderConnections;

/// @endcond
#endif

  private:

    /// unique key for data provider
    QString mKey;

    /// associated terse description
    QString mDescription;

    /// file path
    /// deprecated QGIS 3.10
    QString mLibrary;

    CreateDataProviderFunction mCreateFunction = nullptr;

};

#endif //QGSPROVIDERMETADATA_H
