class QgsSymbolLayerV2Widget /External/;

class QgsSymbolLayerV2AbstractMetadata
{
%TypeHeaderCode
#include <qgssymbollayerv2registry.h>
%End
  public:
    QgsSymbolLayerV2AbstractMetadata( const QString& name, const QString& visibleName, QgsSymbolV2::SymbolType type );

    virtual ~QgsSymbolLayerV2AbstractMetadata();

    QString name() const;
    QString visibleName() const;
    QgsSymbolV2::SymbolType type() const;

    /** Create a symbol layer of this type given the map of properties. */
    virtual QgsSymbolLayerV2* createSymbolLayer( const QgsStringMap& map ) = 0 /Factory/;
    /** Create widget for symbol layer of this type. Can return NULL if there's no GUI */
    virtual QgsSymbolLayerV2Widget* createSymbolLayerWidget( const QgsVectorLayer * ) /Factory/;
    /** Create a symbol layer of this type given the map of properties. */
    virtual QgsSymbolLayerV2* createSymbolLayerFromSld( QDomElement & ) /Factory/;
};


/**
 Convenience metadata class that uses static functions to create symbol layer and its widget.
 */
class QgsSymbolLayerV2Metadata : QgsSymbolLayerV2AbstractMetadata
{
%TypeHeaderCode
#include <qgssymbollayerv2registry.h>
%End

  public:
    virtual QgsSymbolLayerV2* createSymbolLayer( const QgsStringMap& map ) /Factory/;
    virtual QgsSymbolLayerV2Widget* createSymbolLayerWidget( const QgsVectorLayer* vl ) /Factory/;
    virtual QgsSymbolLayerV2* createSymbolLayerFromSld( QDomElement& elem ) /Factory/;

  private:
    QgsSymbolLayerV2Metadata(); // pretend this is private
};


/**
 Registry of available symbol layer classes.
 Implemented as a singleton.
 */
class QgsSymbolLayerV2Registry
{
%TypeHeaderCode
#include <qgssymbollayerv2registry.h>
%End

  public:

    //! return the single instance of this class (instantiate it if not exists)
    static QgsSymbolLayerV2Registry* instance();

    //! return metadata for specified symbol layer. Returns NULL if not found
    QgsSymbolLayerV2AbstractMetadata* symbolLayerMetadata( const QString& name ) const;

    //! register a new symbol layer type. Takes ownership of the metadata instance.
    bool addSymbolLayerType( QgsSymbolLayerV2AbstractMetadata* metadata /Transfer/ );

    //! create a new instance of symbol layer given symbol layer name and properties
    QgsSymbolLayerV2* createSymbolLayer( const QString& name, const QgsStringMap& properties = QgsStringMap() ) const /Factory/;

    //! create a new instance of symbol layer given symbol layer name and SLD
    QgsSymbolLayerV2* createSymbolLayerFromSld( const QString& name, QDomElement &element ) const;

    //! return a list of available symbol layers for a specified symbol type
    QStringList symbolLayersForType( QgsSymbolV2::SymbolType type );

    //! create a new instance of symbol layer for specified symbol type with default settings
    static QgsSymbolLayerV2* defaultSymbolLayer( QgsSymbolV2::SymbolType type ) /Factory/;

  protected:
    QgsSymbolLayerV2Registry();
    ~QgsSymbolLayerV2Registry();

  private:
    QgsSymbolLayerV2Registry( const QgsSymbolLayerV2Registry& rh );

};
