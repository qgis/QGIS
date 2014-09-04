/***************************************************************************
  qgsvisibilitygroups.cpp
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

#include "qgsvisibilitygroups.h"

#include "qgslayertree.h"
#include "qgsproject.h"
#include "qgisapp.h"

#include <QInputDialog>


QgsVisibilityGroups* QgsVisibilityGroups::sInstance;


QgsVisibilityGroups::QgsVisibilityGroups()
    : mMenu( new QMenu )
    , mMenuDirty( false )
{

  mMenu->addAction( QgisApp::instance()->actionShowAllLayers() );
  mMenu->addAction( QgisApp::instance()->actionHideAllLayers() );
  mMenu->addSeparator();

  mMenu->addAction( tr( "Add group..." ), this, SLOT( addGroup() ) );
  mMenuSeparator = mMenu->addSeparator();

  mActionRemoveCurrentGroup = mMenu->addAction( tr( "Remove current group" ), this, SLOT( removeCurrentGroup() ) );

  connect( mMenu, SIGNAL( aboutToShow() ), this, SLOT( menuAboutToShow() ) );

  QgsLayerTreeGroup* root = QgsProject::instance()->layerTreeRoot();
  connect( root, SIGNAL( visibilityChanged( QgsLayerTreeNode*, Qt::CheckState ) ),
           this, SLOT( layerTreeVisibilityChanged( QgsLayerTreeNode*, Qt::CheckState ) ) );
  connect( root, SIGNAL( addedChildren( QgsLayerTreeNode*, int, int ) ),
           this, SLOT( layerTreeAddedChildren( QgsLayerTreeNode*, int, int ) ) );
  connect( root, SIGNAL( willRemoveChildren( QgsLayerTreeNode*, int, int ) ),
           this, SLOT( layerTreeWillRemoveChildren( QgsLayerTreeNode*, int, int ) ) );

  connect( QgsProject::instance(), SIGNAL( readProject( const QDomDocument & ) ),
           this, SLOT( readProject( const QDomDocument & ) ) );
  connect( QgsProject::instance(), SIGNAL( writeProject( QDomDocument & ) ),
           this, SLOT( writeProject( QDomDocument & ) ) );
}

void QgsVisibilityGroups::addVisibleLayersToGroup( QgsLayerTreeGroup* parent, QgsVisibilityGroups::GroupRecord& rec )
{
  foreach ( QgsLayerTreeNode* node, parent->children() )
  {
    if ( QgsLayerTree::isGroup( node ) )
      addVisibleLayersToGroup( QgsLayerTree::toGroup( node ), rec );
    else if ( QgsLayerTree::isLayer( node ) )
    {
      QgsLayerTreeLayer* nodeLayer = QgsLayerTree::toLayer( node );
      if ( nodeLayer->isVisible() )
        rec.mVisibleLayerIDs << nodeLayer->layerId();
    }
  }
}

QgsVisibilityGroups::GroupRecord QgsVisibilityGroups::currentState()
{
  GroupRecord rec;
  QgsLayerTreeGroup* root = QgsProject::instance()->layerTreeRoot();
  addVisibleLayersToGroup( root, rec );
  return rec;
}


QgsVisibilityGroups* QgsVisibilityGroups::instance()
{
  if ( !sInstance )
    sInstance = new QgsVisibilityGroups();

  return sInstance;
}

void QgsVisibilityGroups::addGroup( const QString& name )
{
  mGroups.insert( name, currentState() );

  mMenuDirty = true;
}

void QgsVisibilityGroups::updateGroup( const QString& name )
{
  if ( !mGroups.contains( name ) )
    return;

  mGroups[name] = currentState();

  mMenuDirty = true;
}

void QgsVisibilityGroups::removeGroup( const QString& name )
{
  mGroups.remove( name );

  mMenuDirty = true;
}

void QgsVisibilityGroups::clear()
{
  mGroups.clear();

  mMenuDirty = true;
}

QStringList QgsVisibilityGroups::groups() const
{
  return mGroups.keys();
}

QMenu* QgsVisibilityGroups::menu()
{
  return mMenu;
}


void QgsVisibilityGroups::addGroup()
{
  bool ok;
  QString name = QInputDialog::getText( 0, tr( "Visibility groups" ), tr( "Name of the new group" ), QLineEdit::Normal, QString(), &ok );
  if ( !ok && name.isEmpty() )
    return;

  addGroup( name );
}


void QgsVisibilityGroups::groupTriggerred()
{
  QAction* actionGroup = qobject_cast<QAction*>( sender() );
  if ( !actionGroup )
    return;

  applyState( actionGroup->text() );
}


void QgsVisibilityGroups::applyStateToLayerTreeGroup( QgsLayerTreeGroup* parent, const QSet<QString>& visibleLayerIDs )
{
  foreach ( QgsLayerTreeNode* node, parent->children() )
  {
    if ( QgsLayerTree::isGroup( node ) )
      applyStateToLayerTreeGroup( QgsLayerTree::toGroup( node ), visibleLayerIDs );
    else if ( QgsLayerTree::isLayer( node ) )
    {
      QgsLayerTreeLayer* nodeLayer = QgsLayerTree::toLayer( node );
      nodeLayer->setVisible( visibleLayerIDs.contains( nodeLayer->layerId() ) ? Qt::Checked : Qt::Unchecked );
    }
  }
}


void QgsVisibilityGroups::applyState( const QString& groupName )
{
  if ( !mGroups.contains( groupName ) )
    return;

  const GroupRecord& rec = mGroups[groupName];
  applyStateToLayerTreeGroup( QgsProject::instance()->layerTreeRoot(), QSet<QString>::fromList( rec.mVisibleLayerIDs ) );

  mMenuDirty = true;
}


void QgsVisibilityGroups::removeCurrentGroup()
{
  foreach ( QAction* a, mMenuGroupActions )
  {
    if ( a->isChecked() )
    {
      removeGroup( a->text() );
      break;
    }
  }
}


void QgsVisibilityGroups::menuAboutToShow()
{
  if ( !mMenuDirty )
    return;

  // lazy update of the menu only when necessary - so that we do not do too much work when it is not necessary

  qDeleteAll( mMenuGroupActions );
  mMenuGroupActions.clear();

  GroupRecord rec = currentState();
  bool hasCurrent = false;

  foreach ( const QString& grpName, mGroups.keys() )
  {
    QAction* a = new QAction( grpName, mMenu );
    a->setCheckable( true );
    if ( rec == mGroups[grpName] )
    {
      a->setChecked( true );
      hasCurrent = true;
    }
    connect( a, SIGNAL( triggered() ), this, SLOT( groupTriggerred() ) );
    mMenuGroupActions.append( a );
  }
  mMenu->insertActions( mMenuSeparator, mMenuGroupActions );

  mActionRemoveCurrentGroup->setEnabled( hasCurrent );

  mMenuDirty = false;
}


void QgsVisibilityGroups::layerTreeVisibilityChanged( QgsLayerTreeNode* node, Qt::CheckState state )
{
  Q_UNUSED( node );
  Q_UNUSED( state );

  mMenuDirty = true;
}

void QgsVisibilityGroups::layerTreeAddedChildren( QgsLayerTreeNode* node, int indexFrom, int indexTo )
{
  Q_UNUSED( node );
  Q_UNUSED( indexFrom );
  Q_UNUSED( indexTo );

  mMenuDirty = true;
}

void QgsVisibilityGroups::layerTreeWillRemoveChildren( QgsLayerTreeNode* node, int indexFrom, int indexTo )
{
  Q_UNUSED( node );
  Q_UNUSED( indexFrom );
  Q_UNUSED( indexTo );

  mMenuDirty = true;
}

void QgsVisibilityGroups::readProject( const QDomDocument& doc )
{
  clear();

  QDomElement visGroupsElem = doc.firstChildElement( "qgis" ).firstChildElement( "visibility-groups" );
  if ( visGroupsElem.isNull() )
    return;

  QDomElement visGroupElem = visGroupsElem.firstChildElement( "visibility-group" );
  while ( !visGroupElem.isNull() )
  {
    QString groupName = visGroupElem.attribute( "name" );
    GroupRecord rec;
    QDomElement visGroupLayerElem = visGroupElem.firstChildElement( "layer" );
    while ( !visGroupLayerElem.isNull() )
    {
      rec.mVisibleLayerIDs << visGroupLayerElem.attribute( "id" );
      visGroupLayerElem = visGroupLayerElem.nextSiblingElement( "layer" );
    }
    mGroups.insert( groupName, rec );

    visGroupElem = visGroupElem.nextSiblingElement( "visibility-group" );
  }
}

void QgsVisibilityGroups::writeProject( QDomDocument& doc )
{
  QDomElement visGroupsElem = doc.createElement( "visibility-groups" );
  foreach ( const QString& grpName, mGroups.keys() )
  {
    QDomElement visGroupElem = doc.createElement( "visibility-group" );
    visGroupElem.setAttribute( "name", grpName );
    foreach ( QString layerID, mGroups[grpName].mVisibleLayerIDs )
    {
      QDomElement layerElem = doc.createElement( "layer" );
      layerElem.setAttribute( "id", layerID );
      visGroupElem.appendChild( layerElem );
    }

    visGroupsElem.appendChild( visGroupElem );
  }

  doc.firstChildElement( "qgis" ).appendChild( visGroupsElem );
}
