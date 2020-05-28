/***************************************************************************
                         qgsstyleentityvisitor.h
                         ---------------
    begin                : July 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSTYLEENTITYVISITOR_H
#define QGSSTYLEENTITYVISITOR_H

#include "qgis_core.h"
#include "qgsstyle.h"

/**
 * \class QgsStyleEntityVisitorInterface
 * \ingroup core
 *
 * An interface for classes which can visit style entity (e.g. symbol) nodes (using the visitor pattern).
 *
 * \since QGIS 3.10
 */

class CORE_EXPORT QgsStyleEntityVisitorInterface
{

  public:

    /**
     * Describes the types of nodes which may be visited by the visitor.
     */
    enum class NodeType : int
    {
      Project, //!< QGIS Project node
      Layer, //!< Map layer
      SymbolRule, //!< Rule based symbology or label child rule
      Layouts, //!< Layout collection
      PrintLayout, //!< An individual print layout
      LayoutItem, //!< Individual item in a print layout
      Report, //!< A QGIS print report
      ReportHeader, //!< Report header section
      ReportFooter, //!< Report footer section
      ReportSection, //!< Report sub section
      Annotations, //!< Annotations collection
      Annotation, //!< An individual annotation
    };

    /**
     * Contains information relating to the style entity currently being visited.
     */
    struct StyleLeaf
    {

      /**
       * Constructor for StyleLeaf, visiting the given style \a entity with the specified \a identifier and \a description.
       *
       * Ownership of \a entity is not transferred.
       */
      StyleLeaf( const QgsStyleEntityInterface *entity, const QString &identifier = QString(), const QString &description = QString() )
        : identifier( identifier )
        , description( description )
        , entity( entity )
      {}

      /**
       * A string identifying the style entity. The actual value of \a identifier will vary
       * depending on the class being visited. E.g for a categorized renderer, the
       * identifier will be the category ID associated with the symbol.
       *
       * This may be blank if no identifier is required, e.g. when a renderer has a single
       * symbol only.
       *
       * Note that in some cases where a specific identifier is not available, a generic, untranslated
       * one may be used (e.g. "overview", "grid").
       */
      QString identifier;

      /**
       * A string describing the style entity. The actual value of \a description will vary
       * depending on the class being visited. E.g for a categorized renderer, the
       * description will be the category label associated with the symbol, for a print layout, it will
       * be the name of the layout in the project.
       *
       * This may be blank if no description is associated with a style entity, e.g. when a renderer has a single
       * symbol only.
       *
       * This value may be a generic, translated value in some cases, e.g. "Grid" or "Overview".
       */
      QString description;

      /**
       * Reference to style entity being visited.
       */
      const QgsStyleEntityInterface *entity = nullptr;
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
      Node( QgsStyleEntityVisitorInterface::NodeType type, const QString &identifier, const QString &description )
        : type( type )
        , identifier( identifier )
        , description( description )
      {}

      /**
       * Node type.
       */
      QgsStyleEntityVisitorInterface::NodeType type = QgsStyleEntityVisitorInterface::NodeType::Project;

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

    virtual ~QgsStyleEntityVisitorInterface() = default;

    /**
     * Called when the visitor will visit a style \a entity.
     *
     * Subclasses should return FALSE to abort further visitations, or TRUE to continue
     * visiting after processing this entity.
     */
    virtual bool visit( const QgsStyleEntityVisitorInterface::StyleLeaf &entity )
    {
      Q_UNUSED( entity )
      return true;
    }

    /**
     * Called when the visitor starts visiting a \a node.
     *
     * Subclasses should return FALSE if they do NOT want to visit this particular node - e.g.
     * if the node type is QgsStyleEntityVisitorInterface::NodeType::Layouts and they do not wish to visit
     * layout objects. In this case the visitor will not process the node, and will move to the next available
     * node instead. Return TRUE to proceed with visiting the node.
     *
     * The default implementation returns TRUE.
     */
    virtual bool visitEnter( const QgsStyleEntityVisitorInterface::Node &node )
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
    virtual bool visitExit( const QgsStyleEntityVisitorInterface::Node &node )
    {
      Q_UNUSED( node )
      return true;
    }

};

#endif // QGSSTYLEENTITYVISITOR_H
