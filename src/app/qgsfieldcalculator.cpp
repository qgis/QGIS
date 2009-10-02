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
#include "qgsvectorlayer.h"
#include <QMessageBox>

QgsFieldCalculator::QgsFieldCalculator( QgsVectorLayer* vl ): QDialog(), mVectorLayer( vl )
{
  setupUi( this );
  mOutputFieldTypeComboBox->addItem( tr( "Double" ) );
  mOutputFieldTypeComboBox->addItem( tr( "Integer" ) );
  mOutputFieldTypeComboBox->addItem( tr( "String" ) );

  populateFields();

  //default values for field width and precision
  mOuputFieldWidthSpinBox->setValue( 10 );
  mOutputFieldPrecisionSpinBox->setValue( 3 );

  mUpdateExistingFieldCheckBox->setCheckState( Qt::Checked );

  //disable ok button until there is text for output field and expression
  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
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

    int attributeId = -1; //id of the field (can be existing field or newly created one

    //update existing field
    if ( mUpdateExistingFieldCheckBox->checkState() == Qt::Checked )
    {
      QMap<QString, int>::const_iterator fieldIt = mFieldMap.find( mExistingFieldComboBox->currentText() );
      if ( fieldIt != mFieldMap.end() )
      {
        attributeId = fieldIt.value();
      }
    }
    //create new field
    else
    {
      //create new field
      QgsField newField( mOutputFieldNameLineEdit->text() );
      if ( mOutputFieldTypeComboBox->currentText() == tr( "Double" ) )
      {
        newField.setType( QVariant::Double );
      }
      else if ( mOutputFieldTypeComboBox->currentText() == tr( "Integer" ) )
      {
        newField.setType( QVariant::Int );
      }
      else if ( mOutputFieldTypeComboBox->currentText() == tr( "String" ) )
      {
        newField.setType( QVariant::String );
      }

      newField.setLength( mOuputFieldWidthSpinBox->value() );
      newField.setPrecision( mOutputFieldPrecisionSpinBox->value() );

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
          attributeId = it.key();
          break;
        }
      }
    }


    if ( attributeId == -1 )
    {
      mVectorLayer->destroyEditCommand();
      return;
    }

    //go through all the features and change the new attribute
    QgsFeature feature;
    bool calculationSuccess = true;

    bool onlySelected = ( mOnlyUpdateSelectedCheckBox->checkState() == Qt::Checked );
    QgsFeatureIds selectedIds = mVectorLayer->selectedFeaturesIds();


    mVectorLayer->select( mVectorLayer->pendingAllAttributesList(), QgsRectangle(), false, false );
    while ( mVectorLayer->nextFeature( feature ) )
    {
      if ( onlySelected )
      {
        if ( !selectedIds.contains( feature.id() ) )
        {
          continue;
        }
      }

      QgsSearchTreeValue value = searchTree->valueAgainst( mVectorLayer->pendingFields(), feature.attributeMap() );
      if ( value.isError() )
      {
        calculationSuccess = false;
        break;
      }
      if ( value.isNumeric() )
      {
        mVectorLayer->changeAttributeValue( feature.id(), attributeId, value.number(), false );
      }
      else
      {
        mVectorLayer->changeAttributeValue( feature.id(), attributeId, value.string(), false );
      }

    }

    if ( !calculationSuccess )
    {
      QMessageBox::critical( 0, tr( "Error" ), tr( "An error occured while evaluating the calculation string." ) );

      //remove new attribute
      mVectorLayer->deleteAttribute( attributeId );
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

void QgsFieldCalculator::on_mSamplePushButton_clicked()
{
  getFieldValues( 25 );
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

void QgsFieldCalculator::on_mOutputFieldTypeComboBox_currentIndexChanged( const QString& text )
{
  if ( text == tr( "Double" ) )
  {
    mOutputFieldPrecisionSpinBox->setEnabled( true );
  }
  else
  {
    mOutputFieldPrecisionSpinBox->setEnabled( false );
  }
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


