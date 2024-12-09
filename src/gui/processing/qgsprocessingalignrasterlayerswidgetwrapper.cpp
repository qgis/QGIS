/***************************************************************************
  qgsprocessingalignrasterlayerswidgetwrapper.cpp
  ---------------------
  Date                 : July 2023
  Copyright            : (C) 2023 by Alexander Bruy
  Email                : alexander dot bruy at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingalignrasterlayerswidgetwrapper.h"
#include "moc_qgsprocessingalignrasterlayerswidgetwrapper.cpp"

#include <QBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardItemModel>
#include <QToolButton>

#include "qgspanelwidget.h"
#include "qgsprocessingparameters.h"
#include "qgsprocessingoutputs.h"
#include "qgsprocessingparameteralignrasterlayers.h"
#include "qgsrasterfilewriter.h"
#include "qgis.h"

/// @cond private

//
// QgsProcessingAlignRasterLayerDetailsWidget
//

QgsProcessingAlignRasterLayerDetailsWidget::QgsProcessingAlignRasterLayerDetailsWidget( const QVariant &value, QgsProject *project )
{
  setupUi( this );

  mOutputFileWidget->setStorageMode( QgsFileWidget::SaveFile );
  mOutputFileWidget->setConfirmOverwrite( true );

  QStringList extensions = QgsRasterFileWriter::supportedFormatExtensions();
  QStringList filters;
  for ( const QString &ext : extensions )
  {
    filters << QObject::tr( "%1 files (*.%2)" ).arg( ext.toUpper(), ext.toLower() );
  }
  mOutputFileWidget->setFilter( filters.join( QLatin1String( ";;" ) ) + QStringLiteral( ";;" ) + QObject::tr( "All files (*.*)" ) );

  cmbResamplingMethod->addItem( tr( "Nearest Neighbour" ), static_cast<int>( Qgis::GdalResampleAlgorithm::RA_NearestNeighbour ) );
  cmbResamplingMethod->addItem( tr( "Bilinear (2x2 Kernel)" ), static_cast<int>( Qgis::GdalResampleAlgorithm::RA_Bilinear ) );
  cmbResamplingMethod->addItem( tr( "Cubic (4x4 Kernel)" ), static_cast<int>( Qgis::GdalResampleAlgorithm::RA_Cubic ) );
  cmbResamplingMethod->addItem( tr( "Cubic B-Spline (4x4 Kernel)" ), static_cast<int>( Qgis::GdalResampleAlgorithm::RA_CubicSpline ) );
  cmbResamplingMethod->addItem( tr( "Lanczos (6x6 Kernel)" ), static_cast<int>( Qgis::GdalResampleAlgorithm::RA_Lanczos ) );
  cmbResamplingMethod->addItem( tr( "Average" ), static_cast<int>( Qgis::GdalResampleAlgorithm::RA_Average ) );
  cmbResamplingMethod->addItem( tr( "Mode" ), static_cast<int>( Qgis::GdalResampleAlgorithm::RA_Mode ) );
  cmbResamplingMethod->addItem( tr( "Maximum" ), static_cast<int>( Qgis::GdalResampleAlgorithm::RA_Max ) );
  cmbResamplingMethod->addItem( tr( "Minimum" ), static_cast<int>( Qgis::GdalResampleAlgorithm::RA_Min ) );
  cmbResamplingMethod->addItem( tr( "Median" ), static_cast<int>( Qgis::GdalResampleAlgorithm::RA_Median ) );
  cmbResamplingMethod->addItem( tr( "First Quartile (Q1)" ), static_cast<int>( Qgis::GdalResampleAlgorithm::RA_Q1 ) );
  cmbResamplingMethod->addItem( tr( "Third Quartile (Q3)" ), static_cast<int>( Qgis::GdalResampleAlgorithm::RA_Q3 ) );

  mContext.setProject( project );

  const QgsAlignRasterData::RasterItem item = QgsProcessingParameterAlignRasterLayers::variantMapAsItem( value.toMap(), mContext );
  mInputPath = item.inputFilename;
  mOutputFileWidget->setFilePath( item.outputFilename );
  cmbResamplingMethod->setCurrentIndex( cmbResamplingMethod->findData( static_cast<int>( item.resampleMethod ) ) );
  chkRescaleValues->setChecked( item.rescaleValues );

  connect( mOutputFileWidget, &QgsFileWidget::fileChanged, this, &QgsPanelWidget::widgetChanged );
  connect( cmbResamplingMethod, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsPanelWidget::widgetChanged );
  connect( chkRescaleValues, &QCheckBox::stateChanged, this, &QgsPanelWidget::widgetChanged );
}

QVariant QgsProcessingAlignRasterLayerDetailsWidget::value() const
{
  QgsAlignRasterData::RasterItem item( mInputPath, mOutputFileWidget->filePath() );
  item.resampleMethod = static_cast<Qgis::GdalResampleAlgorithm>( cmbResamplingMethod->currentData().toInt() );
  item.rescaleValues = chkRescaleValues->isChecked();
  return QgsProcessingParameterAlignRasterLayers::itemAsVariantMap( item );
}


//
// QgsProcessingAlignRasterLayersPanelWidget
//

QgsProcessingAlignRasterLayersPanelWidget::QgsProcessingAlignRasterLayersPanelWidget(
  const QVariant &value,
  QgsProject *project,
  QWidget *parent
)
  : QgsProcessingMultipleSelectionPanelWidget( QVariantList(), QVariantList(), parent )
  , mProject( project )
{
  connect( listView(), &QListView::doubleClicked, this, &QgsProcessingAlignRasterLayersPanelWidget::configureRaster );

  QPushButton *configureLayerButton = new QPushButton( tr( "Configure Raster…" ) );
  connect( configureLayerButton, &QPushButton::clicked, this, &QgsProcessingAlignRasterLayersPanelWidget::configureRaster );
  buttonBox()->addButton( configureLayerButton, QDialogButtonBox::ActionRole );

  // populate the list: first layers already selected, then layers from project not yet selected
  mContext.setProject( project );

  QSet<const QString> seenFiles;
  const QVariantList valueList = value.toList();
  for ( const QVariant &v : valueList )
  {
    const QgsAlignRasterData::RasterItem item = QgsProcessingParameterAlignRasterLayers::variantMapAsItem( v.toMap(), mContext );
    addOption( v, titleForItem( item ), true );
    seenFiles.insert( item.inputFilename );
  }

  const QList<QgsRasterLayer *> options = QgsProcessingUtils::compatibleRasterLayers( project );
  for ( const QgsRasterLayer *layer : options )
  {
    if ( seenFiles.contains( layer->source() ) )
      continue;

    QVariantMap vm;
    vm["inputFile"] = layer->source();
    vm["outputFile"] = QString();
    vm["resampleMethod"] = static_cast<int>( Qgis::GdalResampleAlgorithm::RA_NearestNeighbour );
    vm["v"] = false;

    const QString title = layer->source();
    addOption( vm, title, false );
  }
}

void QgsProcessingAlignRasterLayersPanelWidget::configureRaster()
{
  const QModelIndexList selection = listView()->selectionModel()->selectedIndexes();
  if ( selection.size() != 1 )
  {
    QMessageBox::warning( this, tr( "Configure Raster" ), tr( "Please select a single layer." ) );
    return;
  }

  QStandardItem *item = mModel->itemFromIndex( selection[0] );
  const QVariant value = item->data( Qt::UserRole );

  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );
  if ( panel && panel->dockMode() )
  {
    QgsProcessingAlignRasterLayerDetailsWidget *widget = new QgsProcessingAlignRasterLayerDetailsWidget( value, mProject );
    widget->setPanelTitle( tr( "Configure Raster" ) );
    widget->buttonBox()->hide();

    connect( widget, &QgsProcessingAlignRasterLayerDetailsWidget::widgetChanged, this, [=]() {
      setItemValue( item, widget->value() );
    } );
    panel->openPanel( widget );
  }
  else
  {
    QDialog dlg;
    dlg.setWindowTitle( tr( "Configure Raster" ) );
    QVBoxLayout *vLayout = new QVBoxLayout();
    QgsProcessingAlignRasterLayerDetailsWidget *widget = new QgsProcessingAlignRasterLayerDetailsWidget( value, mProject );
    vLayout->addWidget( widget );
    connect( widget->buttonBox(), &QDialogButtonBox::accepted, &dlg, &QDialog::accept );
    connect( widget->buttonBox(), &QDialogButtonBox::rejected, &dlg, &QDialog::reject );
    dlg.setLayout( vLayout );
    if ( dlg.exec() )
    {
      setItemValue( item, widget->value() );
    }
  }
}

void QgsProcessingAlignRasterLayersPanelWidget::setItemValue( QStandardItem *item, const QVariant &value )
{
  mContext.setProject( mProject );

  const QgsAlignRasterData::RasterItem rasterItem = QgsProcessingParameterAlignRasterLayers::variantMapAsItem( value.toMap(), mContext );

  item->setText( titleForItem( rasterItem ) );
  item->setData( value, Qt::UserRole );
}

QString QgsProcessingAlignRasterLayersPanelWidget::titleForItem( const QgsAlignRasterData::RasterItem &item )
{
  return item.inputFilename;
}


//
// QgsProcessingAlignRasterLayersWidget
//

QgsProcessingAlignRasterLayersWidget::QgsProcessingAlignRasterLayersWidget( QWidget *parent )
  : QWidget( parent )
{
  QHBoxLayout *hl = new QHBoxLayout();
  hl->setContentsMargins( 0, 0, 0, 0 );

  mLineEdit = new QLineEdit();
  mLineEdit->setEnabled( false );
  hl->addWidget( mLineEdit, 1 );

  mToolButton = new QToolButton();
  mToolButton->setText( QString( QChar( 0x2026 ) ) );
  hl->addWidget( mToolButton );

  setLayout( hl );

  updateSummaryText();

  connect( mToolButton, &QToolButton::clicked, this, &QgsProcessingAlignRasterLayersWidget::showDialog );
}

void QgsProcessingAlignRasterLayersWidget::setValue( const QVariant &value )
{
  if ( value.isValid() )
    mValue = value.userType() == QMetaType::Type::QVariantList ? value.toList() : QVariantList() << value;
  else
    mValue.clear();

  updateSummaryText();
  emit changed();
}

void QgsProcessingAlignRasterLayersWidget::setProject( QgsProject *project )
{
  mProject = project;
}

void QgsProcessingAlignRasterLayersWidget::showDialog()
{
  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );
  if ( panel && panel->dockMode() )
  {
    QgsProcessingAlignRasterLayersPanelWidget *widget = new QgsProcessingAlignRasterLayersPanelWidget( mValue, mProject );
    widget->setPanelTitle( tr( "Input layers" ) );
    connect( widget, &QgsProcessingMultipleSelectionPanelWidget::selectionChanged, this, [=]() {
      setValue( widget->selectedOptions() );
    } );
    connect( widget, &QgsProcessingMultipleSelectionPanelWidget::acceptClicked, widget, &QgsPanelWidget::acceptPanel );
    panel->openPanel( widget );
  }
  else
  {
    QDialog dlg;
    dlg.setWindowTitle( tr( "Input layers" ) );
    QVBoxLayout *vLayout = new QVBoxLayout();
    QgsProcessingAlignRasterLayersPanelWidget *widget = new QgsProcessingAlignRasterLayersPanelWidget( mValue, mProject );
    vLayout->addWidget( widget );
    widget->buttonBox()->addButton( QDialogButtonBox::Cancel );
    connect( widget->buttonBox(), &QDialogButtonBox::accepted, &dlg, &QDialog::accept );
    connect( widget->buttonBox(), &QDialogButtonBox::rejected, &dlg, &QDialog::reject );
    dlg.setLayout( vLayout );
    if ( dlg.exec() )
    {
      setValue( widget->selectedOptions() );
    }
  }
}

void QgsProcessingAlignRasterLayersWidget::updateSummaryText()
{
  mLineEdit->setText( tr( "%n raster layer(s) selected", nullptr, mValue.count() ) );
}


//
// QgsProcessingAlignRasterLayersWidgetWrapper
//

QgsProcessingAlignRasterLayersWidgetWrapper::QgsProcessingAlignRasterLayersWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{
}

QString QgsProcessingAlignRasterLayersWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterAlignRasterLayers::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingAlignRasterLayersWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingAlignRasterLayersWidgetWrapper( parameter, type );
}

QWidget *QgsProcessingAlignRasterLayersWidgetWrapper::createWidget()
{
  mPanel = new QgsProcessingAlignRasterLayersWidget( nullptr );
  mPanel->setProject( widgetContext().project() );
  connect( mPanel, &QgsProcessingAlignRasterLayersWidget::changed, this, [=] {
    emit widgetValueHasChanged( this );
  } );
  return mPanel;
}

void QgsProcessingAlignRasterLayersWidgetWrapper::setWidgetContext( const QgsProcessingParameterWidgetContext &context )
{
  QgsAbstractProcessingParameterWidgetWrapper::setWidgetContext( context );
  if ( mPanel )
  {
    mPanel->setProject( context.project() );
  }
}

void QgsProcessingAlignRasterLayersWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext &context )
{
  Q_UNUSED( context )
  if ( mPanel )
  {
    mPanel->setValue( value );
  }
}

QVariant QgsProcessingAlignRasterLayersWidgetWrapper::widgetValue() const
{
  return mPanel ? mPanel->value() : QVariant();
}

QStringList QgsProcessingAlignRasterLayersWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList()
         << QgsProcessingParameterMultipleLayers::typeName()
         << QgsProcessingParameterMapLayer::typeName()
         << QgsProcessingParameterVectorLayer::typeName()
         << QgsProcessingParameterFeatureSource::typeName()
         << QgsProcessingParameterFile::typeName()
         << QgsProcessingParameterString::typeName();
}

QStringList QgsProcessingAlignRasterLayersWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList()
         << QgsProcessingOutputString::typeName()
         << QgsProcessingOutputMapLayer::typeName()
         << QgsProcessingOutputVectorLayer::typeName()
         << QgsProcessingOutputMultipleLayers::typeName()
         << QgsProcessingOutputFile::typeName();
}

/// @endcond
