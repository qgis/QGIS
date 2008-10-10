
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
#include <QFileDialog>
#include <QMessageBox>


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
}

QgsInterpolationDialog::~QgsInterpolationDialog()
{

}

void QgsInterpolationDialog::on_buttonBox_accepted()
{
  if ( !mInterpolatorDialog )
  {
    return;
  }

  //read file name
  QString fileName = mOutputFileLineEdit->text();
  QFileInfo theFileInfo( fileName );
  if ( !theFileInfo.dir().exists() )
  {
    QMessageBox::information( 0, "File name invalid", "Please enter a valid file name" );
    return;
  }

  //get Vectorlayer
  QgsVectorLayer* theVectorLayer = getCurrentVectorLayer();
  if ( !theVectorLayer )
  {
    return;
  }

  QgsVectorDataProvider* theProvider = theVectorLayer->dataProvider();
  if ( !theProvider )
  {
    return;
  }

  QPair<QgsVectorLayer*, QgsInterpolator::InputType> layerPair( theVectorLayer, QgsInterpolator::POINTS );
  QList< QPair <QgsVectorLayer*, QgsInterpolator::InputType> > layerInputList;
  layerInputList.push_back( layerPair );
  mInterpolatorDialog->setInputData( layerInputList );
  QgsInterpolator* theInterpolator = mInterpolatorDialog->createInterpolator();

  if ( !theInterpolator )
  {
    return;
  }

  if ( mUseZCoordCheckBox->checkState() == Qt::Checked )
  {
    theInterpolator->enableZCoordInterpolation();
  }
  else
  {
    int attributeIndex = theProvider->fieldNameIndex( mInterpolationAttributeComboBox->currentText() );
    theInterpolator->enableAttributeValueInterpolation( attributeIndex );
  }

  //create grid file writer
  QgsGridFileWriter theWriter( theInterpolator, fileName, theVectorLayer->extent(), mNumberOfColumnsSpinBox->value(), mNumberOfRowsSpinBox->value() );
  if ( theWriter.writeFile( true ) == 0 )
  {
    mIface->addRasterLayer( fileName, "Interpolation" );
    accept();
  }
}

void QgsInterpolationDialog::on_mInputLayerComboBox_currentIndexChanged( const QString& text )
{
  mInterpolationAttributeComboBox->clear();
  mUseZCoordCheckBox->setEnabled( false );

  //get current vector layer
  QgsVectorLayer* theVectorLayer = getCurrentVectorLayer();

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

void QgsInterpolationDialog::on_mOutputFileButton_clicked()
{
  QString rasterFileName = QFileDialog::getSaveFileName( 0 );
  if ( !rasterFileName.isEmpty() )
  {
    mOutputFileLineEdit->setText( rasterFileName );
  }
}

void QgsInterpolationDialog::on_mConfigureInterpolationButton_clicked()
{
  if ( mInterpolatorDialog )
  {
    mInterpolatorDialog->exec();
  }
}

QgsVectorLayer* QgsInterpolationDialog::getCurrentVectorLayer()
{
  QString text = mInputLayerComboBox->currentText();

  QMap<QString, QgsMapLayer*> mapLayers = QgsMapLayerRegistry::instance()->mapLayers();
  QMap<QString, QgsMapLayer*>::iterator layer_it = mapLayers.begin();

  for ( ; layer_it != mapLayers.end(); ++layer_it )
  {
    if ( layer_it.value()->name() == text )
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
