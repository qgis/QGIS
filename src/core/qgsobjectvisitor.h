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

#include "qgis_core.h"
#include "qgis.h"


/**
 * \class QgsObjectEntityInterface
 * \ingroup core
 * \brief An interface for entities to be used with the QgsObjectEntityVisitorInterface.
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsObjectEntityInterface
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    switch ( sipCpp->type() )
    {
      case QgsObjectEntityInterface::EmbeddedScript:
        sipType = sipType_QgsEmbeddedScriptEntity;
        break;
    }
    SIP_END
#endif

  public:

    /**
     * Describes the types of nodes which may be visited by the visitor.
     */
    enum class Type : int
    {
      EmbeddedScript, //!< Embedded script within a project, map layer, etc.
    };

    virtual ~QgsObjectEntityInterface() = default;

    /**
     * Returns the type of object entity.
     */
    virtual QgsObjectEntityInterface::Type type() const = 0;

};

/**
 * \class QgsEmbeddedScriptEntity
 * \ingroup core
 * \brief A embedded script entity for QgsObjectEntityVisitorInterface.
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsEmbeddedScriptEntity : public QgsObjectEntityInterface
{
  public:

    /**
     * Constructor for QgsEmbeddedScriptEntity.
     */
    QgsEmbeddedScriptEntity( Qgis::EmbeddedScriptType embeddedScriptType, const QString &name, const QString &script )
      : mEmbeddedScriptType( embeddedScriptType )
      , mName( name )
      , mScript( script )
    {}

    QgsObjectEntityInterface::Type type() const override;

    /**
     * Returns the entity's embedded script type.
     */
    Qgis::EmbeddedScriptType embeddedScriptType() const { return mEmbeddedScriptType; }

    /**
     * Returns the entity's name.
     */
    Qgis::EmbeddedScriptType name() const { return mName; }

    /**
     * Returns the entity's script
     */
    Qgis::EmbeddedScriptType script() const { return mScript; }

  private:

    Qgis::EmbeddedScriptType mEmbeddedScriptType;
    QString mName;
    QString mScript;

};


/**
 * \class QgsObjectEntityVisitorInterface
 * \ingroup core
 *
 * \brief An interface for classes which can visit object entity (e.g. embedded script) nodes using the visitor pattern.
 *
 * \since QGIS 4.0
 */

class CORE_EXPORT QgsObjectEntityVisitorInterface
{

  public:

    /**
     * Describes the types of nodes which may be visited by the visitor.
     */
    enum class NodeType : int
    {
      Project, //!< QGIS Project
      Layer,   //!< Map layer
    };

    /**
     * Contains information relating to the object entity currently being visited.
     */
    struct ObjectLeaf
    {

      /**
       * Constructor for ObjectLeaf, visiting the given object \a entity with the specified \a identifier and \a description.
       *
       * Ownership of \a entity is not transferred.
       */
      ObjectLeaf( const QgsObjectEntityInterface *entity, const QString &identifier = QString(), const QString &description = QString() )
        : identifier( identifier )
        , description( description )
        , entity( entity )
      {}

      /**
       * A string identifying the object entity. The actual value of \a identifier will vary
       * depending on the class being visited.
       *
       * This may be blank if no identifier is required.
       */
      QString identifier;

      /**
       * A string describing the object entity. The actual value of \a description will vary
       * depending on the class being visited.
       *
       * This may be blank if no description is required.
       */
      QString description;

      /**
       * Reference to object entity being visited.
       */
      const QgsObjectEntityInterface *entity = nullptr;
    };

    /**
     * Contains information relating to a node (i.e. a group of symbols or other nodes)
     * being visited.
     */
    struct Node
    {

      /**
       * Constructor for Node, visiting the node with the specified \a identifier and \a description.
       */
      Node( QgsObjectEntityVisitorInterface::NodeType type, const QString &identifier, const QString &description )
        : type( type )
        , identifier( identifier )
        , description( description )
      {}

      /**
       * Node type.
       */
      QgsObjectEntityVisitorInterface::NodeType type = QgsObjectEntityVisitorInterface::NodeType::Project;

      /**
       * A string identifying the node. The actual value of \a identifier will vary
       * depending on the node being visited. E.g for a rule based renderer, the
       * identifier will be a rule ID. For a project, node identifiers will be
       * layer IDs.
       */
      QString identifier;

      /**
       * A string describing the node. The actual value of \a description will vary
       * depending on the node being visited. E.g for a rule based renderer, the
       * identifier will be a rule label. For a project, node identifiers will be
       * layer names.
       */
      QString description;

    };

    virtual ~QgsObjectEntityVisitorInterface() = default;

    /**
     * Called when the visitor will visit a object \a entity.
     *
     * Subclasses should return FALSE to abort further visitations, or TRUE to continue
     * visiting after processing this entity.
     */
    virtual bool visit( const QgsObjectEntityVisitorInterface::ObjectLeaf &entity )
    {
      Q_UNUSED( entity )
      return true;
    }

    /**
     * Called when the visitor starts visiting a \a node.
     *
     * Subclasses should return FALSE if they do NOT want to visit this particular node.
     * Return TRUE to proceed with visiting the node.
     *
     * The default implementation returns TRUE.
     */
    virtual bool visitEnter( const QgsObjectEntityVisitorInterface::Node &node )
    {
      Q_UNUSED( node )
      return true;
    }

    /**
     * Called when the visitor stops visiting a \a node.
     *
     * Subclasses should return FALSE to abort further visitations, or TRUE to continue
     * visiting other nodes.
     *
     * The default implementation returns TRUE.
     */
    virtual bool visitExit( const QgsObjectEntityVisitorInterface::Node &node )
    {
      Q_UNUSED( node )
      return true;
    }

};

#endif // QGSOBJECTVISITOR_H
