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

#include "qgis_core.h"
#include "qgis.h"

class QgsPathResolver;
class QgsVectorLayer;
class QgsSymbolLayerWidget SIP_EXTERNAL;
class QgsSymbolLayer;
class QDomElement;
class QgsReadWriteContext;

/**
 * \ingroup core
 * \brief Stores metadata about one symbol layer class.
 *
 * \note It's necessary to implement createSymbolLayer() function.
 *   In C++ you can use QgsSymbolLayerMetadata convenience class.
 */
class CORE_EXPORT QgsSymbolLayerAbstractMetadata
{
  public:

    /**
     * Constructor for QgsSymbolLayerAbstractMetadata.
     * \param name internal symbol layer name (unique identifier)
     * \param visibleName user visible, translated name for symbol layer
     * \param type associated symbol type
     */
    QgsSymbolLayerAbstractMetadata( const QString &name, const QString &visibleName, Qgis::SymbolType type )
      : mName( name )
      , mVisibleName( visibleName )
      , mType( type )
    {}

    virtual ~QgsSymbolLayerAbstractMetadata() = default;

    QString name() const { return mName; }
    QString visibleName() const { return mVisibleName; }
    Qgis::SymbolType type() const { return mType; }

    //! Create a symbol layer of this type given the map of properties.
    virtual QgsSymbolLayer *createSymbolLayer( const QVariantMap &map ) = 0 SIP_FACTORY;
    //! Create widget for symbol layer of this type. Can return NULLPTR if there's no GUI
    virtual QgsSymbolLayerWidget *createSymbolLayerWidget( QgsVectorLayer * ) SIP_FACTORY { return nullptr; }
    //! Create a symbol layer of this type given the map of properties.
    virtual QgsSymbolLayer *createSymbolLayerFromSld( QDomElement & ) SIP_FACTORY { return nullptr; }

    /**
     * Resolve paths in symbol layer's properties (if there are any paths).
     * When saving is TRUE, paths are converted from absolute to relative,
     * when saving is FALSE, paths are converted from relative to absolute.
     * This ensures that paths in project files can be relative, but in symbol layer
     * instances the paths are always absolute
     * \since QGIS 3.0
     */
    virtual void resolvePaths( QVariantMap &properties, const QgsPathResolver &pathResolver, bool saving )
    {
      Q_UNUSED( properties )
      Q_UNUSED( pathResolver )
      Q_UNUSED( saving )
    }

    /**
     * Resolve fonts from the  symbol layer's \a properties.
     *
     * This tests whether the required fonts from the encoded \a properties are available on the system, and records
     * warnings in the \a context if not.
     *
     * \since QGIS 3.20
     */
    virtual void resolveFonts( const QVariantMap &properties, const QgsReadWriteContext &context )
    {
      Q_UNUSED( properties )
      Q_UNUSED( context )
    }

  protected:
    QString mName;
    QString mVisibleName;
    Qgis::SymbolType mType;
};

typedef QgsSymbolLayer *( *QgsSymbolLayerCreateFunc )( const QVariantMap & ) SIP_SKIP;
typedef QgsSymbolLayerWidget *( *QgsSymbolLayerWidgetFunc )( QgsVectorLayer * ) SIP_SKIP;
typedef QgsSymbolLayer *( *QgsSymbolLayerCreateFromSldFunc )( QDomElement & ) SIP_SKIP;
typedef void ( *QgsSymbolLayerPathResolverFunc )( QVariantMap &, const QgsPathResolver &, bool ) SIP_SKIP;
typedef void ( *QgsSymbolLayerFontResolverFunc )( const QVariantMap &, const QgsReadWriteContext & ) SIP_SKIP;

/**
 * \ingroup core
 * \brief Convenience metadata class that uses static functions to create symbol layer and its widget.
 */
class CORE_EXPORT QgsSymbolLayerMetadata : public QgsSymbolLayerAbstractMetadata
{
  public:
    //! \note not available in Python bindings
    QgsSymbolLayerMetadata( const QString &name, const QString &visibleName,
                            Qgis::SymbolType type,
                            QgsSymbolLayerCreateFunc pfCreate,
                            QgsSymbolLayerCreateFromSldFunc pfCreateFromSld = nullptr,
                            QgsSymbolLayerPathResolverFunc pfPathResolver = nullptr,
                            QgsSymbolLayerWidgetFunc pfWidget = nullptr,
                            QgsSymbolLayerFontResolverFunc pfFontResolver = nullptr ) SIP_SKIP
  : QgsSymbolLayerAbstractMetadata( name, visibleName, type )
    , mCreateFunc( pfCreate )
    , mWidgetFunc( pfWidget )
    , mCreateFromSldFunc( pfCreateFromSld )
    , mPathResolverFunc( pfPathResolver )
    , mFontResolverFunc( pfFontResolver )
    {}

    //! \note not available in Python bindings
    QgsSymbolLayerCreateFunc createFunction() const { return mCreateFunc; } SIP_SKIP
    //! \note not available in Python bindings
    QgsSymbolLayerWidgetFunc widgetFunction() const { return mWidgetFunc; } SIP_SKIP
    //! \note not available in Python bindings
    QgsSymbolLayerCreateFromSldFunc createFromSldFunction() const { return mCreateFromSldFunc; } SIP_SKIP
    //! \note not available in Python bindings
    QgsSymbolLayerPathResolverFunc pathResolverFunction() const { return mPathResolverFunc; } SIP_SKIP

    //! \note not available in Python bindings
    void setWidgetFunction( QgsSymbolLayerWidgetFunc f ) { mWidgetFunc = f; } SIP_SKIP

    QgsSymbolLayer *createSymbolLayer( const QVariantMap &map ) override SIP_FACTORY { return mCreateFunc ? mCreateFunc( map ) : nullptr; }
    QgsSymbolLayerWidget *createSymbolLayerWidget( QgsVectorLayer *vl ) override SIP_FACTORY { return mWidgetFunc ? mWidgetFunc( vl ) : nullptr; }
    QgsSymbolLayer *createSymbolLayerFromSld( QDomElement &elem ) override SIP_FACTORY { return mCreateFromSldFunc ? mCreateFromSldFunc( elem ) : nullptr; }
    void resolvePaths( QVariantMap &properties, const QgsPathResolver &pathResolver, bool saving ) override
    {
      if ( mPathResolverFunc )
        mPathResolverFunc( properties, pathResolver, saving );
    }

    void resolveFonts( const QVariantMap &properties, const QgsReadWriteContext &context ) override
    {
      if ( mFontResolverFunc )
        mFontResolverFunc( properties, context );
    }

  protected:
    QgsSymbolLayerCreateFunc mCreateFunc;
    QgsSymbolLayerWidgetFunc mWidgetFunc;
    QgsSymbolLayerCreateFromSldFunc mCreateFromSldFunc;
    QgsSymbolLayerPathResolverFunc mPathResolverFunc;

    /**
     * Font resolver function pointer.
     *
     * \since QGIS 3.20
     */
    QgsSymbolLayerFontResolverFunc mFontResolverFunc;

  private:
#ifdef SIP_RUN
    QgsSymbolLayerMetadata();
#endif
};


/**
 * \ingroup core
 * \brief Registry of available symbol layer classes.
 *
 * QgsSymbolLayerRegistry is not usually directly created, but rather accessed through
 * QgsApplication::symbolLayerRegistry().
 */
class CORE_EXPORT QgsSymbolLayerRegistry
{
  public:

    QgsSymbolLayerRegistry();
    ~QgsSymbolLayerRegistry();

    //! QgsSymbolLayerRegistry cannot be copied.
    QgsSymbolLayerRegistry( const QgsSymbolLayerRegistry &rh ) = delete;
    //! QgsSymbolLayerRegistry cannot be copied.
    QgsSymbolLayerRegistry &operator=( const QgsSymbolLayerRegistry &rh ) = delete;

    //! Returns metadata for specified symbol layer. Returns NULLPTR if not found
    QgsSymbolLayerAbstractMetadata *symbolLayerMetadata( const QString &name ) const;

    //! Registers a new symbol layer type. Takes ownership of the metadata instance.
    bool addSymbolLayerType( QgsSymbolLayerAbstractMetadata *metadata SIP_TRANSFER );

    /**
     * Removes a symbol layer type
     * \since QGIS 3.22.2
     */
    bool removeSymbolLayerType( QgsSymbolLayerAbstractMetadata *metadata );

    //! create a new instance of symbol layer given symbol layer name and properties
    QgsSymbolLayer *createSymbolLayer( const QString &name, const QVariantMap &properties = QVariantMap() ) const SIP_FACTORY;

    //! create a new instance of symbol layer given symbol layer name and SLD
    QgsSymbolLayer *createSymbolLayerFromSld( const QString &name, QDomElement &element ) const SIP_FACTORY;

    /**
     * Resolve paths in properties of a particular symbol layer.
     * This normally means converting relative paths to absolute paths when loading
     * and converting absolute paths to relative paths when saving.
     * \since QGIS 3.0
     */
    void resolvePaths( const QString &name, QVariantMap &properties, const QgsPathResolver &pathResolver, bool saving ) const;

    /**
     * Resolve fonts from the \a properties of a particular symbol layer.
     *
     * This tests whether the required fonts from the encoded \a properties are available on the system, and records
     * warnings in the \a context if not.
     *
     * \since QGIS 3.20
     */
    void resolveFonts( const QString &name, QVariantMap &properties, const QgsReadWriteContext &context ) const;

    //! Returns a list of available symbol layers for a specified symbol type
    QStringList symbolLayersForType( Qgis::SymbolType type );

    //! create a new instance of symbol layer for specified symbol type with default settings
    static QgsSymbolLayer *defaultSymbolLayer( Qgis::SymbolType type ) SIP_FACTORY;

  private:
#ifdef SIP_RUN
    QgsSymbolLayerRegistry( const QgsSymbolLayerRegistry &rh );
#endif

    QMap<QString, QgsSymbolLayerAbstractMetadata *> mMetadata;
};

#endif
