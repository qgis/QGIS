/***************************************************************************
  qgspointcloudlayerproperties.cpp
  --------------------------------------
  Date                 : October 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointcloudlayerproperties.h"

#include "qgsfileutils.h"
#include "qgshelp.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsmaplayerstyleguiutils.h"
#include "qgspointcloudlayer.h"
#include "qgsgui.h"
#include "qgsnative.h"
#include "qgsapplication.h"
#include "qgsmetadatawidget.h"
#include "qgsmaplayerloadstyledialog.h"
#include "qgsmaplayerconfigwidgetfactory.h"
#include "qgsmaplayerconfigwidget.h"
#include "qgspointcloudattributemodel.h"
#include "qgsdatumtransformdialog.h"
#include "qgspointcloudlayerelevationproperties.h"
#include "qgspointcloudquerybuilder.h"
#include "qgspointcloudrenderer.h"

#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>

QgsPointCloudLayerProperties::QgsPointCloudLayerProperties( QgsPointCloudLayer *lyr, QgsMapCanvas *canvas, QgsMessageBar *, QWidget *parent, Qt::WindowFlags flags )
  : QgsOptionsDialogBase( QStringLiteral( "PointCloudLayerProperties" ), parent, flags )
  , mLayer( lyr )
  , mMapCanvas( canvas )
{
  setupUi( this );

  connect( this, &QDialog::accepted, this, &QgsPointCloudLayerProperties::apply );
  connect( this, &QDialog::rejected, this, &QgsPointCloudLayerProperties::onCancel );
  connect( buttonBox->button( QDialogButtonBox::Apply ), &QAbstractButton::clicked, this, &QgsPointCloudLayerProperties::apply );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsPointCloudLayerProperties::showHelp );
  connect( pbnQueryBuilder, &QPushButton::clicked, this, &QgsPointCloudLayerProperties::pbnQueryBuilder_clicked );

  connect( mCrsSelector, &QgsProjectionSelectionWidget::crsChanged, this, &QgsPointCloudLayerProperties::crsChanged );

  mScaleRangeWidget->setMapCanvas( mMapCanvas );
  chkUseScaleDependentRendering->setChecked( lyr->hasScaleBasedVisibility() );
  mScaleRangeWidget->setScaleRange( lyr->minimumScale(), lyr->maximumScale() );

  // QgsOptionsDialogBase handles saving/restoring of geometry, splitter and current tab states,
  // switching vertical tabs between icon/text to icon-only modes (splitter collapsed to left),
  // and connecting QDialogButtonBox's accepted/rejected signals to dialog's accept/reject slots
  initOptionsBase( false );

  mOptsPage_Information->setContentsMargins( 0, 0, 0, 0 );

  QVBoxLayout *layout = new QVBoxLayout( metadataFrame );
  layout->setContentsMargins( 0, 0, 0, 0 );
  metadataFrame->setContentsMargins( 0, 0, 0, 0 );
  mMetadataWidget = new QgsMetadataWidget( this, mLayer );
  mMetadataWidget->layout()->setContentsMargins( 0, 0, 0, 0 );
  mMetadataWidget->setMapCanvas( mMapCanvas );
  layout->addWidget( mMetadataWidget );
  metadataFrame->setLayout( layout );
  mOptsPage_Metadata->setContentsMargins( 0, 0, 0, 0 );

  // update based on lyr's current state
  syncToLayer();

  QgsSettings settings;
  if ( !settings.contains( QStringLiteral( "/Windows/PointCloudLayerProperties/tab" ) ) )
  {
    settings.setValue( QStringLiteral( "Windows/PointCloudLayerProperties/tab" ),
                       mOptStackedWidget->indexOf( mOptsPage_Information ) );
  }

  QString title = tr( "Layer Properties - %1" ).arg( mLayer->name() );

  mBtnStyle = new QPushButton( tr( "Style" ) );
  QMenu *menuStyle = new QMenu( this );
  menuStyle->addAction( tr( "Load Style…" ), this, &QgsPointCloudLayerProperties::loadStyle );
  menuStyle->addAction( tr( "Save Style…" ), this, &QgsPointCloudLayerProperties::saveStyleAs );
  menuStyle->addSeparator();
  menuStyle->addAction( tr( "Save as Default" ), this, &QgsPointCloudLayerProperties::saveDefaultStyle );
  menuStyle->addAction( tr( "Restore Default" ), this, &QgsPointCloudLayerProperties::loadDefaultStyle );
  mBtnStyle->setMenu( menuStyle );
  connect( menuStyle, &QMenu::aboutToShow, this, &QgsPointCloudLayerProperties::aboutToShowStyleMenu );

  buttonBox->addButton( mBtnStyle, QDialogButtonBox::ResetRole );

  mBtnMetadata = new QPushButton( tr( "Metadata" ), this );
  QMenu *menuMetadata = new QMenu( this );
  mActionLoadMetadata = menuMetadata->addAction( tr( "Load Metadata…" ), this, &QgsPointCloudLayerProperties::loadMetadata );
  mActionSaveMetadataAs = menuMetadata->addAction( tr( "Save Metadata…" ), this, &QgsPointCloudLayerProperties::saveMetadataAs );
  menuMetadata->addSeparator();
  menuMetadata->addAction( tr( "Save as Default" ), this, &QgsPointCloudLayerProperties::saveDefaultMetadata );
  menuMetadata->addAction( tr( "Restore Default" ), this, &QgsPointCloudLayerProperties::loadDefaultMetadata );

  mBtnMetadata->setMenu( menuMetadata );
  buttonBox->addButton( mBtnMetadata, QDialogButtonBox::ResetRole );

  //Add help page references
  mOptsPage_Information->setProperty( "helpPage", QStringLiteral( "working_with_point_clouds/point_clouds.html#information-properties" ) );
  mOptsPage_Source->setProperty( "helpPage", QStringLiteral( "working_with_point_clouds/point_clouds.html#source-properties" ) );
  mOptsPage_Rendering->setProperty( "helpPage", QStringLiteral( "working_with_point_clouds/point_clouds.html#rendering-properties" ) );
  mOptsPage_Metadata->setProperty( "helpPage", QStringLiteral( "working_with_point_clouds/point_clouds.html#metadata-properties" ) );
  mOptsPage_Statistics->setProperty( "helpPage", QStringLiteral( "working_with_point_clouds/point_clouds.html#statistics-properties" ) );

  mStatisticsTableView->setModel( new QgsPointCloudAttributeStatisticsModel( mLayer, mStatisticsTableView ) );
  mStatisticsTableView->verticalHeader()->hide();

  mBackupCrs = mLayer->crs();

  const QgsPointCloudStatistics stats = mLayer->statistics();

  if ( !stats.classesOf( QStringLiteral( "Classification" ) ).isEmpty() )
  {
    mClassificationStatisticsTableView->setModel( new QgsPointCloudClassificationStatisticsModel( mLayer, QStringLiteral( "Classification" ), mStatisticsTableView ) );
    mClassificationStatisticsTableView->verticalHeader()->hide();
  }
  else
  {
    mClassificationStatsGroupBox->hide();
  }

  mStatisticsCalculationWarningLabel->setHidden( mLayer->statisticsCalculationState() != QgsPointCloudLayer::PointCloudStatisticsCalculationState::Calculated );

  connect( mLayer, &QgsPointCloudLayer::statisticsCalculationStateChanged, this, [this]( QgsPointCloudLayer::PointCloudStatisticsCalculationState state )
  {
    mStatisticsCalculationWarningLabel->setHidden( state != QgsPointCloudLayer::PointCloudStatisticsCalculationState::Calculated );
  } );

  if ( !mLayer->styleManager()->isDefault( mLayer->styleManager()->currentStyle() ) )
    title += QStringLiteral( " (%1)" ).arg( mLayer->styleManager()->currentStyle() );
  restoreOptionsBaseUi( title );
}

void QgsPointCloudLayerProperties::addPropertiesPageFactory( const QgsMapLayerConfigWidgetFactory *factory )
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

void QgsPointCloudLayerProperties::apply()
{
  mMetadataWidget->acceptMetadata();

  mLayer->setName( mLayerOrigNameLineEdit->text() );

  mLayer->setScaleBasedVisibility( chkUseScaleDependentRendering->isChecked() );
  mLayer->setMinimumScale( mScaleRangeWidget->minimumScale() );
  mLayer->setMaximumScale( mScaleRangeWidget->maximumScale() );

  mBackupCrs = mLayer->crs();

  for ( QgsMapLayerConfigWidget *w : mConfigWidgets )
    w->apply();

  mLayer->triggerRepaint();
}

void QgsPointCloudLayerProperties::onCancel()
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

void QgsPointCloudLayerProperties::syncToLayer()
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
  connect( mInformationTextBrowser, &QTextBrowser::anchorClicked, this, &QgsPointCloudLayerProperties::urlClicked );

  mCrsSelector->setCrs( mLayer->crs() );

  mSubsetGroupBox->setEnabled( true );
  txtSubsetSQL->setText( mLayer->subsetString() );
  txtSubsetSQL->setReadOnly( true );
  txtSubsetSQL->setCaretWidth( 0 );
  txtSubsetSQL->setCaretLineVisible( false );
  pbnQueryBuilder->setEnabled( mLayer &&
                               mLayer->dataProvider() &&
                               mLayer->dataProvider()->supportsSubsetString() );

  for ( QgsMapLayerConfigWidget *w : mConfigWidgets )
    w->syncToLayer( mLayer );

  mStatisticsCalculationWarningLabel->setHidden( mLayer->statisticsCalculationState() != QgsPointCloudLayer::PointCloudStatisticsCalculationState::Calculated );
}


void QgsPointCloudLayerProperties::loadDefaultStyle()
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

void QgsPointCloudLayerProperties::saveDefaultStyle()
{
  apply(); // make sure the style to save is up-to-date

  // a flag passed by reference
  bool defaultSavedFlag = false;
  // TODO Once the deprecated `saveDefaultStyle()` method is gone, just
  // remove the NOWARN_DEPRECATED tags
  Q_NOWARN_DEPRECATED_PUSH
  // after calling this the above flag will be set true for success
  // or false if the save operation failed
  const QString myMessage = mLayer->saveDefaultStyle( defaultSavedFlag );
  Q_NOWARN_DEPRECATED_POP
  if ( !defaultSavedFlag )
  {
    // let the user know what went wrong
    QMessageBox::information( this,
                              tr( "Default Style" ),
                              myMessage
                            );
  }
}

void QgsPointCloudLayerProperties::loadStyle()
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

void QgsPointCloudLayerProperties::saveStyleAs()
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

void QgsPointCloudLayerProperties::aboutToShowStyleMenu()
{
  QMenu *m = qobject_cast<QMenu *>( sender() );

  QgsMapLayerStyleGuiUtils::instance()->removesExtraMenuSeparators( m );
  // re-add style manager actions!
  m->addSeparator();
  QgsMapLayerStyleGuiUtils::instance()->addStyleManagerActions( m, mLayer );
}

void QgsPointCloudLayerProperties::loadMetadata()
{
  QgsSettings myQSettings;  // where we keep last used filter in persistent state
  const QString myLastUsedDir = myQSettings.value( QStringLiteral( "style/lastStyleDir" ), QDir::homePath() ).toString();

  const QString myFileName = QFileDialog::getOpenFileName( this, tr( "Load layer metadata from metadata file" ), myLastUsedDir,
                             tr( "QGIS Layer Metadata File" ) + " (*.qmd)" );
  if ( myFileName.isNull() )
  {
    return;
  }

  QString myMessage;
  bool defaultLoadedFlag = false;
  myMessage = mLayer->loadNamedMetadata( myFileName, defaultLoadedFlag );

  //reset if the default style was loaded OK only
  if ( defaultLoadedFlag )
  {
    mMetadataWidget->setMetadata( &mLayer->metadata() );
  }
  else
  {
    //let the user know what went wrong
    QMessageBox::warning( this, tr( "Load Metadata" ), myMessage );
  }

  const QFileInfo myFI( myFileName );
  const QString myPath = myFI.path();
  myQSettings.setValue( QStringLiteral( "style/lastStyleDir" ), myPath );

  activateWindow(); // set focus back to properties dialog
}

void QgsPointCloudLayerProperties::saveMetadataAs()
{
  QgsSettings myQSettings;  // where we keep last used filter in persistent state
  const QString myLastUsedDir = myQSettings.value( QStringLiteral( "style/lastStyleDir" ), QDir::homePath() ).toString();

  QString myOutputFileName = QFileDialog::getSaveFileName( this, tr( "Save Layer Metadata as QMD" ),
                             myLastUsedDir, tr( "QMD File" ) + " (*.qmd)" );
  if ( myOutputFileName.isNull() ) //dialog canceled
  {
    return;
  }

  mMetadataWidget->acceptMetadata();

  //ensure the user never omitted the extension from the file name
  if ( !myOutputFileName.endsWith( QgsMapLayer::extensionPropertyType( QgsMapLayer::Metadata ), Qt::CaseInsensitive ) )
  {
    myOutputFileName += QgsMapLayer::extensionPropertyType( QgsMapLayer::Metadata );
  }

  bool defaultLoadedFlag = false;
  const QString message = mLayer->saveNamedMetadata( myOutputFileName, defaultLoadedFlag );
  if ( defaultLoadedFlag )
    myQSettings.setValue( QStringLiteral( "style/lastStyleDir" ), QFileInfo( myOutputFileName ).absolutePath() );
  else
    QMessageBox::information( this, tr( "Save Metadata" ), message );
}

void QgsPointCloudLayerProperties::saveDefaultMetadata()
{
  mMetadataWidget->acceptMetadata();

  bool defaultSavedFlag = false;
  const QString errorMsg = mLayer->saveDefaultMetadata( defaultSavedFlag );
  if ( !defaultSavedFlag )
  {
    QMessageBox::warning( this, tr( "Default Metadata" ), errorMsg );
  }
}

void QgsPointCloudLayerProperties::loadDefaultMetadata()
{
  bool defaultLoadedFlag = false;
  const QString myMessage = mLayer->loadNamedMetadata( mLayer->metadataUri(), defaultLoadedFlag );
  //reset if the default metadata was loaded OK only
  if ( defaultLoadedFlag )
  {
    mMetadataWidget->setMetadata( &mLayer->metadata() );
  }
  else
  {
    QMessageBox::information( this, tr( "Default Metadata" ), myMessage );
  }
}

void QgsPointCloudLayerProperties::showHelp()
{
  const QVariant helpPage = mOptionsStackedWidget->currentWidget()->property( "helpPage" );

  if ( helpPage.isValid() )
  {
    QgsHelp::openHelp( helpPage.toString() );
  }
  else
  {
    QgsHelp::openHelp( QStringLiteral( "working_with_point_clouds/point_clouds.html" ) );
  }
}

void QgsPointCloudLayerProperties::urlClicked( const QUrl &url )
{
  const QFileInfo file( url.toLocalFile() );
  if ( file.exists() && !file.isDir() )
    QgsGui::nativePlatformInterface()->openFileExplorerAndSelectFile( url.toLocalFile() );
  else
    QDesktopServices::openUrl( url );
}

void QgsPointCloudLayerProperties::pbnQueryBuilder_clicked()
{
  QgsPointCloudQueryBuilder qb { mLayer };
  qb.setSubsetString( mLayer->subsetString() );
  if ( qb.exec() )
  {
    txtSubsetSQL->setText( qb.subsetString() );
  }
}

void QgsPointCloudLayerProperties::crsChanged( const QgsCoordinateReferenceSystem &crs )
{
  QgsDatumTransformDialog::run( crs, QgsProject::instance()->crs(), this, mMapCanvas, tr( "Select transformation for the layer" ) );
  mLayer->setCrs( crs );
  mMetadataWidget->crsChanged();
}

void QgsPointCloudLayerProperties::optionsStackedWidget_CurrentChanged( int index )
{
  QgsOptionsDialogBase::optionsStackedWidget_CurrentChanged( index );

  const bool isMetadataPanel = ( index == mOptStackedWidget->indexOf( mOptsPage_Metadata ) );
  mBtnStyle->setVisible( ! isMetadataPanel );
  mBtnMetadata->setVisible( isMetadataPanel );
}

//
// QgsPointCloudAttributeStatisticsModel
//
QgsPointCloudAttributeStatisticsModel::QgsPointCloudAttributeStatisticsModel( QgsPointCloudLayer *layer, QObject *parent )
  : QAbstractTableModel( parent )
  , mLayer( layer )
  , mAttributes( layer->attributes() )
{

}

int QgsPointCloudAttributeStatisticsModel::columnCount( const QModelIndex & ) const
{
  return StDev + 1;
}

int QgsPointCloudAttributeStatisticsModel::rowCount( const QModelIndex & ) const
{
  return mAttributes.count();
}

QVariant QgsPointCloudAttributeStatisticsModel::data( const QModelIndex &index, int role ) const
{
  if ( index.row() < 0 || index.row() >= mAttributes.count() )
    return QVariant();

  const QgsPointCloudAttribute &attr = mAttributes.at( index.row() );
  const QgsPointCloudStatistics stats = mLayer->statistics();

  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
    {
      switch ( index.column() )
      {
        case Name:
          return attr.name();

        case Min:
          return stats.minimum( attr.name() );
        case Max:
          return stats.maximum( attr.name() );
        case Mean:
          return stats.mean( attr.name() );
        case StDev:
          return stats.stDev( attr.name() );

      }
      return QVariant();
    }

    case Qt::TextAlignmentRole:
    {
      switch ( index.column() )
      {
        case Name:
          return static_cast<Qt::Alignment::Int>( Qt::AlignLeft | Qt::AlignVCenter );

        case Min:
        case Max:
        case Mean:
        case StDev:
          return static_cast<Qt::Alignment::Int>( Qt::AlignRight | Qt::AlignVCenter );

      }
      return QVariant();
    }

    case Qt::FontRole:
    {
      if ( index.column() == Name )
      {
        QFont f;
        f.setBold( true );
        return f;
      }
      return QVariant();
    }

    case Qt::DecorationRole:
      if ( index.column() == Name )
        return QgsPointCloudAttributeModel::iconForAttributeType( attr.type() );
      else
        return QVariant();

    default:
      return QVariant();
  }
}

QVariant QgsPointCloudAttributeStatisticsModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation == Qt::Horizontal && role == Qt::DisplayRole )
  {
    switch ( section )
    {
      case Name:
        return tr( "Attribute" );

      case Min:
        return tr( "Minimum" );
      case Max:
        return tr( "Maximum" );
      case Mean:
        return tr( "Mean" );
      case StDev:
        return tr( "Standard Deviation" );
    }
  }
  return QVariant();
}


//
// QgsPointCloudClassificationStatisticsModel
//
QgsPointCloudClassificationStatisticsModel::QgsPointCloudClassificationStatisticsModel( QgsPointCloudLayer *layer, const QString &attribute, QObject *parent )
  : QAbstractTableModel( parent )
  , mLayer( layer )
  , mAttribute( attribute )
{
  mClassifications = layer->statistics().classesOf( attribute );
  std::sort( mClassifications.begin(), mClassifications.end(), []( int a, int b ) -> bool { return a < b; } );
}

int QgsPointCloudClassificationStatisticsModel::columnCount( const QModelIndex & ) const
{
  return Percent + 1;
}

int QgsPointCloudClassificationStatisticsModel::rowCount( const QModelIndex & ) const
{
  return mClassifications.count();
}

QVariant QgsPointCloudClassificationStatisticsModel::data( const QModelIndex &index, int role ) const
{
  if ( index.row() < 0 || index.row() >= mClassifications.count() )
    return QVariant();

  const QVariant classValue = mClassifications.at( index.row() );
  const QgsPointCloudStatistics stats = mLayer->statistics();

  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
    {
      switch ( index.column() )
      {
        case Value:
          return classValue.toString();

        case Classification:
          return QgsPointCloudDataProvider::translatedLasClassificationCodes().value( classValue.toInt() );

        case Count:
          return stats.availableClasses( mAttribute ).value( classValue.toInt(), 0 );

        case Percent:
        {
          qint64 pointCount = stats.sampledPointsCount();
          return ( ( double )stats.availableClasses( mAttribute ).value( classValue.toInt(), 0 ) ) / pointCount * 100;
        }

      }
      return QVariant();
    }

    case Qt::TextAlignmentRole:
    {
      switch ( index.column() )
      {
        case Classification:
          return QVariant( Qt::AlignLeft | Qt::AlignVCenter );

        case Value:
        case Count:
        case Percent:
          return QVariant( Qt::AlignRight | Qt::AlignVCenter );

      }
      return QVariant();
    }

    case Qt::FontRole:
    {
      if ( index.column() == Classification )
      {
        QFont f;
        f.setBold( true );
        return f;
      }
      return QVariant();
    }

    default:
      return QVariant();
  }
}

QVariant QgsPointCloudClassificationStatisticsModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation == Qt::Horizontal && role == Qt::DisplayRole )
  {
    switch ( section )
    {
      case Value:
        return QVariant();

      case Classification:
        return tr( "Classification" );
      case Count:
        return tr( "Count" );
      case Percent:
        return tr( "%" );
    }
  }
  return QVariant();
}

