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
#include <QFileDialog>

#include "qgsmaplayerstylemanagerwidget.h"
#include "qgssettings.h"
#include "qgslogger.h"
#include "qgsmaplayer.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerconfigwidget.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsvectordataprovider.h"
#include "qgsrasterdataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsrasterlayer.h"
#include "qgsapplication.h"
#include "qgsproviderregistry.h"

QgsMapLayerStyleManagerWidget::QgsMapLayerStyleManagerWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
  : QgsMapLayerConfigWidget( layer, canvas, parent )
{
  mModel = new QStandardItemModel( this );
  mStyleList = new QListView( this );
  mStyleList->setModel( mModel );
  mStyleList->setViewMode( QListView::ListMode );
  mStyleList->setResizeMode( QListView::Adjust );

  QToolBar *toolbar = new QToolBar( this );
  QAction *addAction = toolbar->addAction( tr( "Add" ) );
  addAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "symbologyAdd.svg" ) ) );
  connect( addAction, &QAction::triggered, this, &QgsMapLayerStyleManagerWidget::addStyle );
  QAction *removeAction = toolbar->addAction( tr( "Remove Current" ) );
  removeAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "symbologyRemove.svg" ) ) );
  connect( removeAction, &QAction::triggered, this, &QgsMapLayerStyleManagerWidget::removeStyle );
  QAction *loadFromFileAction = toolbar->addAction( tr( "Load Style" ) );
  loadFromFileAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionFileOpen.svg" ) ) );
  connect( loadFromFileAction, &QAction::triggered, this, &QgsMapLayerStyleManagerWidget::loadStyle );
  QAction *saveAsDefaultAction = toolbar->addAction( tr( "Save as Default" ) );
  connect( saveAsDefaultAction, &QAction::triggered, this, &QgsMapLayerStyleManagerWidget::saveAsDefault );
  QAction *loadDefaultAction = toolbar->addAction( tr( "Restore Default" ) );
  connect( loadDefaultAction, &QAction::triggered, this, &QgsMapLayerStyleManagerWidget::loadDefault );


  // Save style doesn't work correctly yet so just disable for now.
//  QAction* saveToFileAction = toolbar->addAction( tr( "Save Style" ) );
//  connect( saveToFileAction, SIGNAL( triggered() ), this, SLOT( saveStyle() ) );

  //broken connect - not sure what the purpose of this was?
//  connect( canvas, &QgsMapCanvas::mapCanvasRefreshed, this, SLOT( updateCurrent() ) );

  connect( mStyleList, &QAbstractItemView::clicked, this, &QgsMapLayerStyleManagerWidget::styleClicked );

  setLayout( new QVBoxLayout() );
  layout()->setContentsMargins( 0, 0, 0, 0 );
  layout()->addWidget( toolbar );
  layout()->addWidget( mStyleList );

  connect( mLayer->styleManager(), &QgsMapLayerStyleManager::currentStyleChanged, this, &QgsMapLayerStyleManagerWidget::currentStyleChanged );
  connect( mLayer->styleManager(), &QgsMapLayerStyleManager::styleAdded, this, &QgsMapLayerStyleManagerWidget::styleAdded );
  connect( mLayer->styleManager(), &QgsMapLayerStyleManager::styleRemoved, this, &QgsMapLayerStyleManagerWidget::styleRemoved );
  connect( mLayer->styleManager(), &QgsMapLayerStyleManager::styleRenamed, this, &QgsMapLayerStyleManagerWidget::styleRenamed );

  mModel->clear();

  const QStringList styles = mLayer->styleManager()->styles();
  for ( const QString &styleName : styles )
  {
    QStandardItem *item = new QStandardItem( styleName );
    item->setData( styleName );
    mModel->appendRow( item );
  }

  const QString active = mLayer->styleManager()->currentStyle();
  currentStyleChanged( active );

  connect( mModel, &QStandardItemModel::itemChanged, this, &QgsMapLayerStyleManagerWidget::renameStyle );
}

void QgsMapLayerStyleManagerWidget::styleClicked( const QModelIndex &index )
{
  if ( !mLayer || !index.isValid() )
    return;

  const QString name = index.data().toString();
  mLayer->styleManager()->setCurrentStyle( name );
}

void QgsMapLayerStyleManagerWidget::currentStyleChanged( const QString &name )
{
  const QList<QStandardItem *> items = mModel->findItems( name );
  if ( items.isEmpty() )
    return;

  QStandardItem *item = items.at( 0 );

  mStyleList->setCurrentIndex( item->index() );
}

void QgsMapLayerStyleManagerWidget::styleAdded( const QString &name )
{
  QgsDebugMsg( QStringLiteral( "Style added" ) );
  QStandardItem *item = new QStandardItem( name );
  item->setData( name );
  mModel->appendRow( item );
}

void QgsMapLayerStyleManagerWidget::styleRemoved( const QString &name )
{
  const QList<QStandardItem *> items = mModel->findItems( name );
  if ( items.isEmpty() )
    return;

  QStandardItem *item = items.at( 0 );
  mModel->removeRow( item->row() );
}

void QgsMapLayerStyleManagerWidget::styleRenamed( const QString &oldname, const QString &newname )
{
  const QList<QStandardItem *> items = mModel->findItems( oldname );
  if ( items.isEmpty() )
    return;

  QStandardItem *item = items.at( 0 );
  item->setText( newname );
  item->setData( newname );
}

void QgsMapLayerStyleManagerWidget::addStyle()
{
  bool ok;
  const QString text = QInputDialog::getText( nullptr, tr( "New Style" ),
                       tr( "Style name:" ), QLineEdit::Normal,
                       QStringLiteral( "new style" ), &ok );
  if ( !ok || text.isEmpty() )
    return;

  const bool res = mLayer->styleManager()->addStyleFromLayer( text );
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
  const QString current = mLayer->styleManager()->currentStyle();
  const QList<QStandardItem *> items = mModel->findItems( current );
  if ( items.isEmpty() )
    return;

  QStandardItem *item = items.at( 0 );
  const bool res = mLayer->styleManager()->removeStyle( current );
  if ( res )
  {
    mModel->removeRow( item->row() );
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "Failed to remove current style" ) );
  }

}

void QgsMapLayerStyleManagerWidget::renameStyle( QStandardItem *item )
{
  const QString oldName = item->data().toString();
  const QString newName = item->text();
  item->setData( newName );
  whileBlocking( this )->mLayer->styleManager()->renameStyle( oldName, newName );
}

void QgsMapLayerStyleManagerWidget::saveAsDefault()
{
  QString errorMsg;

  if ( QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( mLayer ) )
  {
    if ( layer->dataProvider()->isSaveAndLoadStyleToDatabaseSupported() )
    {
      QMessageBox askToUser;
      askToUser.setWindowTitle( tr( "Save Style" ) );
      askToUser.setText( tr( "Save default style to: " ) );
      askToUser.setIcon( QMessageBox::Question );
      askToUser.addButton( tr( "Cancel" ), QMessageBox::RejectRole );
      askToUser.addButton( tr( "Local Database" ), QMessageBox::NoRole );
      askToUser.addButton( tr( "Datasource Database" ), QMessageBox::YesRole );

      switch ( askToUser.exec() )
      {
        case 0:
          return;
        case 2:
        {
          QString errorMessage;
          if ( QgsProviderRegistry::instance()->styleExists( layer->providerType(), layer->source(), QString(), errorMessage ) )
          {
            if ( QMessageBox::question( nullptr, tr( "Save style in database" ),
                                        tr( "A matching style already exists in the database for this layer. Do you want to overwrite it?" ),
                                        QMessageBox::Yes | QMessageBox::No ) == QMessageBox::No )
            {
              return;
            }
          }
          else if ( !errorMessage.isEmpty() )
          {
            QMessageBox::warning( nullptr, tr( "Save style in database" ),
                                  errorMessage );
            return;
          }

          layer->saveStyleToDatabase( QString(), QString(), true, QString(), errorMsg );
          if ( errorMsg.isNull() )
          {
            return;
          }
          break;
        }
        default:
          break;
      }
    }
  }

  bool defaultSavedFlag = false;
  // TODO Once the deprecated `saveDefaultStyle()` method is gone, just
  // remove the NOWARN_DEPRECATED tags
  Q_NOWARN_DEPRECATED_PUSH
  errorMsg = mLayer->saveDefaultStyle( defaultSavedFlag );
  Q_NOWARN_DEPRECATED_POP
  if ( !defaultSavedFlag )
  {
    QMessageBox::warning( this, tr( "Default Style" ), errorMsg );
  }

}

void QgsMapLayerStyleManagerWidget::loadDefault()
{
  QString msg;
  bool defaultLoadedFlag = false;

  if ( QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( mLayer ) )
  {
    if ( layer->dataProvider()->isSaveAndLoadStyleToDatabaseSupported() )
    {
      QMessageBox askToUser;
      askToUser.setWindowTitle( tr( "Load Style" ) );
      askToUser.setText( tr( "Load default style from: " ) );
      askToUser.setIcon( QMessageBox::Question );
      askToUser.addButton( tr( "Cancel" ), QMessageBox::RejectRole );
      askToUser.addButton( tr( "Local Database" ), QMessageBox::NoRole );
      askToUser.addButton( tr( "Datasource Database" ), QMessageBox::YesRole );

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
  if ( QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( mLayer ) )
  {
    myMessage = layer->loadNamedStyle( mLayer->styleURI(), defaultLoadedFlag, true );
  }
  if ( QgsRasterLayer *layer = qobject_cast<QgsRasterLayer *>( mLayer ) )
  {
    myMessage = layer->loadNamedStyle( mLayer->styleURI(), defaultLoadedFlag );
  }

//  QString myMessage = layer->loadDefaultStyle( defaultLoadedFlag );
  //reset if the default style was loaded OK only


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
  QgsSettings myQSettings;  // where we keep last used filter in persistent state
  const QString myLastUsedDir = myQSettings.value( QStringLiteral( "style/lastStyleDir" ), QDir::homePath() ).toString();

  const QString myFileName = QFileDialog::getOpenFileName( this, tr( "Load layer properties from style file" ), myLastUsedDir,
                             tr( "QGIS Layer Style File" ) + " (*.qml);;" + tr( "SLD File" ) + " (*.sld)" );
  if ( myFileName.isNull() )
  {
    return;
  }

  QString myMessage;
  bool defaultLoadedFlag = false;

  if ( myFileName.endsWith( QLatin1String( ".sld" ), Qt::CaseInsensitive ) )
  {
    // load from SLD
    myMessage = mLayer->loadSldStyle( myFileName, defaultLoadedFlag );
  }
  else
  {
    myMessage = mLayer->loadNamedStyle( myFileName, defaultLoadedFlag );
  }
  //reset if the default style was loaded OK only
  if ( defaultLoadedFlag )
  {
    emit widgetChanged();
  }
  else
  {
    //let the user know what went wrong
    QMessageBox::warning( this, tr( "Load Style" ), myMessage );
  }

  const QFileInfo myFI( myFileName );
  const QString myPath = myFI.path();
  myQSettings.setValue( QStringLiteral( "style/lastStyleDir" ), myPath );

}
