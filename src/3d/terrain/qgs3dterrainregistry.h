/***************************************************************************
  qgs3dterrainregistry.h
  --------------------------------------
  Date                 : November 2024
  Copyright            : (C) 2024 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DTERRAINREGISTRY_H
#define QGS3DTERRAINREGISTRY_H

#include "qgis_3d.h"
#include "qgis_sip.h"
#include <QString>

#include <QDomElement>
#include <QMap>
#include <QIcon>

class QgsReadWriteContext;
class QgsAbstractTerrainSettings;
class QgsTerrainGenerator;
class QgsProjectElevationProperties;
class QgsRectangle;

/**
 * \ingroup core
 * \brief Stores metadata about one 3D terrain class.
 *
 * \note It's necessary to implement createTerrainSettings() function.
 * In C++ you can use Qgs3DTerrainMetadata convenience class.
 *
 * \since QGIS 3.42
 */
class _3D_EXPORT Qgs3DTerrainAbstractMetadata
{
  public:
    /**
     * Constructor for Qgs3DTerrainAbstractMetadata, with the specified \a type and \a visibleName.
     *
     * An optional \a icon can be specified to represent the material type.
     */
    Qgs3DTerrainAbstractMetadata( const QString &type, const QString &visibleName, const QIcon &icon = QIcon() )
      : mType( type )
      , mVisibleName( visibleName )
      , mIcon( icon )
    {}

    virtual ~Qgs3DTerrainAbstractMetadata() = default;

    /**
     * Returns the unique terrain type string.
     */
    QString type() const { return mType; }

    /**
     * Returns the terrain's visible (translated) name.
     */
    QString visibleName() const { return mVisibleName; }

    /**
     * Returns an icon representing the terrain type, if available.
     */
    QIcon icon() const { return mIcon; }

    /**
     * Creates a new instance of this terrain settings type.
     *
     * Caller takes ownership of the returned object.
     */
    virtual QgsAbstractTerrainSettings *createTerrainSettings() = 0 SIP_FACTORY;

    /**
     * Creates a new instance of this terrain generator type.
     *
     * Caller takes ownership of the returned object.
     *
     * \note Not available in Python bindings
     */
    SIP_SKIP virtual QgsTerrainGenerator *createTerrainGenerator() { return nullptr; }

  private:
    QString mType;
    QString mVisibleName;
    QIcon mIcon;
};

//! Terrain settings creation function
SIP_SKIP typedef QgsAbstractTerrainSettings *( *QgsTerrainSettingsCreateFunc )();
//! Terrain generator creation function
SIP_SKIP typedef QgsTerrainGenerator *( *QgsTerrainGeneratorCreateFunc )();

#ifndef SIP_RUN

/**
 * \ingroup core
 * \brief Convenience metadata class that uses static functions to create 3D terrain objects.
 *
 * \note Not available in Python bindings.
 *
 * \since QGIS 3.42
 */
class _3D_EXPORT Qgs3DTerrainMetadata : public Qgs3DTerrainAbstractMetadata
{
  public:
    /**
     * Constructor for Qgs3DTerrainMetadata, with the specified \a type and \a visibleName.
     *
     * The \a pfSettingsCreate and \a pfGeneratorCreate arguments are used to specify
     * static functions for creating the terrain objects.
     *
     * An optional \a icon can be specified to represent the terrain type.
     */
    Qgs3DTerrainMetadata( const QString &type, const QString &visibleName, QgsTerrainSettingsCreateFunc pfSettingsCreate, QgsTerrainGeneratorCreateFunc pfGeneratorCreate, const QIcon &icon = QIcon() )
      : Qgs3DTerrainAbstractMetadata( type, visibleName, icon )
      , mCreateFunc( pfSettingsCreate )
      , mGeneratorCreateFunc( pfGeneratorCreate )
    {}

    /**
     * Returns the terrain setting's creation function.
     */
    QgsTerrainSettingsCreateFunc createSettingsFunction() const { return mCreateFunc; }

    /**
     * Returns the terrain generator creation function.
     */
    QgsTerrainGeneratorCreateFunc createGeneratorFunction() const { return mGeneratorCreateFunc; }

    QgsAbstractTerrainSettings *createTerrainSettings() override SIP_FACTORY { return mCreateFunc ? mCreateFunc() : nullptr; }
    QgsTerrainGenerator *createTerrainGenerator() override { return mGeneratorCreateFunc ? mGeneratorCreateFunc() : nullptr; }

  private:
    QgsTerrainSettingsCreateFunc mCreateFunc;
    QgsTerrainGeneratorCreateFunc mGeneratorCreateFunc;
};
#endif


/**
 * \ingroup core
 * \brief Registry of available 3d terrain classes.
 *
 * Qgs3DTerrainRegistry is not usually directly created, but rather accessed through
 * Qgs3D::terrainRegistry().
 *
 * \since QGIS 3.42
 */
class _3D_EXPORT Qgs3DTerrainRegistry
{
  public:
    Qgs3DTerrainRegistry();
    ~Qgs3DTerrainRegistry();

    Qgs3DTerrainRegistry( const Qgs3DTerrainRegistry &rh ) = delete;
    Qgs3DTerrainRegistry &operator=( const Qgs3DTerrainRegistry &rh ) = delete;

    //! Returns metadata for specified terrain \a type. Returns NULLPTR if not found
    Qgs3DTerrainAbstractMetadata *terrainMetadata( const QString &type ) const;

    /**
     * Returns a list of all available terrain types.
     */
    QStringList types() const;

    //! Registers a new terrain type. Takes ownership of the \a metadata instance.
    bool addType( Qgs3DTerrainAbstractMetadata *metadata SIP_TRANSFER );

    /**
     * Creates a new instance of the terrain settings of the specified \a type.
     *
     * The caller takes ownership of the returned object.
     *
     * Returns NULLPTR if the specified type is not found in the registry.
     */
    QgsAbstractTerrainSettings *createTerrainSettings( const QString &type ) const SIP_FACTORY;

    /**
     * Creates a new instance of the terrain generator of the specified \a type.
     *
     * The caller takes ownership of the returned object.
     *
     * Returns NULLPTR if the specified type is not found in the registry.
     *
     * \note Not available in Python bindings
     */
    SIP_SKIP QgsTerrainGenerator *createTerrainGenerator( const QString &type ) const;

    /**
     * Create terrain settings directly from a project's elevation \a properties.
     */
    QgsAbstractTerrainSettings *configureTerrainFromProject( QgsProjectElevationProperties *properties ) SIP_FACTORY;

  private:
#ifdef SIP_RUN
    Qgs3DTerrainRegistry( const Qgs3DTerrainRegistry &rh );
#endif

    QMap<QString, Qgs3DTerrainAbstractMetadata *> mMetadata;
    //! List of terrains, maintained in the order that they have been added
    QStringList mTerrainOrder;
};


#endif // QGS3DTERRAINREGISTRY_H
