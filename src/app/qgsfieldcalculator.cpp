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

#include "qgisapp.h"
#include "qgsfieldcalculator.h"
#include "qgsdistancearea.h"
#include "qgsexpression.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"

#include <QMessageBox>
#include <QSettings>

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
  mOutputFieldWidthSpinBox->setValue( 10 );
  mOutputFieldPrecisionSpinBox->setValue( 3 );

  mUpdateExistingGroupBox->setEnabled( vl->dataProvider()->capabilities() & QgsVectorDataProvider::ChangeAttributeValues );
  mNewFieldGroupBox->setEnabled( vl->dataProvider()->capabilities() & QgsVectorDataProvider::AddAttributes );

  Q_ASSERT( mNewFieldGroupBox->isEnabled() || mUpdateExistingGroupBox->isEnabled() );

  if ( mNewFieldGroupBox->isEnabled() )
  {
    mNewFieldGroupBox->setChecked( true );
  }
  else
  {
    mNewFieldGroupBox->setToolTip( tr( "Not available for layer" ) );
    mUpdateExistingGroupBox->setChecked( true );
    mUpdateExistingGroupBox->setCheckable( false );
  }

  if ( mUpdateExistingGroupBox->isEnabled() )
  {
    mUpdateExistingGroupBox->setChecked( !mNewFieldGroupBox->isEnabled() );
  }
  else
  {
    mUpdateExistingGroupBox->setToolTip( tr( "Not available for layer" ) );
    mNewFieldGroupBox->setChecked( true );
    mNewFieldGroupBox->setCheckable( false );
  }

  mOnlyUpdateSelectedCheckBox->setChecked( vl->selectedFeaturesIds().size() > 0 );
}

QgsFieldCalculator::~QgsFieldCalculator()
{
}

void QgsFieldCalculator::accept()
{

  // Set up QgsDistanceArea each time we (re-)calculate
  QgsDistanceArea myDa;

  myDa.setSourceCrs( mVectorLayer->crs().srsid() );
  myDa.setEllipsoidalMode( QgisApp::instance()->mapCanvas()->mapRenderer()->hasCrsTransformEnabled() );
  myDa.setEllipsoid( QgsProject::instance()->readEntry( "Measure", "/Ellipsoid", GEO_NONE ) );


  QString calcString = builder->expressionText();
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
  if ( mUpdateExistingGroupBox->isChecked() || !mNewFieldGroupBox->isEnabled() )
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
                       mOutputFieldWidthSpinBox->value(),
                       mOutputFieldPrecisionSpinBox->value() );

    if ( !mVectorLayer->addAttribute( newField ) )
    {
      QMessageBox::critical( 0, tr( "Provider error" ), tr( "Could not add the new field to the provider." ) );
      mVectorLayer->destroyEditCommand();
      return;
    }

    //get index of the new field
    const QgsFields& fields = mVectorLayer->pendingFields();

    for ( int idx = 0; idx < fields.count(); ++idx )
    {
      if ( fields[idx].name() == mOutputFieldNameLineEdit->text() )
      {
        mAttributeId = idx;
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

  bool onlySelected = mOnlyUpdateSelectedCheckBox->isChecked();
  QgsFeatureIds selectedIds = mVectorLayer->selectedFeaturesIds();

  bool useGeometry = exp.needsGeometry();
  int rownum = 1;

  QgsFeatureIterator fit = mVectorLayer->getFeatures( QgsFeatureRequest().setFlags( useGeometry ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry ) );
  while ( fit.nextFeature( feature ) )
  {
    if ( onlySelected )
    {
      if ( !selectedIds.contains( feature.id() ) )
      {
        continue;
      }
    }

    exp.setCurrentRowNumber( rownum );
    exp.setGeomCalculator( myDa );

    QVariant value = exp.evaluate( &feature );
    if ( exp.hasEvalError() )
    {
      calculationSuccess = false;
      error = exp.evalErrorString();
      break;
    }
    else
    {
      // FIXME workaround while QgsVectorLayer::changeAttributeValue's emitSignal is ignored (see #7071)
      mVectorLayer->blockSignals( true );
      mVectorLayer->changeAttributeValue( feature.id(), mAttributeId, value, false );
      mVectorLayer->blockSignals( false );
    }

    rownum++;
  }


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
  mOutputFieldWidthSpinBox->setMinimum( mOutputFieldTypeComboBox->itemData( index, Qt::UserRole + 2 ).toInt() );
  mOutputFieldWidthSpinBox->setMaximum( mOutputFieldTypeComboBox->itemData( index, Qt::UserRole + 3 ).toInt() );
  mOutputFieldWidthSpinBox->setEnabled( mOutputFieldWidthSpinBox->minimum() < mOutputFieldWidthSpinBox->maximum() );
  if ( mOutputFieldWidthSpinBox->value() < mOutputFieldWidthSpinBox->minimum() )
    mOutputFieldWidthSpinBox->setValue( mOutputFieldWidthSpinBox->minimum() );
  if ( mOutputFieldWidthSpinBox->value() > mOutputFieldWidthSpinBox->maximum() )
    mOutputFieldWidthSpinBox->setValue( mOutputFieldWidthSpinBox->maximum() );

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

  const QgsFields& fields = mVectorLayer->pendingFields();
  for ( int idx = 0; idx < fields.count(); ++idx )
  {

    QString fieldName = fields[idx].name();

    //insert into field list and field combo box
    mFieldMap.insert( fieldName, idx );
    mExistingFieldComboBox->addItem( fieldName );
  }
}

void QgsFieldCalculator::setOkButtonState()
{
  QPushButton* okButton = mButtonBox->button( QDialogButtonBox::Ok );

  if (( mNewFieldGroupBox->isChecked() || !mUpdateExistingGroupBox->isEnabled() )
      && mOutputFieldNameLineEdit->text().isEmpty() )
  {
    okButton->setToolTip( tr( "Please enter a field name" ) );
    okButton->setEnabled( false );
    return;
  }

  if ( !builder->isExpressionValid() )
  {
    okButton->setToolTip( okButton->toolTip() + tr( "\n The expression is invalid see (more info) for details" ) );
    okButton->setEnabled( false );
    return;
  }

  okButton->setToolTip( "" );
  okButton->setEnabled( true );
}
