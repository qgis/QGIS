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

#include "qgsattributetypedialog.h"
#include "qgsattributetypeloaddialog.h"
#include "qgsvectordataprovider.h"
#include "qgsmaplayerregistry.h"

#include "qgslogger.h"

#include <QTableWidgetItem>
#include <QFile>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>

#include <climits>
#include <cfloat>

QgsAttributeTypeDialog::QgsAttributeTypeDialog( QgsVectorLayer *vl )
    : QDialog()
    , mLayer( vl )
{
  setupUi( this );
  tableWidget->insertRow( 0 );
  connect( selectionComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( setStackPage( int ) ) );
  connect( removeSelectedButton, SIGNAL( clicked() ), this, SLOT( removeSelectedButtonPushed() ) );
  connect( loadFromLayerButton, SIGNAL( clicked() ), this, SLOT( loadFromLayerButtonPushed() ) );
  connect( loadFromCSVButton, SIGNAL( clicked() ), this, SLOT( loadFromCSVButtonPushed() ) );
  connect( tableWidget, SIGNAL( cellChanged( int, int ) ), this, SLOT( vCellChanged( int, int ) ) );

  valueRelationLayer->clear();
  foreach ( QgsMapLayer *l, QgsMapLayerRegistry::instance()->mapLayers() )
  {
    QgsVectorLayer *vl = qobject_cast< QgsVectorLayer * >( l );
    if ( vl )
      valueRelationLayer->addItem( vl->name(), vl->id() );
  }

  connect( valueRelationLayer, SIGNAL( currentIndexChanged( int ) ), this, SLOT( updateLayerColumns( int ) ) );
  valueRelationLayer->setCurrentIndex( -1 );
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

QgsVectorLayer::ValueRelationData QgsAttributeTypeDialog::valueRelationData()
{
  return mValueRelationData;
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
  Q_UNUSED( column );
  if ( row == tableWidget->rowCount() - 1 )
  {
    tableWidget->insertRow( row + 1 );
  } //else check type
}

void QgsAttributeTypeDialog::removeSelectedButtonPushed()
{
  QList<QTableWidgetItem *> list = tableWidget->selectedItems();
  QSet<int> rowsToRemove;
  int removed = 0;
  int i;
  for ( i = 0; i < list.size(); i++ )
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
    case QgsVectorLayer::LineEdit:
      setPage( 0 );
      break;

    case QgsVectorLayer::Classification:
      setPage( 1 );
      break;

    case QgsVectorLayer::EditRange:
    case QgsVectorLayer::SliderRange:
    case QgsVectorLayer::DialRange:
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
      break;

    case QgsVectorLayer::Calendar:
      setPage( 11 );
      break;

    case QgsVectorLayer::ValueRelation:
      setPage( 12 );
      break;

    case QgsVectorLayer::UuidGenerator:
      setPage( 13 );
      break;
  }
}

void QgsAttributeTypeDialog::setValueMap( QMap<QString, QVariant> valueMap )
{
  mValueMap = valueMap;
}

void QgsAttributeTypeDialog::setRange( QgsVectorLayer::RangeData range )
{
  mRangeData = range;
}

void QgsAttributeTypeDialog::setValueRelation( QgsVectorLayer::ValueRelationData valueRelation )
{
  mValueRelationData = valueRelation;
}

void QgsAttributeTypeDialog::setIndex( int index, QgsVectorLayer::EditType editType )
{
  mIndex = index;
  //need to set index for combobox

  setWindowTitle( defaultWindowTitle() + " \"" + mLayer->pendingFields()[index].name() + "\"" );
  QgsAttributeList attributeList = QgsAttributeList();
  attributeList.append( index );
  mLayer->select( attributeList, QgsRectangle(), false );

  QgsFeature f;
  QString text;
  //calculate min and max for range for this field
  if ( mLayer->pendingFields()[index].type() == QVariant::Int )
  {
    rangeWidget->clear();
    rangeWidget->addItems( QStringList() << tr( "Editable" ) << tr( "Slider" ) << tr( "Dial" ) );
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

    rangeWidget->clear();
    rangeWidget->addItems( QStringList() << tr( "Editable" ) << tr( "Slider" ) );
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

      int row = 0;
      for ( QMap<QString, QVariant>::iterator mit = mValueMap.begin(); mit != mValueMap.end(); mit++, row++ )
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
    case QgsVectorLayer::DialRange:
    {
      if ( mLayer->pendingFields()[mIndex].type() != QVariant::Int )
      {
        minimumSpinBox->setValue( mRangeData.mMin.toInt() );
        maximumSpinBox->setValue( mRangeData.mMax.toInt() );
        stepSpinBox->setValue( mRangeData.mStep.toInt() );
      }
      else if ( mLayer->pendingFields()[mIndex].type() == QVariant::Double )
      {
        minimumDoubleSpinBox->setValue( mRangeData.mMin.toDouble() );
        maximumDoubleSpinBox->setValue( mRangeData.mMax.toDouble() );
        stepDoubleSpinBox->setValue( mRangeData.mStep.toDouble() );
      }
      if ( editType == QgsVectorLayer::EditRange )
      {
        rangeWidget->setCurrentIndex( 0 );
      }
      else if ( editType == QgsVectorLayer::SliderRange )
      {
        rangeWidget->setCurrentIndex( 1 );
      }
      else
      {
        rangeWidget->setCurrentIndex( 2 );
      }
    }
    break;

    case QgsVectorLayer::UniqueValuesEditable:
      editableUniqueValues->setChecked( true );
      break;

    case QgsVectorLayer::ValueRelation:
      valueRelationLayer->setCurrentIndex( valueRelationLayer->findData( mValueRelationData.mLayer ) );
      valueRelationKeyColumn->setCurrentIndex( valueRelationKeyColumn->findText( mValueRelationData.mKey ) );
      valueRelationValueColumn->setCurrentIndex( valueRelationValueColumn->findText( mValueRelationData.mValue ) );
      valueRelationAllowNull->setChecked( mValueRelationData.mAllowNull );
      valueRelationOrderByValue->setChecked( mValueRelationData.mOrderByValue );
      valueRelationAllowMulti->setChecked( mValueRelationData.mAllowMulti );
      break;

    case QgsVectorLayer::LineEdit:
    case QgsVectorLayer::UniqueValues:
    case QgsVectorLayer::Classification:
    case QgsVectorLayer::CheckBox:
    case QgsVectorLayer::FileName:
    case QgsVectorLayer::Enumeration:
    case QgsVectorLayer::Immutable:
    case QgsVectorLayer::Hidden:
    case QgsVectorLayer::TextEdit:
    case QgsVectorLayer::Calendar:
    case QgsVectorLayer::UuidGenerator:
      break;
  }
}


void QgsAttributeTypeDialog::setPage( int index )
{
  selectionComboBox->setCurrentIndex( index );
  setStackPage( index );
}

void QgsAttributeTypeDialog::setStackPage( int index )
{
  stackedWidget->setCurrentIndex( index );

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
      rangeStackedWidget->setCurrentIndex( 0 );
      //load data
      minimumSpinBox->setValue( mRangeData.mMin.toInt() );
      maximumSpinBox->setValue( mRangeData.mMax.toInt() );
      stepSpinBox->setValue( mRangeData.mStep.toInt() );
    }
    else
    {
      rangeStackedWidget->setCurrentIndex( 1 );
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
      switch ( rangeWidget->currentIndex() )
      {
        case 0:
          mEditType = QgsVectorLayer::EditRange;
          break;
        case 1:
          mEditType = QgsVectorLayer::SliderRange;
          break;
        case 2:
          mEditType = QgsVectorLayer::DialRange;
          break;
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
        QTableWidgetItem *ki = tableWidget->item( i, 0 );
        QTableWidgetItem *vi = tableWidget->item( i, 1 );

        if ( !ki )
          continue;

        if ( !vi || vi->text().isNull() )
        {
          mValueMap.insert( ki->text(), ki->text() );
        }
        else
        {
          mValueMap.insert( vi->text(), ki->text() );
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
    case 11:
      mEditType = QgsVectorLayer::Calendar;
      break;
    case 12:
      mEditType = QgsVectorLayer::ValueRelation;
      mValueRelationData.mLayer = valueRelationLayer->itemData( valueRelationLayer->currentIndex() ).toString();
      mValueRelationData.mKey = valueRelationKeyColumn->currentText();
      mValueRelationData.mValue = valueRelationValueColumn->currentText();
      mValueRelationData.mAllowNull = valueRelationAllowNull->isChecked();
      mValueRelationData.mOrderByValue = valueRelationOrderByValue->isChecked();
      mValueRelationData.mAllowMulti = valueRelationAllowMulti->isChecked();
      break;
    case 13:
      mEditType = QgsVectorLayer::UuidGenerator;
      break;
  }

  QDialog::accept();
}

QString QgsAttributeTypeDialog::defaultWindowTitle()
{
  return tr( "Attribute Edit Dialog" );
}

void QgsAttributeTypeDialog::updateLayerColumns( int idx )
{
  valueRelationKeyColumn->clear();
  valueRelationValueColumn->clear();

  QString id = valueRelationLayer->itemData( idx ).toString();

  QgsVectorLayer *vl = qobject_cast< QgsVectorLayer *>( QgsMapLayerRegistry::instance()->mapLayer( id ) );
  if ( !vl )
    return;

  foreach ( const QgsField &f, vl->pendingFields() )
  {
    valueRelationKeyColumn->addItem( f.name() );
    valueRelationValueColumn->addItem( f.name() );
  }

  valueRelationKeyColumn->setCurrentIndex( valueRelationKeyColumn->findText( mValueRelationData.mKey ) );
  valueRelationValueColumn->setCurrentIndex( valueRelationValueColumn->findText( mValueRelationData.mValue ) );
}
