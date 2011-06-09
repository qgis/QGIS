#include "qgsellipsesymbollayerv2widget.h"
#include "qgsellipsesymbollayerv2.h"
#include "qgsmaplayerregistry.h"
#include "qgsvectorlayer.h"
#include <QColorDialog>

QgsEllipseSymbolLayerV2Widget::QgsEllipseSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent ): QgsSymbolLayerV2Widget( parent, vl )
{
  setupUi( this );
  QStringList names;
  names << "circle" << "rectangle" << "cross" << "triangle";
  QSize iconSize = mShapeListWidget->iconSize();

  QStringList::const_iterator nameIt = names.constBegin();
  for(; nameIt != names.constEnd(); ++nameIt )
  {
    QgsEllipseSymbolLayerV2* lyr = new QgsEllipseSymbolLayerV2();
    lyr->setSymbolName( *nameIt );
    lyr->setOutlineColor( QColor( 0, 0, 0 ) );
    lyr->setFillColor( QColor( 200, 200, 200 ) );
    lyr->setSymbolWidth(4);
    lyr->setSymbolHeight(2);
    QIcon icon = QgsSymbolLayerV2Utils::symbolLayerPreviewIcon( lyr, QgsSymbolV2::MM, iconSize );
    QListWidgetItem* item = new QListWidgetItem( icon, QString(), mShapeListWidget );
    item->setData( Qt::UserRole, *nameIt );
    delete lyr;
  }

  blockComboSignals( true );
  fillDataDefinedComboBoxes();
  blockComboSignals( false );
}

void QgsEllipseSymbolLayerV2Widget::setSymbolLayer( QgsSymbolLayerV2* layer )
{
  if( layer->layerType() != "EllipseMarker" )
  {
    return;
  }

  mLayer = static_cast<QgsEllipseSymbolLayerV2*>( layer );
  mWidthSpinBox->setValue( mLayer->symbolWidth() );
  mHeightSpinBox->setValue( mLayer->symbolHeight() );
  mOutlineWidthSpinBox->setValue( mLayer->outlineWidth() );

  QList<QListWidgetItem *> symbolItemList = mShapeListWidget->findItems( mLayer->symbolName(), Qt::MatchExactly );
  if( symbolItemList.size() > 0 )
  {
    mShapeListWidget->setCurrentItem( symbolItemList.at( 0 ) );
  }

  //set combo entries to current values
  blockComboSignals( true );
  if( mLayer )
  {
    if( mLayer->widthField().first != -1 )
    {
      mDDSymbolWidthComboBox->setCurrentIndex( mDDSymbolWidthComboBox->findText( mLayer->widthField().second ) );
    }
    if( mLayer->heightField().first != -1 )
    {
      mDDSymbolHeightComboBox->setCurrentIndex( mDDSymbolHeightComboBox->findText( mLayer->heightField().second ) );
    }
    if( mLayer->outlineWidthField().first != -1 )
    {
      mDDOutlineWidthComboBox->setCurrentIndex( mDDOutlineWidthComboBox->findText( mLayer->outlineWidthField().second ) );
    }
    if( mLayer->fillColorField().first != -1 )
    {
      mDDFillColorComboBox->setCurrentIndex( mDDFillColorComboBox->findText( mLayer->fillColorField().second ) );
    }
    if( mLayer->outlineColorField().first != -1 )
    {
      mDDOutlineColorComboBox->setCurrentIndex( mDDOutlineColorComboBox->findText( mLayer->outlineColorField().second ) );
    }
  }
  blockComboSignals( false );
}

QgsSymbolLayerV2* QgsEllipseSymbolLayerV2Widget::symbolLayer()
{
  return mLayer;
}

void QgsEllipseSymbolLayerV2Widget::on_mShapeListWidget_itemSelectionChanged()
{
  if( mLayer )
  {
    QListWidgetItem* item = mShapeListWidget->currentItem();
    if( item )
    {
      mLayer->setSymbolName( item->data( Qt::UserRole ).toString() );
      emit changed();
    }
  }
}

void QgsEllipseSymbolLayerV2Widget::on_mWidthSpinBox_valueChanged( double d )
{
  if( mLayer )
  {
    mLayer->setSymbolWidth( d );
    emit changed();
  }
}

void QgsEllipseSymbolLayerV2Widget::on_mHeightSpinBox_valueChanged( double d )
{
  if( mLayer )
  {
    mLayer->setSymbolHeight( d );
    emit changed();
  }
}

void QgsEllipseSymbolLayerV2Widget::on_mOutlineWidthSpinBox_valueChanged( double d )
{
  if( mLayer )
  {
    mLayer->setOutlineWidth( d );
    emit changed();
  }
}

void QgsEllipseSymbolLayerV2Widget::on_btnChangeColorBorder_clicked()
{
  if( mLayer )
  {
    QColor newColor = QColorDialog::getColor( mLayer->outlineColor() );
    if( newColor.isValid() )
    {
      mLayer->setOutlineColor( newColor );
      emit changed();
    }
  }
}

void QgsEllipseSymbolLayerV2Widget::on_btnChangeColorFill_clicked()
{
  if( mLayer )
  {
    QColor newColor = QColorDialog::getColor( mLayer->fillColor() );
    if( newColor.isValid() )
    {
      mLayer->setFillColor( newColor );
      emit changed();
    }
  }
}

void QgsEllipseSymbolLayerV2Widget::fillDataDefinedComboBoxes()
{
  mDDSymbolWidthComboBox->clear();
  mDDSymbolWidthComboBox->addItem( "", -1 );
  mDDSymbolHeightComboBox->clear();
  mDDSymbolHeightComboBox->addItem( "", -1 );
  mDDOutlineWidthComboBox->clear();
  mDDOutlineWidthComboBox->addItem( "", -1 );
  mDDFillColorComboBox->clear();
  mDDFillColorComboBox->addItem( "", -1 );
  mDDOutlineColorComboBox->clear();
  mDDOutlineColorComboBox->addItem( "", -1 );
  mDDShapeComboBox->clear();
  mDDShapeComboBox->addItem( "", -1 );

  if( mVectorLayer )
  {
    const QgsFieldMap& fm =mVectorLayer->pendingFields();
    QgsFieldMap::const_iterator fieldIt = fm.constBegin();
    for(; fieldIt != fm.constEnd(); ++fieldIt )
    {
      QString fieldName = fieldIt.value().name();
      int index = fieldIt.key();

      mDDSymbolWidthComboBox->addItem( fieldName, index );
      mDDSymbolHeightComboBox->addItem( fieldName, index );
      mDDOutlineWidthComboBox->addItem( fieldName, index );
      mDDFillColorComboBox->addItem( fieldName, index );
      mDDOutlineColorComboBox->addItem( fieldName, index );
      mDDShapeComboBox->addItem( fieldName, index );
    }
  }
}

void QgsEllipseSymbolLayerV2Widget::on_mDDSymbolWidthComboBox_currentIndexChanged( int idx )
{
  if( mLayer )
  {
    mLayer->setWidthField( mDDSymbolWidthComboBox->itemData( idx ).toInt(),  mDDSymbolWidthComboBox->itemText( idx ) );
  }
}

void QgsEllipseSymbolLayerV2Widget::on_mDDSymbolHeightComboBox_currentIndexChanged( int idx )
{
  if( mLayer )
  {
    mLayer->setHeightField( mDDSymbolHeightComboBox->itemData( idx ).toInt(),  mDDSymbolHeightComboBox->itemText( idx ));
  }
}

void QgsEllipseSymbolLayerV2Widget::on_mDDOutlineWidthComboBox_currentIndexChanged( int idx )
{
  if( mLayer )
  {
    mLayer->setOutlineWidthField( mDDOutlineWidthComboBox->itemData( idx ).toInt(),  mDDOutlineWidthComboBox->itemText( idx ) );
  }
}

void QgsEllipseSymbolLayerV2Widget::on_mDDFillColorComboBox_currentIndexChanged( int idx )
{
  if( mLayer )
  {
    mLayer->setFillColorField( mDDFillColorComboBox->itemData( idx ).toInt(), mDDFillColorComboBox->itemText( idx ) );
  }
}

void QgsEllipseSymbolLayerV2Widget::on_mDDOutlineColorComboBox_currentIndexChanged( int idx )
{
  if( mLayer )
  {
    mLayer->setOutlineColorField( mDDOutlineColorComboBox->itemData( idx ).toInt(), mDDOutlineColorComboBox->itemText( idx ) );
  }
}

void QgsEllipseSymbolLayerV2Widget::on_mDDShapeComboBox_currentIndexChanged( int idx )
{
  //todo...
}

void QgsEllipseSymbolLayerV2Widget::blockComboSignals( bool block )
{
  mDDSymbolWidthComboBox->blockSignals( block );
  mDDSymbolHeightComboBox->blockSignals( block );
  mDDOutlineWidthComboBox->blockSignals( block );
  mDDFillColorComboBox->blockSignals( block );
  mDDOutlineColorComboBox->blockSignals( block);
  mDDShapeComboBox->blockSignals( block );
}
