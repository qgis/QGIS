/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsdataprovider.h                                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/








class QgsDataProvider : QObject
{
%Docstring
 Abstract base class for spatial data provider implementations.

 This object needs to inherit from QObject to enable event
 processing in the Postgres/PostGIS provider (QgsPostgresProvider).
 It is called *here* so that this vtable and the vtable for
 QgsPostgresProvider don't get misaligned -
 the QgsVectorLayer class factory (which refers
 to generic QgsVectorDataProvider's) depends on it.
%End

%TypeHeaderCode
#include "qgsdataprovider.h"
%End
%ConvertToSubClassCode
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
%End
  public:

    enum DataCapability
    {
      NoDataCapabilities,
      File,
      Dir,
      Database,
      Net
    };

    enum ProviderProperty
    {
      EvaluateDefaultValues,
      CustomData
    };

    QgsDataProvider( const QString &uri = QString() );
%Docstring
 Create a new dataprovider with the specified in the ``uri``.
%End

    virtual QgsCoordinateReferenceSystem crs() const = 0;
%Docstring
 Returns the coordinate system for the data source.
 If the provider isn't capable of returning its projection then an invalid
 QgsCoordinateReferenceSystem will be returned.
 :rtype: QgsCoordinateReferenceSystem
%End


    virtual void setDataSourceUri( const QString &uri );
%Docstring
 Set the data source specification. This may be a path or database
 connection string
 \param uri source specification
%End

    virtual QString dataSourceUri( bool expandAuthConfig = false ) const;
%Docstring
 Get the data source specification. This may be a path or database
 connection string
 \param expandAuthConfig Whether to expand any assigned authentication configuration
 :return: data source specification
.. note::

   The default authentication configuration expansion is FALSE. This keeps credentials
 out of layer data source URIs and project files. Expansion should be specifically done
 only when needed within a provider
 :rtype: str
%End

    void setUri( const QgsDataSourceUri &uri );
%Docstring
 Set the data source specification.

.. versionadded:: 3.0
%End

    QgsDataSourceUri uri() const;
%Docstring
 Get the data source specification.

.. versionadded:: 3.0
 :rtype: QgsDataSourceUri
%End

    virtual QgsRectangle extent() const = 0;
%Docstring
 Returns the extent of the layer
 :return: QgsRectangle containing the extent of the layer
 :rtype: QgsRectangle
%End


    virtual bool isValid() const = 0;
%Docstring
 Returns true if this is a valid layer. It is up to individual providers
 to determine what constitutes a valid layer.
 :rtype: bool
%End


    virtual void updateExtents();
%Docstring
 Update the extents of the layer. Not implemented by default.
%End


    virtual bool setSubsetString( const QString &subset, bool updateFeatureCount = true );
%Docstring
 Set the subset string used to create a subset of features in
 the layer. This may be a sql where clause or any other string
 that can be used by the data provider to create a subset.
 Must be implemented in the dataprovider.
 :rtype: bool
%End


    virtual bool supportsSubsetString() const;
%Docstring
 Returns true if the provider supports setting of subset strings.
 :rtype: bool
%End

    virtual QString subsetString() const;
%Docstring
 Returns the subset definition string (typically sql) currently in
 use by the layer and used by the provider to limit the feature set.
 Must be overridden in the dataprovider, otherwise returns a null
 QString.
 :rtype: str
%End


    virtual QStringList subLayers() const;
%Docstring
 Sub-layers handled by this provider, in order from bottom to top

 Sub-layers are used when the provider's source can combine layers
 it knows about in some way before it hands them off to the provider.
 :rtype: list of str
%End


    virtual QStringList subLayerStyles() const;
%Docstring
 Sub-layer styles for each sub-layer handled by this provider,
 in order from bottom to top

 Sub-layer styles are used to abstract the way the provider's source can symbolise
 layers in some way at the server, before it serves them to the provider.
 :rtype: list of str
%End


    virtual uint subLayerCount() const;
%Docstring
 return the number of layers for the current data source
 :rtype: uint
%End


    virtual void setLayerOrder( const QStringList &layers );
%Docstring
 Reorder the list of layer names to be rendered by this provider
 (in order from bottom to top)
.. note::

     layers must have been previously added.
%End


    virtual void setSubLayerVisibility( const QString &name, bool vis );
%Docstring
 Set the visibility of the given sublayer name
%End


    virtual QString name() const = 0;
%Docstring
 Return a provider name

 Essentially just returns the provider key.  Should be used to build file
 dialogs so that providers can be shown with their supported types. Thus
 if more than one provider supports a given format, the user is able to
 select a specific provider to open that file.

 \note

 Instead of being pure virtual, might be better to generalize this
 behavior and presume that none of the sub-classes are going to do
 anything strange with regards to their name or description?
 :rtype: str
%End


    virtual QString description() const = 0;
%Docstring
 Return description

 Return a terse string describing what the provider is.

 \note

 Instead of being pure virtual, might be better to generalize this
 behavior and presume that none of the sub-classes are going to do
 anything strange with regards to their name or description?
 :rtype: str
%End


    virtual QString fileVectorFilters() const;
%Docstring
 Return vector file filter string

 Returns a string suitable for a QFileDialog of vector file formats
 supported by the data provider.  Naturally this will be an empty string
 for those data providers that do not deal with plain files, such as
 databases and servers.

.. note::

   It'd be nice to eventually be raster/vector neutral.
 :rtype: str
%End


    virtual QString fileRasterFilters() const;
%Docstring
 Return raster file filter string

 Returns a string suitable for a QFileDialog of raster file formats
 supported by the data provider.  Naturally this will be an empty string
 for those data providers that do not deal with plain files, such as
 databases and servers.

.. note::

   It'd be nice to eventually be raster/vector neutral.
 :rtype: str
%End

    virtual void reloadData();
%Docstring
 Reloads the data from the source. Needs to be implemented by providers with data caches to
 synchronize with changes in the data source
%End

    virtual QDateTime timestamp() const;
%Docstring
Time stamp of data source in the moment when data/metadata were loaded by provider
 :rtype: QDateTime
%End

    virtual QDateTime dataTimestamp() const;
%Docstring
Current time stamp of data source
 :rtype: QDateTime
%End

    virtual QgsError error() const;
%Docstring
 Get current status error. This error describes some principal problem
  for which provider cannot work and thus is not valid. It is not last error
  after accessing data by block(), identify() etc.
 :rtype: QgsError
%End

    virtual void invalidateConnections( const QString &connection );
%Docstring
 Invalidate connections corresponding to specified name
.. versionadded:: 2.16
%End

    virtual bool enterUpdateMode();
%Docstring
 Enter update mode.

 This is aimed at providers that can open differently the connection to
 the datasource, according it to be in update mode or in read-only mode.
 A call to this method shall be balanced with a call to leaveUpdateMode(),
 if this method returns true.

 Most providers will have an empty implementation for that method.

 For backward compatibility, providers that implement enterUpdateMode() should
 still make sure to allow editing operations to work even if enterUpdateMode()
 is not explicitly called.

 Several successive calls to enterUpdateMode() can be done. So there is
 a concept of stack of calls that must be handled by the provider. Only the first
 call to enterUpdateMode() will really turn update mode on.

 :return: true in case of success (or no-op implementation), false in case of failure.

.. versionadded:: 2.16
 :rtype: bool
%End

    virtual bool leaveUpdateMode();
%Docstring
 Leave update mode.

 This is aimed at providers that can open differently the connection to
 the datasource, according it to be in update mode or in read-only mode.
 This method shall be balanced with a successful call to enterUpdateMode().

 Most providers will have an empty implementation for that method.

 Several successive calls to enterUpdateMode() can be done. So there is
 a concept of stack of calls that must be handled by the provider. Only the last
 call to leaveUpdateMode() will really turn update mode off.

 :return: true in case of success (or no-op implementation), false in case of failure.

.. versionadded:: 2.16
 :rtype: bool
%End

    void setProviderProperty( ProviderProperty property, const QVariant &value );
%Docstring
 Allows setting arbitrary properties on the provider.
 It depends on the provider which properties are supported.

.. versionadded:: 2.16
%End

    QVariant providerProperty( ProviderProperty property, const QVariant &defaultValue = QVariant() ) const;
%Docstring
 Get the current value of a certain provider property.
 It depends on the provider which properties are supported.

.. versionadded:: 2.16
 :rtype: QVariant
%End

    virtual void setListening( bool isListening );
%Docstring
 Set whether the provider will listen to datasource notifications
 If set, the provider will issue notify signals.

 The default implementation does nothing.

.. seealso:: notify

.. versionadded:: 3.0
%End

  signals:

    void fullExtentCalculated();
%Docstring
   This is emitted whenever the worker thread has fully calculated the
   PostGIS extents for this layer, and its event has been received by this
   provider.
%End

    void dataChanged();
%Docstring
   This is emitted whenever an asynchronous operation has finished
   and the data should be redrawn

   When emitted from a QgsVectorDataProvider, any cached information such as
   feature ids should be invalidated.
%End

    void notify( const QString &msg );
%Docstring
 Emitted when datasource issues a notification

.. seealso:: setListening

.. versionadded:: 3.0
%End


  protected:



    void appendError( const QgsErrorMessage &message );
%Docstring
Add error message
%End

    void setError( const QgsError &error );
%Docstring
Set error message
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsdataprovider.h                                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
