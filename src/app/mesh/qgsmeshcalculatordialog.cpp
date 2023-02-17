/***************************************************************************
                          qgsmeshcalculatordialog.cpp
                          ---------------------------
    begin                : January 2019
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgdalutils.h"
#include "qgsmeshcalculatordialog.h"
#include "qgsproject.h"
#include "qgsmeshcalcnode.h"
#include "qgsmeshdataprovider.h"
#include "qgsproviderregistry.h"
#include "qgsmeshlayer.h"
#include "qgssettings.h"
#include "qgsgui.h"
#include "qgsvectorlayer.h"
#include "qgsmaplayerproxymodel.h"
#include "qgswkbtypes.h"
#include "qgsfeatureiterator.h"
#include "qgsmeshdatasetgrouptreeview.h"
#include "qgshelp.h"

#include "cpl_string.h"
#include "gdal.h"
#include "qgis.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QFontDatabase>
#include <QMap>

QgsMeshCalculatorDialog::QgsMeshCalculatorDialog( QgsMeshLayer *meshLayer, QWidget *parent, Qt::WindowFlags f )
  : QDialog( parent, f ),
    mLayer( meshLayer )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  cboLayerMask->setFilters( QgsMapLayerProxyModel::PolygonLayer );
  QgsMeshDatasetGroupListModel *model = new QgsMeshDatasetGroupListModel( this );
  model->syncToLayer( meshLayer );
  model->setDisplayProviderName( true );
  mDatasetsListWidget->setModel( model );
  mVariableNames = model->variableNames();

  getMeshDrivers();
  populateDriversComboBox( );
  connect( mOutputFormatComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsMeshCalculatorDialog::updateInfoMessage );
  connect( mOutputFormatComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsMeshCalculatorDialog::onOutputFormatChange );
  connect( mOutputGroupNameLineEdit, &QLineEdit::textChanged, this, &QgsMeshCalculatorDialog::updateInfoMessage );

  connect( mDatasetsListWidget, &QListView::doubleClicked, this, &QgsMeshCalculatorDialog::datasetGroupEntry );
  connect( mCurrentLayerExtentButton, &QPushButton::clicked, this, &QgsMeshCalculatorDialog::mCurrentLayerExtentButton_clicked );
  connect( mAllTimesButton, &QPushButton::clicked, this, &QgsMeshCalculatorDialog::mAllTimesButton_clicked );
  connect( mExpressionTextEdit, &QTextEdit::textChanged, this, &QgsMeshCalculatorDialog::updateInfoMessage );

  connect( useMaskCb, &QRadioButton::toggled, this, &QgsMeshCalculatorDialog::toggleExtendMask );
  maskBox->setVisible( false );
  useMaskCb->setEnabled( cboLayerMask->count() );

  mXMaxSpinBox->setShowClearButton( false );
  mXMinSpinBox->setShowClearButton( false );
  mYMaxSpinBox->setShowClearButton( false );
  mYMinSpinBox->setShowClearButton( false );

  connect( mPlusPushButton, &QPushButton::clicked, this, &QgsMeshCalculatorDialog::mPlusPushButton_clicked );
  connect( mMinusPushButton, &QPushButton::clicked, this, &QgsMeshCalculatorDialog::mMinusPushButton_clicked );
  connect( mLessButton, &QPushButton::clicked, this, &QgsMeshCalculatorDialog::mLessButton_clicked );
  connect( mLesserEqualButton, &QPushButton::clicked, this, &QgsMeshCalculatorDialog::mLesserEqualButton_clicked );
  connect( mMultiplyPushButton, &QPushButton::clicked, this, &QgsMeshCalculatorDialog::mMultiplyPushButton_clicked );
  connect( mDividePushButton, &QPushButton::clicked, this, &QgsMeshCalculatorDialog::mDividePushButton_clicked );
  connect( mGreaterButton, &QPushButton::clicked, this, &QgsMeshCalculatorDialog::mGreaterButton_clicked );
  connect( mGreaterEqualButton, &QPushButton::clicked, this, &QgsMeshCalculatorDialog::mGreaterEqualButton_clicked );
  connect( mOpenBracketPushButton, &QPushButton::clicked, this, &QgsMeshCalculatorDialog::mOpenBracketPushButton_clicked );
  connect( mCloseBracketPushButton, &QPushButton::clicked, this, &QgsMeshCalculatorDialog::mCloseBracketPushButton_clicked );
  connect( mEqualButton, &QPushButton::clicked, this, &QgsMeshCalculatorDialog::mEqualButton_clicked );
  connect( mNotEqualButton, &QPushButton::clicked, this, &QgsMeshCalculatorDialog::mNotEqualButton_clicked );
  connect( mMinButton, &QPushButton::clicked, this, &QgsMeshCalculatorDialog::mMinButton_clicked );
  connect( mMaxButton, &QPushButton::clicked, this, &QgsMeshCalculatorDialog::mMaxButton_clicked );
  connect( mAbsButton, &QPushButton::clicked, this, &QgsMeshCalculatorDialog::mAbsButton_clicked );
  connect( mPowButton, &QPushButton::clicked, this, &QgsMeshCalculatorDialog::mPowButton_clicked );
  connect( mIfButton, &QPushButton::clicked, this, &QgsMeshCalculatorDialog::mIfButton_clicked );
  connect( mAndButton, &QPushButton::clicked, this, &QgsMeshCalculatorDialog::mAndButton_clicked );
  connect( mOrButton, &QPushButton::clicked, this, &QgsMeshCalculatorDialog::mOrButton_clicked );
  connect( mNotButton, &QPushButton::clicked, this, &QgsMeshCalculatorDialog::mNotButton_clicked );
  connect( mSumAggrButton, &QPushButton::clicked, this, &QgsMeshCalculatorDialog::mSumAggrButton_clicked );
  connect( mMaxAggrButton, &QPushButton::clicked, this, &QgsMeshCalculatorDialog::mMaxAggrButton_clicked );
  connect( mMinAggrButton, &QPushButton::clicked, this, &QgsMeshCalculatorDialog::mMinAggrButton_clicked );
  connect( mAverageAggrButton, &QPushButton::clicked, this, &QgsMeshCalculatorDialog::mAverageAggrButton_clicked );
  connect( mNoDataButton, &QPushButton::clicked, this, &QgsMeshCalculatorDialog::mNoDataButton_clicked );

  mExpressionTextEdit->setCurrentFont( QFontDatabase::systemFont( QFontDatabase::FixedFont ) );

  useFullLayerExtent();
  repopulateTimeCombos();
  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, [ = ]
  {
    QgsHelp::openHelp( QStringLiteral( "working_with_mesh/mesh_properties.html#mesh-calculator" ) );
  } );

  const QgsSettings settings;
  mOutputDatasetFileWidget->setStorageMode( QgsFileWidget::SaveFile );
  mOutputDatasetFileWidget->setDialogTitle( tr( "Enter Mesh Dataset File" ) );
  mOutputDatasetFileWidget->setDefaultRoot( settings.value( QStringLiteral( "/MeshCalculator/lastOutputDir" ), QDir::homePath() ).toString() );
  onOutputFormatChange();
  connect( mOutputDatasetFileWidget, &QgsFileWidget::fileChanged, this, &QgsMeshCalculatorDialog::updateInfoMessage );

  connect( mUseVirtualProviderCheckBox, &QCheckBox::clicked, this, &QgsMeshCalculatorDialog::onVirtualCheckboxChange );
  onVirtualCheckboxChange();
}

QgsMeshCalculatorDialog::~QgsMeshCalculatorDialog() = default;


QString QgsMeshCalculatorDialog::formulaString() const
{
  return mExpressionTextEdit->toPlainText();
}

QgsMeshLayer *QgsMeshCalculatorDialog::meshLayer() const
{
  return mLayer;
}

QString QgsMeshCalculatorDialog::outputFile() const
{
  const QString ret = mOutputDatasetFileWidget->filePath();
  return controlSuffix( ret );
}

QgsRectangle QgsMeshCalculatorDialog::outputExtent() const
{
  const QgsRectangle ret(
    mXMinSpinBox->value(),
    mYMinSpinBox->value(),
    mXMaxSpinBox->value(),
    mYMaxSpinBox->value()
  );
  return ret;
}

QgsGeometry QgsMeshCalculatorDialog::maskGeometry() const
{
  QgsVectorLayer *mask_layer = qobject_cast<QgsVectorLayer *> ( cboLayerMask->currentLayer() );
  if ( mask_layer )
  {
    return maskGeometry( mask_layer );
  }
  return QgsGeometry();
}

QString QgsMeshCalculatorDialog::driver() const
{
  return mOutputFormatComboBox->currentData().toString();
}

QString QgsMeshCalculatorDialog::groupName() const
{
  return mOutputGroupNameLineEdit->text();
}

QgsGeometry QgsMeshCalculatorDialog::maskGeometry( QgsVectorLayer *layer ) const
{
  QgsFeatureIterator it = layer->getFeatures();
  QVector<QgsGeometry> geometries;
  QgsFeature feat;
  while ( it.nextFeature( feat ) )
  {
    geometries.push_back( feat.geometry() );
  }
  const QgsGeometry ret = QgsGeometry::unaryUnion( geometries ) ;
  return ret;
}

double QgsMeshCalculatorDialog::startTime() const
{
  if ( mStartTimeComboBox->currentIndex() > -1 )
    return mStartTimeComboBox->itemData( mStartTimeComboBox->currentIndex() ).toDouble();
  else
    return 0;
}

double QgsMeshCalculatorDialog::endTime() const
{
  if ( mEndTimeComboBox->currentIndex() > -1 )
    return mEndTimeComboBox->itemData( mEndTimeComboBox->currentIndex() ).toDouble();
  else
    return 0;
}

std::unique_ptr<QgsMeshCalculator> QgsMeshCalculatorDialog::calculator() const
{
  std::unique_ptr<QgsMeshCalculator> calc;
  QgsMeshDatasetGroup::Type destination = QgsMeshDatasetGroup::Persistent;

  if ( mUseVirtualProviderCheckBox->isChecked() )
    destination = QgsMeshDatasetGroup::Virtual;

  switch ( destination )
  {
    case QgsMeshDatasetGroup::Persistent:
      if ( useExtentCb->isChecked() )
      {
        calc.reset(
          new QgsMeshCalculator(
            formulaString(),
            driver(),
            groupName(),
            outputFile(),
            outputExtent(),
            startTime(),
            endTime(),
            meshLayer()
          )
        );
      }
      else
      {
        calc.reset(
          new QgsMeshCalculator(
            formulaString(),
            driver(),
            groupName(),
            outputFile(),
            maskGeometry(),
            startTime(),
            endTime(),
            meshLayer()
          )
        );
      }
      break;
    case QgsMeshDatasetGroup::Virtual:
      if ( useExtentCb->isChecked() )
      {
        calc.reset(
          new QgsMeshCalculator(
            formulaString(),
            groupName(),
            outputExtent(),
            destination,
            meshLayer(),
            startTime(),
            endTime()
          )
        );
      }
      else
      {
        calc.reset(
          new QgsMeshCalculator(
            formulaString(),
            groupName(),
            maskGeometry(),
            destination,
            meshLayer(),
            startTime(),
            endTime()
          )
        );
      }
      break;
    default:
      break;
  }

  return calc;
}

void QgsMeshCalculatorDialog::datasetGroupEntry( const QModelIndex &index )
{
  const QString group = quoteDatasetGroupEntry( datasetGroupName( index ) );
  mExpressionTextEdit->insertPlainText( QStringLiteral( " %1 " ).arg( group ) );
}

void QgsMeshCalculatorDialog::toggleExtendMask()
{
  bool visible = useMaskCb->isChecked();
  extendBox->setVisible( !visible );
  maskBox->setVisible( visible );
}

void QgsMeshCalculatorDialog::updateInfoMessage()
{
  QgsMeshDriverMetadata::MeshDriverCapability requiredCapability;

  // expression is valid
  const QgsMeshCalculator::Result result = QgsMeshCalculator::expressionIsValid(
        formulaString(),
        meshLayer(),
        requiredCapability
      );
  const bool expressionValid = result == QgsMeshCalculator::Success;

  // selected driver is appropriate
  const bool notInFile = mUseVirtualProviderCheckBox->isChecked();
  bool driverValid = false;
  if ( expressionValid )
  {
    const QString driverKey = driver();
    if ( mMeshDrivers.contains( driverKey ) )
    {
      const QgsMeshDriverMetadata meta = mMeshDrivers[driverKey];
      driverValid = meta.capabilities().testFlag( requiredCapability );
    }
  }
  else
  {
    // can't determine if driver is valid when expression is invalid
    driverValid = true;
  }

  // output path is selected
  bool filePathValid = false;
  QString outputPath = outputFile();
  if ( !outputPath.isEmpty() )
  {
    outputPath = QFileInfo( outputPath ).absolutePath();
    filePathValid = QFileInfo( outputPath ).isWritable();
  }

  // group name
  const bool groupNameValid = !groupName().isEmpty() && !mVariableNames.contains( groupName() );

  if ( expressionValid &&
       ( notInFile || ( driverValid && filePathValid ) )  &&
       groupNameValid )
  {
    mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( true );
    mExpressionValidLabel->setText( tr( "Expression valid" ) );
  }
  else
  {
    mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
    if ( !expressionValid )
      mExpressionValidLabel->setText( tr( "Expression invalid" ) );
    else if ( !filePathValid && !notInFile )
      mExpressionValidLabel->setText( tr( "Invalid file path" ) );
    else if ( !driverValid && !notInFile )
      mExpressionValidLabel->setText( tr( "Selected driver cannot store data defined on %1" ).arg( requiredCapability == QgsMeshDriverMetadata::CanWriteFaceDatasets ? tr( " faces " ) : tr( " vertices " ) ) );
    else if ( !groupNameValid )
      mExpressionValidLabel->setText( tr( "Invalid group name" ) );
  }
}

void QgsMeshCalculatorDialog::onVirtualCheckboxChange()
{
  mOutputDatasetFileWidget->setVisible( !mUseVirtualProviderCheckBox->isChecked() );
  mOutputDatasetFileLabel->setVisible( !mUseVirtualProviderCheckBox->isChecked() );
  mOutputFormatComboBox->setVisible( !mUseVirtualProviderCheckBox->isChecked() );
  mOutputFormatLabel->setVisible( !mUseVirtualProviderCheckBox->isChecked() );
  updateInfoMessage();
}

void QgsMeshCalculatorDialog::onOutputFormatChange()
{
  const QString suffix = currentOutputSuffix();
  if ( !suffix.isEmpty() )
  {
    QString filter = mOutputFormatComboBox->currentText();
    filter.append( QStringLiteral( " (*.%1)" ).arg( suffix ) );
    mOutputDatasetFileWidget->setFilter( filter );

    // if output filename is already defined we need to replace old suffix
    QString fileName = mOutputDatasetFileWidget->filePath();
    if ( !fileName.isEmpty() )
    {
      mOutputDatasetFileWidget->setFilePath( controlSuffix( fileName ) );
    }
  }
  else
  {
    mOutputDatasetFileWidget->setFilter( tr( "All Files (*)" ) );
  }
}

QString QgsMeshCalculatorDialog::datasetGroupName( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return QString();

  return index.data( Qt::DisplayRole ).toString();
}

void QgsMeshCalculatorDialog::mCurrentLayerExtentButton_clicked()
{
  useFullLayerExtent();
}

void QgsMeshCalculatorDialog::mAllTimesButton_clicked()
{
  useAllTimesFromLayer();
}

void QgsMeshCalculatorDialog::mPlusPushButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " + " ) );
}

void QgsMeshCalculatorDialog::mMinusPushButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " - " ) );
}

void QgsMeshCalculatorDialog::mLessButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " < " ) );
}

void QgsMeshCalculatorDialog::mLesserEqualButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " <= " ) );
}

void QgsMeshCalculatorDialog::mMultiplyPushButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " * " ) );
}

void QgsMeshCalculatorDialog::mDividePushButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " / " ) );
}

void QgsMeshCalculatorDialog::mGreaterButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " > " ) );
}

void QgsMeshCalculatorDialog::mGreaterEqualButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " >= " ) );
}

void QgsMeshCalculatorDialog::mOpenBracketPushButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " ( " ) );
}

void QgsMeshCalculatorDialog::mCloseBracketPushButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " ) " ) );
}

void QgsMeshCalculatorDialog::mEqualButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " = " ) );
}

void QgsMeshCalculatorDialog::mNotEqualButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " != " ) );
}

void QgsMeshCalculatorDialog::mMinButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " min ( A , B ) " ) );
}

void QgsMeshCalculatorDialog::mMaxButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " max ( A , B ) " ) );
}

void QgsMeshCalculatorDialog::mAbsButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " abs ( " ) );
}

void QgsMeshCalculatorDialog::mPowButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " ^ " ) );
}

void QgsMeshCalculatorDialog::mIfButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " if ( 1 = 1 , NODATA , NODATA ) " ) );
}

void QgsMeshCalculatorDialog::mAndButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " and " ) );
}

void QgsMeshCalculatorDialog::mOrButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " or " ) );
}

void QgsMeshCalculatorDialog::mNotButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " not " ) );
}

void QgsMeshCalculatorDialog::mSumAggrButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " sum_aggr ( " ) );
}

void QgsMeshCalculatorDialog::mMaxAggrButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " max_aggr ( " ) );
}

void QgsMeshCalculatorDialog::mMinAggrButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " min_aggr ( " ) );
}

void QgsMeshCalculatorDialog::mAverageAggrButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " average_aggr ( " ) );
}

void QgsMeshCalculatorDialog::mNoDataButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " NODATA " ) );
}

QString QgsMeshCalculatorDialog::quoteDatasetGroupEntry( const QString group )
{
  QString ret( group );
  ret = QStringLiteral( "\"%1\"" ).arg( ret.replace( "\"", "\\\"" ) );
  return ret;
}

QString QgsMeshCalculatorDialog::controlSuffix( const QString &fileName ) const
{
  if ( fileName.isEmpty() )
    return fileName;

  const QFileInfo fileInfo( fileName );

  const QString appropriateSuffix = currentOutputSuffix();

  const QString existingSuffix = fileInfo.suffix();
  if ( !( existingSuffix.isEmpty() && appropriateSuffix.isEmpty() )
       && existingSuffix != appropriateSuffix )
  {
    const int pos = fileName.lastIndexOf( '.' );
    QString ret = fileName.left( pos + 1 );
    ret.append( appropriateSuffix );

    return ret;
  }

  return fileName;
}

QString QgsMeshCalculatorDialog::currentOutputSuffix() const
{
  const QString currentDriver = mOutputFormatComboBox->currentData().toString();
  QString suffix;
  if ( mMeshDrivers.contains( currentDriver ) )
    suffix = mMeshDrivers[currentDriver].writeDatasetOnFileSuffix();

  return suffix;
}

void QgsMeshCalculatorDialog::getMeshDrivers()
{
  QgsProviderMetadata *providerMetadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "mdal" ) );
  if ( providerMetadata )
  {
    const QList<QgsMeshDriverMetadata> allDrivers = providerMetadata->meshDriversMetadata();
    for ( const QgsMeshDriverMetadata &meta : allDrivers )
    {
      if ( meta.capabilities().testFlag( QgsMeshDriverMetadata::MeshDriverCapability::CanWriteFaceDatasets ) ||
           meta.capabilities().testFlag( QgsMeshDriverMetadata::MeshDriverCapability::CanWriteEdgeDatasets ) ||
           meta.capabilities().testFlag( QgsMeshDriverMetadata::MeshDriverCapability::CanWriteVertexDatasets ) )
        mMeshDrivers[meta.name()] = meta;
    }
  }
}

void QgsMeshCalculatorDialog::populateDriversComboBox( )
{

  whileBlocking( mOutputFormatComboBox )->clear();

  const QList< QgsMeshDriverMetadata > vals = mMeshDrivers.values();
  for ( const QgsMeshDriverMetadata &meta : vals )
  {
    whileBlocking( mOutputFormatComboBox )->addItem( meta.description(), meta.name() );
  }
  mOutputFormatComboBox->setCurrentIndex( 0 );
}

void QgsMeshCalculatorDialog::useFullLayerExtent()
{
  QgsMeshLayer *layer = meshLayer();
  if ( !layer )
    return;

  const QgsRectangle layerExtent = layer->extent();
  mXMinSpinBox->setValue( layerExtent.xMinimum() );
  mXMaxSpinBox->setValue( layerExtent.xMaximum() );
  mYMinSpinBox->setValue( layerExtent.yMinimum() );
  mYMaxSpinBox->setValue( layerExtent.yMaximum() );
}

void QgsMeshCalculatorDialog::useAllTimesFromLayer()
{
  const QString datasetGroupName = currentDatasetGroup();
  setTimesByDatasetGroupName( datasetGroupName );
}

QString QgsMeshCalculatorDialog::currentDatasetGroup() const
{
  const QModelIndex index = mDatasetsListWidget->currentIndex();

  if ( !index.isValid() )
    return QString();

  return datasetGroupName( index );
}

void QgsMeshCalculatorDialog::setTimesByDatasetGroupName( const QString group )
{
  QgsMeshLayer *layer = meshLayer();
  if ( !layer || !layer->dataProvider() )
    return;
  const QgsMeshDataProvider *dp = layer->dataProvider();

  // find group index from group name
  int groupIndex = -1;
  for ( int i = 0; i < dp->datasetGroupCount(); ++i )
  {
    const QgsMeshDatasetGroupMetadata meta = dp->datasetGroupMetadata( i );
    if ( meta.name() == group )
    {
      groupIndex = i;
      break;
    }
  }

  if ( groupIndex < 0 )
    return; //not found

  const int datasetCount = dp->datasetCount( groupIndex );
  if ( datasetCount < 1 )
    return; // group without datasets


  // find maximum and minimum time in this group
  const double minTime = dp->datasetMetadata( QgsMeshDatasetIndex( groupIndex, 0 ) ).time();
  int idx = mStartTimeComboBox->findData( minTime );
  if ( idx >= 0 )
    mStartTimeComboBox->setCurrentIndex( idx );

  const double maxTime = dp->datasetMetadata( QgsMeshDatasetIndex( groupIndex, datasetCount - 1 ) ).time();
  idx = mEndTimeComboBox->findData( maxTime );
  if ( idx >= 0 )
    mEndTimeComboBox->setCurrentIndex( idx );
}

void QgsMeshCalculatorDialog::repopulateTimeCombos()
{
  QgsMeshLayer *layer = meshLayer();
  if ( !layer || !layer->dataProvider() )
    return;
  const QgsMeshDataProvider *dp = layer->dataProvider();

  // extract all times from all datasets
  QMap<qint64, double> times;

  const QList<int> groupIndexes = layer->datasetGroupsIndexes();
  for ( int dsgi : groupIndexes )
  {
    int datasetCount = layer->datasetCount( QgsMeshDatasetIndex( dsgi, 0 ) );
    for ( int datasetIndex = 0; datasetIndex < datasetCount; ++datasetIndex )
    {
      qint64 timeMs = layer->datasetRelativeTimeInMilliseconds( QgsMeshDatasetIndex( dsgi, datasetIndex ) );
      if ( timeMs == INVALID_MESHLAYER_TIME )
        continue;
      const QgsMeshDatasetMetadata meta = dp->datasetMetadata( QgsMeshDatasetIndex( dsgi, datasetIndex ) );
      times[timeMs] = meta.time();
    }
  }

  mStartTimeComboBox->blockSignals( true );
  mEndTimeComboBox->blockSignals( true );
  mStartTimeComboBox->clear();
  mEndTimeComboBox->clear();

  // populate combos
  for ( auto it = times.constBegin(); it != times.constEnd(); ++it )
  {
    double time = it.value();
    const QString strTime = layer->formatTime( time );
    mStartTimeComboBox->addItem( strTime, time );
    mEndTimeComboBox->addItem( strTime, time );
  }

  mStartTimeComboBox->blockSignals( false );
  mEndTimeComboBox->blockSignals( false );


  if ( !times.empty() )
  {
    mStartTimeComboBox->setCurrentIndex( 0 );
    mEndTimeComboBox->setCurrentIndex( times.size() - 1 );
  }
}
