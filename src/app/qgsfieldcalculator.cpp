/***************************************************************************
    qgsfieldcalculator.cpp
    ---------------------
    begin                : September 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco at hugis dot net
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfieldcalculator.h"
#include "qgssearchtreenode.h"
#include "qgssearchstring.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include <QMessageBox>

QgsFieldCalculator::QgsFieldCalculator( QgsVectorLayer* vl )
    : QDialog()
    , mVectorLayer( vl )
    , mAttributeId( -1 )
{
  setupUi( this );

  if ( !vl )
  {
    return;
  }

  populateFields();
  populateOutputFieldTypes();

  //default values for field width and precision
  mOuputFieldWidthSpinBox->setValue( 10 );
  mOutputFieldPrecisionSpinBox->setValue( 3 );

  //disable ok button until there is text for output field and expression
  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( false );

  // disable creation of new fields if not supported by data provider
  if ( !( vl->dataProvider()->capabilities() & QgsVectorDataProvider::AddAttributes ) )
  {
    mUpdateExistingFieldCheckBox->setCheckState( Qt::Checked );
    mUpdateExistingFieldCheckBox->setEnabled( false ); // must stay checked
    mNewFieldGroupBox->setEnabled( false );
    mNewFieldGroupBox->setTitle( mNewFieldGroupBox->title() + tr( " (not supported by provider)" ) );
  }

  if ( vl->selectedFeaturesIds().size() > 0 )
  {
    mOnlyUpdateSelectedCheckBox->setChecked( true );
  }

  if ( vl->geometryType() != QGis::Polygon )
  {
    mAreaButton->setEnabled( false );
  }
  if ( vl->geometryType() != QGis::Line )
  {
    mLengthButton->setEnabled( false );
  }
}

QgsFieldCalculator::~QgsFieldCalculator()
{

}

void QgsFieldCalculator::accept()
{
  if ( mVectorLayer && mVectorLayer->isEditable() )
  {
    QString calcString = mExpressionTextEdit->toPlainText();

    //create QgsSearchString
    QgsSearchString searchString;
    if ( !searchString.setString( calcString ) )
    {
      //expression not valid
      QMessageBox::critical( 0, tr( "Syntax error" ), tr( QString( "Invalid expression syntax. The error message of the parser is: '" + searchString.parserErrorMsg() + "'" ).toLocal8Bit().data() ) );
      return;
    }

    //get QgsSearchTreeNode
    QgsSearchTreeNode* searchTree = searchString.tree();
    if ( !searchTree )
    {
      return;
    }

    mVectorLayer->beginEditCommand( "Field calculator" );

    //update existing field
    if ( mUpdateExistingFieldCheckBox->checkState() == Qt::Checked )
    {
      QMap<QString, int>::const_iterator fieldIt = mFieldMap.find( mExistingFieldComboBox->currentText() );
      if ( fieldIt != mFieldMap.end() )
      {
        mAttributeId = fieldIt.value();
      }
    }
    //create new field
    else
    {
      //create new field
      QgsField newField( mOutputFieldNameLineEdit->text(),
                         ( QVariant::Type ) mOutputFieldTypeComboBox->itemData( mOutputFieldTypeComboBox->currentIndex(), Qt::UserRole ).toInt(),
                         mOutputFieldTypeComboBox->itemData( mOutputFieldTypeComboBox->currentIndex(), Qt::UserRole + 1 ).toString(),
                         mOuputFieldWidthSpinBox->value(),
                         mOutputFieldPrecisionSpinBox->value() );

      if ( !mVectorLayer->addAttribute( newField ) )
      {
        QMessageBox::critical( 0, tr( "Provider error" ), tr( "Could not add the new field to the provider." ) );
        mVectorLayer->destroyEditCommand();
        return;
      }

      //get index of the new field
      const QgsFieldMap fieldList = mVectorLayer->pendingFields();

      QgsFieldMap::const_iterator it = fieldList.constBegin();
      for ( ; it != fieldList.constEnd(); ++it )
      {
        if ( it.value().name() == mOutputFieldNameLineEdit->text() )
        {
          mAttributeId = it.key();
          break;
        }
      }
    }

    if ( mAttributeId == -1 )
    {
      mVectorLayer->destroyEditCommand();
      return;
    }

    //go through all the features and change the new attribute
    QgsFeature feature;
    bool calculationSuccess = true;

    bool onlySelected = ( mOnlyUpdateSelectedCheckBox->checkState() == Qt::Checked );
    QgsFeatureIds selectedIds = mVectorLayer->selectedFeaturesIds();

    // block layerModified signals (that would trigger table update)
    mVectorLayer->blockSignals( true );

    bool useGeometry = calcString.contains( "$area" ) || calcString.contains( "$length" );
    int rownum = 1;

    mVectorLayer->select( mVectorLayer->pendingAllAttributesList(), QgsRectangle(), useGeometry, false );
    while ( mVectorLayer->nextFeature( feature ) )
    {
      if ( onlySelected )
      {
        if ( !selectedIds.contains( feature.id() ) )
        {
          continue;
        }
      }

      searchTree->setCurrentRowNumber( rownum );

      QgsSearchTreeValue value;
      searchTree->getValue( value, searchTree, mVectorLayer->pendingFields(), feature );
      if ( value.isError() )
      {
        //insert NULL value for this feature and continue the calculation
        if ( searchTree->errorMsg() == QObject::tr( "Division by zero." ) )
        {
          mVectorLayer->changeAttributeValue( feature.id(), mAttributeId, QVariant(), false );
        }
        else
        {
          calculationSuccess = false;
          break;
        }
      }
      else if ( value.isNumeric() )
      {
        mVectorLayer->changeAttributeValue( feature.id(), mAttributeId, value.number(), false );
      }
      else if ( value.isNull() )
      {
        mVectorLayer->changeAttributeValue( feature.id(), mAttributeId, QVariant(), false );
      }
      else
      {
        mVectorLayer->changeAttributeValue( feature.id(), mAttributeId, value.string(), false );
      }

      rownum++;
    }

    // stop blocking layerModified signals and make sure that one layerModified signal is emitted
    mVectorLayer->blockSignals( false );
    mVectorLayer->setModified( true, false );

    if ( !calculationSuccess )
    {
      QMessageBox::critical( 0, tr( "Error" ), tr( "An error occured while evaluating the calculation string." ) );
      mVectorLayer->destroyEditCommand();
      return;
    }

    mVectorLayer->endEditCommand();
  }
  QDialog::accept();
}

void QgsFieldCalculator::populateFields()
{
  if ( !mVectorLayer )
  {
    return;
  }

  const QgsFieldMap fieldMap = mVectorLayer->pendingFields();
  QgsFieldMap::const_iterator fieldIt = fieldMap.constBegin();
  for ( ; fieldIt != fieldMap.constEnd(); ++fieldIt )
  {

    QString fieldName = fieldIt.value().name();

    //insert into field list and field combo box
    mFieldMap.insert( fieldName, fieldIt.key() );
    mFieldsListWidget->addItem( fieldName );
    mExistingFieldComboBox->addItem( fieldName );
  }
}

void QgsFieldCalculator::populateOutputFieldTypes()
{
  if ( !mVectorLayer )
  {
    return;
  }

  QgsVectorDataProvider* provider = mVectorLayer->dataProvider();
  if ( !provider )
  {
    return;
  }

  mOutputFieldTypeComboBox->blockSignals( true );
  const QList< QgsVectorDataProvider::NativeType > &typelist = provider->nativeTypes();
  for ( int i = 0; i < typelist.size(); i++ )
  {
    mOutputFieldTypeComboBox->addItem( typelist[i].mTypeDesc );
    mOutputFieldTypeComboBox->setItemData( i, static_cast<int>( typelist[i].mType ), Qt::UserRole );
    mOutputFieldTypeComboBox->setItemData( i, typelist[i].mTypeName, Qt::UserRole + 1 );
    mOutputFieldTypeComboBox->setItemData( i, typelist[i].mMinLen, Qt::UserRole + 2 );
    mOutputFieldTypeComboBox->setItemData( i, typelist[i].mMaxLen, Qt::UserRole + 3 );
    mOutputFieldTypeComboBox->setItemData( i, typelist[i].mMinPrec, Qt::UserRole + 4 );
    mOutputFieldTypeComboBox->setItemData( i, typelist[i].mMaxPrec, Qt::UserRole + 5 );
  }
  mOutputFieldTypeComboBox->blockSignals( false );
  mOutputFieldTypeComboBox->setCurrentIndex( 0 );
  on_mOutputFieldTypeComboBox_activated( 0 );
}

void QgsFieldCalculator::on_mUpdateExistingFieldCheckBox_stateChanged( int state )
{
  if ( state == Qt::Checked )
  {
    mNewFieldGroupBox->setEnabled( false );
  }
  else
  {
    mNewFieldGroupBox->setEnabled( true );
  }
  setOkButtonState();
}

void QgsFieldCalculator::on_mFieldsListWidget_itemDoubleClicked( QListWidgetItem * item )
{
  if ( !item )
  {
    return;
  }
  mExpressionTextEdit->insertPlainText( item->text() );
}

void QgsFieldCalculator::on_mValueListWidget_itemDoubleClicked( QListWidgetItem * item )
{
  if ( !item )
  {
    return;
  }
  mExpressionTextEdit->insertPlainText( " " + item->text() + " " );
}

void QgsFieldCalculator::on_mPlusPushButton_clicked()
{
  mExpressionTextEdit->insertPlainText( " + " );
}

void QgsFieldCalculator::on_mMinusPushButton_clicked()
{
  mExpressionTextEdit->insertPlainText( " - " );
}

void QgsFieldCalculator::on_mMultiplyPushButton_clicked()
{
  mExpressionTextEdit->insertPlainText( " * " );
}

void QgsFieldCalculator::on_mDividePushButton_clicked()
{
  mExpressionTextEdit->insertPlainText( " / " );
}

void QgsFieldCalculator::on_mSqrtButton_clicked()
{
  mExpressionTextEdit->insertPlainText( " sqrt ( " );
}

void QgsFieldCalculator::on_mExpButton_clicked()
{
  mExpressionTextEdit->insertPlainText( " ^ " );
}

void QgsFieldCalculator::on_mSinButton_clicked()
{
  mExpressionTextEdit->insertPlainText( " sin ( " );
}

void QgsFieldCalculator::on_mCosButton_clicked()
{
  mExpressionTextEdit->insertPlainText( " cos ( " );
}

void QgsFieldCalculator::on_mTanButton_clicked()
{
  mExpressionTextEdit->insertPlainText( " tan ( " );
}

void QgsFieldCalculator::on_mASinButton_clicked()
{
  mExpressionTextEdit->insertPlainText( " asin ( " );
}

void QgsFieldCalculator::on_mACosButton_clicked()
{
  mExpressionTextEdit->insertPlainText( " acos ( " );
}

void QgsFieldCalculator::on_mATanButton_clicked()
{
  mExpressionTextEdit->insertPlainText( " atan ( " );
}

void QgsFieldCalculator::on_mOpenBracketPushButton_clicked()
{
  mExpressionTextEdit->insertPlainText( " ( " );
}

void QgsFieldCalculator::on_mCloseBracketPushButton_clicked()
{
  mExpressionTextEdit->insertPlainText( " ) " );
}

void QgsFieldCalculator::on_mToRealButton_clicked()
{
  mExpressionTextEdit->insertPlainText( " to real ( " );
}

void QgsFieldCalculator::on_mToIntButton_clicked()
{
  mExpressionTextEdit->insertPlainText( " to int ( " );
}

void QgsFieldCalculator::on_mToStringButton_clicked()
{
  mExpressionTextEdit->insertPlainText( " to string ( " );
}

void QgsFieldCalculator::on_mLengthButton_clicked()
{
  mExpressionTextEdit->insertPlainText( "$length" );
}

void QgsFieldCalculator::on_mAreaButton_clicked()
{
  mExpressionTextEdit->insertPlainText( "$area" );
}

void QgsFieldCalculator::on_mRowNumButton_clicked()
{
  mExpressionTextEdit->insertPlainText( "$rownum" );
}

void QgsFieldCalculator::on_mConcatButton_clicked()
{
  mExpressionTextEdit->insertPlainText( "||" );
}

void QgsFieldCalculator::on_mAllPushButton_clicked()
{
  getFieldValues( 0 );
}

void QgsFieldCalculator::on_mOutputFieldNameLineEdit_textChanged( const QString& text )
{
  setOkButtonState();
}

void QgsFieldCalculator::on_mExpressionTextEdit_textChanged()
{
  setOkButtonState();
}

void QgsFieldCalculator::on_mOutputFieldTypeComboBox_activated( int index )
{
  mOuputFieldWidthSpinBox->setMinimum( mOutputFieldTypeComboBox->itemData( index, Qt::UserRole + 2 ).toInt() );
  mOuputFieldWidthSpinBox->setMaximum( mOutputFieldTypeComboBox->itemData( index, Qt::UserRole + 3 ).toInt() );
  mOuputFieldWidthSpinBox->setEnabled( mOuputFieldWidthSpinBox->minimum() < mOuputFieldWidthSpinBox->maximum() );
  if ( mOuputFieldWidthSpinBox->value() < mOuputFieldWidthSpinBox->minimum() )
    mOuputFieldWidthSpinBox->setValue( mOuputFieldWidthSpinBox->minimum() );
  if ( mOuputFieldWidthSpinBox->value() > mOuputFieldWidthSpinBox->maximum() )
    mOuputFieldWidthSpinBox->setValue( mOuputFieldWidthSpinBox->maximum() );

  mOutputFieldPrecisionSpinBox->setMinimum( mOutputFieldTypeComboBox->itemData( index, Qt::UserRole + 4 ).toInt() );
  mOutputFieldPrecisionSpinBox->setMaximum( mOutputFieldTypeComboBox->itemData( index, Qt::UserRole + 5 ).toInt() );
  mOutputFieldPrecisionSpinBox->setEnabled( mOutputFieldPrecisionSpinBox->minimum() < mOutputFieldPrecisionSpinBox->maximum() );
  if ( mOutputFieldPrecisionSpinBox->value() < mOutputFieldPrecisionSpinBox->minimum() )
    mOutputFieldPrecisionSpinBox->setValue( mOutputFieldPrecisionSpinBox->minimum() );
  if ( mOutputFieldPrecisionSpinBox->value() > mOutputFieldPrecisionSpinBox->maximum() )
    mOutputFieldPrecisionSpinBox->setValue( mOutputFieldPrecisionSpinBox->maximum() );
}

void QgsFieldCalculator::getFieldValues( int limit )
{
  mValueListWidget->clear();

  if ( !mVectorLayer )
  {
    return;
  }

  QListWidgetItem* currentItem = mFieldsListWidget->currentItem();
  if ( !currentItem )
  {
    return;
  }

  QMap<QString, int>::const_iterator attIt = mFieldMap.find( currentItem->text() );
  if ( attIt == mFieldMap.constEnd() )
  {
    return;
  }

  int attributeIndex = attIt.value();
  QgsField field = mVectorLayer->pendingFields()[attributeIndex];
  bool numeric = ( field.type() == QVariant::Int || field.type() == QVariant::Double );

  QgsAttributeList attList;
  attList << attributeIndex;

  mVectorLayer->select( attList, QgsRectangle(), false );
  QgsFeature f;
  int resultCounter = 0;

  mValueListWidget->setUpdatesEnabled( false );
  mValueListWidget->blockSignals( true );
  QSet<QString> insertedValues;

  while ( mVectorLayer->nextFeature( f ) && ( limit == 0 || resultCounter != limit ) )
  {
    QString value = f.attributeMap()[attributeIndex].toString();
    if ( !numeric )
    {
      value = ( "'" + value + "'" );
    }
    //QList<QListWidgetItem *> existingItems = mValueListWidget->findItems(value, Qt::MatchExactly);
    //if(existingItems.isEmpty())
    if ( !insertedValues.contains( value ) )
    {
      mValueListWidget->addItem( value );
      insertedValues.insert( value );
      ++resultCounter;
    }
  }
  mValueListWidget->setUpdatesEnabled( true );
  mValueListWidget->blockSignals( false );
}

void QgsFieldCalculator::setOkButtonState()
{
  bool okEnabled = true;
  if (( mOutputFieldNameLineEdit->text().isEmpty() && mUpdateExistingFieldCheckBox->checkState() == Qt::Unchecked )\
      || mExpressionTextEdit->toPlainText().isEmpty() )
  {
    okEnabled = false;
  }
  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( okEnabled );
}


void QgsFieldCalculator::on_mFieldsListWidget_currentItemChanged( QListWidgetItem * current, QListWidgetItem * previous )
{
  getFieldValues( 25 );
}
