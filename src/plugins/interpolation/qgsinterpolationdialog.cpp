
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

  //enter available layers into the combo box
  QMap<QString, QgsMapLayer*> mapLayers = QgsMapLayerRegistry::instance()->mapLayers();
  QMap<QString, QgsMapLayer*>::iterator layer_it = mapLayers.begin();

  for ( ; layer_it != mapLayers.end(); ++layer_it )
  {
    QgsVectorLayer* vl = dynamic_cast<QgsVectorLayer*>( layer_it.value() );
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
  if ( !mInterpolatorDialog )
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
    QComboBox* itemCombo = dynamic_cast<QComboBox*>( mLayersTreeWidget->itemWidget( mLayersTreeWidget->topLevelItem( i ), 2 ) );
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
  QgsGridFileWriter theWriter( theInterpolator, fileName, combinedLayerExtent, mNumberOfColumnsSpinBox->value(), mNumberOfRowsSpinBox->value() );
  if ( theWriter.writeFile( true ) == 0 )
  {
    mIface->addRasterLayer( fileName, "Interpolation" );
    accept();
  }

  delete theInterpolator;
}

void QgsInterpolationDialog::on_mInputLayerComboBox_currentIndexChanged( const QString& text )
{
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
      return dynamic_cast<QgsVectorLayer*>( layer_it.value() );
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
