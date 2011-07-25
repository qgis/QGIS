
#ifndef QGSSYMBOLLAYERV2REGISTRY_H
#define QGSSYMBOLLAYERV2REGISTRY_H

#include "qgssymbolv2.h"
#include "qgssymbollayerv2.h"

class QgsVectorLayer;

/**
 Stores metadata about one symbol layer class.

 @note It's necessary to implement createSymbolLayer() function.
   In C++ you can use QgsSymbolLayerV2Metadata convenience class.
 */
class CORE_EXPORT QgsSymbolLayerV2AbstractMetadata
{
  public:
    QgsSymbolLayerV2AbstractMetadata( QString name, QString visibleName, QgsSymbolV2::SymbolType type )
        : mName( name ), mVisibleName( visibleName ), mType( type ) {}

    QString name() const { return mName; }
    QString visibleName() const { return mVisibleName; }
    QgsSymbolV2::SymbolType type() const { return mType; }

    /** create a symbol layer of this type given the map of properties. */
    virtual QgsSymbolLayerV2* createSymbolLayer( const QgsStringMap& map ) = 0;
    /** create widget for symbol layer of this type. Can return NULL if there's no GUI */
    virtual QgsSymbolLayerV2Widget* createSymbolLayerWidget( const QgsVectorLayer* vl ) { return NULL; }

  protected:
    QString mName;
    QString mVisibleName;
    QgsSymbolV2::SymbolType mType;
};

typedef QgsSymbolLayerV2*( *QgsSymbolLayerV2CreateFunc )( const QgsStringMap& );
typedef QgsSymbolLayerV2Widget*( *QgsSymbolLayerV2WidgetFunc )( const QgsVectorLayer* );

/**
 Convenience metadata class that uses static functions to create symbol layer and its widget.
 */
class CORE_EXPORT QgsSymbolLayerV2Metadata : public QgsSymbolLayerV2AbstractMetadata
{
  public:
    QgsSymbolLayerV2Metadata( QString name, QString visibleName,
                              QgsSymbolV2::SymbolType type,
                              QgsSymbolLayerV2CreateFunc pfCreate,
                              QgsSymbolLayerV2WidgetFunc pfWidget = NULL )
        : QgsSymbolLayerV2AbstractMetadata( name, visibleName, type ), mCreateFunc( pfCreate ), mWidgetFunc( pfWidget ) {}

    QgsSymbolLayerV2CreateFunc createFunction() const { return mCreateFunc; }
    QgsSymbolLayerV2WidgetFunc widgetFunction() const { return mWidgetFunc; }

    void setWidgetFunction( QgsSymbolLayerV2WidgetFunc f ) { mWidgetFunc = f; }

    virtual QgsSymbolLayerV2* createSymbolLayer( const QgsStringMap& map ) { return mCreateFunc ? mCreateFunc( map ) : NULL; }
    virtual QgsSymbolLayerV2Widget* createSymbolLayerWidget( const QgsVectorLayer* vl ) { return mWidgetFunc ? mWidgetFunc( vl ) : NULL; }

  protected:
    QgsSymbolLayerV2CreateFunc mCreateFunc;
    QgsSymbolLayerV2WidgetFunc mWidgetFunc;
};


/**
 Registry of available symbol layer classes.
 Implemented as a singleton.
 */
class CORE_EXPORT QgsSymbolLayerV2Registry
{
  public:

    //! return the single instance of this class (instantiate it if not exists)
    static QgsSymbolLayerV2Registry* instance();

    //! return metadata for specified symbol layer. Returns NULL if not found
    QgsSymbolLayerV2AbstractMetadata* symbolLayerMetadata( QString name ) const;

    //! register a new symbol layer type. Takes ownership of the metadata instance.
    bool addSymbolLayerType( QgsSymbolLayerV2AbstractMetadata* metadata );

    //! create a new instance of symbol layer given symbol layer name and properties
    QgsSymbolLayerV2* createSymbolLayer( QString name, const QgsStringMap& properties = QgsStringMap() ) const;

    //! return a list of available symbol layers for a specified symbol type
    QStringList symbolLayersForType( QgsSymbolV2::SymbolType type );

    //! create a new instance of symbol layer for specified symbol type with default settings
    static QgsSymbolLayerV2* defaultSymbolLayer( QgsSymbolV2::SymbolType type );

  protected:
    QgsSymbolLayerV2Registry();
    ~QgsSymbolLayerV2Registry();

    static QgsSymbolLayerV2Registry* mInstance;
    QMap<QString, QgsSymbolLayerV2AbstractMetadata*> mMetadata;

};

#endif
