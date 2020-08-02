/***************************************************************************
  qgsmaterialregistry.h
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

#ifndef QGSMATERIALREGISTRY_H
#define QGSMATERIALREGISTRY_H

#include "qgis_3d.h"
#include "qgis_sip.h"

#include <QDomElement>
#include <QMap>

class QgsAbstractMaterialSettings;
class QgsReadWriteContext;
class QgsMaterialSettingsWidget SIP_EXTERNAL;

/**
 * \ingroup core
 * Stores metadata about one 3D material settings class.
 *
 * \note It's necessary to implement createMaterialSettings() function.
 * In C++ you can use QgsMaterialSettingsMetadata convenience class.
 *
 * \since QGIS 3.16
 */
class _3D_EXPORT QgsMaterialSettingsAbstractMetadata
{
  public:

    /**
     * Constructor for QgsMaterialSettingsAbstractMetadata, with the specified \a type and \a visibleName.
     */
    QgsMaterialSettingsAbstractMetadata( const QString &type, const QString &visibleName )
      : mType( type )
      , mVisibleName( visibleName )
    {}

    virtual ~QgsMaterialSettingsAbstractMetadata() = default;

    /**
     * Returns the unique material type string.
     */
    QString type() const { return mType; }

    /**
     * Returns the material's visible (translated) name.
     */
    QString visibleName() const { return mVisibleName; }

    /**
     * Creates a new instance of this material settings type.
     *
     * Caller takes ownership of the returned object.
     */
    virtual QgsAbstractMaterialSettings *create() = 0 SIP_FACTORY;

#ifndef SIP_RUN

    /**
     * Create a widget for configuring a material of this type.
     *
     * Can return NULLPTR if there's no GUI.
     *
     * \note Not available in Python bindings
     */
    virtual QgsMaterialSettingsWidget *createWidget() SIP_FACTORY { return nullptr; }
#endif

  private:
    QString mType;
    QString mVisibleName;
};

//! Material settings creation function
typedef QgsAbstractMaterialSettings *( *QgsMaterialSettingsCreateFunc )() SIP_SKIP;

//! Material settings widget creation function
typedef QgsMaterialSettingsWidget *( *QgsMaterialSettingsWidgetFunc )() SIP_SKIP;

#ifndef SIP_RUN

/**
 * \ingroup core
 * Convenience metadata class that uses static functions to create a 3D material settings object and its widget.
 *
 * \note Not available in Python bindings.
 *
 * \since QGIS 3.16
 */
class _3D_EXPORT QgsMaterialSettingsMetadata : public QgsMaterialSettingsAbstractMetadata
{
  public:

    /**
     * Constructor for QgsMaterialSettingsMetadata, with the specified \a type and \a visibleName.
     *
     * The \a pfCreate and \a pfWidget arguments are used to specify
     * static functions for creating the material settings type and configuration widget.
     */
    QgsMaterialSettingsMetadata( const QString &type, const QString &visibleName,
                                 QgsMaterialSettingsCreateFunc pfCreate,
                                 QgsMaterialSettingsWidgetFunc pfWidget = nullptr )
      : QgsMaterialSettingsAbstractMetadata( type, visibleName )
      , mCreateFunc( pfCreate )
      , mWidgetFunc( pfWidget )
    {}

    /**
     * Returns the material setting's creation function.
     */
    QgsMaterialSettingsCreateFunc createFunction() const { return mCreateFunc; }

    /**
     * Returns the material settings's widget creation function.
     *
     * \see setWidgetFunction()
     */
    QgsMaterialSettingsWidgetFunc widgetFunction() const { return mWidgetFunc; }

    /**
     * Sets the material settings's widget creation \a function.
     *
     * \see widgetFunction()
     */
    void setWidgetFunction( QgsMaterialSettingsWidgetFunc function ) { mWidgetFunc = function; }

    QgsAbstractMaterialSettings *create() override SIP_FACTORY { return mCreateFunc ? mCreateFunc() : nullptr; }
    QgsMaterialSettingsWidget *createWidget() override SIP_FACTORY { return mWidgetFunc ? mWidgetFunc() : nullptr; }

  private:
    QgsMaterialSettingsCreateFunc mCreateFunc;
    QgsMaterialSettingsWidgetFunc mWidgetFunc;

};
#endif


/**
 * \ingroup core
 * Registry of available 3d material settings classes.
 *
 * QgsMaterialRegistry is not usually directly created, but rather accessed through
 * Qgs3D::materialRegistry().
 *
 * \since QGIS 3.16
 */
class _3D_EXPORT QgsMaterialRegistry
{
  public:

    QgsMaterialRegistry();
    ~QgsMaterialRegistry();

    //! QgsMaterialRegistry cannot be copied.
    QgsMaterialRegistry( const QgsMaterialRegistry &rh ) = delete;
    //! QgsMaterialRegistry cannot be copied.
    QgsMaterialRegistry &operator=( const QgsMaterialRegistry &rh ) = delete;

    //! Returns metadata for specified material settings \a type. Returns NULLPTR if not found
    QgsMaterialSettingsAbstractMetadata *materialSettingsMetadata( const QString &type ) const;

    /**
     * Returns a list of all available material settings types.
     */
    QStringList materialSettingsTypes() const;

    //! Registers a new material settings type. Takes ownership of the \a metadata instance.
    bool addMaterialSettingsType( QgsMaterialSettingsAbstractMetadata *metadata SIP_TRANSFER );

    /**
     * Creates a new instance of the material settings of the specified \a type.
     *
     * The caller takes ownership of the returned object.
     *
     * Returns NULLPTR if the specified type is not found in the registry.
     */
    QgsAbstractMaterialSettings *createMaterialSettings( const QString &type ) const SIP_FACTORY;

  private:
#ifdef SIP_RUN
    QgsMaterialRegistry( const QgsMaterialRegistry &rh );
#endif

    QMap<QString, QgsMaterialSettingsAbstractMetadata *> mMetadata;
};


#endif // QGSMATERIALREGISTRY_H
