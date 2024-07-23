/***************************************************************************
  qgs3dsymbolregistry.h
  --------------------------------------
  Date                 : July 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DSYMBOLREGISTRY_H
#define QGS3DSYMBOLREGISTRY_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgswkbtypes.h"

#include <QDomElement>
#include <QMap>

class QgsAbstract3DSymbol;
class QgsReadWriteContext;
class Qgs3DSymbolWidget;
class QgsVectorLayer;
class QgsFeature3DHandler;

/**
 * \ingroup core
 * \brief Stores metadata about one 3D symbol class.
 *
 * \note It's necessary to implement createSymbol() function.
 * In C++ you can use Qgs3DSymbolMetadata convenience class.
 *
 * \since QGIS 3.16
 */
class CORE_EXPORT Qgs3DSymbolAbstractMetadata
{
  public:

    /**
     * Constructor for Qgs3DSymbolAbstractMetadata, with the specified \a type and \a visibleName.
     */
    Qgs3DSymbolAbstractMetadata( const QString &type, const QString &visibleName )
      : mType( type )
      , mVisibleName( visibleName )
    {}

    virtual ~Qgs3DSymbolAbstractMetadata() = default;

    /**
     * Returns the unique symbol type string.
     */
    QString type() const { return mType; }

    /**
     * Returns the symbol's visible (translated) name.
     */
    QString visibleName() const { return mVisibleName; }

    /**
     * Creates a new instance of this symbol type.
     *
     * Caller takes ownership of the returned symbol.
     */
    virtual QgsAbstract3DSymbol *create() = 0 SIP_FACTORY;

#ifndef SIP_RUN

    /**
     * Create a widget for configuring a symbol of this type.
     *
     * Can return NULLPTR if there's no GUI.
     *
     * \note Not available in Python bindings
     */
    virtual Qgs3DSymbolWidget *createSymbolWidget( QgsVectorLayer * ) SIP_FACTORY { return nullptr; }

    /**
     * Creates a feature handler for a \a symbol of matching type, for the specified vector \a layer.
     *
     * Caller takes ownership of the returned handler.
     *
     * \note Not available in Python bindings
     */
    virtual QgsFeature3DHandler *createFeatureHandler( QgsVectorLayer *layer, const QgsAbstract3DSymbol *symbol ) SIP_FACTORY { Q_UNUSED( layer ); Q_UNUSED( symbol ); return nullptr; }
#endif

  private:
    QString mType;
    QString mVisibleName;
};

//! 3D symbol creation function
typedef QgsAbstract3DSymbol *( *Qgs3DSymbolCreateFunc )() SIP_SKIP;

//! 3D symbol widget creation function
typedef QgsFeature3DHandler *( *Qgs3DSymbolFeatureHandlerFunc )( QgsVectorLayer *, const QgsAbstract3DSymbol * ) SIP_SKIP;

//! 3D symbol widget creation function
typedef Qgs3DSymbolWidget *( *Qgs3DSymbolWidgetFunc )( QgsVectorLayer * ) SIP_SKIP;

#ifndef SIP_RUN

/**
 * \ingroup core
 * \brief Convenience metadata class that uses static functions to create a 3D symbol and its widget.
 *
 * \note Not available in Python bindings.
 *
 * \since QGIS 3.16
 */
class CORE_EXPORT Qgs3DSymbolMetadata : public Qgs3DSymbolAbstractMetadata
{
  public:

    /**
     * Constructor for Qgs3DSymbolMetadata, with the specified \a type and \a visibleName.
     *
     * The \a pfCreate, \a pfWidget and \a pfHandler arguments are used to specify
     * static functions for creating the symbol type and configuration widget.
     */
    Qgs3DSymbolMetadata( const QString &type, const QString &visibleName,
                         Qgs3DSymbolCreateFunc pfCreate,
                         Qgs3DSymbolWidgetFunc pfWidget = nullptr,
                         Qgs3DSymbolFeatureHandlerFunc pfHandler = nullptr ) SIP_SKIP
  : Qgs3DSymbolAbstractMetadata( type, visibleName )
    , mCreateFunc( pfCreate )
    , mWidgetFunc( pfWidget )
    , mFeatureHandlerFunc( pfHandler )
    {}

    /**
     * Returns the symbol type's creation function.
     */
    Qgs3DSymbolCreateFunc createFunction() const { return mCreateFunc; }

    /**
     * Returns the symbol type's widget creation function.
     *
     * \see setWidgetFunction()
     */
    Qgs3DSymbolWidgetFunc widgetFunction() const { return mWidgetFunc; }

    /**
     * Sets the symbol type's widget creation \a function.
     *
     * \see widgetFunction()
     */
    void setWidgetFunction( Qgs3DSymbolWidgetFunc function ) { mWidgetFunc = function; }

    /**
     * Sets the symbol type's feature handler creation \a function.
     */
    void setFeatureHandlerFunction( Qgs3DSymbolFeatureHandlerFunc function ) { mFeatureHandlerFunc = function; }

    QgsAbstract3DSymbol *create() override SIP_FACTORY { return mCreateFunc ? mCreateFunc() : nullptr; }
    Qgs3DSymbolWidget *createSymbolWidget( QgsVectorLayer *vl ) override SIP_FACTORY { return mWidgetFunc ? mWidgetFunc( vl ) : nullptr; }
    QgsFeature3DHandler *createFeatureHandler( QgsVectorLayer *layer, const QgsAbstract3DSymbol *symbol ) override SIP_FACTORY { return mFeatureHandlerFunc ? mFeatureHandlerFunc( layer, symbol ) : nullptr; }

  private:
    Qgs3DSymbolCreateFunc mCreateFunc;
    Qgs3DSymbolWidgetFunc mWidgetFunc;
    Qgs3DSymbolFeatureHandlerFunc mFeatureHandlerFunc;

};
#endif


/**
 * \ingroup core
 * \brief Registry of available 3D symbol classes.
 *
 * Qgs3DSymbolRegistry is not usually directly created, but rather accessed through
 * QgsApplication::symbol3DRegistry().
 *
 * \since QGIS 3.16
 */
class CORE_EXPORT Qgs3DSymbolRegistry
{
  public:

    Qgs3DSymbolRegistry();
    ~Qgs3DSymbolRegistry();

    Qgs3DSymbolRegistry( const Qgs3DSymbolRegistry &rh ) = delete;
    Qgs3DSymbolRegistry &operator=( const Qgs3DSymbolRegistry &rh ) = delete;

    //! Returns metadata for specified symbol \a type. Returns NULLPTR if not found
    Qgs3DSymbolAbstractMetadata *symbolMetadata( const QString &type ) const;

    /**
     * Returns a list of all available symbol types.
     */
    QStringList symbolTypes() const;

    //! Registers a new symbol type. Takes ownership of the \a metadata instance.
    bool addSymbolType( Qgs3DSymbolAbstractMetadata *metadata SIP_TRANSFER );

    /**
     * Creates a new instance of a symbol of the specified \a type.
     *
     * The caller takes ownership of the returned symbol.
     *
     * Returns NULLPTR if the specified type is not found in the registry.
     */
    QgsAbstract3DSymbol *createSymbol( const QString &type ) const SIP_FACTORY;

    /**
     * Creates a new instance of the default 3D symbol for the specified geometry \a type.
     *
     * The caller takes ownership of the returned symbol.
     */
    QgsAbstract3DSymbol *defaultSymbolForGeometryType( Qgis::GeometryType type ) SIP_FACTORY;

#ifndef SIP_RUN

    /**
     * Creates a feature handler for a \a symbol, for the specified vector \a layer.
     *
     * Caller takes ownership of the returned handler.
     *
     * \note Not available in Python bindings
     */
    QgsFeature3DHandler *createHandlerForSymbol( QgsVectorLayer *layer, const QgsAbstract3DSymbol *symbol ) SIP_FACTORY;
#endif

  private:
#ifdef SIP_RUN
    Qgs3DSymbolRegistry( const Qgs3DSymbolRegistry &rh );
#endif

    QMap<QString, Qgs3DSymbolAbstractMetadata *> mMetadata;
};


#endif // QGS3DSYMBOLREGISTRY_H
