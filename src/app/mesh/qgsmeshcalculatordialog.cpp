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
#include "qgsmeshlayer.h"
#include "qgssettings.h"
#include "qgsgui.h"
#include "qgsvectorlayer.h"
#include "qgsmaplayerproxymodel.h"
#include "qgswkbtypes.h"
#include "qgsfeatureiterator.h"

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
  getDatasetGroupNames();

  connect( mDatasetsListWidget, &QListWidget::itemDoubleClicked, this, &QgsMeshCalculatorDialog::mDatasetsListWidget_doubleClicked );
  connect( mCurrentLayerExtentButton, &QPushButton::clicked, this, &QgsMeshCalculatorDialog::mCurrentLayerExtentButton_clicked );
  connect( mAllTimesButton, &QPushButton::clicked, this, &QgsMeshCalculatorDialog::mAllTimesButton_clicked );
  connect( mExpressionTextEdit, &QTextEdit::textChanged, this, &QgsMeshCalculatorDialog::mExpressionTextEdit_textChanged );

  connect( useMaskCb, &QCheckBox::stateChanged, this, &QgsMeshCalculatorDialog::toggleExtendMask );
  connect( useExtentCb, &QCheckBox::stateChanged, this, &QgsMeshCalculatorDialog::toggleExtendMask );
  maskBox->setVisible( false );

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

  QgsSettings settings;
  mOutputDatasetFileWidget->setStorageMode( QgsFileWidget::SaveFile );
  mOutputDatasetFileWidget->setDialogTitle( tr( "Enter mesh dataset file" ) );
  mOutputDatasetFileWidget->setDefaultRoot( settings.value( QStringLiteral( "/MeshCalculator/lastOutputDir" ), QDir::homePath() ).toString() );
  connect( mOutputDatasetFileWidget, &QgsFileWidget::fileChanged, this, [ = ]() { setAcceptButtonState(); } );
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
  QString ret = mOutputDatasetFileWidget->filePath();
  return addSuffix( ret );
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

QgsGeometry QgsMeshCalculatorDialog::maskGeometry( QgsVectorLayer *layer ) const
{
  QgsFeatureIterator it = layer->getFeatures();
  QVector<QgsGeometry> geometries;
  QgsFeature feat;
  while ( it.nextFeature( feat ) )
  {
    geometries.push_back( feat.geometry() );
  }
  QgsGeometry ret = QgsGeometry::unaryUnion( geometries ) ;
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
  if ( useExtentCb->isChecked() )
  {
    calc.reset(
      new QgsMeshCalculator(
        formulaString(),
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
        outputFile(),
        maskGeometry(),
        startTime(),
        endTime(),
        meshLayer()
      )
    );
  }
  return calc;
}

void QgsMeshCalculatorDialog::toggleExtendMask( int state )
{
  Q_UNUSED( state )
  if ( useMaskCb->checkState() == Qt::Checked )
  {
    extendBox->setVisible( false );
    maskBox->setVisible( true );
  }
  else
  {
    extendBox->setVisible( true );
    maskBox->setVisible( false );
  }
}

void QgsMeshCalculatorDialog::mDatasetsListWidget_doubleClicked( QListWidgetItem *item )
{
  if ( !item )
    return;

  const QString group = quoteDatasetGroupEntry( item->text() );
  mExpressionTextEdit->insertPlainText( QStringLiteral( " %1 " ).arg( group ) );
}

void QgsMeshCalculatorDialog::mCurrentLayerExtentButton_clicked()
{
  useFullLayerExtent();
}

void QgsMeshCalculatorDialog::mAllTimesButton_clicked()
{
  useAllTimesFromLayer();
}

void QgsMeshCalculatorDialog::mExpressionTextEdit_textChanged()
{
  if ( expressionValid() )
  {
    mExpressionValidLabel->setText( tr( "Expression Valid" ) );
    if ( filePathValid() )
    {
      mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( true );
      return;
    }
  }
  else
  {
    mExpressionValidLabel->setText( tr( "Expression invalid or datasets type not supported" ) );
  }
  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
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

void QgsMeshCalculatorDialog::setAcceptButtonState()
{
  if ( expressionValid() && filePathValid() )
    mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( true );
  else
    mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
}

QString QgsMeshCalculatorDialog::quoteDatasetGroupEntry( const QString group )
{
  QString ret( group );
  ret = QStringLiteral( "\"%1\"" ).arg( ret.replace( "\"", "\\\"" ) );
  return ret;
}

void QgsMeshCalculatorDialog::getDatasetGroupNames()
{
  if ( !meshLayer() || !meshLayer()->dataProvider() )
    return;

  const QgsMeshDataProvider *dp = meshLayer()->dataProvider();
  Q_ASSERT( dp );

  for ( int i = 0; i < dp->datasetGroupCount(); ++i )
  {
    const QgsMeshDatasetGroupMetadata meta = dp->datasetGroupMetadata( i );
    mDatasetsListWidget->addItem( meta.name() );
  }
}

bool QgsMeshCalculatorDialog::expressionValid() const
{
  QgsMeshCalculator::Result result = QgsMeshCalculator::expression_valid(
                                       formulaString(),
                                       meshLayer()
                                     );
  return ( result == QgsMeshCalculator::Success );
}

bool QgsMeshCalculatorDialog::filePathValid() const
{
  QString outputPath = outputFile();
  if ( outputPath.isEmpty() )
    return false;

  outputPath = QFileInfo( outputPath ).absolutePath();
  return QFileInfo( outputPath ).isWritable();
}

QString QgsMeshCalculatorDialog::addSuffix( const QString fileName ) const
{
  if ( fileName.isEmpty() )
    return fileName;

  // TODO construct list from MDAL and drivers
  // that can be used to write data
  // for now, MDAL only supports dat files
  const QString allowedSuffix = QStringLiteral( ".dat" );

  if ( fileName.endsWith( allowedSuffix ) )
    return fileName;

  return fileName + allowedSuffix;
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
  if ( mDatasetsListWidget->count() == 0 )
    return QString();

  const QList<QListWidgetItem *> items = mDatasetsListWidget->selectedItems();
  if ( !items.empty() )
    return items[0]->text();
  else
    return mDatasetsListWidget->item( 0 )->text();
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

  int datasetCount = dp->datasetCount( groupIndex );
  if ( datasetCount < 1 )
    return; // group without datasets


  // find maximum and minimum time in this group
  double minTime = dp->datasetMetadata( QgsMeshDatasetIndex( groupIndex, 0 ) ).time();
  int idx = mStartTimeComboBox->findData( minTime );
  if ( idx >= 0 )
    mStartTimeComboBox->setCurrentIndex( idx );

  double maxTime = dp->datasetMetadata( QgsMeshDatasetIndex( groupIndex, datasetCount - 1 ) ).time();
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
  QMap<QString, double> times;

  for ( int groupIndex = 0; groupIndex < dp->datasetGroupCount(); ++groupIndex )
  {
    for ( int datasetIndex = 0; datasetIndex < dp->datasetCount( groupIndex ); ++datasetIndex )
    {
      const QgsMeshDatasetMetadata meta = dp->datasetMetadata( QgsMeshDatasetIndex( groupIndex, datasetIndex ) );
      const double time = meta.time();
      const QString timestr = layer->formatTime( time );

      times[timestr] = time;
    }
  }

  // sort by text
  auto keys = times.keys();
  keys.sort();

  mStartTimeComboBox->blockSignals( true );
  mEndTimeComboBox->blockSignals( true );
  mStartTimeComboBox->clear();
  mEndTimeComboBox->clear();

  // populate combos
  for ( const QString &key : keys )
  {
    mStartTimeComboBox->addItem( key, times[key] );
    mEndTimeComboBox->addItem( key, times[key] );
  }

  mStartTimeComboBox->blockSignals( false );
  mEndTimeComboBox->blockSignals( false );


  if ( !times.empty() )
  {
    mStartTimeComboBox->setCurrentIndex( 0 );
    mEndTimeComboBox->setCurrentIndex( times.size() - 1 );
  }
}
