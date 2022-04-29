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

#include <QElapsedTimer>
#include <QVariant>
#include <QColor>
#include <QUrl>
#include <memory>
#include <deque>

class QAction;
class QgsDevToolsModelGroup;

/**
 * \ingroup app
 * \class QgsDevToolsModelNode
 * \brief Base class for nodes in a dev tools model.
 */
class QgsDevToolsModelNode
{
  public:

    //! Custom node data roles
    enum Roles
    {
      RoleStatus = Qt::UserRole + 1, //!< Request status role
      RoleId, //!< Request ID role
    };

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
    virtual QList< QAction * > actions( QObject *parent );

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
 * \ingroup app
 * \class QgsDevToolsModelGroup
 * \brief Base class for dev tools model "group" nodes, which contain children of their own.
 */
class QgsDevToolsModelGroup : public QgsDevToolsModelNode
{
  public:

    /**
     * Adds a \a child node to this node.
     */
    void addChild( std::unique_ptr< QgsDevToolsModelNode > child );

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

    int childCount() const override final { return mChildren.size(); }
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
    std::deque< std::unique_ptr< QgsDevToolsModelNode > > mChildren;

  private:

    QString mGroupTitle;

};

/**
 * \ingroup app
 * \class QgsDevToolsModelValueNode
 * \brief A "key: value" style node for a dev tools model.
 */
class QgsDevToolsModelValueNode : public QgsDevToolsModelNode
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

    QVariant data( int role = Qt::DisplayRole ) const override final;
    int childCount() const override final { return 0; }
    QList< QAction * > actions( QObject *parent ) override final;

  private:

    QString mKey;
    QString mValue;
    QColor mColor;
};

#endif // QGSDEVTOOLSMODELNODE_H
