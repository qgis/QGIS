/***************************************************************************
                         qgsattributetypedialog.cpp  -  description
                             -------------------
    begin                : June 2009
    copyright            : (C) 2000 by Richard Kostecky
    email                : cSf.Kostej@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include "qgsattributetypedialog.h"

#include "qgsattributetypeloaddialog.h"

#include "qgsvectordataprovider.h"
#include "qgslogger.h"

#include <QTableWidgetItem>


QgsAttributeTypeDialog::QgsAttributeTypeDialog( QgsVectorLayer *vl )
    : QDialog(),
    mLayer( vl )
{
  setupUi( this );
  tableWidget->insertRow(0);
  connect( selectionComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( setStackPage( int ) ) );
  connect( removeSelectedButton, SIGNAL( pressed( ) ), this, SLOT( removeSelectedButtonPushed( ) ) );
  connect( loadFromLayerButton, SIGNAL( pressed( ) ), this, SLOT( loadFromLayerButtonPushed( ) ) );
  connect( tableWidget,  SIGNAL( cellChanged( int, int ) ), this, SLOT( vCellChanged( int, int ) ) );
}


QgsAttributeTypeDialog::~QgsAttributeTypeDialog()
{

}

QgsVectorLayer::EditType QgsAttributeTypeDialog::editType()
{
  return mEditType;
}

QgsVectorLayer::RangeData QgsAttributeTypeDialog::rangeData()
{
  return mRangeData;
}

QMap<QString, QVariant> &QgsAttributeTypeDialog::valueMap()
{
  return mValueMap;
}


void QgsAttributeTypeDialog::vCellChanged(int row, int column)
{
  if (row == tableWidget->rowCount() -1)
  {
    tableWidget->insertRow(row + 1);
  } //else check type
}

void QgsAttributeTypeDialog::removeSelectedButtonPushed()
{
  QList<QTableWidgetItem *> list = tableWidget->selectedItems();
  QList<QTableWidgetItem *>::iterator it = list.begin();
  QSet<int> rowsToRemove;
  int removed = 0;
  int i = 0;
  for (; i < list.size(); i++)
  {
    if (list[i]->column() == 0)
    {
      int row = list[i]->row();
      if (!rowsToRemove.contains( row ))
      {
        rowsToRemove.insert( row );
      }
    }
  }
  for (i = 0; i< rowsToRemove.values().size(); i++)
  {
    tableWidget->removeRow( rowsToRemove.values()[i] - removed );
    removed++;
  }
}

void QgsAttributeTypeDialog::loadFromLayerButtonPushed()
{
  QgsAttributeTypeLoadDialog layerDialog( mLayer );
  if ( !layerDialog.exec() )
    return;

  tableWidget->clearContents();
  for (int i = tableWidget->rowCount() -1; i > 0; i--)
  {
    tableWidget->removeRow(i);
  }
  int row = 0;
  QMap<QString, QVariant> &map = layerDialog.valueMap();
  for ( QMap<QString, QVariant>::iterator mit = map.begin(); mit != map.end(); mit++, row++ )
  {
    tableWidget->insertRow( row );
    if ( mit.value().isNull() )
    {
      tableWidget->setItem( row, 0, new QTableWidgetItem( mit.key() ) );
    }
    else
    {
      tableWidget->setItem( row, 0, new QTableWidgetItem( mit.key() ) );
      tableWidget->setItem( row, 1, new QTableWidgetItem( mit.value().toString() ) );
    }
  }

}


void QgsAttributeTypeDialog::setPageForIndex( int index )
{
  if ( mLayer->editType( index ) ==  QgsVectorLayer::LineEdit )
  {
    setPage( 0 );
  }
  else if ( mLayer->editType( index ) == QgsVectorLayer::Classification)
  {
    setPage( 1 );
  }
  else if ( mLayer->editType( index ) == QgsVectorLayer::EditRange ||
            mLayer->editType( index ) == QgsVectorLayer::SliderRange )
  {
    setPage( 2 );
  }
  else if ( mLayer->editType( index ) == QgsVectorLayer::UniqueValues ||
            mLayer->editType( index ) == QgsVectorLayer::UniqueValuesEditable )
  {
    setPage( 3 );
  }
  else if ( mLayer->editType( index ) == QgsVectorLayer::FileName)
  {
    setPage( 4 );
  }
  else if ( mLayer->editType( index ) == QgsVectorLayer::ValueMap)
  {
    setPage( 5 );
  }
  else if ( mLayer->editType( index ) == QgsVectorLayer::Enumeration)
  {
    setPage( 6 );
  }
  else if ( mLayer->editType( index ) == QgsVectorLayer::Immutable)
  {
    setPage( 7 );
  }
}

void QgsAttributeTypeDialog::setPageForEditType( QgsVectorLayer::EditType editType )
{
  if ( editType ==  QgsVectorLayer::LineEdit )
  {
    setPage( 0 );
  }
  else if ( editType == QgsVectorLayer::Classification)
  {
    setPage( 1 );
  }
  else if ( editType == QgsVectorLayer::EditRange ||
            editType == QgsVectorLayer::SliderRange )
  {
    setPage( 2 );
  }
  else if ( editType == QgsVectorLayer::UniqueValues ||
            editType == QgsVectorLayer::UniqueValuesEditable )
  {
    setPage( 3 );
  }
  else if ( editType == QgsVectorLayer::FileName)
  {
    setPage( 4 );
  }
  else if ( editType == QgsVectorLayer::ValueMap)
  {
    setPage( 5 );
  }
  else if ( editType == QgsVectorLayer::Enumeration)
  {
    setPage( 6 );
  }
  else if ( editType == QgsVectorLayer::Immutable)
  {
    setPage( 7 );
  }
}

void QgsAttributeTypeDialog::setValueMap(QMap<QString, QVariant> valueMap)
{
  mValueMap = valueMap;
}

void QgsAttributeTypeDialog::setRange(QgsVectorLayer::RangeData range)
{
  mRangeData = range;
}

void QgsAttributeTypeDialog::setIndex( int index, int editTypeInt )
{
  mIndex = index;
  //need to set index for combobox
  QgsVectorLayer::EditType editType;
  if (editTypeInt > -1)
  {
    editType = QgsVectorLayer::EditType( editTypeInt );
  }
  else
  {
    editType = mLayer->editType( index );
  }

  setWindowTitle( defaultWindowTitle() + " \"" + mLayer->pendingFields()[index].name() + "\"");
  QgsAttributeList attributeList = QgsAttributeList();
  attributeList.append( index );
  mLayer->select( attributeList, QgsRectangle(), false );

  QgsFeature f;
  QString text;
  //calculate min and max for range for this field
  if (mLayer->pendingFields()[index].type() == QVariant::Int)
  {
    sliderRadioButton->setDisabled( false );
    int min;
    int max;
    //filling initial values
    if (mLayer->nextFeature( f ))
    {
      min = f.attributeMap()[index].toInt();
      max = f.attributeMap()[index].toInt();
    }
    for ( ; mLayer->nextFeature( f );  )
    {
      QVariant val = f.attributeMap()[index];
      if ( val.isValid() && !val.isNull() )
      {
        int valInt = val.toInt();
        if (min > valInt)
            min = valInt;
        if (max < valInt)
            max = valInt;
      }
      text = tr( "Current minimum for this value is %1 and current maximum is %2." ).arg( min ).arg( max );
    }
  }
  else if (mLayer->pendingFields()[index].type() == QVariant::Double)
  {
    double dMin;
    double dMax;

    if (mLayer->nextFeature( f ))
    {
      dMin = f.attributeMap()[index].toDouble();
      dMax = f.attributeMap()[index].toDouble();
    }

    sliderRadioButton->setDisabled( true );
    editableRadioButton->setChecked( true );
    for ( ; mLayer->nextFeature( f );  )
    {
      QVariant val = f.attributeMap()[index];
      if ( val.isValid() && !val.isNull() )
      {
        double dVal =  val.toDouble();
        if (dMin > dVal)
            dMin = dVal;
        if (dMax < dVal)
            dMax = dVal;
      }
      text = tr( "Current minimum for this value is %1 and current maximum is %2." ).arg( dMin ).arg( dMax );
    }
  }
  else
  {
    text = tr( "Attribute has no integer or real type, therefore range is not usable.");
  }
  valuesLabel->setText(text);

  //setPageForIndex( index );
  setPageForEditType( editType );

  if ( editType == QgsVectorLayer::ValueMap)
  {

    tableWidget->clearContents();
    for (int i = tableWidget->rowCount() -1; i > 0; i--)
    {
      tableWidget->removeRow(i);
    }

    // if some value map already present use it
    QMap<QString, QVariant> map;
    if (!mValueMap.empty())
    {
      map = mValueMap;
    }
    else
    {
      map = mLayer->valueMap( index );
    }

    int row = 0;
    for ( QMap<QString, QVariant>::iterator mit = map.begin(); mit != map.end(); mit++, row++ )
    {
      tableWidget->insertRow( row );
      if ( mit.value().isNull() )
      {
        tableWidget->setItem( row, 0, new QTableWidgetItem( mit.key() ) );
      }
      else
      {
        tableWidget->setItem( row, 0, new QTableWidgetItem( mit.value().toString() ) );
        tableWidget->setItem( row, 1, new QTableWidgetItem( mit.key() ) );
      }
    }

  }
  else if ( editType == QgsVectorLayer::EditRange ||
            editType == QgsVectorLayer::SliderRange )
  {
    if (mLayer->pendingFields()[mIndex].type() != QVariant::Int)
    {
      minimumSpinBox->setValue( mLayer->range( index ).mMin.toInt() );
      maximumSpinBox->setValue( mLayer->range( index ).mMax.toInt() );
      stepSpinBox->setValue( mLayer->range( index ).mStep.toInt() );
    }
    else if ( mLayer->pendingFields()[mIndex].type() == QVariant::Double )
    {
      minimumDoubleSpinBox->setValue( mLayer->range( index ).mMin.toDouble() );
      maximumDoubleSpinBox->setValue( mLayer->range( index ).mMax.toDouble() );
      stepDoubleSpinBox->setValue( mLayer->range( index ).mStep.toDouble() );
    }
    if ( editType == QgsVectorLayer::EditRange )
      editableRadioButton->setChecked( true );
    else //slider range
      sliderRadioButton->setChecked( true );
  }
}


void QgsAttributeTypeDialog::setPage( int index )
{
  this->selectionComboBox->setCurrentIndex( index );
  setStackPage( index );
}

void QgsAttributeTypeDialog::setStackPage( int index )
{
  this->stackedWidget->setCurrentIndex( index );

  bool okDisabled = false;
  if ( index == 2 )
  {
    if (mLayer->pendingFields()[mIndex].type() != QVariant::Double &&
        mLayer->pendingFields()[mIndex].type() != QVariant::Int)
    {
      okDisabled = true;
    }
    else if (mLayer->pendingFields()[mIndex].type() != QVariant::Double)
    {
      this->rangeStackedWidget->setCurrentIndex( 0 );
      //load data
      minimumSpinBox->setValue( mRangeData.mMin.toInt() );
      maximumSpinBox->setValue( mRangeData.mMax.toInt() );
      stepSpinBox->setValue( mRangeData.mStep.toInt() );
    }
    else
    {
      this->rangeStackedWidget->setCurrentIndex( 1 );
      //load data
      minimumDoubleSpinBox->setValue( mRangeData.mMin.toDouble() );
      maximumDoubleSpinBox->setValue( mRangeData.mMax.toDouble() );
      stepDoubleSpinBox->setValue( mRangeData.mStep.toDouble() );
    }
  }
  else if ( index == 6 )
  {
    QStringList list;
    mLayer->dataProvider()->enumValues( mIndex, list );
    if ( list.size() == 0 )
    {
      okDisabled = true;
      enumerationWarningLabel->setText( tr( "Enumeration is not available for this attribute" ) );
    }
    else
    {
        enumerationWarningLabel->setText( "" );
    }

  }
  stackedWidget->currentWidget()->setDisabled( okDisabled );
  buttonBox->button( QDialogButtonBox::Ok )->setDisabled( okDisabled );
}

void QgsAttributeTypeDialog::accept()
{
  //store data to output variables
  switch (selectionComboBox->currentIndex())
  {
    case 0:
      mEditType = QgsVectorLayer::LineEdit;
      break;
    case 1:
      mEditType = QgsVectorLayer::Classification;
      break;
    case 2:
      //store range data
      if ( mLayer->pendingFields()[mIndex].type() == QVariant::Int )
      {
        mRangeData = QgsVectorLayer::RangeData( minimumSpinBox->value(),
                                                maximumSpinBox->value(),
                                                stepSpinBox->value() );
      }
      else
      {
        mRangeData = QgsVectorLayer::RangeData( minimumDoubleSpinBox->value(),
                                                maximumDoubleSpinBox->value(),
                                                stepDoubleSpinBox->value() );
      }
      //select correct one
      if (editableRadioButton->isChecked())
      {
        mEditType = QgsVectorLayer::EditRange;
      }
      else
      {
        mEditType = QgsVectorLayer::SliderRange;
      }
      break;
    case 3:
      if (editableUniqueValues->isChecked())
      {
        mEditType = QgsVectorLayer::UniqueValuesEditable;
      }
      else
      {
        mEditType = QgsVectorLayer::UniqueValues;
      }
      break;
    case 4:
      mEditType = QgsVectorLayer::FileName;
      break;
    case 5:
      //store data to map
      mValueMap.clear();
      for (int i = 0; i < tableWidget->rowCount() - 1; i++)
      {
        if ( tableWidget->item(i, 1)->text().isNull() )
        {
          mValueMap.insert(tableWidget->item(i, 0)->text(), tableWidget->item(i, 0)->text());
        }
        else
        {
          mValueMap.insert(tableWidget->item(i, 1)->text(), tableWidget->item(i, 0)->text());
        }
      }
      mEditType = QgsVectorLayer::ValueMap;
      break;
    case 6:
      mEditType = QgsVectorLayer::Enumeration;
      break;
    case 7:
      mEditType = QgsVectorLayer::Immutable;
      break;
    default:
      mEditType = QgsVectorLayer::LineEdit;
  }

  QDialog::accept();
}

QString QgsAttributeTypeDialog::defaultWindowTitle()
{
  return tr( "Attribute Edit Dialog" );
}
