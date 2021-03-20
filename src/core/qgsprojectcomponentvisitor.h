/***************************************************************************
                         qgsprojectcomponentvisitor.h
                         ---------------
    begin                : June 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 * -
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 * -
 ***************************************************************************/

#ifndef QGSPROJECTCOMPONENTVISITOR_H
#define QGSPROJECTCOMPONENTVISITOR_H

#include "qgis_core.h"
#include <QString>

class QgsProjectComponentInterface;

/**
 * \class QgsComponentEntityVisitorInterface
 * \ingroup core
 *
 * An interface for classes which can visit project components (e.g. symbols, expressions, etc) using the visitor pattern.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsProjectComponentVisitorInterface
{

  public:

    /**
     * Describes the types of nodes which may be visited by the visitor.
     */
    enum class NodeType : int
    {
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

      Other, //!< Other/unclassified type
    };

    /**
     * Contains information relating to the project component currently being visited.
     */
    struct ComponentLeaf
    {

      /**
       * Constructor for ComponentLeaf, visiting the given \a component with the specified \a identifier and \a description.
       *
       * Ownership of \a component is not transferred.
       */
      ComponentLeaf( const QgsProjectComponentInterface *component, const QString &identifier = QString(), const QString &description = QString() )
        : identifier( identifier )
        , description( description )
        , component( component )
      {}

      /**
       * A string identifying the project component. The actual value of \a identifier will vary
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
       * A string describing the project copmonent. The actual value of \a description will vary
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
       * Reference to component being visited.
       */
      const QgsProjectComponentInterface *component = nullptr;
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
      Node( QgsProjectComponentVisitorInterface::NodeType type, const QString &identifier, const QString &description )
        : type( type )
        , identifier( identifier )
        , description( description )
      {}

      /**
       * Node type.
       */
      QgsProjectComponentVisitorInterface::NodeType type = QgsProjectComponentVisitorInterface::NodeType::Other;

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

    virtual ~QgsProjectComponentVisitorInterface() = default;

    /**
     * Called when the visitor will visit a project \a component.
     *
     * Subclasses should return FALSE to abort further visitations, or TRUE to continue
     * visiting after processing this component.
     */
    virtual bool visit( const QgsProjectComponentVisitorInterface::ComponentLeaf &component )
    {
      Q_UNUSED( component )
      return true;
    }

    /**
     * Called when the visitor starts visiting a \a node.
     *
     * Subclasses should return FALSE if they do NOT want to visit this particular node - e.g.
     * if the node type is QgsProjectComponentVisitorInterface::NodeType::Layouts and they do not wish to visit
     * layout objects. In this case the visitor will not process the node, and will move to the next available
     * node instead. Return TRUE to proceed with visiting the node.
     *
     * The default implementation returns TRUE.
     */
    virtual bool visitEnter( const QgsProjectComponentVisitorInterface::Node &node )
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
    virtual bool visitExit( const QgsProjectComponentVisitorInterface::Node &node )
    {
      Q_UNUSED( node )
      return true;
    }

};

#endif // QGSPROJECTCOMPONENTVISITOR_H
