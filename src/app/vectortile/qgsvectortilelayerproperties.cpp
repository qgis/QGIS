/***************************************************************************
  qgsvectortilelayerproperties.cpp
  --------------------------------------
  Date                 : May 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectortilelayerproperties.h"

#include "qgsfileutils.h"
#include "qgshelp.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsmaplayerstyleguiutils.h"
#include "qgsvectortilebasicrendererwidget.h"
#include "qgsvectortilebasiclabelingwidget.h"
#include "qgsvectortilelayer.h"

#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>


QgsVectorTileLayerProperties::QgsVectorTileLayerProperties( QgsVectorTileLayer *lyr, QgsMapCanvas *canvas, QgsMessageBar *messageBar, QWidget *parent, Qt::WindowFlags flags )
  : QgsOptionsDialogBase( QStringLiteral( "VectorTileLayerProperties" ), parent, flags )
  , mLayer( lyr )
{
  setupUi( this );

  mRendererWidget = new QgsVectorTileBasicRendererWidget( nullptr, canvas, messageBar, this );
  mOptsPage_Style->layout()->addWidget( mRendererWidget );

  mLabelingWidget = new QgsVectorTileBasicLabelingWidget( nullptr, canvas, messageBar, this );
  mOptsPage_Labeling->layout()->addWidget( mLabelingWidget );

  connect( this, &QDialog::accepted, this, &QgsVectorTileLayerProperties::apply );
  connect( buttonBox->button( QDialogButtonBox::Apply ), &QAbstractButton::clicked, this, &QgsVectorTileLayerProperties::apply );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsVectorTileLayerProperties::showHelp );

  // QgsOptionsDialogBase handles saving/restoring of geometry, splitter and current tab states,
  // switching vertical tabs between icon/text to icon-only modes (splitter collapsed to left),
  // and connecting QDialogButtonBox's accepted/rejected signals to dialog's accept/reject slots
  initOptionsBase( false );

  // update based on lyr's current state
  syncToLayer();

  QgsSettings settings;
  // if dialog hasn't been opened/closed yet, default to Styles tab, which is used most often
  // this will be read by restoreOptionsBaseUi()
  if ( !settings.contains( QStringLiteral( "/Windows/VectorTileLayerProperties/tab" ) ) )
  {
    settings.setValue( QStringLiteral( "Windows/VectorTileLayerProperties/tab" ),
                       mOptStackedWidget->indexOf( mOptsPage_Style ) );
  }

  QString title = QString( tr( "Layer Properties - %1" ) ).arg( mLayer->name() );

  if ( !mLayer->styleManager()->isDefault( mLayer->styleManager()->currentStyle() ) )
    title += QStringLiteral( " (%1)" ).arg( mLayer->styleManager()->currentStyle() );
  restoreOptionsBaseUi( title );

  QPushButton *btnStyle = new QPushButton( tr( "Style" ) );
  QMenu *menuStyle = new QMenu( this );
  menuStyle->addAction( tr( "Load Style…" ), this, &QgsVectorTileLayerProperties::loadStyle );
  menuStyle->addAction( tr( "Save Style…" ), this, &QgsVectorTileLayerProperties::saveStyleAs );
  menuStyle->addSeparator();
  menuStyle->addAction( tr( "Save as Default" ), this, &QgsVectorTileLayerProperties::saveDefaultStyle );
  menuStyle->addAction( tr( "Restore Default" ), this, &QgsVectorTileLayerProperties::loadDefaultStyle );
  btnStyle->setMenu( menuStyle );
  connect( menuStyle, &QMenu::aboutToShow, this, &QgsVectorTileLayerProperties::aboutToShowStyleMenu );

  buttonBox->addButton( btnStyle, QDialogButtonBox::ResetRole );
}

void QgsVectorTileLayerProperties::apply()
{
  mRendererWidget->apply();
  mLabelingWidget->apply();
}

void QgsVectorTileLayerProperties::syncToLayer()
{
  /*
   * Information Tab
   */
  QString info;
  info += QStringLiteral( "<table>" );
  info += QStringLiteral( "<tr><td>%1: </td><td>%2</td><tr>" ).arg( tr( "Uri" ) ).arg( mLayer->source() );
  info += QStringLiteral( "<tr><td>%1: </td><td>%2</td><tr>" ).arg( tr( "Source Type" ) ).arg( mLayer->sourceType() );
  info += QStringLiteral( "<tr><td>%1: </td><td>%2</td><tr>" ).arg( tr( "Source Path" ) ).arg( mLayer->sourcePath() );
  info += QStringLiteral( "<tr><td>%1: </td><td>%2 - %3</td><tr>" ).arg( tr( "Zoom Levels" ) ).arg( mLayer->sourceMinZoom() ).arg( mLayer->sourceMaxZoom() );
  info += QStringLiteral( "</table>" );
  mInformationTextBrowser->setText( info );

  /*
   * Symbology Tab
   */
  mRendererWidget->setLayer( mLayer );

  /*
   * Labels Tab
   */
  mLabelingWidget->setLayer( mLayer );
}


void QgsVectorTileLayerProperties::loadDefaultStyle()
{
  bool defaultLoadedFlag = false;
  QString myMessage = mLayer->loadDefaultStyle( defaultLoadedFlag );
  // reset if the default style was loaded OK only
  if ( defaultLoadedFlag )
  {
    syncToLayer();
  }
  else
  {
    // otherwise let the user know what went wrong
    QMessageBox::information( this,
                              tr( "Default Style" ),
                              myMessage
                            );
  }
}

void QgsVectorTileLayerProperties::saveDefaultStyle()
{
  apply(); // make sure the style to save is up-to-date

  // a flag passed by reference
  bool defaultSavedFlag = false;
  // after calling this the above flag will be set true for success
  // or false if the save operation failed
  QString myMessage = mLayer->saveDefaultStyle( defaultSavedFlag );
  if ( !defaultSavedFlag )
  {
    // let the user know what went wrong
    QMessageBox::information( this,
                              tr( "Default Style" ),
                              myMessage
                            );
  }
}

void QgsVectorTileLayerProperties::loadStyle()
{
  QgsSettings settings;
  QString lastUsedDir = settings.value( QStringLiteral( "style/lastStyleDir" ), QDir::homePath() ).toString();

  QString fileName = QFileDialog::getOpenFileName(
                       this,
                       tr( "Load rendering setting from style file" ),
                       lastUsedDir,
                       tr( "QGIS Layer Style File" ) + " (*.qml)" );
  if ( fileName.isEmpty() )
    return;

  // ensure the user never omits the extension from the file name
  if ( !fileName.endsWith( QLatin1String( ".qml" ), Qt::CaseInsensitive ) )
    fileName += QLatin1String( ".qml" );

  bool defaultLoadedFlag = false;
  QString message = mLayer->loadNamedStyle( fileName, defaultLoadedFlag );
  if ( defaultLoadedFlag )
  {
    settings.setValue( QStringLiteral( "style/lastStyleDir" ), QFileInfo( fileName ).absolutePath() );
    syncToLayer();
  }
  else
  {
    QMessageBox::information( this, tr( "Load Style" ), message );
  }
}

void QgsVectorTileLayerProperties::saveStyleAs()
{
  QgsSettings settings;
  QString lastUsedDir = settings.value( QStringLiteral( "style/lastStyleDir" ), QDir::homePath() ).toString();

  QString outputFileName = QFileDialog::getSaveFileName(
                             this,
                             tr( "Save layer properties as style file" ),
                             lastUsedDir,
                             tr( "QGIS Layer Style File" ) + " (*.qml)" );
  if ( outputFileName.isEmpty() )
    return;

  // ensure the user never omits the extension from the file name
  outputFileName = QgsFileUtils::ensureFileNameHasExtension( outputFileName, QStringList() << QStringLiteral( "qml" ) );

  apply(); // make sure the style to save is up-to-date

  // then export style
  bool defaultLoadedFlag = false;
  QString message;
  message = mLayer->saveNamedStyle( outputFileName, defaultLoadedFlag );

  if ( defaultLoadedFlag )
  {
    settings.setValue( QStringLiteral( "style/lastStyleDir" ), QFileInfo( outputFileName ).absolutePath() );
  }
  else
    QMessageBox::information( this, tr( "Save Style" ), message );
}

void QgsVectorTileLayerProperties::aboutToShowStyleMenu()
{
  QMenu *m = qobject_cast<QMenu *>( sender() );

  QgsMapLayerStyleGuiUtils::instance()->removesExtraMenuSeparators( m );
  // re-add style manager actions!
  m->addSeparator();
  QgsMapLayerStyleGuiUtils::instance()->addStyleManagerActions( m, mLayer );
}

void QgsVectorTileLayerProperties::showHelp()
{
  const QVariant helpPage = mOptionsStackedWidget->currentWidget()->property( "helpPage" );

  if ( helpPage.isValid() )
  {
    QgsHelp::openHelp( helpPage.toString() );
  }
  else
  {
    QgsHelp::openHelp( QStringLiteral( "working_with_vector_tiles/vector_tiles_properties.html" ) );
  }
}
