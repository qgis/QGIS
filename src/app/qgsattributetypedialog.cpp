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
#include <QFile>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>

#include <climits>
#include <cfloat>

QgsAttributeTypeDialog::QgsAttributeTypeDialog( QgsVectorLayer *vl )
    : QDialog(),
    mLayer( vl )
{
  setupUi( this );
  tableWidget->insertRow( 0 );
  connect( selectionComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( setStackPage( int ) ) );
  connect( removeSelectedButton, SIGNAL( pressed( ) ), this, SLOT( removeSelectedButtonPushed( ) ) );
  connect( loadFromLayerButton, SIGNAL( pressed( ) ), this, SLOT( loadFromLayerButtonPushed( ) ) );
  connect( loadFromCSVButton, SIGNAL( pressed( ) ), this, SLOT( loadFromCSVButtonPushed( ) ) );
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

QPair<QString, QString> QgsAttributeTypeDialog::checkedState()
{
  return QPair<QString, QString>( leCheckedState->text(), leUncheckedState->text() );
}

void QgsAttributeTypeDialog::setCheckedState( QString checked, QString unchecked )
{
  leCheckedState->setText( checked );
  leUncheckedState->setText( unchecked );
}

void QgsAttributeTypeDialog::vCellChanged( int row, int column )
{
  if ( row == tableWidget->rowCount() - 1 )
  {
    tableWidget->insertRow( row + 1 );
  } //else check type
}

void QgsAttributeTypeDialog::removeSelectedButtonPushed()
{
  QList<QTableWidgetItem *> list = tableWidget->selectedItems();
  QList<QTableWidgetItem *>::iterator it = list.begin();
  QSet<int> rowsToRemove;
  int removed = 0;
  int i = 0;
  for ( ; i < list.size(); i++ )
  {
    if ( list[i]->column() == 0 )
    {
      int row = list[i]->row();
      if ( !rowsToRemove.contains( row ) )
      {
        rowsToRemove.insert( row );
      }
    }
  }
  for ( i = 0; i < rowsToRemove.values().size(); i++ )
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

  updateMap( layerDialog.valueMap() );
}

void QgsAttributeTypeDialog::loadFromCSVButtonPushed()
{
  QString fileName = QFileDialog::getOpenFileName( 0 , tr( "Select a file" ) );
  if ( fileName.isNull() )
    return;

  QFile f( fileName );

  if ( !f.open( QIODevice::ReadOnly ) )
  {
    QMessageBox::information( NULL,
                              tr( "Error" ),
                              tr( "Could not open file %1\nError was:%2" ).arg( fileName ).arg( f.errorString() ),
                              QMessageBox::Cancel );
    return;
  }

  QTextStream s( &f );
  s.setAutoDetectUnicode( true );

  QRegExp re0( "^([^;]*);(.*)$" );
  re0.setMinimal( true );
  QRegExp re1( "^([^,]*),(.*)$" );
  re1.setMinimal( true );
  QMap<QString, QVariant> map;

  s.readLine();

  while ( !s.atEnd() )
  {
    QString l = s.readLine().trimmed();

    QString key, val;
    if ( re0.indexIn( l ) >= 0 && re0.numCaptures() == 2 )
    {
      key = re0.cap( 1 ).trimmed();
      val = re0.cap( 2 ).trimmed();
    }
    else if ( re1.indexIn( l ) >= 0 && re1.numCaptures() == 2 )
    {
      key = re1.cap( 1 ).trimmed();
      val = re1.cap( 2 ).trimmed();
    }
    else
      continue;

    if (( key.startsWith( "\"" ) && key.endsWith( "\"" ) ) ||
        ( key.startsWith( "'" ) && key.endsWith( "'" ) ) )
    {
      key = key.mid( 1, key.length() - 2 );
    }

    if (( val.startsWith( "\"" ) && val.endsWith( "\"" ) ) ||
        ( val.startsWith( "'" ) && val.endsWith( "'" ) ) )
    {
      val = val.mid( 1, val.length() - 2 );
    }

    map[ key ] = val;
  }

  updateMap( map );
}

void QgsAttributeTypeDialog::updateMap( const QMap<QString, QVariant> &map )
{
  tableWidget->clearContents();
  for ( int i = tableWidget->rowCount() - 1; i > 0; i-- )
  {
    tableWidget->removeRow( i );
  }
  int row = 0;
  for ( QMap<QString, QVariant>::const_iterator mit = map.begin(); mit != map.end(); mit++, row++ )
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


void QgsAttributeTypeDialog::setPageForEditType( QgsVectorLayer::EditType editType )
{
  switch ( editType )
  {
    case QgsVectorLayer::Classification:
      setPage( 1 );
      break;

    case QgsVectorLayer::EditRange:
    case QgsVectorLayer::SliderRange:
      setPage( 2 );
      break;

    case QgsVectorLayer::UniqueValues:
    case QgsVectorLayer::UniqueValuesEditable:
      setPage( 3 );
      break;

    case QgsVectorLayer::FileName:
      setPage( 4 );
      break;

    case QgsVectorLayer::ValueMap:
      setPage( 5 );
      break;

    case QgsVectorLayer::Enumeration:
      setPage( 6 );
      break;

    case QgsVectorLayer::Immutable:
      setPage( 7 );
      break;

    case QgsVectorLayer::Hidden:
      setPage( 8 );
      break;

    case QgsVectorLayer::CheckBox:
      setPage( 9 );
      break;

    case QgsVectorLayer::TextEdit:
      setPage( 10 );

    case QgsVectorLayer::LineEdit:
      setPage( 0 );
      break;
  }
}

void QgsAttributeTypeDialog::setPageForIndex( int index )
{
  setPageForEditType( mLayer->editType( index ) );
}

void QgsAttributeTypeDialog::setValueMap( QMap<QString, QVariant> valueMap )
{
  mValueMap = valueMap;
}

void QgsAttributeTypeDialog::setRange( QgsVectorLayer::RangeData range )
{
  mRangeData = range;
}

void QgsAttributeTypeDialog::setIndex( int index, int editTypeInt )
{
  mIndex = index;
  //need to set index for combobox
  QgsVectorLayer::EditType editType;
  if ( editTypeInt > -1 )
  {
    editType = QgsVectorLayer::EditType( editTypeInt );
  }
  else
  {
    editType = mLayer->editType( index );
  }

  setWindowTitle( defaultWindowTitle() + " \"" + mLayer->pendingFields()[index].name() + "\"" );
  QgsAttributeList attributeList = QgsAttributeList();
  attributeList.append( index );
  mLayer->select( attributeList, QgsRectangle(), false );

  QgsFeature f;
  QString text;
  //calculate min and max for range for this field
  if ( mLayer->pendingFields()[index].type() == QVariant::Int )
  {
    sliderRadioButton->setDisabled( false );
    int min = INT_MIN;
    int max = INT_MAX;
    while ( mLayer->nextFeature( f ) )
    {
      QVariant val = f.attributeMap()[index];
      if ( val.isValid() && !val.isNull() )
      {
        int valInt = val.toInt();
        if ( min > valInt )
          min = valInt;
        if ( max < valInt )
          max = valInt;
      }
      text = tr( "Current minimum for this value is %1 and current maximum is %2." ).arg( min ).arg( max );
    }
  }
  else if ( mLayer->pendingFields()[index].type() == QVariant::Double )
  {
    double dMin = -DBL_MAX;
    double dMax = DBL_MAX;

    sliderRadioButton->setDisabled( true );
    editableRadioButton->setChecked( true );
    while ( mLayer->nextFeature( f ) )
    {
      QVariant val = f.attributeMap()[index];
      if ( val.isValid() && !val.isNull() )
      {
        double dVal =  val.toDouble();
        if ( dMin > dVal )
          dMin = dVal;
        if ( dMax < dVal )
          dMax = dVal;
      }
      text = tr( "Current minimum for this value is %1 and current maximum is %2." ).arg( dMin ).arg( dMax );
    }
  }
  else
  {
    text = tr( "Attribute has no integer or real type, therefore range is not usable." );
  }
  valuesLabel->setText( text );

  //setPageForIndex( index );
  setPageForEditType( editType );

  switch ( editType )
  {
    case QgsVectorLayer::ValueMap:
    {

      tableWidget->clearContents();
      for ( int i = tableWidget->rowCount() - 1; i > 0; i-- )
      {
        tableWidget->removeRow( i );
      }

      // if some value map already present use it
      QMap<QString, QVariant> map;
      if ( !mValueMap.empty() )
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
    break;

    case QgsVectorLayer::EditRange:
    case QgsVectorLayer::SliderRange:
    {
      if ( mLayer->pendingFields()[mIndex].type() != QVariant::Int )
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
    break;

    case QgsVectorLayer::UniqueValuesEditable:
      editableUniqueValues->setChecked( editType == QgsVectorLayer::UniqueValuesEditable );
      break;

    default:
      break;
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
    if ( mLayer->pendingFields()[mIndex].type() != QVariant::Double &&
         mLayer->pendingFields()[mIndex].type() != QVariant::Int )
    {
      okDisabled = true;
    }
    else if ( mLayer->pendingFields()[mIndex].type() != QVariant::Double )
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
  switch ( selectionComboBox->currentIndex() )
  {
    default:
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
      if ( editableRadioButton->isChecked() )
      {
        mEditType = QgsVectorLayer::EditRange;
      }
      else
      {
        mEditType = QgsVectorLayer::SliderRange;
      }
      break;
    case 3:
      if ( editableUniqueValues->isChecked() )
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
      for ( int i = 0; i < tableWidget->rowCount() - 1; i++ )
      {
        if ( tableWidget->item( i, 1 )->text().isNull() )
        {
          mValueMap.insert( tableWidget->item( i, 0 )->text(), tableWidget->item( i, 0 )->text() );
        }
        else
        {
          mValueMap.insert( tableWidget->item( i, 1 )->text(), tableWidget->item( i, 0 )->text() );
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
    case 8:
      mEditType = QgsVectorLayer::Hidden;
      break;
    case 9:
      mEditType = QgsVectorLayer::CheckBox;
      break;
    case 10:
      mEditType = QgsVectorLayer::TextEdit;
      break;
  }

  QDialog::accept();
}

QString QgsAttributeTypeDialog::defaultWindowTitle()
{
  return tr( "Attribute Edit Dialog" );
}
