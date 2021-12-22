/***************************************************************************
  qgsannotationlayerproperties.cpp
  --------------------------------------
  Date                 : September 2021
  Copyright            : (C) 2021 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsannotationlayerproperties.h"

#include "qgsfileutils.h"
#include "qgshelp.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsmaplayerstyleguiutils.h"
#include "qgsgui.h"
#include "qgsnative.h"
#include "qgsapplication.h"
#include "qgsmaplayerloadstyledialog.h"
#include "qgsmaplayerconfigwidgetfactory.h"
#include "qgsmaplayerconfigwidget.h"
#include "qgsdatumtransformdialog.h"
#include "qgspainteffect.h"
#include "qgsproject.h"
#include "qgsprojectutils.h"
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>

QgsAnnotationLayerProperties::QgsAnnotationLayerProperties( QgsAnnotationLayer *layer, QgsMapCanvas *canvas, QgsMessageBar *, QWidget *parent, Qt::WindowFlags flags )
  : QgsOptionsDialogBase( QStringLiteral( "AnnotationLayerProperties" ), parent, flags )
  , mLayer( layer )
  , mMapCanvas( canvas )
{
  setupUi( this );

  connect( this, &QDialog::accepted, this, &QgsAnnotationLayerProperties::apply );
  connect( this, &QDialog::rejected, this, &QgsAnnotationLayerProperties::onCancel );
  connect( buttonBox->button( QDialogButtonBox::Apply ), &QAbstractButton::clicked, this, &QgsAnnotationLayerProperties::apply );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsAnnotationLayerProperties::showHelp );

  connect( mCrsSelector, &QgsProjectionSelectionWidget::crsChanged, this, &QgsAnnotationLayerProperties::crsChanged );

  // QgsOptionsDialogBase handles saving/restoring of geometry, splitter and current tab states,
  // switching vertical tabs between icon/text to icon-only modes (splitter collapsed to left),
  // and connecting QDialogButtonBox's accepted/rejected signals to dialog's accept/reject slots
  initOptionsBase( false );

  mOptsPage_Information->setContentsMargins( 0, 0, 0, 0 );

  // update based on layer's current state
  syncToLayer();

  QgsSettings settings;
  if ( !settings.contains( QStringLiteral( "/Windows/AnnotationLayerProperties/tab" ) ) )
  {
    settings.setValue( QStringLiteral( "Windows/AnnotationLayerProperties/tab" ),
                       mOptStackedWidget->indexOf( mOptsPage_Information ) );
  }

  QString title = tr( "Layer Properties - %1" ).arg( mLayer->name() );

  mBtnStyle = new QPushButton( tr( "Style" ) );
  QMenu *menuStyle = new QMenu( this );
  menuStyle->addAction( tr( "Load Style…" ), this, &QgsAnnotationLayerProperties::loadStyle );
  menuStyle->addAction( tr( "Save Style…" ), this, &QgsAnnotationLayerProperties::saveStyleAs );
  menuStyle->addSeparator();
  menuStyle->addAction( tr( "Save as Default" ), this, &QgsAnnotationLayerProperties::saveDefaultStyle );
  menuStyle->addAction( tr( "Restore Default" ), this, &QgsAnnotationLayerProperties::loadDefaultStyle );
  mBtnStyle->setMenu( menuStyle );
  connect( menuStyle, &QMenu::aboutToShow, this, &QgsAnnotationLayerProperties::aboutToShowStyleMenu );

  buttonBox->addButton( mBtnStyle, QDialogButtonBox::ResetRole );

  mBackupCrs = mLayer->crs();

  if ( !mLayer->styleManager()->isDefault( mLayer->styleManager()->currentStyle() ) )
    title += QStringLiteral( " (%1)" ).arg( mLayer->styleManager()->currentStyle() );
  restoreOptionsBaseUi( title );
}

QgsAnnotationLayerProperties::~QgsAnnotationLayerProperties() = default;

void QgsAnnotationLayerProperties::addPropertiesPageFactory( const QgsMapLayerConfigWidgetFactory *factory )
{
  if ( !factory->supportsLayer( mLayer ) || !factory->supportLayerPropertiesDialog() )
  {
    return;
  }

  QgsMapLayerConfigWidget *page = factory->createWidget( mLayer, mMapCanvas, false, this );
  mConfigWidgets << page;

  const QString beforePage = factory->layerPropertiesPagePositionHint();
  if ( beforePage.isEmpty() )
    addPage( factory->title(), factory->title(), factory->icon(), page );
  else
    insertPage( factory->title(), factory->title(), factory->icon(), page, beforePage );

  page->syncToLayer( mLayer );
}

void QgsAnnotationLayerProperties::apply()
{
  mLayer->setName( mLayerOrigNameLineEdit->text() );

  // scale based layer visibility
  mLayer->setScaleBasedVisibility( mScaleVisibilityGroupBox->isChecked() );
  mLayer->setMaximumScale( mScaleRangeWidget->maximumScale() );
  mLayer->setMinimumScale( mScaleRangeWidget->minimumScale() );

  mBackupCrs = mLayer->crs();

  // set the blend mode and opacity for the layer
  mBlendModeComboBox->setShowClippingModes( QgsProjectUtils::layerIsContainedInGroupLayer( QgsProject::instance(), mLayer ) );
  mLayer->setBlendMode( mBlendModeComboBox->blendMode() );
  mLayer->setOpacity( mOpacityWidget->opacity() );

  if ( mPaintEffect )
    mLayer->setPaintEffect( mPaintEffect->clone() );

  for ( QgsMapLayerConfigWidget *w : mConfigWidgets )
    w->apply();

  mLayer->triggerRepaint();
}

void QgsAnnotationLayerProperties::onCancel()
{
  if ( mBackupCrs != mLayer->crs() )
    mLayer->setCrs( mBackupCrs );

  if ( mOldStyle.xmlData() != mLayer->styleManager()->style( mLayer->styleManager()->currentStyle() ).xmlData() )
  {
    // need to reset style to previous - style applied directly to the layer (not in apply())
    QString myMessage;
    QDomDocument doc( QStringLiteral( "qgis" ) );
    int errorLine, errorColumn;
    doc.setContent( mOldStyle.xmlData(), false, &myMessage, &errorLine, &errorColumn );
    mLayer->importNamedStyle( doc, myMessage );
    syncToLayer();
  }
}

void QgsAnnotationLayerProperties::syncToLayer()
{
  // populate the general information
  mLayerOrigNameLineEdit->setText( mLayer->name() );

  /*
   * Information Tab
   */
  QString myStyle = QgsApplication::reportStyleSheet();
  myStyle.append( QStringLiteral( "body { margin: 10px; }\n " ) );
  mInformationTextBrowser->clear();
  mInformationTextBrowser->document()->setDefaultStyleSheet( myStyle );
  mInformationTextBrowser->setHtml( mLayer->htmlMetadata() );
  mInformationTextBrowser->setOpenLinks( false );
  connect( mInformationTextBrowser, &QTextBrowser::anchorClicked, this, &QgsAnnotationLayerProperties::urlClicked );

  mCrsSelector->setCrs( mLayer->crs() );

  // scale based layer visibility
  mScaleRangeWidget->setScaleRange( mLayer->minimumScale(), mLayer->maximumScale() );
  mScaleVisibilityGroupBox->setChecked( mLayer->hasScaleBasedVisibility() );
  mScaleRangeWidget->setMapCanvas( mMapCanvas );

  // opacity and blend modes
  mBlendModeComboBox->setBlendMode( mLayer->blendMode() );
  mOpacityWidget->setOpacity( mLayer->opacity() );

  if ( mLayer->paintEffect() )
  {
    mPaintEffect.reset( mLayer->paintEffect()->clone() );
    mEffectWidget->setPaintEffect( mPaintEffect.get() );
  }

  for ( QgsMapLayerConfigWidget *w : mConfigWidgets )
    w->syncToLayer( mLayer );
}


void QgsAnnotationLayerProperties::loadDefaultStyle()
{
  bool defaultLoadedFlag = false;
  const QString myMessage = mLayer->loadDefaultStyle( defaultLoadedFlag );
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

void QgsAnnotationLayerProperties::saveDefaultStyle()
{
  apply(); // make sure the style to save is up-to-date

  // a flag passed by reference
  bool defaultSavedFlag = false;
  // after calling this the above flag will be set true for success
  // or false if the save operation failed
  const QString myMessage = mLayer->saveDefaultStyle( defaultSavedFlag );
  if ( !defaultSavedFlag )
  {
    // let the user know what went wrong
    QMessageBox::information( this,
                              tr( "Default Style" ),
                              myMessage
                            );
  }
}

void QgsAnnotationLayerProperties::loadStyle()
{
  QgsSettings settings;
  const QString lastUsedDir = settings.value( QStringLiteral( "style/lastStyleDir" ), QDir::homePath() ).toString();

  QString fileName = QFileDialog::getOpenFileName(
                       this,
                       tr( "Load layer properties from style file" ),
                       lastUsedDir,
                       tr( "QGIS Layer Style File" ) + " (*.qml)" );
  if ( fileName.isEmpty() )
    return;

  // ensure the user never omits the extension from the file name
  if ( !fileName.endsWith( QLatin1String( ".qml" ), Qt::CaseInsensitive ) )
    fileName += QLatin1String( ".qml" );

  mOldStyle = mLayer->styleManager()->style( mLayer->styleManager()->currentStyle() );

  bool defaultLoadedFlag = false;
  const QString message = mLayer->loadNamedStyle( fileName, defaultLoadedFlag );
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

void QgsAnnotationLayerProperties::saveStyleAs()
{
  QgsSettings settings;
  const QString lastUsedDir = settings.value( QStringLiteral( "style/lastStyleDir" ), QDir::homePath() ).toString();

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

void QgsAnnotationLayerProperties::aboutToShowStyleMenu()
{
  QMenu *m = qobject_cast<QMenu *>( sender() );

  QgsMapLayerStyleGuiUtils::instance()->removesExtraMenuSeparators( m );
  // re-add style manager actions!
  m->addSeparator();
  QgsMapLayerStyleGuiUtils::instance()->addStyleManagerActions( m, mLayer );
}

void QgsAnnotationLayerProperties::showHelp()
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

void QgsAnnotationLayerProperties::urlClicked( const QUrl &url )
{
  const QFileInfo file( url.toLocalFile() );
  if ( file.exists() && !file.isDir() )
    QgsGui::nativePlatformInterface()->openFileExplorerAndSelectFile( url.toLocalFile() );
  else
    QDesktopServices::openUrl( url );
}

void QgsAnnotationLayerProperties::crsChanged( const QgsCoordinateReferenceSystem &crs )
{
  QgsDatumTransformDialog::run( crs, QgsProject::instance()->crs(), this, mMapCanvas, tr( "Select transformation for the layer" ) );
  mLayer->setCrs( crs );
}

void QgsAnnotationLayerProperties::optionsStackedWidget_CurrentChanged( int index )
{
  QgsOptionsDialogBase::optionsStackedWidget_CurrentChanged( index );

  mBtnStyle->setVisible( true );
}

