/***************************************************************************
    qgssymbollayerregistry.h
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

#ifndef QGSSYMBOLLAYERREGISTRY_H
#define QGSSYMBOLLAYERREGISTRY_H

#include "qgssymbol.h"

class QgsVectorLayer;
class QgsSymbolLayerWidget;

/** \ingroup core
 Stores metadata about one symbol layer class.

 @note It's necessary to implement createSymbolLayer() function.
   In C++ you can use QgsSymbolLayerMetadata convenience class.
 */
class CORE_EXPORT QgsSymbolLayerAbstractMetadata
{
  public:
    QgsSymbolLayerAbstractMetadata( const QString& name, const QString& visibleName, QgsSymbol::SymbolType type )
        : mName( name )
        , mVisibleName( visibleName )
        , mType( type )
    {}

    virtual ~QgsSymbolLayerAbstractMetadata() {}

    QString name() const { return mName; }
    QString visibleName() const { return mVisibleName; }
    QgsSymbol::SymbolType type() const { return mType; }

    /** Create a symbol layer of this type given the map of properties. */
    virtual QgsSymbolLayer* createSymbolLayer( const QgsStringMap& map ) = 0;
    /** Create widget for symbol layer of this type. Can return NULL if there's no GUI */
    virtual QgsSymbolLayerWidget* createSymbolLayerWidget( const QgsVectorLayer * ) { return nullptr; }
    /** Create a symbol layer of this type given the map of properties. */
    virtual QgsSymbolLayer* createSymbolLayerFromSld( QDomElement & ) { return nullptr; }


  protected:
    QString mName;
    QString mVisibleName;
    QgsSymbol::SymbolType mType;
};

typedef QgsSymbolLayer*( *QgsSymbolLayerCreateFunc )( const QgsStringMap& );
typedef QgsSymbolLayerWidget*( *QgsSymbolLayerWidgetFunc )( const QgsVectorLayer* );
typedef QgsSymbolLayer*( *QgsSymbolLayerCreateFromSldFunc )( QDomElement& );

/** \ingroup core
 Convenience metadata class that uses static functions to create symbol layer and its widget.
 */
class CORE_EXPORT QgsSymbolLayerMetadata : public QgsSymbolLayerAbstractMetadata
{
  public:
    //! @note not available in python bindings
    QgsSymbolLayerMetadata( const QString& name, const QString& visibleName,
                            QgsSymbol::SymbolType type,
                            QgsSymbolLayerCreateFunc pfCreate,
                            QgsSymbolLayerWidgetFunc pfWidget = nullptr )
        : QgsSymbolLayerAbstractMetadata( name, visibleName, type )
        , mCreateFunc( pfCreate )
        , mWidgetFunc( pfWidget )
        , mCreateFromSldFunc( nullptr )
    {}

    //! @note not available in python bindings
    QgsSymbolLayerMetadata( const QString& name, const QString& visibleName,
                            QgsSymbol::SymbolType type,
                            QgsSymbolLayerCreateFunc pfCreate,
                            QgsSymbolLayerCreateFromSldFunc pfCreateFromSld,
                            QgsSymbolLayerWidgetFunc pfWidget = nullptr )
        : QgsSymbolLayerAbstractMetadata( name, visibleName, type )
        , mCreateFunc( pfCreate )
        , mWidgetFunc( pfWidget )
        , mCreateFromSldFunc( pfCreateFromSld )
    {}

    //! @note not available in python bindings
    QgsSymbolLayerCreateFunc createFunction() const { return mCreateFunc; }
    //! @note not available in python bindings
    QgsSymbolLayerWidgetFunc widgetFunction() const { return mWidgetFunc; }
    //! @note not available in python bindings
    QgsSymbolLayerCreateFromSldFunc createFromSldFunction() const { return mCreateFromSldFunc; }

    //! @note not available in python bindings
    void setWidgetFunction( QgsSymbolLayerWidgetFunc f ) { mWidgetFunc = f; }

    virtual QgsSymbolLayer* createSymbolLayer( const QgsStringMap& map ) override { return mCreateFunc ? mCreateFunc( map ) : nullptr; }
    virtual QgsSymbolLayerWidget* createSymbolLayerWidget( const QgsVectorLayer* vl ) override { return mWidgetFunc ? mWidgetFunc( vl ) : nullptr; }
    virtual QgsSymbolLayer* createSymbolLayerFromSld( QDomElement& elem ) override { return mCreateFromSldFunc ? mCreateFromSldFunc( elem ) : nullptr; }

  protected:
    QgsSymbolLayerCreateFunc mCreateFunc;
    QgsSymbolLayerWidgetFunc mWidgetFunc;
    QgsSymbolLayerCreateFromSldFunc mCreateFromSldFunc;
};


/** \ingroup core
 Registry of available symbol layer classes.
 Implemented as a singleton.
 */
class CORE_EXPORT QgsSymbolLayerRegistry
{
  public:

    //! return the single instance of this class (instantiate it if not exists)
    static QgsSymbolLayerRegistry* instance();

    //! return metadata for specified symbol layer. Returns NULL if not found
    QgsSymbolLayerAbstractMetadata* symbolLayerMetadata( const QString& name ) const;

    //! register a new symbol layer type. Takes ownership of the metadata instance.
    bool addSymbolLayerType( QgsSymbolLayerAbstractMetadata* metadata );

    //! create a new instance of symbol layer given symbol layer name and properties
    QgsSymbolLayer* createSymbolLayer( const QString& name, const QgsStringMap& properties = QgsStringMap() ) const;

    //! create a new instance of symbol layer given symbol layer name and SLD
    QgsSymbolLayer* createSymbolLayerFromSld( const QString& name, QDomElement &element ) const;

    //! return a list of available symbol layers for a specified symbol type
    QStringList symbolLayersForType( QgsSymbol::SymbolType type );

    //! create a new instance of symbol layer for specified symbol type with default settings
    static QgsSymbolLayer* defaultSymbolLayer( QgsSymbol::SymbolType type );

  protected:
    QgsSymbolLayerRegistry();
    ~QgsSymbolLayerRegistry();

    QMap<QString, QgsSymbolLayerAbstractMetadata*> mMetadata;

  private:
    QgsSymbolLayerRegistry( const QgsSymbolLayerRegistry& rh );
    QgsSymbolLayerRegistry& operator=( const QgsSymbolLayerRegistry& rh );
};

#endif
