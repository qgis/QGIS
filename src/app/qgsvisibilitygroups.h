/***************************************************************************
  qgsvisibilitygroups.h
  --------------------------------------
  Date                 : September 2014
  Copyright            : (C) 2014 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVISIBILITYGROUPS_H
#define QGSVISIBILITYGROUPS_H

#include <QMap>
#include <QObject>
#include <QSet>
#include <QStringList>

class QAction;
class QDomDocument;
class QMenu;

class QgsLayerTreeNode;
class QgsLayerTreeGroup;

/**
 * Controller class that allows creation of visibility groups consisting of currently visible
 * map layers in map canvas.
 */
class QgsVisibilityGroups : public QObject
{
    Q_OBJECT
  public:

    static QgsVisibilityGroups* instance();

    //! Add a new group using the current state of project's layer tree
    void addGroup( const QString& name );
    //! Update existing group using the current state of project's layer tree
    void updateGroup( const QString& name );
    //! Remove existing group
    void removeGroup( const QString& name );

    //! Remove all groups
    void clear();

    //! Return list of existing group names
    QStringList groups() const;

    //! Return list of layer IDs that should be visible for particular group
    QStringList groupVisibleLayers( const QString& name ) const;

    //! Apply check states of legend nodes of a given layer as defined in the group
    void applyGroupCheckedLegendNodesToLayer( const QString& name, const QString& layerID );

    //! Convenience menu that lists available groups and actions for management
    QMenu* menu();

  signals:
    void groupsChanged();

  protected slots:
    void addGroup();
    void groupTriggerred();
    void removeCurrentGroup();
    void menuAboutToShow();

    void readProject( const QDomDocument& doc );
    void writeProject( QDomDocument& doc );

    void registryLayersRemoved( QStringList layerIDs );

  protected:
    QgsVisibilityGroups(); // singleton

    typedef struct GroupRecord
    {
      bool operator==( const GroupRecord& other ) const
      {
        return mVisibleLayerIDs == other.mVisibleLayerIDs && mPerLayerCheckedLegendSymbols == other.mPerLayerCheckedLegendSymbols;
      }
      bool operator!=( const GroupRecord& other ) const
      {
        return !( *this == other );
      }

      //! List of layers that are visible
      QSet<QString> mVisibleLayerIDs;
      //! For layers that have checkable legend symbols and not all symbols are checked - list which ones are
      QMap<QString, QSet<QString> > mPerLayerCheckedLegendSymbols;
    } GroupRecord;

    typedef QMap<QString, GroupRecord> GroupRecordMap;

    void addVisibleLayersToGroup( QgsLayerTreeGroup* parent, GroupRecord& rec );
    void applyStateToLayerTreeGroup( QgsLayerTreeGroup* parent, const GroupRecord& rec );

    GroupRecord currentState();
    void applyState( const QString& groupName );

    static QgsVisibilityGroups* sInstance;

    GroupRecordMap mGroups;

    QMenu* mMenu;
    QAction* mMenuSeparator;
    QAction* mActionRemoveCurrentGroup;
    QList<QAction*> mMenuGroupActions;
};


#endif // QGSVISIBILITYGROUPS_H
