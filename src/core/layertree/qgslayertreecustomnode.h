/***************************************************************************
                       qgslayertreecustomnode.h
                       ------------------------
    begin                : July 2025
    copyright            : (C) 2025 by Germán Carrillo
    email                : german at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERTREECUSTOMNODE_H
#define QGSLAYERTREECUSTOMNODE_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgslayertreenode.h"

#include <QObject>

/**
 * \ingroup core
 * \brief Layer tree custom node serves as a node for objects that are not layers nor groups.
 *
 * They are created, and can be found based on a node ID, which should be unique in the whole
 * layer tree.
 *
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsLayerTreeCustomNode : public QgsLayerTreeNode
{
    Q_OBJECT
  public:

    /**
     * Constructor to create custom nodes that represent application objects other than layers and groups.
     *
     * \param nodeId    Unique identifier of the custom node.
     * \param nodeName  Optional name of the custom node.
     * \param checked   Whether the custom node should be checked or not once it is created.
     */
    explicit QgsLayerTreeCustomNode( const QString &nodeId, const QString &nodeName = QString(), bool checked = true );

#ifndef SIP_RUN
    QgsLayerTreeCustomNode( const QgsLayerTreeCustomNode &other );
#endif

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = u"<QgsLayerTreeCustomNode: %1>"_s.arg( sipCpp->name() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    /**
     * Returns the node's unique identifier.
     */
    QString nodeId() const { return mId; };

    /**
     * Returns the node's name.
     */
    QString name() const override;

    /**
     * Sets the node's name.
     */
    void setName( const QString &name ) override;

    /**
     * Read custom node from XML element <layer-tree-custom-node> and return the newly created node (or NULLPTR on error).
     */
    static QgsLayerTreeCustomNode *readXml( const QDomElement &element, const QgsReadWriteContext &context ) SIP_FACTORY;  // cppcheck-suppress duplInheritedMember

    /**
     * Write custom node as XML element <layer-tree-custom-node> and add it to the given parent element.
     */
    void writeXml( QDomElement &parentElement, const QgsReadWriteContext &context ) override;

    QString dump() const override;

    QgsLayerTreeCustomNode *clone() const override SIP_FACTORY;

    void resolveReferences( const QgsProject *project, bool looseMatching = false ) override;

  private:
#ifdef SIP_RUN

    /**
     * Copies are not allowed
     */
    QgsLayerTreeCustomNode( const QgsLayerTreeCustomNode &other );
#endif

    QgsLayerTreeCustomNode &operator= ( const QgsLayerTreeCustomNode & ) = delete;

    QString mId;
    QString mName;
};

#endif // QGSLAYERTREECUSTOMNODE_H
