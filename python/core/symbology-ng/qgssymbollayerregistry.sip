class QgsSymbolLayerWidget /External/;

class QgsSymbolLayerAbstractMetadata
{
%TypeHeaderCode
#include <qgssymbollayerregistry.h>
%End
  public:
    QgsSymbolLayerAbstractMetadata( const QString& name, const QString& visibleName, QgsSymbol::SymbolType type );

    virtual ~QgsSymbolLayerAbstractMetadata();

    QString name() const;
    QString visibleName() const;
    QgsSymbol::SymbolType type() const;

    /** Create a symbol layer of this type given the map of properties. */
    virtual QgsSymbolLayer* createSymbolLayer( const QgsStringMap& map ) = 0 /Factory/;
    /** Create widget for symbol layer of this type. Can return NULL if there's no GUI */
    virtual QgsSymbolLayerWidget* createSymbolLayerWidget( const QgsVectorLayer * ) /Factory/;
    /** Create a symbol layer of this type given the map of properties. */
    virtual QgsSymbolLayer* createSymbolLayerFromSld( QDomElement & ) /Factory/;
};


/**
 Convenience metadata class that uses static functions to create symbol layer and its widget.
 */
class QgsSymbolLayerMetadata : QgsSymbolLayerAbstractMetadata
{
%TypeHeaderCode
#include <qgssymbollayerregistry.h>
%End

  public:
    virtual QgsSymbolLayer* createSymbolLayer( const QgsStringMap& map ) /Factory/;
    virtual QgsSymbolLayerWidget* createSymbolLayerWidget( const QgsVectorLayer* vl ) /Factory/;
    virtual QgsSymbolLayer* createSymbolLayerFromSld( QDomElement& elem ) /Factory/;

  private:
    QgsSymbolLayerMetadata(); // pretend this is private
};


/**
 Registry of available symbol layer classes.
 Implemented as a singleton.
 */
class QgsSymbolLayerRegistry
{
%TypeHeaderCode
#include <qgssymbollayerregistry.h>
%End

  public:

    //! return the single instance of this class (instantiate it if not exists)
    static QgsSymbolLayerRegistry* instance();

    //! return metadata for specified symbol layer. Returns NULL if not found
    QgsSymbolLayerAbstractMetadata* symbolLayerMetadata( const QString& name ) const;

    //! register a new symbol layer type. Takes ownership of the metadata instance.
    bool addSymbolLayerType( QgsSymbolLayerAbstractMetadata* metadata /Transfer/ );

    //! create a new instance of symbol layer given symbol layer name and properties
    QgsSymbolLayer* createSymbolLayer( const QString& name, const QgsStringMap& properties = QgsStringMap() ) const /Factory/;

    //! create a new instance of symbol layer given symbol layer name and SLD
    QgsSymbolLayer* createSymbolLayerFromSld( const QString& name, QDomElement &element ) const;

    //! return a list of available symbol layers for a specified symbol type
    QStringList symbolLayersForType( QgsSymbol::SymbolType type );

    //! create a new instance of symbol layer for specified symbol type with default settings
    static QgsSymbolLayer* defaultSymbolLayer( QgsSymbol::SymbolType type ) /Factory/;

  protected:
    QgsSymbolLayerRegistry();
    ~QgsSymbolLayerRegistry();

  private:
    QgsSymbolLayerRegistry( const QgsSymbolLayerRegistry& rh );

};
