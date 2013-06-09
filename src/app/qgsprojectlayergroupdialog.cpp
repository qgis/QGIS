/***************************************************************************
    qgsembedlayerdialog.cpp
    ---------------------
    begin                : June 2011
    copyright            : (C) 2011 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsprojectlayergroupdialog.h"
#include "qgsproject.h"
#include "qgisapp.h"
#include "qgsapplication.h"

#include <QDomDocument>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>

QgsProjectLayerGroupDialog::QgsProjectLayerGroupDialog( QWidget * parent, const QString& projectFile, Qt::WindowFlags f ): QDialog( parent, f ),
    mShowEmbeddedContent( false )
{
  setupUi( this );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/EmbedLayer/geometry" ).toByteArray() );

  if ( !projectFile.isEmpty() )
  {
    mProjectFileLineEdit->setText( projectFile );
    mProjectFileLabel->hide();
    mProjectFileLineEdit->hide();
    mBrowseFileToolButton->hide();
    mShowEmbeddedContent = true;
    changeProjectFile();
  }

  QObject::connect( mButtonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );
}

QgsProjectLayerGroupDialog::~QgsProjectLayerGroupDialog()
{
  QSettings settings;
  settings.setValue( "/Windows/EmbedLayer/geometry", saveGeometry() );
}

QStringList QgsProjectLayerGroupDialog::selectedGroups() const
{
  QStringList groups;
  QList<QTreeWidgetItem*> items = mTreeWidget->selectedItems();
  QList<QTreeWidgetItem*>::iterator itemIt = items.begin();
  for ( ; itemIt != items.end(); ++itemIt )
  {
    if (( *itemIt )->data( 0, Qt::UserRole ).toString() == "group" )
    {
      groups.push_back(( *itemIt )->text( 0 ) );
    }
  }
  return groups;
}

QStringList QgsProjectLayerGroupDialog::selectedLayerIds() const
{
  QStringList layerIds;
  QList<QTreeWidgetItem*> items = mTreeWidget->selectedItems();
  QList<QTreeWidgetItem*>::iterator itemIt = items.begin();
  for ( ; itemIt != items.end(); ++itemIt )
  {
    if (( *itemIt )->data( 0, Qt::UserRole ).toString() == "layer" )
    {
      layerIds.push_back(( *itemIt )->data( 0, Qt::UserRole + 1 ).toString() );
    }
  }
  return layerIds;
}

QStringList QgsProjectLayerGroupDialog::selectedLayerNames() const
{
  QStringList layerNames;
  QList<QTreeWidgetItem*> items = mTreeWidget->selectedItems();
  QList<QTreeWidgetItem*>::iterator itemIt = items.begin();
  for ( ; itemIt != items.end(); ++itemIt )
  {
    if (( *itemIt )->data( 0, Qt::UserRole ).toString() == "layer" )
    {
      layerNames.push_back(( *itemIt )->text( 0 ) );
    }
  }
  return layerNames;
}

QString QgsProjectLayerGroupDialog::selectedProjectFile() const
{
  return mProjectFileLineEdit->text();
}

void QgsProjectLayerGroupDialog::on_mBrowseFileToolButton_clicked()
{
  //line edit might emit editingFinished signal when loosing focus
  mProjectFileLineEdit->blockSignals( true );

  QSettings s;
  QString projectFile = QFileDialog::getOpenFileName( this,
                        tr( "Select project file" ),
                        s.value( "/qgis/last_embedded_project_path" ).toString() ,
                        tr( "QGIS files" ) + " (*.qgs *.QGS)" );
  if ( !projectFile.isEmpty() )
  {
    mProjectFileLineEdit->setText( projectFile );
  }
  changeProjectFile();
  mProjectFileLineEdit->blockSignals( false );
}

void QgsProjectLayerGroupDialog::on_mProjectFileLineEdit_editingFinished()
{
  changeProjectFile();
}

void QgsProjectLayerGroupDialog::changeProjectFile()
{
  QFile projectFile( mProjectFileLineEdit->text() );
  if ( !projectFile.exists() )
  {
    return;
  }

  if ( mProjectPath == mProjectFileLineEdit->text() )
  {
    //already up to date
    return;
  }

  //check we are not embedding from/to the same project
  if ( mProjectFileLineEdit->isVisible() && mProjectFileLineEdit->text() == QgsProject::instance()->fileName() )
  {
    QMessageBox::critical( 0, tr( "Recursive embedding not possible" ), tr( "It is not possible to embed layers / groups from the current project." ) );
    return;
  }

  mTreeWidget->clear();

  //parse project file and fill tree
  if ( !projectFile.open( QIODevice::ReadOnly ) )
  {
    return;
  }

  QDomDocument projectDom;
  if ( !projectDom.setContent( &projectFile ) )
  {
    return;
  }

  QDomElement legendElem = projectDom.documentElement().firstChildElement( "legend" );
  if ( legendElem.isNull() )
  {
    return;
  }

  QDomNodeList legendChildren = legendElem.childNodes();
  QDomElement currentChildElem;

  for ( int i = 0; i < legendChildren.size(); ++i )
  {
    currentChildElem = legendChildren.at( i ).toElement();
    if ( currentChildElem.tagName() == "legendlayer" )
    {
      addLegendLayerToTreeWidget( currentChildElem );
    }
    else if ( currentChildElem.tagName() == "legendgroup" )
    {
      addLegendGroupToTreeWidget( currentChildElem );
    }
  }

  mProjectPath = mProjectFileLineEdit->text();
}

void QgsProjectLayerGroupDialog::addLegendGroupToTreeWidget( const QDomElement& groupElem, QTreeWidgetItem* parent )
{
  QDomNodeList groupChildren = groupElem.childNodes();
  QDomElement currentChildElem;

  if ( !mShowEmbeddedContent && groupElem.attribute( "embedded" ) == "1" )
  {
    return;
  }

  QTreeWidgetItem* groupItem = 0;
  if ( !parent )
  {
    groupItem = new QTreeWidgetItem( mTreeWidget );
  }
  else
  {
    groupItem = new QTreeWidgetItem( parent );
  }
  groupItem->setIcon( 0, QgsApplication::getThemeIcon( "mActionFolder.png" ) );
  groupItem->setText( 0, groupElem.attribute( "name" ) );
  groupItem->setData( 0, Qt::UserRole, "group" );

  for ( int i = 0; i < groupChildren.size(); ++i )
  {
    currentChildElem = groupChildren.at( i ).toElement();
    if ( currentChildElem.tagName() == "legendlayer" )
    {
      addLegendLayerToTreeWidget( currentChildElem, groupItem );
    }
    else if ( currentChildElem.tagName() == "legendgroup" )
    {
      addLegendGroupToTreeWidget( currentChildElem, groupItem );
    }
  }
}

void QgsProjectLayerGroupDialog::addLegendLayerToTreeWidget( const QDomElement& layerElem, QTreeWidgetItem* parent )
{
  if ( !mShowEmbeddedContent && layerElem.attribute( "embedded" ) == "1" )
  {
    return;
  }

  QTreeWidgetItem* item = 0;
  if ( parent )
  {
    item = new QTreeWidgetItem( parent );
  }
  else
  {
    item = new QTreeWidgetItem( mTreeWidget );
  }
  item->setText( 0, layerElem.attribute( "name" ) );
  item->setData( 0, Qt::UserRole, "layer" );
  item->setData( 0, Qt::UserRole + 1, layerElem.firstChildElement( "filegroup" ).firstChildElement( "legendlayerfile" ).attribute( "layerid" ) );
}

void QgsProjectLayerGroupDialog::on_mTreeWidget_itemSelectionChanged()
{
  mTreeWidget->blockSignals( true );
  QList<QTreeWidgetItem*> items = mTreeWidget->selectedItems();
  QList<QTreeWidgetItem*>::iterator itemIt = items.begin();
  for ( ; itemIt != items.end(); ++itemIt )
  {
    //deselect children recursively
    unselectChildren( *itemIt );
  }
  mTreeWidget->blockSignals( false );
}

void QgsProjectLayerGroupDialog::unselectChildren( QTreeWidgetItem* item )
{
  if ( !item )
  {
    return;
  }

  QTreeWidgetItem* currentChild = 0;
  for ( int i = 0; i < item->childCount(); ++i )
  {
    currentChild = item->child( i );
    currentChild->setSelected( false );
    unselectChildren( currentChild );
  }
}

void QgsProjectLayerGroupDialog::on_mButtonBox_accepted()
{
  QSettings s;
  QFileInfo fi( mProjectPath );
  if ( fi.exists() )
  {
    s.setValue( "/qgis/last_embedded_project_path", fi.absolutePath() );
  }
  accept();
}


