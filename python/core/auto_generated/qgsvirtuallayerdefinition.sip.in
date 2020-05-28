/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsvirtuallayerdefinition.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsVirtualLayerDefinition
{
%Docstring
Class to manipulate the definition of a virtual layer

It is used to extract parameters from an initial virtual layer definition as well as
to store the complete, expanded definition once types have been detected.
%End

%TypeHeaderCode
#include "qgsvirtuallayerdefinition.h"
%End
  public:

    class SourceLayer
{
%Docstring
A SourceLayer is either a reference to a live layer in the registry
or all the parameters needed to load it (provider key, source, etc.)
%End

%TypeHeaderCode
#include "qgsvirtuallayerdefinition.h"
%End
      public:
        SourceLayer( const QString &name, const QString &ref );
%Docstring
Constructor variant to build a live layer reference
%End
        SourceLayer( const QString &name, const QString &source, const QString &provider, const QString &encoding );
%Docstring
Constructor variant to build a layer with a provider and a source
%End

        bool isReferenced() const;
%Docstring
Is it a live layer or not ?
%End

        QString reference() const;
%Docstring
The reference (id) of the live layer
%End

        QString name() const;
%Docstring
Name of the layer
%End

        QString provider() const;
%Docstring
Provider key
%End

        QString source() const;
%Docstring
The source url used by the provider to build the layer
%End

        QString encoding() const;
%Docstring
Optional encoding for the provider
%End

    };

    QgsVirtualLayerDefinition( const QString &filePath = "" );
%Docstring
Constructor with an optional file path
%End

    static QgsVirtualLayerDefinition fromUrl( const QUrl &url );
%Docstring
Constructor to build a definition from a QUrl
The path part of the URL is extracted as well as the following optional keys:
layer_ref=layer_id[:name]               represents a live layer referenced by its ID. An optional name can be given
layer=provider:source[:name[:encoding]] represents a layer given by its provider key, its source url (URL-encoded).
An optional name and encoding can be given
geometry=column_name[:type:srid]        gives the definition of the geometry column.
Type can be either a WKB type code or a string (point, linestring, etc.)
srid is an integer
uid=column_name                         is the name of a column with unique integer values.
nogeometry                              is a flag to force the layer to be a non-geometry layer
query=sql                               represents the SQL query. Must be URL-encoded
field=column_name:[int|real|text]       represents a field with its name and its type
%End

    QUrl toUrl() const;
%Docstring
Convert the definition into a QUrl
%End

    QString toString() const;
%Docstring
Convert into a QString that can be read by the virtual layer provider
%End

    void addSource( const QString &name, const QString &ref );
%Docstring
Add a live layer source layer
%End

    void addSource( const QString &name, const QString &source, const QString &provider, const QString &encoding = "" );
%Docstring
Add a layer with a source, a provider and an encoding
%End

    typedef QList<QgsVirtualLayerDefinition::SourceLayer> SourceLayers;

    const QgsVirtualLayerDefinition::SourceLayers &sourceLayers() const;
%Docstring
Gets access to the source layers
%End

    QString query() const;
%Docstring
Gets the SQL query
%End
    void setQuery( const QString &query );
%Docstring
Sets the SQL query
%End

    QString filePath() const;
%Docstring
Gets the file path. May be empty
%End
    void setFilePath( const QString &filePath );
%Docstring
Sets the file path
%End

    QString uid() const;
%Docstring
Gets the name of the field with unique identifiers
%End
    void setUid( const QString &uid );
%Docstring
Sets the name of the field with unique identifiers
%End

    void setLazy( bool lazy );
%Docstring
Sets the lazy mode. If ``lazy`` is ``True``, then the loading is
delayed until an explicit reloading of the layer.

:param lazy: ``True`` to delay the loading, ``False`` otherwise

.. seealso:: :py:func:`QgsDataProvider.reloadData`

.. seealso:: :py:func:`isLazy`

.. versionadded:: 3.2
%End

    bool isLazy() const;
%Docstring
Returns the lazy mode.

:return: ``True`` if the loading is delayed, ``False`` otherwise.

.. seealso:: :py:func:`setLazy`

.. versionadded:: 3.2
%End

    QString geometryField() const;
%Docstring
Gets the name of the geometry field. Empty if no geometry field
%End
    void setGeometryField( const QString &geometryField );
%Docstring
Sets the name of the geometry field
%End

    QgsWkbTypes::Type geometryWkbType() const;
%Docstring
Gets the type of the geometry
QgsWkbTypes.NoGeometry to hide any geometry
QgsWkbTypes.Unknown for unknown types
%End
    void setGeometryWkbType( QgsWkbTypes::Type t );
%Docstring
Sets the type of the geometry
%End

    long geometrySrid() const;
%Docstring
Gets the SRID of the geometry
%End
    void setGeometrySrid( long srid );
%Docstring
Sets the SRID of the geometry
%End

    QgsFields fields() const;
%Docstring
Gets field definitions
%End
    void setFields( const QgsFields &fields );
%Docstring
Sets field definitions
%End

    bool hasSourceLayer( const QString &name ) const;
%Docstring
Convenience method to test if a given source layer is part of the definition
%End

    bool hasReferencedLayers() const;
%Docstring
Convenience method to test whether the definition has referenced (live) layers
%End

    bool hasDefinedGeometry() const;
%Docstring
Convenient method to test if the geometry is defined (not NoGeometry and not Unknown)
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsvirtuallayerdefinition.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
