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
#include "moc_qgspointcloudlayerproperties.cpp"
#include "qgshelp.h"
#include "qgsmaplayerstyleguiutils.h"
#include "qgspointcloudlayer.h"
#include "qgsgui.h"
#include "qgsapplication.h"
#include "qgsmetadatawidget.h"
#include "qgsmaplayerconfigwidget.h"
#include "qgsmaplayerstylemanager.h"
#include "qgspointcloudattributemodel.h"
#include "qgsdatumtransformdialog.h"
#include "qgspointcloudquerybuilder.h"

#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>

QgsPointCloudLayerProperties::QgsPointCloudLayerProperties( QgsPointCloudLayer *lyr, QgsMapCanvas *canvas, QgsMessageBar *, QWidget *parent, Qt::WindowFlags flags )
  : QgsLayerPropertiesDialog( lyr, canvas, QStringLiteral( "PointCloudLayerProperties" ), parent, flags )
  , mLayer( lyr )
{
  setupUi( this );

  connect( this, &QDialog::accepted, this, &QgsPointCloudLayerProperties::apply );
  connect( this, &QDialog::rejected, this, &QgsPointCloudLayerProperties::rollback );
  connect( buttonBox->button( QDialogButtonBox::Apply ), &QAbstractButton::clicked, this, &QgsPointCloudLayerProperties::apply );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsPointCloudLayerProperties::showHelp );
  connect( pbnQueryBuilder, &QPushButton::clicked, this, &QgsPointCloudLayerProperties::pbnQueryBuilder_clicked );

  connect( mCrsSelector, &QgsProjectionSelectionWidget::crsChanged, this, &QgsPointCloudLayerProperties::crsChanged );

  mScaleRangeWidget->setMapCanvas( mCanvas );
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
  mMetadataWidget->setMapCanvas( mCanvas );
  layout->addWidget( mMetadataWidget );
  metadataFrame->setLayout( layout );
  mOptsPage_Metadata->setContentsMargins( 0, 0, 0, 0 );

  setMetadataWidget( mMetadataWidget, mOptsPage_Metadata );

  // update based on lyr's current state
  syncToLayer();
  connect( lyr->styleManager(), &QgsMapLayerStyleManager::currentStyleChanged, this, &QgsPointCloudLayerProperties::syncToLayer );

  QgsSettings settings;
  if ( !settings.contains( QStringLiteral( "/Windows/PointCloudLayerProperties/tab" ) ) )
  {
    settings.setValue( QStringLiteral( "Windows/PointCloudLayerProperties/tab" ), mOptStackedWidget->indexOf( mOptsPage_Information ) );
  }

  mBtnStyle = new QPushButton( tr( "Style" ) );
  QMenu *menuStyle = new QMenu( this );
  menuStyle->addAction( tr( "Load Style…" ), this, &QgsPointCloudLayerProperties::loadStyleFromFile );
  menuStyle->addAction( tr( "Save Style…" ), this, &QgsPointCloudLayerProperties::saveStyleToFile );
  menuStyle->addSeparator();
  menuStyle->addAction( tr( "Save as Default" ), this, &QgsPointCloudLayerProperties::saveStyleAsDefault );
  menuStyle->addAction( tr( "Restore Default" ), this, &QgsPointCloudLayerProperties::loadDefaultStyle );
  mBtnStyle->setMenu( menuStyle );
  connect( menuStyle, &QMenu::aboutToShow, this, &QgsPointCloudLayerProperties::aboutToShowStyleMenu );

  buttonBox->addButton( mBtnStyle, QDialogButtonBox::ResetRole );

  mBtnMetadata = new QPushButton( tr( "Metadata" ), this );
  QMenu *menuMetadata = new QMenu( this );
  mActionLoadMetadata = menuMetadata->addAction( tr( "Load Metadata…" ), this, &QgsPointCloudLayerProperties::loadMetadataFromFile );
  mActionSaveMetadataAs = menuMetadata->addAction( tr( "Save Metadata…" ), this, &QgsPointCloudLayerProperties::saveMetadataToFile );
  menuMetadata->addSeparator();
  menuMetadata->addAction( tr( "Save as Default" ), this, &QgsPointCloudLayerProperties::saveMetadataAsDefault );
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

  connect( mLayer, &QgsPointCloudLayer::statisticsCalculationStateChanged, this, [this]( QgsPointCloudLayer::PointCloudStatisticsCalculationState state ) {
    mStatisticsCalculationWarningLabel->setHidden( state != QgsPointCloudLayer::PointCloudStatisticsCalculationState::Calculated );
  } );

  initialize();
}

void QgsPointCloudLayerProperties::apply()
{
  mMetadataWidget->acceptMetadata();

  mLayer->setName( mLayerOrigNameLineEdit->text() );

  mLayer->setScaleBasedVisibility( chkUseScaleDependentRendering->isChecked() );
  mLayer->setMinimumScale( mScaleRangeWidget->minimumScale() );
  mLayer->setMaximumScale( mScaleRangeWidget->maximumScale() );

  mBackupCrs = mLayer->crs();

  for ( QgsMapLayerConfigWidget *w : std::as_const( mConfigWidgets ) )
    w->apply();

  mLayer->triggerRepaint();
}

void QgsPointCloudLayerProperties::rollback()
{
  if ( mBackupCrs != mLayer->crs() )
    mLayer->setCrs( mBackupCrs );

  QgsLayerPropertiesDialog::rollback();
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
  connect( mInformationTextBrowser, &QTextBrowser::anchorClicked, this, &QgsPointCloudLayerProperties::openUrl );

  mCrsSelector->setCrs( mLayer->crs() );

  mSubsetGroupBox->setEnabled( true );
  txtSubsetSQL->setText( mLayer->subsetString() );
  txtSubsetSQL->setReadOnly( true );
  txtSubsetSQL->setCaretWidth( 0 );
  txtSubsetSQL->setCaretLineVisible( false );
  pbnQueryBuilder->setEnabled( mLayer->dataProvider() && mLayer->dataProvider()->supportsSubsetString() && !mLayer->isEditable() );

  for ( QgsMapLayerConfigWidget *w : std::as_const( mConfigWidgets ) )
    w->syncToLayer( mLayer );

  mStatisticsCalculationWarningLabel->setHidden( mLayer->statisticsCalculationState() != QgsPointCloudLayer::PointCloudStatisticsCalculationState::Calculated );
}

void QgsPointCloudLayerProperties::aboutToShowStyleMenu()
{
  QMenu *m = qobject_cast<QMenu *>( sender() );

  QgsMapLayerStyleGuiUtils::instance()->removesExtraMenuSeparators( m );
  // re-add style manager actions!
  m->addSeparator();
  QgsMapLayerStyleGuiUtils::instance()->addStyleManagerActions( m, mLayer );
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
  QgsDatumTransformDialog::run( crs, QgsProject::instance()->crs(), this, mCanvas, tr( "Select transformation for the layer" ) );
  mLayer->setCrs( crs );
  mMetadataWidget->crsChanged();
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
          return ( ( double ) stats.availableClasses( mAttribute ).value( classValue.toInt(), 0 ) ) / pointCount * 100;
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
