/***************************************************************************
    qgssymbollayerv2registry.h
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSYMBOLLAYERV2REGISTRY_H
#define QGSSYMBOLLAYERV2REGISTRY_H

#include "qgssymbolv2.h"
#include "qgssymbollayerv2.h"

class QgsVectorLayer;

/** \ingroup core
 Stores metadata about one symbol layer class.

 @note It's necessary to implement createSymbolLayer() function.
   In C++ you can use QgsSymbolLayerV2Metadata convenience class.
 */
class CORE_EXPORT QgsSymbolLayerV2AbstractMetadata
{
  public:
    QgsSymbolLayerV2AbstractMetadata( const QString& name, const QString& visibleName, QgsSymbolV2::SymbolType type )
        : mName( name )
        , mVisibleName( visibleName )
        , mType( type )
    {}

    virtual ~QgsSymbolLayerV2AbstractMetadata() {}

    QString name() const { return mName; }
    QString visibleName() const { return mVisibleName; }
    QgsSymbolV2::SymbolType type() const { return mType; }

    /** Create a symbol layer of this type given the map of properties. */
    virtual QgsSymbolLayerV2* createSymbolLayer( const QgsStringMap& map ) = 0;
    /** Create widget for symbol layer of this type. Can return NULL if there's no GUI */
    virtual QgsSymbolLayerV2Widget* createSymbolLayerWidget( const QgsVectorLayer * ) { return nullptr; }
    /** Create a symbol layer of this type given the map of properties. */
    virtual QgsSymbolLayerV2* createSymbolLayerFromSld( QDomElement & ) { return nullptr; }


  protected:
    QString mName;
    QString mVisibleName;
    QgsSymbolV2::SymbolType mType;
};

typedef QgsSymbolLayerV2*( *QgsSymbolLayerV2CreateFunc )( const QgsStringMap& );
typedef QgsSymbolLayerV2Widget*( *QgsSymbolLayerV2WidgetFunc )( const QgsVectorLayer* );
typedef QgsSymbolLayerV2*( *QgsSymbolLayerV2CreateFromSldFunc )( QDomElement& );

/** \ingroup core
 Convenience metadata class that uses static functions to create symbol layer and its widget.
 */
class CORE_EXPORT QgsSymbolLayerV2Metadata : public QgsSymbolLayerV2AbstractMetadata
{
  public:
    //! @note not available in python bindings
    QgsSymbolLayerV2Metadata( const QString& name, const QString& visibleName,
                              QgsSymbolV2::SymbolType type,
                              QgsSymbolLayerV2CreateFunc pfCreate,
                              QgsSymbolLayerV2WidgetFunc pfWidget = nullptr )
        : QgsSymbolLayerV2AbstractMetadata( name, visibleName, type )
        , mCreateFunc( pfCreate )
        , mWidgetFunc( pfWidget )
        , mCreateFromSldFunc( nullptr )
    {}

    //! @note not available in python bindings
    QgsSymbolLayerV2Metadata( const QString& name, const QString& visibleName,
                              QgsSymbolV2::SymbolType type,
                              QgsSymbolLayerV2CreateFunc pfCreate,
                              QgsSymbolLayerV2CreateFromSldFunc pfCreateFromSld,
                              QgsSymbolLayerV2WidgetFunc pfWidget = nullptr )
        : QgsSymbolLayerV2AbstractMetadata( name, visibleName, type )
        , mCreateFunc( pfCreate )
        , mWidgetFunc( pfWidget )
        , mCreateFromSldFunc( pfCreateFromSld )
    {}

    //! @note not available in python bindings
    QgsSymbolLayerV2CreateFunc createFunction() const { return mCreateFunc; }
    //! @note not available in python bindings
    QgsSymbolLayerV2WidgetFunc widgetFunction() const { return mWidgetFunc; }
    //! @note not available in python bindings
    QgsSymbolLayerV2CreateFromSldFunc createFromSldFunction() const { return mCreateFromSldFunc; }

    //! @note not available in python bindings
    void setWidgetFunction( QgsSymbolLayerV2WidgetFunc f ) { mWidgetFunc = f; }

    virtual QgsSymbolLayerV2* createSymbolLayer( const QgsStringMap& map ) override { return mCreateFunc ? mCreateFunc( map ) : nullptr; }
    virtual QgsSymbolLayerV2Widget* createSymbolLayerWidget( const QgsVectorLayer* vl ) override { return mWidgetFunc ? mWidgetFunc( vl ) : nullptr; }
    virtual QgsSymbolLayerV2* createSymbolLayerFromSld( QDomElement& elem ) override { return mCreateFromSldFunc ? mCreateFromSldFunc( elem ) : nullptr; }

  protected:
    QgsSymbolLayerV2CreateFunc mCreateFunc;
    QgsSymbolLayerV2WidgetFunc mWidgetFunc;
    QgsSymbolLayerV2CreateFromSldFunc mCreateFromSldFunc;
};


/** \ingroup core
 Registry of available symbol layer classes.
 Implemented as a singleton.
 */
class CORE_EXPORT QgsSymbolLayerV2Registry
{
  public:

    //! return the single instance of this class (instantiate it if not exists)
    static QgsSymbolLayerV2Registry* instance();

    //! return metadata for specified symbol layer. Returns NULL if not found
    QgsSymbolLayerV2AbstractMetadata* symbolLayerMetadata( const QString& name ) const;

    //! register a new symbol layer type. Takes ownership of the metadata instance.
    bool addSymbolLayerType( QgsSymbolLayerV2AbstractMetadata* metadata );

    //! create a new instance of symbol layer given symbol layer name and properties
    QgsSymbolLayerV2* createSymbolLayer( const QString& name, const QgsStringMap& properties = QgsStringMap() ) const;

    //! create a new instance of symbol layer given symbol layer name and SLD
    QgsSymbolLayerV2* createSymbolLayerFromSld( const QString& name, QDomElement &element ) const;

    //! return a list of available symbol layers for a specified symbol type
    QStringList symbolLayersForType( QgsSymbolV2::SymbolType type );

    //! create a new instance of symbol layer for specified symbol type with default settings
    static QgsSymbolLayerV2* defaultSymbolLayer( QgsSymbolV2::SymbolType type );

  protected:
    QgsSymbolLayerV2Registry();
    ~QgsSymbolLayerV2Registry();

    QMap<QString, QgsSymbolLayerV2AbstractMetadata*> mMetadata;

  private:
    QgsSymbolLayerV2Registry( const QgsSymbolLayerV2Registry& rh );
    QgsSymbolLayerV2Registry& operator=( const QgsSymbolLayerV2Registry& rh );
};

#endif
