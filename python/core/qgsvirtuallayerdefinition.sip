/**
 * Class to manipulate the definition of a virtual layer
 *
 * It is used to extract parameters from an initial virtual layer definition as well as
 * to store the complete, expanded definition once types have been detected.
 */
class QgsVirtualLayerDefinition
{
%TypeHeaderCode
#include <qgsvirtuallayerdefinition.h>
%End
  public:
    /**
     * A SourceLayer is either a reference to a live layer in the registry
     * or all the parameters needed to load it (provider key, source, etc.)
     */
    class SourceLayer
    {
      public:
        //! Constructor variant to build a live layer reference
        SourceLayer( const QString& name, const QString& ref );
        //! Constructor variant to build a layer with a provider and a source
        SourceLayer( const QString& name, const QString& source, const QString& provider, const QString& encoding );

        //! Is it a live layer or not ?
        bool isReferenced() const;

        //! The reference (id) of the live layer
        QString reference() const;

        //! Name of the layer
        QString name() const;

        //! Provider key
        QString provider() const;

        //! The source url used by the provider to build the layer
        QString source() const;

        //! Optional encoding for the provider
        QString encoding() const;
    };

    //! Constructor with an optional file path
    QgsVirtualLayerDefinition( const QString& filePath = "" );

    //! Constructor to build a definition from a QUrl
    //! The path part of the URL is extracted as well as the following optional keys:
    //! layer_ref=layer_id[:name]               represents a live layer referenced by its ID. An optional name can be given
    //! layer=provider:source[:name[:encoding]] represents a layer given by its provider key, its source url (URL-encoded).
    //!                                         An optional name and encoding can be given
    //! geometry=column_name[:type:srid]        gives the definition of the geometry column.
    //!                                         Type can be either a WKB type code or a string (point, linestring, etc.)
    //!                                         srid is an integer
    //! uid=column_name                         is the name of a column with unique integer values.
    //! nogeometry                              is a flag to force the layer to be a non-geometry layer
    //! query=sql                               represents the SQL query. Must be URL-encoded
    //! field=column_name:[int|real|text]       represents a field with its name and its type
    static QgsVirtualLayerDefinition fromUrl( const QUrl& url );

    //! Convert the definition into a QUrl
    QUrl toUrl() const;

    //! Convert into a QString that can be read by the virtual layer provider
    QString toString() const;

    //! Add a live layer source layer
    void addSource( const QString& name, const QString& ref );

    //! Add a layer with a source, a provider and an encoding
    void addSource( const QString& name, const QString& source, const QString& provider, const QString& encoding = "" );

    //! List of source layers
    typedef QList<QgsVirtualLayerDefinition::SourceLayer> SourceLayers;

    //! Get access to the source layers
    const SourceLayers& sourceLayers() const;

    //! Get the SQL query
    QString query() const;
    //! Set the SQL query
    void setQuery( const QString& query );

    //! Get the file path. May be empty
    QString filePath() const;
    //! Set the file path
    void setFilePath( const QString& filePath );

    //! Get the name of the field with unique identifiers
    QString uid() const;
    //! Set the name of the field with unique identifiers
    void setUid( const QString& uid );

    //! Get the name of the geometry field. Empty if no geometry field
    QString geometryField() const;
    //! Set the name of the geometry field
    void setGeometryField( const QString& geometryField );

    //! Get the type of the geometry
    //! QgsWKBTypes::NoGeometry to hide any geometry
    //! QgsWKBTypes::Unknown for unknown types
    QgsWKBTypes::Type geometryWkbType() const;
    //! Set the type of the geometry
    void setGeometryWkbType( QgsWKBTypes::Type t );

    //! Get the SRID of the geometry
    long geometrySrid() const;
    //! Set the SRID of the geometry
    void setGeometrySrid( long srid );

    //! Get field definitions
    const QgsFields& fields() const;
    //! Set field definitions
    void setFields( const QgsFields& fields );

    //! Convenience method to test if a given source layer is part of the definition
    bool hasSourceLayer( const QString& name ) const;

    //! Convenience method to test whether the definition has referenced (live) layers
    bool hasReferencedLayers() const;

    //! Convenient method to test if the geometry is defined (not NoGeometry and not Unknown)
    bool hasDefinedGeometry() const;
};

