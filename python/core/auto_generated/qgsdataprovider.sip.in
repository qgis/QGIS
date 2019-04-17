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
    else if ( qobject_cast<QgsMeshDataProvider *>( sipCpp ) )
    {
      sipType = sipType_QgsMeshDataProvider;
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


    struct ProviderOptions
    {
      QgsCoordinateTransformContext transformContext;
    };

    QgsDataProvider( const QString &uri = QString(), const QgsDataProvider::ProviderOptions &providerOptions = QgsDataProvider::ProviderOptions() );
%Docstring
Create a new dataprovider with the specified in the ``uri``.

Additional creation options are specified within the ``options`` value.
%End

    virtual QgsCoordinateReferenceSystem crs() const = 0;
%Docstring
Returns the coordinate system for the data source.
If the provider isn't capable of returning its projection then an invalid
QgsCoordinateReferenceSystem will be returned.
%End

    virtual void setDataSourceUri( const QString &uri );
%Docstring
Set the data source specification. This may be a path or database
connection string

:param uri: source specification
%End

    virtual QString dataSourceUri( bool expandAuthConfig = false ) const;
%Docstring
Gets the data source specification. This may be a path or database
connection string

:param expandAuthConfig: Whether to expand any assigned authentication configuration

:return: data source specification

.. note::

   The default authentication configuration expansion is ``False``. This keeps credentials
   out of layer data source URIs and project files. Expansion should be specifically done
   only when needed within a provider
%End

    void setUri( const QgsDataSourceUri &uri );
%Docstring
Set the data source specification.

.. versionadded:: 3.0
%End

    QgsDataSourceUri uri() const;
%Docstring
Gets the data source specification.

.. versionadded:: 3.0
%End

    virtual QgsRectangle extent() const = 0;
%Docstring
Returns the extent of the layer

:return: QgsRectangle containing the extent of the layer
%End


    virtual bool isValid() const = 0;
%Docstring
Returns ``True`` if this is a valid layer. It is up to individual providers
to determine what constitutes a valid layer.
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
%End


    virtual bool supportsSubsetString() const;
%Docstring
Returns ``True`` if the provider supports setting of subset strings.
%End

    virtual QString subsetString() const;
%Docstring
Returns the subset definition string (typically sql) currently in
use by the layer and used by the provider to limit the feature set.
Must be overridden in the dataprovider, otherwise returns a null
QString.
%End


    virtual QStringList subLayers() const;
%Docstring
Sub-layers handled by this provider, in order from bottom to top

Sub-layers are used when the provider's source can combine layers
it knows about in some way before it hands them off to the provider.

.. seealso:: SUBLAYER_SEPARATOR
%End

    static QString SUBLAYER_SEPARATOR;

    virtual QStringList subLayerStyles() const;
%Docstring
Sub-layer styles for each sub-layer handled by this provider,
in order from bottom to top

Sub-layer styles are used to abstract the way the provider's source can symbolise
layers in some way at the server, before it serves them to the provider.
%End


    virtual uint subLayerCount() const;
%Docstring
Returns the number of layers for the current data source
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
Returns a provider name

Essentially just returns the provider key.  Should be used to build file
dialogs so that providers can be shown with their supported types. Thus
if more than one provider supports a given format, the user is able to
select a specific provider to open that file.

\note

Instead of being pure virtual, might be better to generalize this
behavior and presume that none of the sub-classes are going to do
anything strange with regards to their name or description?
%End


    virtual QString description() const = 0;
%Docstring
Returns description

Returns a terse string describing what the provider is.

\note

Instead of being pure virtual, might be better to generalize this
behavior and presume that none of the sub-classes are going to do
anything strange with regards to their name or description?
%End


    virtual QString fileVectorFilters() const;
%Docstring
Returns vector file filter string

Returns a string suitable for a QFileDialog of vector file formats
supported by the data provider.  Naturally this will be an empty string
for those data providers that do not deal with plain files, such as
databases and servers.

.. note::

   It'd be nice to eventually be raster/vector neutral.
%End


    virtual QString fileRasterFilters() const;
%Docstring
Returns raster file filter string

Returns a string suitable for a QFileDialog of raster file formats
supported by the data provider.  Naturally this will be an empty string
for those data providers that do not deal with plain files, such as
databases and servers.

.. note::

   It'd be nice to eventually be raster/vector neutral.
%End

    virtual void reloadData();
%Docstring
Reloads the data from the source. Needs to be implemented by providers with data caches to
synchronize with changes in the data source
%End

    virtual QDateTime timestamp() const;
%Docstring
Time stamp of data source in the moment when data/metadata were loaded by provider
%End

    virtual QDateTime dataTimestamp() const;
%Docstring
Current time stamp of data source
%End

    virtual QgsError error() const;
%Docstring
Gets current status error. This error describes some principal problem
for which provider cannot work and thus is not valid. It is not last error
after accessing data by block(), identify() etc.
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
if this method returns ``True``.

Most providers will have an empty implementation for that method.

For backward compatibility, providers that implement enterUpdateMode() should
still make sure to allow editing operations to work even if enterUpdateMode()
is not explicitly called.

Several successive calls to enterUpdateMode() can be done. So there is
a concept of stack of calls that must be handled by the provider. Only the first
call to enterUpdateMode() will really turn update mode on.

:return: ``True`` in case of success (or no-op implementation), ``False`` in case of failure.

.. versionadded:: 2.16
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

:return: ``True`` in case of success (or no-op implementation), ``False`` in case of failure.

.. versionadded:: 2.16
%End

    void setProviderProperty( ProviderProperty property, const QVariant &value );
%Docstring
Allows setting arbitrary properties on the provider.
It depends on the provider which properties are supported.

.. versionadded:: 2.16
%End

    QVariant providerProperty( ProviderProperty property, const QVariant &defaultValue = QVariant() ) const;
%Docstring
Gets the current value of a certain provider property.
It depends on the provider which properties are supported.

.. versionadded:: 2.16
%End

    virtual void setListening( bool isListening );
%Docstring
Set whether the provider will listen to datasource notifications
If set, the provider will issue notify signals.

The default implementation does nothing.

.. seealso:: :py:func:`notify`

.. versionadded:: 3.0
%End


    virtual QgsLayerMetadata layerMetadata() const;
%Docstring
Returns layer metadata collected from the provider's source.

Individual data providers must implement this method if they support collecting metadata.

.. seealso:: :py:func:`writeLayerMetadata`

.. versionadded:: 3.0
%End

    virtual bool writeLayerMetadata( const QgsLayerMetadata &metadata );
%Docstring
Writes layer ``metadata`` to the underlying provider source. Support depends
on individual provider capabilities.

Returns ``True`` if metadata was successfully written to the data provider.

.. seealso:: :py:func:`layerMetadata`

.. versionadded:: 3.0
%End



  signals:

    void fullExtentCalculated();
%Docstring
Emitted whenever a deferred extent calculation is completed by the provider.

Layers should connect to this signal and update their cached extents whenever
it is emitted.
%End

    void dataChanged();
%Docstring
Emitted whenever a change is made to the data provider which may have
caused changes in the provider's data OUTSIDE of QGIS.

When emitted from a :py:class:`QgsVectorDataProvider`, any cached information such as
feature ids should be invalidated.

.. warning::

   This signal is NOT emitted when changes are made to a provider
   from INSIDE QGIS -- e.g. when adding features to a vector layer, deleting features
   or modifying existing features. Instead, the specific QgsVectorLayer signals
   should be used to detect these operations.
%End

    void notify( const QString &msg );
%Docstring
Emitted when the datasource issues a notification.

.. seealso:: :py:func:`setListening`

.. versionadded:: 3.0
%End


  protected:



    void appendError( const QgsErrorMessage &message );
%Docstring
Add error message
%End

    void setError( const QgsError &error );
%Docstring
Sets error message
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsdataprovider.h                                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
