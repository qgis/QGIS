/***************************************************************************
    QgsDatabaseQueryLoggernode.h
    -------------------------
    begin                : October 2021
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QgsDatabaseQueryLoggerNODE_H
#define QgsDatabaseQueryLoggerNODE_H

#include <QElapsedTimer>
#include <QVariant>
#include <QColor>
#include <QUrl>
#include <memory>
#include <deque>

class QAction;
class QgsDatabaseQueryLoggerGroup;

/**
 * \ingroup app
 * \class QgsDatabaseQueryLoggerNode
 * \brief Base class for nodes in the query logger model.
 */
class QgsDatabaseQueryLoggerNode
{
  public:

    //! Custom node data roles
    enum Roles
    {
      RoleStatus = Qt::UserRole + 1, //!< Request status role
      RoleId, //!< Request ID role
    };

    virtual ~QgsDatabaseQueryLoggerNode();

    /**
     * Returns the node's parent node.
     *
     * If parent is NULLPTR, the node is a root node
     */
    QgsDatabaseQueryLoggerGroup *parent() { return mParent; }

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

    QgsDatabaseQueryLoggerNode();

  private:

    QgsDatabaseQueryLoggerGroup *mParent = nullptr;
    friend class QgsDatabaseQueryLoggerGroup;
};

/**
 * \ingroup app
 * \class QgsDatabaseQueryLoggerGroup
 * \brief Base class for query logger model "group" nodes, which contain children of their own.
 */
class QgsDatabaseQueryLoggerGroup : public QgsDatabaseQueryLoggerNode
{
  public:

    /**
     * Adds a \a child node to this node.
     */
    void addChild( std::unique_ptr< QgsDatabaseQueryLoggerNode > child );

    /**
     * Returns the index of the specified \a child node.
     *
     * \warning \a child must be a valid child of this node.
     */
    int indexOf( QgsDatabaseQueryLoggerNode *child ) const;

    /**
     * Returns the child at the specified \a index.
     */
    QgsDatabaseQueryLoggerNode *childAt( int index );

    /**
     * Clears the group, removing all its children.
     */
    void clear();

    int childCount() const override final { return mChildren.size(); }
    QVariant data( int role = Qt::DisplayRole ) const override;
    QVariant toVariant() const override;

//  protected:

    /**
     * Constructor for a QgsDatabaseQueryLoggerGroup, with the specified \a title.
     */
    QgsDatabaseQueryLoggerGroup( const QString &title );

    /**
     * Adds a simple \a key: \a value node to the group.
     */
    void addKeyValueNode( const QString &key, const QString &value, const QColor &color = QColor() );

  private:

    std::deque< std::unique_ptr< QgsDatabaseQueryLoggerNode > > mChildren;
    QString mGroupTitle;
    friend class QgsDatabaseQueryLoggerRootNode;
    friend class QgsAppQueryLogger;

};

/**
 * \ingroup app
 * \class QgsDatabaseQueryLoggerValueNode
 * \brief A "key: value" style node for the query logger model.
 */
class QgsDatabaseQueryLoggerValueNode : public QgsDatabaseQueryLoggerNode
{
  public:

    /**
     * Constructor for QgsDatabaseQueryLoggerValueNode, with the specified \a key (usually translated) and \a value.
     */
    QgsDatabaseQueryLoggerValueNode( const QString &key, const QString &value, const QColor &color = QColor() );

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

/**
 * \ingroup app
 * \class QgsDatabaseQueryLoggerRootNode
 * \brief Root node for the query logger model.
 */
class QgsDatabaseQueryLoggerRootNode final : public QgsDatabaseQueryLoggerGroup
{
  public:

    QgsDatabaseQueryLoggerRootNode();
    QVariant data( int role = Qt::DisplayRole ) const override final;

    /**
     * Removes a \a row from the root group.
     */
    void removeRow( int row );

    QVariant toVariant() const override;
};


#endif // QgsDatabaseQueryLoggerNODE_H
