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
#include <QStringList>

class QAction;
class QDomDocument;
class QMenu;

class QgsLayerTreeNode;
class QgsLayerTreeGroup;

class QgsVisibilityGroups : public QObject
{
    Q_OBJECT
  public:

    static QgsVisibilityGroups* instance();

    void addGroup( const QString& name );
    void updateGroup( const QString& name );
    void removeGroup( const QString& name );

    void clear();

    QStringList groups() const;

    QMenu* menu();

  signals:
    void groupsChanged();

  protected slots:
    void addGroup();
    void groupTriggerred();
    void removeCurrentGroup();
    void menuAboutToShow();
    void layerTreeVisibilityChanged( QgsLayerTreeNode* node, Qt::CheckState state );
    void layerTreeAddedChildren( QgsLayerTreeNode* node, int indexFrom, int indexTo );
    void layerTreeWillRemoveChildren( QgsLayerTreeNode* node, int indexFrom, int indexTo );

    void readProject( const QDomDocument& doc );
    void writeProject( QDomDocument& doc );

  protected:
    QgsVisibilityGroups(); // singleton

    typedef struct GroupRecord
    {
      bool operator==( const GroupRecord& other ) const
      {
        return mVisibleLayerIDs == other.mVisibleLayerIDs;
      }

      //! List of layers that are visible
      QStringList mVisibleLayerIDs;
      //! For layers that have checkable legend symbols and not all symbols are checked - list which ones are
      //QMap<QString, QStringList> mPerLayerCheckedLegendSymbols;
    } GroupRecord;

    typedef QMap<QString, GroupRecord> GroupRecordMap;

    void addVisibleLayersToGroup( QgsLayerTreeGroup* parent, GroupRecord& rec );
    void applyStateToLayerTreeGroup( QgsLayerTreeGroup* parent, const QSet<QString>& visibleLayerIDs );

    GroupRecord currentState();
    void applyState( const QString& groupName );

    static QgsVisibilityGroups* sInstance;

    GroupRecordMap mGroups;

    QMenu* mMenu;
    bool mMenuDirty;
    QAction* mMenuSeparator;
    QAction* mActionRemoveCurrentGroup;
    QList<QAction*> mMenuGroupActions;
};


#endif // QGSVISIBILITYGROUPS_H
