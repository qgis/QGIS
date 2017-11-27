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
 :rtype: bool
%End

        QString reference() const;
%Docstring
The reference (id) of the live layer
 :rtype: str
%End

        QString name() const;
%Docstring
Name of the layer
 :rtype: str
%End

        QString provider() const;
%Docstring
Provider key
 :rtype: str
%End

        QString source() const;
%Docstring
The source url used by the provider to build the layer
 :rtype: str
%End

        QString encoding() const;
%Docstring
Optional encoding for the provider
 :rtype: str
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
 :rtype: QgsVirtualLayerDefinition
%End

    QUrl toUrl() const;
%Docstring
Convert the definition into a QUrl
 :rtype: QUrl
%End

    QString toString() const;
%Docstring
Convert into a QString that can be read by the virtual layer provider
 :rtype: str
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
Get access to the source layers
 :rtype: QgsVirtualLayerDefinition.SourceLayers
%End

    QString query() const;
%Docstring
Get the SQL query
 :rtype: str
%End
    void setQuery( const QString &query );
%Docstring
Set the SQL query
%End

    QString filePath() const;
%Docstring
Get the file path. May be empty
 :rtype: str
%End
    void setFilePath( const QString &filePath );
%Docstring
Set the file path
%End

    QString uid() const;
%Docstring
Get the name of the field with unique identifiers
 :rtype: str
%End
    void setUid( const QString &uid );
%Docstring
Set the name of the field with unique identifiers
%End

    QString geometryField() const;
%Docstring
Get the name of the geometry field. Empty if no geometry field
 :rtype: str
%End
    void setGeometryField( const QString &geometryField );
%Docstring
Set the name of the geometry field
%End

    QgsWkbTypes::Type geometryWkbType() const;
%Docstring
 Get the type of the geometry
 QgsWkbTypes.NoGeometry to hide any geometry
 QgsWkbTypes.Unknown for unknown types
 :rtype: QgsWkbTypes.Type
%End
    void setGeometryWkbType( QgsWkbTypes::Type t );
%Docstring
Set the type of the geometry
%End

    long geometrySrid() const;
%Docstring
Get the SRID of the geometry
 :rtype: long
%End
    void setGeometrySrid( long srid );
%Docstring
Set the SRID of the geometry
%End

    QgsFields fields() const;
%Docstring
Get field definitions
 :rtype: QgsFields
%End
    void setFields( const QgsFields &fields );
%Docstring
Set field definitions
%End

    bool hasSourceLayer( const QString &name ) const;
%Docstring
Convenience method to test if a given source layer is part of the definition
 :rtype: bool
%End

    bool hasReferencedLayers() const;
%Docstring
Convenience method to test whether the definition has referenced (live) layers
 :rtype: bool
%End

    bool hasDefinedGeometry() const;
%Docstring
Convenient method to test if the geometry is defined (not NoGeometry and not Unknown)
 :rtype: bool
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsvirtuallayerdefinition.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
