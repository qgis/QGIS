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
#include "qgsexpression.h"
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
    return;

  builder->setLayer( vl );
  builder->loadFieldNames();

  populateFields();
  populateOutputFieldTypes();

  connect( builder, SIGNAL( expressionParsed( bool ) ), this, SLOT( setOkButtonState() ) );

  //default values for field width and precision
  mOuputFieldWidthSpinBox->setValue( 10 );
  mOutputFieldPrecisionSpinBox->setValue( 3 );

  // disable creation of new fields if not supported by data provider
  if ( !( vl->dataProvider()->capabilities() & QgsVectorDataProvider::AddAttributes ) )
  {
    mUpdateExistingGroupBox->setCheckable( false );
    mNewFieldGroupBox->setChecked( false );
    mNewFieldGroupBox->setTitle( mNewFieldGroupBox->title() + tr( " (not supported by provider)" ) );
  }

  if ( vl->selectedFeaturesIds().size() > 0 )
  {
    mOnlyUpdateSelectedCheckBox->setChecked( true );
  }
}

QgsFieldCalculator::~QgsFieldCalculator()
{

}

void QgsFieldCalculator::accept()
{

  QString calcString = builder->getExpressionString();
  QgsExpression exp( calcString );

  if ( !mVectorLayer || !mVectorLayer->isEditable() )
    return;

  if ( ! exp.prepare( mVectorLayer->pendingFields() ) )
  {
    QMessageBox::critical( 0, tr( "Evaluation error" ), exp.evalErrorString() );
    return;
  }

  mVectorLayer->beginEditCommand( "Field calculator" );

  //update existing field
  if ( mUpdateExistingGroupBox->isChecked() )
  {
    QMap<QString, int>::const_iterator fieldIt = mFieldMap.find( mExistingFieldComboBox->currentText() );
    if ( fieldIt != mFieldMap.end() )
    {
      mAttributeId = fieldIt.value();
    }
  }
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
  QString error;

  bool onlySelected = ( mOnlyUpdateSelectedCheckBox->isChecked() );
  QgsFeatureIds selectedIds = mVectorLayer->selectedFeaturesIds();

  // block layerModified signals (that would trigger table update)
  mVectorLayer->blockSignals( true );

  bool useGeometry = exp.needsGeometry();
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

    exp.setCurrentRowNumber( rownum );

    QVariant value = exp.evaluate( &feature );
    if ( exp.hasEvalError() )
    {
      calculationSuccess = false;
      error = exp.evalErrorString();
      break;
    }
    else
    {
      mVectorLayer->changeAttributeValue( feature.id(), mAttributeId, value, false );
    }

    rownum++;
  }

  // stop blocking layerModified signals and make sure that one layerModified signal is emitted
  mVectorLayer->blockSignals( false );
  mVectorLayer->setModified( true, false );

  if ( !calculationSuccess )
  {
    QMessageBox::critical( 0, tr( "Error" ), tr( "An error occured while evaluating the calculation string:\n%1" ).arg( error ) );
    mVectorLayer->destroyEditCommand();
    return;
  }

  mVectorLayer->endEditCommand();
  QDialog::accept();
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

void QgsFieldCalculator::on_mNewFieldGroupBox_toggled( bool on )
{
  mUpdateExistingGroupBox->setChecked( !on );
}

void QgsFieldCalculator::on_mUpdateExistingGroupBox_toggled( bool on )
{
  mNewFieldGroupBox->setChecked( !on );
  setOkButtonState();
}


void QgsFieldCalculator::on_mOutputFieldNameLineEdit_textChanged( const QString &text )
{
  Q_UNUSED( text );
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

void QgsFieldCalculator::populateFields()
{
  if ( !mVectorLayer )
    return;

  const QgsFieldMap fieldMap = mVectorLayer->pendingFields();
  QgsFieldMap::const_iterator fieldIt = fieldMap.constBegin();
  for ( ; fieldIt != fieldMap.constEnd(); ++fieldIt )
  {

    QString fieldName = fieldIt.value().name();

    //insert into field list and field combo box
    mFieldMap.insert( fieldName, fieldIt.key() );
    mExistingFieldComboBox->addItem( fieldName );
  }
}

void QgsFieldCalculator::setOkButtonState()
{
  QPushButton* okButton = mButtonBox->button( QDialogButtonBox::Ok );
  okButton->setToolTip( "" );

  bool emptyFieldName = mOutputFieldNameLineEdit->text().isEmpty();
  bool expressionValid = builder->isExpressionValid();

  if ( emptyFieldName )
    okButton->setToolTip( tr( "Please enter a field name" ) );

  if ( !expressionValid )
    okButton->setToolTip( okButton->toolTip() + tr( "\n The expression is invalid see (more info) for details" ) );

  bool okEnabled = ( !emptyFieldName || mUpdateExistingGroupBox->isChecked() ) && expressionValid;

  okButton->setEnabled( okEnabled );
}
