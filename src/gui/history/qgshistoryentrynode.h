/***************************************************************************
                            qgshistoryentrynode.h
                            --------------------------
    begin                : April 2023
    copyright            : (C) 2023 by Nyall Dawson
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
#ifndef QGSHISTORYENTRYNODE_H
#define QGSHISTORYENTRYNODE_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgis.h"
#include <QString>
#include <QVariantMap>
#include <memory>
#include <deque>

class QWidget;
class QAction;
class QMenu;
class QgsHistoryEntryGroup;
class QgsHistoryWidgetContext;

/**
 * Base class for nodes representing a QgsHistoryEntry.
 *
 * \ingroup gui
 * \since QGIS 3.32
 */
class GUI_EXPORT QgsHistoryEntryNode
{
  public:
    QgsHistoryEntryNode() = default;
    virtual ~QgsHistoryEntryNode();

    QgsHistoryEntryNode( const QgsHistoryEntryNode &other ) = delete;
    QgsHistoryEntryNode &operator=( const QgsHistoryEntryNode &other ) = delete;

    /**
     * Returns the node's parent node.
     *
     * If parent is NULLPTR, the node is a root node.
     */
    QgsHistoryEntryGroup *parent() { return mParent; }

    /**
     * Returns the node's data for the specified model \a role.
     */
    virtual QVariant data( int role = Qt::DisplayRole ) const = 0;

    /**
     * Returns the number of child nodes owned by this node.
     */
    virtual int childCount() const;

    /**
     * Returns a HTML formatted text string which should be shown to a user when
     * selecting the node.
     *
     * Subclasses should implement this method or createWidget(), but not both.
     *
     * \see createWidget()
     */
    virtual QString html( const QgsHistoryWidgetContext &context ) const;

    /**
     * Returns a new widget which should be shown to users when selecting the node.
     *
     * If a NULLPTR is returned, the node's html() method will be called instead to
     * create the node's content.
     *
     * \see html()
     */
    virtual QWidget *createWidget( const QgsHistoryWidgetContext &context ) SIP_FACTORY;

    /**
     * Called when the node is double-clicked. The default implementation does nothing.
     *
     * Returns TRUE if the node handled the double-click event and it should not
     * be further processed.
     */
    virtual bool doubleClicked( const QgsHistoryWidgetContext &context );

    /**
     * Allows the node to populate a context \a menu before display to the user.
     *
     * Actions should be parented to the specified \a menu.
     */
    virtual void populateContextMenu( QMenu *menu, const QgsHistoryWidgetContext &context );

    /**
     * Returns true if the node matches the specified \a searchString, and
     * should be shown in filtered results with that search string.
     *
     * The default implementation returns TRUE if the string is contained
     * within the node's DisplayRole.
     */
    virtual bool matchesString( const QString &searchString ) const;

  private:
#ifdef SIP_RUN
    QgsHistoryEntryNode( const QgsHistoryEntryNode &other );
#endif

    QgsHistoryEntryGroup *mParent = nullptr;

    friend class QgsHistoryEntryGroup;
};


/**
 * \ingroup gui
 * \class QgsHistoryEntryGroup
 * \brief Base class for history entry "group" nodes, which contain children of their own.
 * \since QGIS 3.32
 */
class GUI_EXPORT QgsHistoryEntryGroup : public QgsHistoryEntryNode
{
  public:
    QgsHistoryEntryGroup() = default;
    ~QgsHistoryEntryGroup() override;

    QgsHistoryEntryGroup( const QgsHistoryEntryGroup &other ) = delete;
    QgsHistoryEntryGroup &operator=( const QgsHistoryEntryGroup &other ) = delete;

    /**
     * Adds a \a child node to this node.
     *
     * Ownership is transferred to the group.
     */
    void addChild( QgsHistoryEntryNode *child SIP_TRANSFER );

    /**
     * Inserts a \a child node at the specified index.
     *
     * Ownership is transferred to the group.
     */
    void insertChild( int index, QgsHistoryEntryNode *child SIP_TRANSFER );

    /**
     * Returns the index of the specified \a child node.
     *
     * \warning \a child must be a valid child of this node.
     */
    int indexOf( QgsHistoryEntryNode *child ) const;

    /**
     * Returns the child at the specified \a index.
     */
    QgsHistoryEntryNode *childAt( int index );

    /**
     * Removes the child at the specified \a index.
     */
    void removeChildAt( int index );

    /**
     * Clears the group, removing all its children.
     */
    void clear();

    int childCount() const FINAL;

  protected:
    std::deque<std::unique_ptr<QgsHistoryEntryNode>> mChildren SIP_SKIP;

  private:
#ifdef SIP_RUN
    QgsHistoryEntryGroup( const QgsHistoryEntryGroup &other );
#endif
};

#endif // QGSHISTORYENTRYNODE_H
