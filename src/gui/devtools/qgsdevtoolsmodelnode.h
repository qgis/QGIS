/***************************************************************************
    qgsdevtoolsmodelnode.h
    -------------------------
    begin                : March 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDEVTOOLSMODELNODE_H
#define QGSDEVTOOLSMODELNODE_H

#define SIP_NO_FILE

#include <deque>
#include <memory>

#include "qgis_gui.h"

#include <QColor>
#include <QElapsedTimer>
#include <QUrl>
#include <QVariant>

class QAction;
class QgsDevToolsModelGroup;

/**
 * \ingroup gui
 * \class QgsDevToolsModelNode
 * \brief Base class for nodes in a dev tools model.
 * \since QGIS 4.0
 */
class GUI_EXPORT QgsDevToolsModelNode
{
  public:
    virtual ~QgsDevToolsModelNode();

    /**
     * Returns the node's parent node.
     *
     * If parent is NULLPTR, the node is a root node
     */
    QgsDevToolsModelGroup *parent() { return mParent; }

    /**
     * Returns the node's data for the specified model \a role.
     */
    virtual QVariant data( int role = Qt::DisplayRole ) const = 0;

    /**
     * Returns the number of child nodes owned by this node.
     */
    virtual int childCount() const = 0;

    /**
     * Returns a list of actions relating to the node.
     *
     * The actions should be parented to \a parent.
     */
    virtual QList<QAction *> actions( QObject *parent );

    /**
     * Converts the node's contents to a variant.
     */
    virtual QVariant toVariant() const;

  protected:
    QgsDevToolsModelNode();

  private:
    QgsDevToolsModelGroup *mParent = nullptr;
    friend class QgsDevToolsModelGroup;
};

/**
 * \ingroup gui
 * \class QgsDevToolsModelGroup
 * \brief Base class for dev tools model "group" nodes, which contain children of their own.
 * \since QGIS 4.0
 */
class GUI_EXPORT QgsDevToolsModelGroup : public QgsDevToolsModelNode
{
  public:
    ~QgsDevToolsModelGroup() override;

    /**
     * Constructor for a QgsDevToolsModelGroup. Copy constructor is disabled
     */
    QgsDevToolsModelGroup( QgsDevToolsModelGroup &&other ) noexcept = default;

    /**
     * Assignment operator for QgsDevToolsModelGroup. Copy assignment is disabled
     */
    QgsDevToolsModelGroup &operator=( QgsDevToolsModelGroup &&other ) noexcept = default;

  public:
    /**
     * Adds a \a child node to this node.
     *
     * Returns a pointer to the newly added node.
     */
    QgsDevToolsModelNode *addChild( std::unique_ptr<QgsDevToolsModelNode> child );

    /**
     * Returns the index of the specified \a child node.
     *
     * \warning \a child must be a valid child of this node.
     */
    int indexOf( QgsDevToolsModelNode *child ) const;

    /**
     * Returns the child at the specified \a index.
     */
    QgsDevToolsModelNode *childAt( int index );

    /**
     * Clears the group, removing all its children.
     */
    void clear();

    int childCount() const final { return mChildren.size(); }
    QVariant data( int role = Qt::DisplayRole ) const override;
    QVariant toVariant() const override;

  protected:
    /**
     * Constructor for a QgsDevToolsModelGroup, with the specified \a title.
     */
    QgsDevToolsModelGroup( const QString &title );

    /**
     * Adds a simple \a key: \a value node to the group.
     */
    void addKeyValueNode( const QString &key, const QString &value, const QColor &color = QColor() );

  protected:
    std::deque<std::unique_ptr<QgsDevToolsModelNode>> mChildren;

  private:
    // Prevent copying
    QgsDevToolsModelGroup( const QgsDevToolsModelGroup & ) = delete;
    QgsDevToolsModelGroup &operator=( const QgsDevToolsModelGroup & ) = delete;

    QString mGroupTitle;
};

/**
 * \ingroup gui
 * \class QgsDevToolsModelValueNode
 * \brief A "key: value" style node for a dev tools model.
 * \since QGIS 4.0
 */
class GUI_EXPORT QgsDevToolsModelValueNode : public QgsDevToolsModelNode
{
  public:
    /**
     * Constructor for QgsDevToolsModelValueNode, with the specified \a key (usually translated) and \a value.
     */
    QgsDevToolsModelValueNode( const QString &key, const QString &value, const QColor &color = QColor() );

    /**
     * Returns the node's key.
     */
    QString key() const { return mKey; }

    /**
     * Returns the node's value.
     */
    QString value() const { return mValue; }

    QVariant data( int role = Qt::DisplayRole ) const final;
    int childCount() const final { return 0; }
    QList<QAction *> actions( QObject *parent ) final;

  private:
    QString mKey;
    QString mValue;
    QColor mColor;
};

#endif // QGSDEVTOOLSMODELNODE_H
