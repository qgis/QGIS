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

#include "qgsapplication.h"
#include "qgsdatumtransformdialog.h"
#include "qgsgui.h"
#include "qgshelp.h"
#include "qgsjsonutils.h"
#include "qgsmapboxglstyleconverter.h"
#include "qgsmaplayerloadstyledialog.h"
#include "qgsmaplayerstyleguiutils.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsmetadatawidget.h"
#include "qgsprovidersourcewidget.h"
#include "qgsprovidersourcewidgetproviderregistry.h"
#include "qgsvectortilebasiclabelingwidget.h"
#include "qgsvectortilebasicrendererwidget.h"
#include "qgsvectortilelayer.h"
#include "qgsvectortileutils.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QTextStream>

#include "moc_qgsvectortilelayerproperties.cpp"

QgsVectorTileLayerProperties::QgsVectorTileLayerProperties( QgsVectorTileLayer *lyr, QgsMapCanvas *canvas, QgsMessageBar *messageBar, QWidget *parent, Qt::WindowFlags flags )
  : QgsLayerPropertiesDialog( lyr, canvas, u"VectorTileLayerProperties"_s, parent, flags )
  , mLayer( lyr )
{
  setupUi( this );

  mRendererWidget = new QgsVectorTileBasicRendererWidget( nullptr, canvas, messageBar, this );
  mOptsPage_Style->layout()->addWidget( mRendererWidget );
  mOptsPage_Style->layout()->setContentsMargins( 0, 0, 0, 0 );

  mLabelingWidget = new QgsVectorTileBasicLabelingWidget( nullptr, canvas, messageBar, this );
  mOptsPage_Labeling->layout()->addWidget( mLabelingWidget );
  mOptsPage_Labeling->layout()->setContentsMargins( 0, 0, 0, 0 );

  connect( this, &QDialog::accepted, this, &QgsVectorTileLayerProperties::apply );
  connect( this, &QDialog::rejected, this, &QgsVectorTileLayerProperties::rollback );
  connect( buttonBox->button( QDialogButtonBox::Apply ), &QAbstractButton::clicked, this, &QgsVectorTileLayerProperties::apply );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsVectorTileLayerProperties::showHelp );

  connect( mCrsSelector, &QgsProjectionSelectionWidget::crsChanged, this, &QgsVectorTileLayerProperties::crsChanged );

  // scale based layer visibility related widgets
  mScaleRangeWidget->setMapCanvas( mCanvas );

  // QgsOptionsDialogBase handles saving/restoring of geometry, splitter and current tab states,
  // switching vertical tabs between icon/text to icon-only modes (splitter collapsed to left),
  // and connecting QDialogButtonBox's accepted/rejected signals to dialog's accept/reject slots
  initOptionsBase( false );

  mSourceGroupBox->hide();

  mOptsPage_Information->setContentsMargins( 0, 0, 0, 0 );

  QVBoxLayout *layout = new QVBoxLayout( metadataFrame );
  layout->setContentsMargins( 0, 0, 0, 0 );
  metadataFrame->setContentsMargins( 0, 0, 0, 0 );
  mMetadataWidget = new QgsMetadataWidget( this, mLayer );
  mMetadataWidget->layout()->setContentsMargins( 0, 0, 0, 0 );
  mMetadataWidget->setMapCanvas( mCanvas );
  layout->addWidget( mMetadataWidget );
  metadataFrame->setLayout( layout );
  mOptsPage_Metadata->setContentsMargins( 0, 0, 0, 0 );

  setMetadataWidget( mMetadataWidget, mOptsPage_Metadata );

  mMapLayerServerPropertiesWidget->setHasWfsTitle( false );

  // update based on lyr's current state
  syncToLayer();

  QgsSettings settings;
  // if dialog hasn't been opened/closed yet, default to Styles tab, which is used most often
  // this will be read by restoreOptionsBaseUi()
  if ( !settings.contains( u"/Windows/VectorTileLayerProperties/tab"_s ) )
  {
    settings.setValue( u"Windows/VectorTileLayerProperties/tab"_s, mOptStackedWidget->indexOf( mOptsPage_Style ) );
  }

  mBtnStyle = new QPushButton( tr( "Style" ) );
  QMenu *menuStyle = new QMenu( this );
  menuStyle->addAction( tr( "Load Style…" ), this, &QgsVectorTileLayerProperties::loadStyle );
  menuStyle->addAction( tr( "Save Style…" ), this, &QgsVectorTileLayerProperties::saveStyleToFile );
  menuStyle->addSeparator();
  menuStyle->addAction( tr( "Save as Default" ), this, &QgsVectorTileLayerProperties::saveStyleAsDefault );
  menuStyle->addAction( tr( "Restore Default" ), this, &QgsVectorTileLayerProperties::loadDefaultStyle );
  mBtnStyle->setMenu( menuStyle );
  connect( menuStyle, &QMenu::aboutToShow, this, &QgsVectorTileLayerProperties::aboutToShowStyleMenu );

  buttonBox->addButton( mBtnStyle, QDialogButtonBox::ResetRole );

  mBtnMetadata = new QPushButton( tr( "Metadata" ), this );
  QMenu *menuMetadata = new QMenu( this );
  mActionLoadMetadata = menuMetadata->addAction( tr( "Load Metadata…" ), this, &QgsVectorTileLayerProperties::loadMetadataFromFile );
  mActionSaveMetadataAs = menuMetadata->addAction( tr( "Save Metadata…" ), this, &QgsVectorTileLayerProperties::saveMetadataToFile );
  mBtnMetadata->setMenu( menuMetadata );
  buttonBox->addButton( mBtnMetadata, QDialogButtonBox::ResetRole );

  initialize();
}

void QgsVectorTileLayerProperties::apply()
{
  if ( mSourceWidget )
  {
    const QString newSource = mSourceWidget->sourceUri();
    if ( newSource != mLayer->source() )
    {
      mLayer->setDataSource( newSource, mLayer->name(), mLayer->providerType(), QgsDataProvider::ProviderOptions() );
    }
  }

  mLayer->setName( mLayerOrigNameLineEd->text() );
  mLayer->setCrs( mCrsSelector->crs() );

  mRendererWidget->apply();
  mLabelingWidget->apply();
  mMetadataWidget->acceptMetadata();

  mLayer->setScaleBasedVisibility( chkUseScaleDependentRendering->isChecked() );
  mLayer->setMinimumScale( mScaleRangeWidget->minimumScale() );
  mLayer->setMaximumScale( mScaleRangeWidget->maximumScale() );

  mMapLayerServerPropertiesWidget->save();
}

void QgsVectorTileLayerProperties::syncToLayer()
{
  if ( !mLayer )
    return;

  /*
   * Information Tab
   */
  const QString myStyle = QgsApplication::reportStyleSheet( QgsApplication::StyleSheetType::WebBrowser );
  // Inject the stylesheet
  const QString html { mLayer->htmlMetadata().replace( "<head>"_L1, QStringLiteral( R"raw(<head><style type="text/css">%1</style>)raw" ) ).arg( myStyle ) };
  mMetadataViewer->setHtml( html );

  /*
   * Source
   */

  mLayerOrigNameLineEd->setText( mLayer->name() );
  mCrsSelector->setCrs( mLayer->crs() );

  if ( !mSourceWidget )
  {
    mSourceWidget = QgsGui::sourceWidgetProviderRegistry()->createWidget( mLayer );
    if ( mSourceWidget )
    {
      QHBoxLayout *layout = new QHBoxLayout();
      layout->addWidget( mSourceWidget );
      mSourceGroupBox->setLayout( layout );
      if ( !mSourceWidget->groupTitle().isEmpty() )
        mSourceGroupBox->setTitle( mSourceWidget->groupTitle() );
      mSourceGroupBox->show();

      connect( mSourceWidget, &QgsProviderSourceWidget::validChanged, this, [this]( bool isValid ) {
        buttonBox->button( QDialogButtonBox::Apply )->setEnabled( isValid );
        buttonBox->button( QDialogButtonBox::Ok )->setEnabled( isValid );
      } );
    }
  }

  if ( mSourceWidget )
  {
    mSourceWidget->setMapCanvas( mCanvas );
    mSourceWidget->setSourceUri( mLayer->source() );
  }

  /*
   * Symbology Tab
   */
  mRendererWidget->syncToLayer( mLayer );

  /*
   * Labels Tab
   */
  mLabelingWidget->setLayer( mLayer );

  /*
   * Rendering
   */
  chkUseScaleDependentRendering->setChecked( mLayer->hasScaleBasedVisibility() );
  mScaleRangeWidget->setScaleRange( mLayer->minimumScale(), mLayer->maximumScale() );

  mMapLayerServerPropertiesWidget->setServerProperties( mLayer->serverProperties() );
}

void QgsVectorTileLayerProperties::saveDefaultStyle()
{
  saveStyleAsDefault();
}

void QgsVectorTileLayerProperties::loadStyle()
{
  const QgsSettings settings; // where we keep last used filter in persistent state

  QgsMapLayerLoadStyleDialog dlg( mLayer );

  if ( dlg.exec() )
  {
    mOldStyle = mLayer->styleManager()->style( mLayer->styleManager()->currentStyle() );
    const QgsMapLayer::StyleCategories categories = dlg.styleCategories();
    const QString type = dlg.fileExtension();
    if ( type.compare( "qml"_L1, Qt::CaseInsensitive ) == 0 )
    {
      QString message;
      bool defaultLoadedFlag = false;
      const QString filePath = dlg.filePath();
      message = mLayer->loadNamedStyle( filePath, defaultLoadedFlag, categories );

      //reset if the default style was loaded OK only
      if ( defaultLoadedFlag )
      {
        syncToLayer();
      }
      else
      {
        //let the user know what went wrong
        QMessageBox::warning( this, tr( "Load Style" ), message );
      }
    }
    else if ( type.compare( "json"_L1, Qt::CaseInsensitive ) == 0 )
    {
      QFile file( dlg.filePath() );
      if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
      {
        QMessageBox::warning( this, tr( "Load Style" ), tr( "Could not read %1" ).arg( QDir::toNativeSeparators( dlg.filePath() ) ) );
      }
      else
      {
        QTextStream in( &file );
        const QString content = in.readAll();

        QgsMapBoxGlStyleConversionContext context;
        // convert automatically from pixel sizes to millimeters, because pixel sizes
        // are a VERY edge case in QGIS and don't play nice with hidpi map renders or print layouts
        context.setTargetUnit( Qgis::RenderUnit::Millimeters );
        //assume source uses 96 dpi
        context.setPixelSizeConversionFactor( 25.4 / 96.0 );

        //load sprites
        QVariantMap styleDefinition = QgsJsonUtils::parseJson( content ).toMap();

        QFileInfo fi( dlg.filePath() );
        QgsVectorTileUtils::loadSprites( styleDefinition, context, u"file://"_s + fi.absolutePath() );

        QgsMapBoxGlStyleConverter converter;

        if ( converter.convert( content, &context ) != QgsMapBoxGlStyleConverter::Success )
        {
          QMessageBox::warning( this, tr( "Load Style" ), converter.errorMessage() );
        }
        else
        {
          if ( dlg.styleCategories().testFlag( QgsMapLayer::StyleCategory::Symbology ) )
          {
            mLayer->setRenderer( converter.renderer() );
          }
          if ( dlg.styleCategories().testFlag( QgsMapLayer::StyleCategory::Labeling ) )
          {
            mLayer->setLabeling( converter.labeling() );
          }
          syncToLayer();
        }
      }
    }
    activateWindow(); // set focus back to properties dialog
  }
}

void QgsVectorTileLayerProperties::saveStyleAs()
{
  saveStyleToFile();
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
    QgsHelp::openHelp( u"working_with_vector_tiles/vector_tiles_properties.html"_s );
  }
}

void QgsVectorTileLayerProperties::crsChanged( const QgsCoordinateReferenceSystem &crs )
{
  QgsDatumTransformDialog::run( crs, QgsProject::instance()->crs(), this, mCanvas, tr( "Select Transformation" ) );
  mLayer->setCrs( crs );
  mMetadataWidget->crsChanged();
}
