/***************************************************************************
    qgsmaplayerstylemanagerwidget.cpp
    ---------------------
    begin                : June 2016
    copyright            : (C) 2016 by Nathan Woodrow
    email                : woodrow dot nathan at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QAction>
#include <QVBoxLayout>
#include <QToolBar>
#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>
#include <QFileDialog>

#include "qgsmaplayerstylemanagerwidget.h"
#include "qgslogger.h"
#include "qgsmaplayer.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerconfigwidget.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsvectordataprovider.h"
#include "qgsrasterdataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsrasterlayer.h"


QgsMapLayerStyleManagerWidget::QgsMapLayerStyleManagerWidget( QgsMapLayer* layer, QgsMapCanvas *canvas, QWidget *parent )
    : QgsMapLayerConfigWidget( layer, canvas, parent )
{
  mModel = new QStandardItemModel( this );
  mStyleList = new QListView( this );
  mStyleList->setModel( mModel );
  mStyleList->setViewMode( QListView::ListMode );
  mStyleList->setResizeMode( QListView::Adjust );

  QToolBar* toolbar = new QToolBar( this );
  QAction* addAction = toolbar->addAction( tr( "Add" ) );
  addAction->setIcon( QgsApplication::getThemeIcon( "symbologyAdd.svg" ) );
  connect( addAction, SIGNAL( triggered() ), this, SLOT( addStyle() ) );
  QAction* removeAction = toolbar->addAction( tr( "Remove Current" ) );
  removeAction->setIcon( QgsApplication::getThemeIcon( "symbologyRemove.svg" ) );
  connect( removeAction, SIGNAL( triggered() ), this, SLOT( removeStyle() ) );
  QAction* loadFromFileAction = toolbar->addAction( tr( "Load Style" ) );
  loadFromFileAction->setIcon( QgsApplication::getThemeIcon( "/mActionFileOpen.svg" ) );
  connect( loadFromFileAction, SIGNAL( triggered() ), this, SLOT( loadStyle() ) );
  QAction* saveAsDefaultAction = toolbar->addAction( tr( "Save as default" ) );
  connect( saveAsDefaultAction, SIGNAL( triggered() ), this, SLOT( saveAsDefault() ) );
  QAction* loadDefaultAction = toolbar->addAction( tr( "Restore default" ) );
  connect( loadDefaultAction, SIGNAL( triggered() ), this, SLOT( loadDefault() ) );


  // Save style doesn't work correctly yet so just disable for now.
//  QAction* saveToFileAction = toolbar->addAction( tr( "Save Style" ) );
//  connect( saveToFileAction, SIGNAL( triggered() ), this, SLOT( saveStyle() ) );

  connect( canvas, SIGNAL( mapCanvasRefreshed() ), this, SLOT( updateCurrent() ) );

  connect( mStyleList, SIGNAL( clicked( QModelIndex ) ), this, SLOT( styleClicked( QModelIndex ) ) );

  setLayout( new QVBoxLayout() );
  layout()->setContentsMargins( 0, 0, 0, 0 );
  layout()->addWidget( toolbar );
  layout()->addWidget( mStyleList );

  connect( mLayer->styleManager(), SIGNAL( currentStyleChanged( QString ) ), this, SLOT( currentStyleChanged( QString ) ) );
  connect( mLayer->styleManager(), SIGNAL( styleAdded( QString ) ), this, SLOT( styleAdded( QString ) ) );
  connect( mLayer->styleManager(), SIGNAL( styleremoved( QString ) ), this, SLOT( styleRemoved( QString ) ) );
  connect( mLayer->styleManager(), SIGNAL( styleRenamed( QString, QString ) ), this, SLOT( styleRenamed( QString, QString ) ) );

  mModel->clear();

  Q_FOREACH ( const QString name, mLayer->styleManager()->styles() )
  {
    QString stylename = name;

    if ( stylename.isEmpty() )
      stylename = "(default)";

    QStandardItem* item = new QStandardItem( stylename );
    mModel->appendRow( item );
  }

  QString active = mLayer->styleManager()->currentStyle();
  currentStyleChanged( active );
}

void QgsMapLayerStyleManagerWidget::styleClicked( QModelIndex index )
{
  if ( !mLayer || !index.isValid() )
    return;

  QString name = index.data().toString();
  if ( name == "(default)" )
    name = "";

  mLayer->styleManager()->setCurrentStyle( name );
}

void QgsMapLayerStyleManagerWidget::currentStyleChanged( QString name )
{
  QList<QStandardItem*> items = mModel->findItems( name );
  if ( items.isEmpty() )
    return;

  QStandardItem* item = items.at( 0 );

  mStyleList->setCurrentIndex( item->index() );
}

void QgsMapLayerStyleManagerWidget::styleAdded( QString name )
{
  QgsDebugMsg( "Style added" );
  QStandardItem* item = new QStandardItem( name );
  mModel->appendRow( item );
}

void QgsMapLayerStyleManagerWidget::styleRemoved( QString name )
{
  QList<QStandardItem*> items = mModel->findItems( name );
  if ( items.isEmpty() )
    return;

  QStandardItem* item = items.at( 0 );
  mModel->removeRow( item->row() );
}

void QgsMapLayerStyleManagerWidget::styleRenamed( QString oldname, QString newname )
{
  QList<QStandardItem*> items = mModel->findItems( oldname );
  if ( items.isEmpty() )
    return;

  QStandardItem* item = items.at( 0 );
  item->setText( newname );
}

void QgsMapLayerStyleManagerWidget::addStyle()
{
  bool ok;
  QString text = QInputDialog::getText( nullptr, tr( "New style" ),
                                        tr( "Style name:" ), QLineEdit::Normal,
                                        "new style", &ok );
  if ( !ok || text.isEmpty() )
    return;

  bool res = mLayer->styleManager()->addStyleFromLayer( text );
  if ( res ) // make it active!
  {
    mLayer->styleManager()->setCurrentStyle( text );
  }
  else
  {
    QgsDebugMsg( "Failed to add style: " + text );
  }
}

void QgsMapLayerStyleManagerWidget::removeStyle()
{
  QString current =  mLayer->styleManager()->currentStyle();
  QList<QStandardItem*> items = mModel->findItems( current );
  if ( items.isEmpty() )
    return;

  QStandardItem* item = items.at( 0 );
  bool res = mLayer->styleManager()->removeStyle( current );
  if ( res )
  {
    mModel->removeRow( item->row() );
  }
  else
  {
    QgsDebugMsg( "Failed to remove current style" );
  }

}

void QgsMapLayerStyleManagerWidget::saveAsDefault()
{
  QString errorMsg;

  if ( QgsVectorLayer* layer = qobject_cast<QgsVectorLayer*>( mLayer ) )
  {
    if ( layer->dataProvider()->isSaveAndLoadStyleToDBSupported() )
    {
      QMessageBox askToUser;
      askToUser.setText( tr( "Save default style to: " ) );
      askToUser.setIcon( QMessageBox::Question );
      askToUser.addButton( tr( "Cancel" ), QMessageBox::RejectRole );
      askToUser.addButton( tr( "Local database" ), QMessageBox::NoRole );
      askToUser.addButton( tr( "Datasource database" ), QMessageBox::YesRole );

      switch ( askToUser.exec() )
      {
        case 0:
          return;
        case 2:
          layer->saveStyleToDatabase( "", "", true, "", errorMsg );
          if ( errorMsg.isNull() )
          {
            return;
          }
          break;
        default:
          break;
      }
    }
  }

  bool defaultSavedFlag = false;
  errorMsg = mLayer->saveDefaultStyle( defaultSavedFlag );
  if ( !defaultSavedFlag )
  {
    QMessageBox::warning( this, tr( "Default Style" ), errorMsg );
  }

}

void QgsMapLayerStyleManagerWidget::loadDefault()
{
  QString msg;
  bool defaultLoadedFlag = false;

  if ( QgsVectorLayer* layer = qobject_cast<QgsVectorLayer*>( mLayer ) )
  {
    if ( layer->dataProvider()->isSaveAndLoadStyleToDBSupported() )
    {
      QMessageBox askToUser;
      askToUser.setText( tr( "Load default style from: " ) );
      askToUser.setIcon( QMessageBox::Question );
      askToUser.addButton( tr( "Cancel" ), QMessageBox::RejectRole );
      askToUser.addButton( tr( "Local database" ), QMessageBox::NoRole );
      askToUser.addButton( tr( "Datasource database" ), QMessageBox::YesRole );

      switch ( askToUser.exec() )
      {
        case 0:
          return;
        case 2:
          msg = layer->loadNamedStyle( mLayer->styleURI(), defaultLoadedFlag );
          if ( !defaultLoadedFlag )
          {
            //something went wrong - let them know why
            QMessageBox::information( this, tr( "Default Style" ), msg );
          }
          if ( msg.compare( tr( "Loaded from Provider" ) ) )
          {
            QMessageBox::information( this, tr( "Default Style" ),
                                      tr( "No default style was found for this layer" ) );
          }
          return;
        default:
          break;
      }
    }
  }

  QString myMessage;
  if ( QgsVectorLayer* layer = qobject_cast<QgsVectorLayer*>( mLayer ) )
  {
    myMessage = layer->loadNamedStyle( mLayer->styleURI(), defaultLoadedFlag, true );
  }
  if ( QgsRasterLayer* layer = qobject_cast<QgsRasterLayer*>( mLayer ) )
  {
    myMessage = layer->loadNamedStyle( mLayer->styleURI(), defaultLoadedFlag );
  }

//  QString myMessage = layer->loadDefaultStyle( defaultLoadedFlag );
  //reset if the default style was loaded ok only


  if ( !defaultLoadedFlag )
  {
    //something went wrong - let them know why
    QMessageBox::information( this, tr( "Default Style" ), myMessage );
  }
  else
  {
    emit widgetChanged();
  }

}

void QgsMapLayerStyleManagerWidget::saveStyle()
{

}

void QgsMapLayerStyleManagerWidget::loadStyle()
{
  QSettings myQSettings;  // where we keep last used filter in persistent state
  QString myLastUsedDir = myQSettings.value( "style/lastStyleDir", QDir::homePath() ).toString();

  QString myFileName = QFileDialog::getOpenFileName( this, tr( "Load layer properties from style file" ), myLastUsedDir,
                       tr( "QGIS Layer Style File" ) + " (*.qml);;" + tr( "SLD File" ) + " (*.sld)" );
  if ( myFileName.isNull() )
  {
    return;
  }

  QString myMessage;
  bool defaultLoadedFlag = false;

  if ( myFileName.endsWith( ".sld", Qt::CaseInsensitive ) )
  {
    // load from SLD
    myMessage = mLayer->loadSldStyle( myFileName, defaultLoadedFlag );
  }
  else
  {
    myMessage = mLayer->loadNamedStyle( myFileName, defaultLoadedFlag );
  }
  //reset if the default style was loaded ok only
  if ( defaultLoadedFlag )
  {
    emit widgetChanged();
  }
  else
  {
    //let the user know what went wrong
    QMessageBox::warning( this, tr( "Load Style" ), myMessage );
  }

  QFileInfo myFI( myFileName );
  QString myPath = myFI.path();
  myQSettings.setValue( "style/lastStyleDir", myPath );

}
