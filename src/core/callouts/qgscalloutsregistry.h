/***************************************************************************
    qgscalloutsregistry.h
    ---------------------
    begin                : July 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCALLOUTSREGISTRY_H
#define QGSCALLOUTSREGISTRY_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgsreadwritecontext.h"
#include <QIcon>

class QgsPathResolver;
class QgsMapLayer;
class QgsCalloutWidget SIP_EXTERNAL;
class QgsCallout;
class QDomElement;

/**
 * \ingroup core
 *  \brief Stores metadata about one callout renderer class.
 *
 *  \note It's necessary to implement createCallout() function.
 *  In C++ you can use QgsCalloutMetadata convenience class.
 *
 *  \since QGIS 3.10
 */
class CORE_EXPORT QgsCalloutAbstractMetadata
{
  public:

    /**
     * Constructor for QgsCalloutAbstractMetadata, with the specified \a name.
     *
     * The \a visibleName argument gives a translated, user friendly string identifying the callout type.
     *
     * The \a icon argument can be used to specify an icon representing the callout.
     */
    QgsCalloutAbstractMetadata( const QString &name, const QString &visibleName, const QIcon &icon = QIcon() )
      : mName( name )
      , mVisibleName( visibleName )
      , mIcon( icon )
    {}

    virtual ~QgsCalloutAbstractMetadata() = default;

    /**
     * Returns the unique name of the callout type. This value is not translated.
     * \see visibleName()
     */
    QString name() const { return mName; }

    /**
     * Returns a friendly display name of the callout type. This value is translated.
     * \see name()
     */
    QString visibleName() const { return mVisibleName; }

    /**
     * Returns an icon representing the callout.
     * \see setIcon()
     */
    QIcon icon() const { return mIcon; }

    /**
     * Sets an \a icon representing the callout.
     * \see icon()
     */
    void setIcon( const QIcon &icon ) { mIcon = icon; }

    /**
     * Create a callout of this type given the map of \a properties.
     *
     * Ownership of the callout is transferred to the caller.
     */
    virtual QgsCallout *createCallout( const QVariantMap &properties, const QgsReadWriteContext &context ) = 0 SIP_FACTORY;

    /**
     * Creates a widget for configuring callouts of this type. Can return NULLPTR if there's no GUI required.
     *
     * Ownership of the widget is transferred to the caller.
     */
    virtual QgsCalloutWidget *createCalloutWidget( QgsMapLayer * );

  protected:
    QString mName;
    QString mVisibleName;
    QIcon mIcon;
};

typedef QgsCallout *( *QgsCalloutCreateFunc )( const QVariantMap &, const QgsReadWriteContext & ) SIP_SKIP;
typedef QgsCalloutWidget *( *QgsCalloutWidgetFunc )( QgsMapLayer * ) SIP_SKIP;

/**
 * \ingroup core
 * \brief Convenience metadata class that uses static functions to create callouts and their widgets.
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsCalloutMetadata : public QgsCalloutAbstractMetadata
{
  public:

    //! \note not available in Python bindings
    QgsCalloutMetadata( const QString &name, const QString &visibleName,
                        const QIcon &icon,
                        QgsCalloutCreateFunc pfCreate,
                        QgsCalloutWidgetFunc pfWidget = nullptr ) SIP_SKIP
  : QgsCalloutAbstractMetadata( name, visibleName, icon )
    , mCreateFunc( pfCreate )
    , mWidgetFunc( pfWidget )
    {}

    //! \note not available in Python bindings
    QgsCalloutCreateFunc createFunction() const SIP_SKIP { return mCreateFunc; }
    //! \note not available in Python bindings
    QgsCalloutWidgetFunc widgetFunction() const SIP_SKIP { return mWidgetFunc; }

    //! \note not available in Python bindings
    void setWidgetFunction( QgsCalloutWidgetFunc f ) SIP_SKIP { mWidgetFunc = f; }

    QgsCallout *createCallout( const QVariantMap &properties, const QgsReadWriteContext &context ) override SIP_FACTORY;
    QgsCalloutWidget *createCalloutWidget( QgsMapLayer *vl ) override SIP_FACTORY;

  protected:
    QgsCalloutCreateFunc mCreateFunc;
    QgsCalloutWidgetFunc mWidgetFunc;

  private:
#ifdef SIP_RUN
    QgsCalloutMetadata();
#endif
};


/**
 * \ingroup core
 * \brief Registry of available callout classes.
 *
 * QgsCalloutRegistry is not usually directly created, but rather accessed through
 * QgsApplication::calloutRegistry().
 *
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsCalloutRegistry
{
  public:

    QgsCalloutRegistry();
    ~QgsCalloutRegistry();

    QgsCalloutRegistry( const QgsCalloutRegistry &rh ) = delete;
    QgsCalloutRegistry &operator=( const QgsCalloutRegistry &rh ) = delete;

    /**
     * Returns the metadata for specified the specified callout \a type. Returns NULLPTR if no matching callout style was found.
     */
    QgsCalloutAbstractMetadata *calloutMetadata( const QString &type ) const;

    /**
     * Registers a new callout type.
     *
     * Ownership of \a metadata is transferred to the registry.
     */
    bool addCalloutType( QgsCalloutAbstractMetadata *metadata SIP_TRANSFER );

    /**
     * Creates a new instance of a callout, given the callout \a type and \a properties.
     *
     * The caller takes ownership of the callout.
     */
    QgsCallout *createCallout( const QString &type, const QVariantMap &properties = QVariantMap(), const QgsReadWriteContext &context = QgsReadWriteContext() ) const SIP_FACTORY;

    /**
     * Creates a new instance of a callout of the specified \a type, using the properties from a DOM \a element.
     *
     * The caller takes ownership of the callout.
     */
    QgsCallout *createCallout( const QString &type, const QDomElement &element, const QgsReadWriteContext &context ) const SIP_FACTORY;

    /**
     * Returns a list of all available callout types.
     */
    QStringList calloutTypes() const;

    /**
     * Create a new instance of a callout with default settings.
     *
     * The caller takes ownership of the callout.
     */
    static QgsCallout *defaultCallout() SIP_FACTORY;

  private:
#ifdef SIP_RUN
    QgsCalloutRegistry( const QgsCalloutRegistry &rh );
#endif

    QMap<QString, QgsCalloutAbstractMetadata *> mMetadata;
};

#endif // QGSCALLOUTSREGISTRY_H
