
/***************************************************************************
                              qgsinterpolationdialog.cpp
                              --------------------------
  begin                : Marco 10, 2008
  copyright            : (C) 2008 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsinterpolationdialog.h"
#include "qgsinterpolatordialog.h"
#include "qgsfield.h"
#include "qgsgridfilewriter.h"
#include "qgsidwinterpolatordialog.h"
#include "qgstininterpolatordialog.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerregistry.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include <QComboBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>


QgsInterpolationDialog::QgsInterpolationDialog( QWidget* parent, QgisInterface* iface ): QDialog( parent ), mIface( iface ), mInterpolatorDialog( 0 )
{
  setupUi( this );

  QSettings settings;
  restoreGeometry( settings.value( "/Interpolation/geometry" ).toByteArray() );

  //enter available layers into the combo box
  QMap<QString, QgsMapLayer*> mapLayers = QgsMapLayerRegistry::instance()->mapLayers();
  QMap<QString, QgsMapLayer*>::iterator layer_it = mapLayers.begin();

  for ( ; layer_it != mapLayers.end(); ++layer_it )
  {
    QgsVectorLayer* vl = qobject_cast<QgsVectorLayer *>( layer_it.value() );
    if ( vl )
    {
      mInputLayerComboBox->insertItem( 0, vl->name() );
    }
  }

  //default resolution 300 * 300
  mNumberOfColumnsSpinBox->setValue( 300 );
  mNumberOfRowsSpinBox->setValue( 300 );

  //only inverse distance weighting available for now
  mInterpolationMethodComboBox->insertItem( 0, tr( "Triangular interpolation (TIN)" ) );
  mInterpolationMethodComboBox->insertItem( 1, tr( "Inverse Distance Weighting (IDW)" ) );
  mInterpolationMethodComboBox->setCurrentIndex( settings.value( "/Interpolation/lastMethod", 0 ).toInt() );

  enableOrDisableOkButton();
}

QgsInterpolationDialog::~QgsInterpolationDialog()
{
}

void QgsInterpolationDialog::enableOrDisableOkButton()
{
  bool enabled = true;

  //no input data
  if ( mLayersTreeWidget->topLevelItemCount() < 1 )
  {
    enabled = false;
  }
  else
  {
    QString fileName = mOutputFileLineEdit->text();
    QFileInfo theFileInfo( fileName );
    if ( fileName.isEmpty() || !theFileInfo.dir().exists() )
    {
      enabled = false;
    }
  }

  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( enabled );
}

void QgsInterpolationDialog::on_buttonBox_accepted()
{
  saveState();

  if ( !mInterpolatorDialog )
  {
    return;
  }

  QgsRectangle outputBBox = currentBoundingBox();
  if ( outputBBox.isEmpty() )
  {
    return;
  }

  //warn the user if there isn't any input layer
  if ( mLayersTreeWidget->topLevelItemCount() < 1 )
  {
    QMessageBox::information( 0, tr( "No input data for interpolation" ), tr( "Please add one or more input layers" ) );
    return;
  }

  //read file name
  QString fileName = mOutputFileLineEdit->text();
  QFileInfo theFileInfo( fileName );
  if ( fileName.isEmpty() || !theFileInfo.dir().exists() )
  {
    QMessageBox::information( 0, tr( "Output file name invalid" ), tr( "Please enter a valid output file name" ) );
    return;
  }

  //add .asc suffix if the user did not provider it already
  QString suffix = theFileInfo.suffix();
  if ( suffix.isEmpty() )
  {
    fileName.append( ".asc" );
  }

  int nLayers = mLayersTreeWidget->topLevelItemCount();
  QList< QgsInterpolator::LayerData > inputLayerList;

  for ( int i = 0; i < nLayers; ++i )
  {
    QString layerName = mLayersTreeWidget->topLevelItem( i )->text( 0 );
    QgsVectorLayer* theVectorLayer = vectorLayerFromName( layerName );
    if ( !theVectorLayer )
    {
      continue;
    }

    QgsVectorDataProvider* theProvider = theVectorLayer->dataProvider();
    if ( !theProvider )
    {
      continue;
    }

    QgsInterpolator::LayerData currentLayerData;
    currentLayerData.vectorLayer = theVectorLayer;

    QString interpolationAttString = mLayersTreeWidget->topLevelItem( i )->text( 1 );
    if ( interpolationAttString == "Z_COORD" )
    {
      currentLayerData.zCoordInterpolation = true;
      currentLayerData.interpolationAttribute = -1;
    }
    else
    {
      currentLayerData.zCoordInterpolation = false;
      int attributeIndex = theProvider->fieldNameIndex( interpolationAttString );
      currentLayerData.interpolationAttribute = attributeIndex;
    }

    //type (point/structure line/ breakline)
    QComboBox* itemCombo = qobject_cast<QComboBox *>( mLayersTreeWidget->itemWidget( mLayersTreeWidget->topLevelItem( i ), 2 ) );
    if ( itemCombo )
    {
      QString typeString = itemCombo->currentText();
      if ( typeString == tr( "Break lines" ) )
      {
        currentLayerData.mInputType = QgsInterpolator::BREAK_LINES;
      }
      else if ( typeString == tr( "Structure lines" ) )
      {
        currentLayerData.mInputType = QgsInterpolator::STRUCTURE_LINES;
      }
      else //Points
      {
        currentLayerData.mInputType = QgsInterpolator::POINTS;
      }
    }
    else
    {
      currentLayerData.mInputType = QgsInterpolator::POINTS;
    }
    inputLayerList.push_back( currentLayerData );
  }

  mInterpolatorDialog->setInputData( inputLayerList );
  QgsInterpolator* theInterpolator = mInterpolatorDialog->createInterpolator();

  if ( !theInterpolator )
  {
    return;
  }

  //create grid file writer
  QgsGridFileWriter theWriter( theInterpolator, fileName, outputBBox, mNumberOfColumnsSpinBox->value(),
                               mNumberOfRowsSpinBox->value(), mCellsizeXSpinBox->value(), mCellSizeYSpinBox->value() );
  if ( theWriter.writeFile( true ) == 0 )
  {
    mIface->addRasterLayer( fileName, QFileInfo( fileName ).baseName() );
    accept();
  }

  delete theInterpolator;
}

void QgsInterpolationDialog::on_buttonBox_rejected()
{
  saveState();
  reject();
}

void QgsInterpolationDialog::on_mInputLayerComboBox_currentIndexChanged( const QString& text )
{
  Q_UNUSED( text );
  mInterpolationAttributeComboBox->clear();
  mUseZCoordCheckBox->setEnabled( false );

  //get current vector layer
  QString currentComboText = mInputLayerComboBox->currentText();
  QgsVectorLayer* theVectorLayer = vectorLayerFromName( currentComboText );

  if ( !theVectorLayer )
  {
    return;
  }

  QgsVectorDataProvider* provider = theVectorLayer->dataProvider();
  if ( !provider )
  {
    return;
  }

  //find out if the layer has 25D type
  QGis::WkbType geomType = provider->geometryType();
  if ( geomType == QGis::WKBPoint25D ||
       geomType == QGis::WKBLineString25D ||
       geomType == QGis::WKBPolygon25D ||
       geomType == QGis::WKBMultiPoint25D ||
       geomType == QGis::WKBMultiLineString25D ||
       geomType == QGis::WKBMultiPolygon25D )
  {
    mUseZCoordCheckBox->setEnabled( true );
  }

  //insert numeric attributes of layer into mInterpolationAttributesComboBox
  const QgsFieldMap& fields = provider->fields();
  QgsFieldMap::const_iterator field_it = fields.constBegin();
  for ( ; field_it != fields.constEnd(); ++field_it )
  {
    QgsField currentField = field_it.value();
    QVariant::Type currentType = currentField.type();
    if ( currentType == QVariant::Int || currentType == QVariant::Double )
    {
      mInterpolationAttributeComboBox->insertItem( 0, currentField.name() );
    }
  }
}

void QgsInterpolationDialog::on_mAddPushButton_clicked()
{
  //read active layer in mInputLayerComboBox
  QString inputLayer = mInputLayerComboBox->currentText();

  //read attribute / z-coordinate interpolation
  QString interpolationAttribute;
  if ( mUseZCoordCheckBox->checkState() == Qt::Checked )
  {
    interpolationAttribute = "Z_COORD";
  }
  else
  {
    interpolationAttribute = mInterpolationAttributeComboBox->currentText();
  }

  QTreeWidgetItem* newLayerItem = new QTreeWidgetItem();
  newLayerItem->setText( 0, inputLayer );
  newLayerItem->setText( 1, interpolationAttribute );

  mLayersTreeWidget->addTopLevelItem( newLayerItem );
  QComboBox* typeComboBox = new QComboBox();
  typeComboBox->addItem( tr( "Points" ) );
  typeComboBox->addItem( tr( "Structure lines" ) );
  typeComboBox->addItem( tr( "Break lines" ) );
  typeComboBox->setCurrentIndex( 0 );
  mLayersTreeWidget->setItemWidget( newLayerItem, 2, typeComboBox );

  //keep bounding box up to date
  setLayersBoundingBox();

  enableOrDisableOkButton();
}

void QgsInterpolationDialog::on_mRemovePushButton_clicked()
{
  QTreeWidgetItem* currentItem = mLayersTreeWidget->currentItem();
  if ( !currentItem )
  {
    return;
  }
  delete currentItem;
  enableOrDisableOkButton();
}

void QgsInterpolationDialog::on_mOutputFileButton_clicked()
{
  //get last output file dir
  QSettings s;
  QString lastOutputDir = s.value( "/Interpolation/lastOutputDir", "" ).toString();

  QString rasterFileName = QFileDialog::getSaveFileName( 0, tr( "Save interpolated raster as..." ), lastOutputDir );
  if ( !rasterFileName.isEmpty() )
  {
    mOutputFileLineEdit->setText( rasterFileName );
    QFileInfo rasterFileInfo( rasterFileName );
    QDir fileDir = rasterFileInfo.absoluteDir();
    if ( fileDir.exists() )
    {
      s.setValue( "/Interpolation/lastOutputDir", rasterFileInfo.absolutePath() );
    }
  }
  enableOrDisableOkButton();
}

void QgsInterpolationDialog::on_mOutputFileLineEdit_textChanged()
{
  if ( mOutputFileLineEdit->text().endsWith( ".asc" ) )
  {
    enableOrDisableOkButton();
  }
}

void QgsInterpolationDialog::on_mConfigureInterpolationButton_clicked()
{
  if ( mInterpolatorDialog )
  {
    mInterpolatorDialog->exec();
  }
}

QgsVectorLayer* QgsInterpolationDialog::vectorLayerFromName( const QString& name )
{
  QMap<QString, QgsMapLayer*> mapLayers = QgsMapLayerRegistry::instance()->mapLayers();
  QMap<QString, QgsMapLayer*>::iterator layer_it = mapLayers.begin();

  for ( ; layer_it != mapLayers.end(); ++layer_it )
  {
    if ( layer_it.value()->name() == name )
    {
      return qobject_cast<QgsVectorLayer *>( layer_it.value() );
      break;
    }
  }

  return 0;
}

void QgsInterpolationDialog::on_mInterpolationMethodComboBox_currentIndexChanged( const QString &text )
{
  delete mInterpolatorDialog;
  if ( text == tr( "Inverse Distance Weighting (IDW)" ) )
  {
    mInterpolatorDialog = new QgsIDWInterpolatorDialog( 0, mIface );
  }
  else if ( text == tr( "Triangular interpolation (TIN)" ) )
  {
    mInterpolatorDialog = new QgsTINInterpolatorDialog( 0, mIface );
  }
}

void QgsInterpolationDialog::on_mNumberOfColumnsSpinBox_valueChanged( int value )
{
  Q_UNUSED( value );
  setNewCellsizeXOnNColumnsChange();
}

void QgsInterpolationDialog::on_mNumberOfRowsSpinBox_valueChanged( int value )
{
  Q_UNUSED( value );
  setNewCellsizeYOnNRowschange();
}

void QgsInterpolationDialog::on_mCellsizeXSpinBox_valueChanged( double value )
{
  Q_UNUSED( value );
  setNColsOnCellsizeXChange();
}

void QgsInterpolationDialog::on_mCellSizeYSpinBox_valueChanged( double value )
{
  Q_UNUSED( value );
  setNRowsOnCellsizeYChange();
}

void QgsInterpolationDialog::on_mXMinLineEdit_textEdited( const QString& text )
{
  Q_UNUSED( text );
  setNewCellsizeOnBoundingBoxChange();
}

void QgsInterpolationDialog::on_mXMaxLineEdit_textEdited( const QString& text )
{
  Q_UNUSED( text );
  setNewCellsizeOnBoundingBoxChange();
}

void QgsInterpolationDialog::on_mYMinLineEdit_textEdited( const QString& text )
{
  Q_UNUSED( text );
  setNewCellsizeOnBoundingBoxChange();
}

void QgsInterpolationDialog::on_mYMaxLineEdit_textEdited( const QString& text )
{
  Q_UNUSED( text );
  setNewCellsizeOnBoundingBoxChange();
}

void QgsInterpolationDialog::on_mBBoxToCurrentExtent_clicked()
{
  if ( mIface )
  {
    QgsMapCanvas* canvas = mIface->mapCanvas();
    if ( canvas )
    {
      QgsRectangle extent = canvas->extent();
      mXMinLineEdit->setText( QString::number( extent.xMinimum() ) );
      mXMaxLineEdit->setText( QString::number( extent.xMaximum() ) );
      mYMinLineEdit->setText( QString::number( extent.yMinimum() ) );
      mYMaxLineEdit->setText( QString::number( extent.yMaximum() ) );
      setNewCellsizeOnBoundingBoxChange();
    }
  }
}

QgsRectangle QgsInterpolationDialog::boundingBoxOfLayers()
{
  int nLayers = mLayersTreeWidget->topLevelItemCount();
  QList< QgsInterpolator::LayerData > inputLayerList;
  QgsRectangle combinedLayerExtent;

  for ( int i = 0; i < nLayers; ++i )
  {
    QString layerName = mLayersTreeWidget->topLevelItem( i )->text( 0 );
    QgsVectorLayer* theVectorLayer = vectorLayerFromName( layerName );
    if ( !theVectorLayer )
    {
      continue;
    }

    QgsVectorDataProvider* theProvider = theVectorLayer->dataProvider();
    if ( !theProvider )
    {
      continue;
    }

    //update extent
    QgsRectangle currentLayerExtent = theVectorLayer->extent();
    if ( combinedLayerExtent.isEmpty() )
    {
      combinedLayerExtent = currentLayerExtent;
    }
    else
    {
      combinedLayerExtent.combineExtentWith( &currentLayerExtent );
    }
  }
  return combinedLayerExtent;
}

void QgsInterpolationDialog::setLayersBoundingBox()
{
  QgsRectangle layersBoundingBox = boundingBoxOfLayers();
  mXMinLineEdit->setText( QString::number( layersBoundingBox.xMinimum() ) );
  mXMaxLineEdit->setText( QString::number( layersBoundingBox.xMaximum() ) );
  mYMinLineEdit->setText( QString::number( layersBoundingBox.yMinimum() ) );
  mYMaxLineEdit->setText( QString::number( layersBoundingBox.yMaximum() ) );
  setNewCellsizeOnBoundingBoxChange();
}

void QgsInterpolationDialog::setNewCellsizeOnBoundingBoxChange()
{
  QgsRectangle currentBbox = currentBoundingBox();
  if ( currentBbox.isEmpty() )
  {
    return;
  }

  if ( currentBbox.width() > 0 && mNumberOfColumnsSpinBox->value() > 0 )
  {
    mCellsizeXSpinBox->blockSignals( true );
    mCellsizeXSpinBox->setValue( currentBbox.width() / mNumberOfColumnsSpinBox->value() );
    mCellsizeXSpinBox->blockSignals( false );
  }
  if ( currentBbox.height() > 0 && mNumberOfRowsSpinBox->value() > 0 )
  {
    mCellSizeYSpinBox->blockSignals( true );
    mCellSizeYSpinBox->setValue( currentBbox.height() / mNumberOfRowsSpinBox->value() );
    mCellSizeYSpinBox->blockSignals( false );
  }
}

void QgsInterpolationDialog::setNewCellsizeXOnNColumnsChange()
{
  QgsRectangle currentBBox = currentBoundingBox();
  if ( !currentBBox.isEmpty() && mNumberOfColumnsSpinBox->value() > 0 )
  {
    mCellsizeXSpinBox->blockSignals( true );
    mCellsizeXSpinBox->setValue( currentBBox.width() / mNumberOfColumnsSpinBox->value() );
    mCellsizeXSpinBox->blockSignals( false );
  }
}

void QgsInterpolationDialog::setNewCellsizeYOnNRowschange()
{
  QgsRectangle currentBBox = currentBoundingBox();
  if ( !currentBBox.isEmpty() && mNumberOfRowsSpinBox->value() > 0 )
  {
    mCellSizeYSpinBox->blockSignals( true );
    mCellSizeYSpinBox->setValue( currentBBox.height() / mNumberOfRowsSpinBox->value() );
    mCellSizeYSpinBox->blockSignals( false );
  }
}

void QgsInterpolationDialog::setNColsOnCellsizeXChange()
{
  QgsRectangle currentBBox = currentBoundingBox();
  int newSize;

  if ( mCellsizeXSpinBox->value() <= 0 )
  {
    return;
  }

  if ( currentBBox.width() <= 0 )
  {
    newSize = 0;
  }
  else
  {
    newSize = ( int )( currentBBox.width() / mCellsizeXSpinBox->value() );
  }

  mNumberOfColumnsSpinBox->blockSignals( true );
  mNumberOfColumnsSpinBox->setValue( newSize );
  mNumberOfColumnsSpinBox->blockSignals( false );
}

void QgsInterpolationDialog::setNRowsOnCellsizeYChange()
{
  QgsRectangle currentBBox = currentBoundingBox();
  int newSize;

  if ( mCellSizeYSpinBox->value() <= 0 )
  {
    return;
  }

  if ( currentBBox.height() <= 0 )
  {
    newSize = 0;
  }
  else
  {
    newSize = ( int )( currentBBox.height() / mCellSizeYSpinBox->value() );
  }

  mNumberOfRowsSpinBox->blockSignals( true );
  mNumberOfRowsSpinBox->setValue( newSize );
  mNumberOfRowsSpinBox->blockSignals( false );
}

QgsRectangle QgsInterpolationDialog::currentBoundingBox()
{
  QString xMinString = mXMinLineEdit->text();
  QString xMaxString = mXMaxLineEdit->text();
  QString yMinString = mYMinLineEdit->text();
  QString yMaxString = mYMaxLineEdit->text();

  bool xMinOk, xMaxOk, yMinOk, yMaxOk;
  double xMin = xMinString.toDouble( &xMinOk );
  double xMax = xMaxString.toDouble( &xMaxOk );
  double yMin = yMinString.toDouble( &yMinOk );
  double yMax = yMaxString.toDouble( &yMaxOk );

  if ( !xMinOk || !xMaxOk || !yMinOk || !yMaxOk )
  {
    QgsRectangle emptyBbox;
    return emptyBbox; //error, return empty bounding box
  }

  return QgsRectangle( xMin, yMin, xMax, yMax );
}

void QgsInterpolationDialog::saveState()
{
  QSettings settings;
  settings.setValue( "/Interpolation/geometry", saveGeometry() );
  settings.setValue( "/Interpolation/lastMethod", mInterpolationMethodComboBox->currentIndex() );
}
