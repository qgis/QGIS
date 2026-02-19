/***************************************************************************
                         qgsobjectvisitor.h
                         ---------------
    begin                : October 2025
    copyright            : (C) 2025 by Mathieu Pellerin
    email                : mathieu at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOBJECTVISITOR_H
#define QGSOBJECTVISITOR_H

#include "qgis.h"
#include "qgis_core.h"

/**
 * \class QgsEmbeddedScriptEntity
 * \ingroup core
 * \brief A embedded script entity for QgsObjectEntityVisitorInterface.
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsEmbeddedScriptEntity
{
  public:

    /**
     * Constructor for QgsEmbeddedScriptEntity.
     */
    QgsEmbeddedScriptEntity( Qgis::EmbeddedScriptType type, const QString &name, const QString &script )
      : mType( type )
      , mName( name )
      , mScript( script )
    {}

    /**
     * Returns the entity's embedded script type.
     */
    Qgis::EmbeddedScriptType type() const { return mType; }

    /**
     * Returns the entity's name.
     */
    QString name() const { return mName; }

    /**
     * Returns the entity's script
     */
    QString script() const { return mScript; }

  private:

    Qgis::EmbeddedScriptType mType;
    QString mName;
    QString mScript;

};


/**
 * \class QgsObjectVisitorContext
 * \ingroup core
 *
 * \brief A QgsObjectEntityVisitorInterface context object.
 *
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsObjectVisitorContext
{
  public:

    QgsObjectVisitorContext() = default;
};


/**
 * \class QgsObjectEntityVisitorInterface
 * \ingroup core
 *
 * \brief An interface for classes which can visit various object entity (e.g. embedded script) nodes using the visitor pattern.
 *
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsObjectEntityVisitorInterface
{

  public:

    virtual ~QgsObjectEntityVisitorInterface() = default;

    /**
     * Called when the visitor will visit an embedded script \a entity.
     *
     * Subclasses should return FALSE to abort further visitations, or TRUE to continue
     * visiting after processing this entity.
     */
    virtual bool visitEmbeddedScript( const QgsEmbeddedScriptEntity &entity, const QgsObjectVisitorContext &context )
    {
      Q_UNUSED( entity )
      Q_UNUSED( context )
      return true;
    }

};


/**
 * \class QgsEmbeddedScriptVisitor
 * \ingroup core
 * \brief An object entity visitor to collect embedded scripts wthin a project and its layers.
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsEmbeddedScriptVisitor : public QgsObjectEntityVisitorInterface
{
  public:

    bool visitEmbeddedScript( const QgsEmbeddedScriptEntity &entity, const QgsObjectVisitorContext & ) override
    {
      mEmbeddedScripts << entity;
      return true;
    }

    //! Returns the collected embedded scripts.
    const QList<QgsEmbeddedScriptEntity> &embeddedScripts() const { return mEmbeddedScripts; }

  private:
    QList<QgsEmbeddedScriptEntity> mEmbeddedScripts;
};

#endif // QGSOBJECTVISITOR_H
