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

  QgsDistanceArea myDa;
  myDa.setSourceCrs( vl->crs().srsid() );
  myDa.setEllipsoidalMode( QgisApp::instance()->mapCanvas()->mapSettings().hasCrsTransformEnabled() );
  myDa.setEllipsoid( QgsProject::instance()->readEntry( "Measure", "/Ellipsoid", GEO_NONE ) );
  builder->setGeomCalculator( myDa );

  //default values for field width and precision
  mOutputFieldWidthSpinBox->setValue( 10 );
  mOutputFieldPrecisionSpinBox->setValue( 3 );

  if ( vl->providerType() == "ogr" && vl->storageType() == "ESRI Shapefile" )
  {
    mOutputFieldNameLineEdit->setMaxLength( 10 );
  }

  if ( !( vl->dataProvider()->capabilities() & QgsVectorDataProvider::AddAttributes ) )
  {
    mCreateVirtualFieldCheckbox->setChecked( true );
    mCreateVirtualFieldCheckbox->setEnabled( false );
    mOnlyVirtualFieldsInfoLabel->setVisible( true );
    mInfoIcon->setVisible( true );
  }
  else
  {
    mOnlyVirtualFieldsInfoLabel->setVisible( false );
    mInfoIcon->setVisible( false );
  }

  if ( !( vl->dataProvider()->capabilities() & QgsVectorDataProvider::ChangeAttributeValues ) )
  {
    mUpdateExistingGroupBox->setEnabled( false );
    mCreateVirtualFieldCheckbox->setChecked( true );
    mCreateVirtualFieldCheckbox->setEnabled( false );
  }

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

  if (( mNewFieldGroupBox->isChecked() && mCreateVirtualFieldCheckbox->isChecked() ) || mVectorLayer->isEditable() )
  {
    mEditModeAutoTurnOnLabel->setVisible( false );
    mInfoIcon->setVisible( false );
  }
  else
  {
    mInfoIcon->setVisible( true );
  }

  bool hasselection = vl->selectedFeatureCount() > 0;
  mOnlyUpdateSelectedCheckBox->setChecked( hasselection );
  mOnlyUpdateSelectedCheckBox->setEnabled( hasselection );
  mOnlyUpdateSelectedCheckBox->setText( tr( "Only update %1 selected features" ).arg( vl->selectedFeatureCount() ) );

  builder->loadRecent( "fieldcalc" );

  mInfoIcon->setPixmap( style()->standardPixmap( QStyle::SP_MessageBoxInformation ) );

  setOkButtonState();

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/QgsFieldCalculator/geometry" ).toByteArray() );
}

QgsFieldCalculator::~QgsFieldCalculator()
{
  QSettings settings;
  settings.setValue( "/Windows/QgsFieldCalculator/geometry", saveGeometry() );
}

void QgsFieldCalculator::accept()
{
  builder->saveToRecent( "fieldcalc" );

  if ( !mVectorLayer )
    return;

  // Set up QgsDistanceArea each time we (re-)calculate
  QgsDistanceArea myDa;

  myDa.setSourceCrs( mVectorLayer->crs().srsid() );
  myDa.setEllipsoidalMode( QgisApp::instance()->mapCanvas()->mapSettings().hasCrsTransformEnabled() );
  myDa.setEllipsoid( QgsProject::instance()->readEntry( "Measure", "/Ellipsoid", GEO_NONE ) );


  QString calcString = builder->expressionText();
  QgsExpression exp( calcString );
  exp.setGeomCalculator( myDa );

  if ( ! exp.prepare( mVectorLayer->pendingFields() ) )
  {
    QMessageBox::critical( 0, tr( "Evaluation error" ), exp.evalErrorString() );
    return;
  }

  if ( mNewFieldGroupBox->isChecked() && mCreateVirtualFieldCheckbox->isChecked() )
  {
    mVectorLayer->addExpressionField( calcString, fieldDefinition() );
  }
  else
  {
    if ( !mVectorLayer->isEditable() )
      mVectorLayer->startEditing();

    QApplication::setOverrideCursor( Qt::WaitCursor );

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
      const QgsField newField = fieldDefinition();

      if ( !mVectorLayer->addAttribute( newField ) )
      {
        QApplication::restoreOverrideCursor();
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

      if ( ! exp.prepare( mVectorLayer->pendingFields() ) )
      {
        QApplication::restoreOverrideCursor();
        QMessageBox::critical( 0, tr( "Evaluation error" ), exp.evalErrorString() );
        return;
      }
    }

    if ( mAttributeId == -1 )
    {
      mVectorLayer->destroyEditCommand();
      QApplication::restoreOverrideCursor();
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

    const QgsField& field = mVectorLayer->pendingFields()[mAttributeId];

    bool newField = !mUpdateExistingGroupBox->isChecked();
    QVariant emptyAttribute;
    if ( newField )
      emptyAttribute = QVariant( field.type() );

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
      QVariant value = exp.evaluate( &feature );
      field.convertCompatible( value );
      if ( exp.hasEvalError() )
      {
        calculationSuccess = false;
        error = exp.evalErrorString();
        break;
      }
      else
      {
        mVectorLayer->changeAttributeValue( feature.id(), mAttributeId, value, newField ? emptyAttribute : feature.attributes().value( mAttributeId ) );
      }

      rownum++;
    }

    QApplication::restoreOverrideCursor();

    if ( !calculationSuccess )
    {
      QMessageBox::critical( 0, tr( "Error" ), tr( "An error occured while evaluating the calculation string:\n%1" ).arg( error ) );
      mVectorLayer->destroyEditCommand();
      return;
    }

    mVectorLayer->endEditCommand();
  }
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
  if ( on && !( mVectorLayer->dataProvider()->capabilities() & QgsVectorDataProvider::AddAttributes ) )
  {
    mOnlyVirtualFieldsInfoLabel->setVisible( true );
  }
  else
  {
    mOnlyVirtualFieldsInfoLabel->setVisible( false );
  }

  if (( mNewFieldGroupBox->isChecked() && mCreateVirtualFieldCheckbox->isChecked() ) || mVectorLayer->isEditable() )
  {
    mEditModeAutoTurnOnLabel->setVisible( false );
  }
  else
  {
    mEditModeAutoTurnOnLabel->setVisible( true );
  }

  mInfoIcon->setVisible( mOnlyVirtualFieldsInfoLabel->isVisible() || mEditModeAutoTurnOnLabel->isVisible() );
}

void QgsFieldCalculator::on_mUpdateExistingGroupBox_toggled( bool on )
{
  mNewFieldGroupBox->setChecked( !on );
  setOkButtonState();

  if ( on )
  {
    mOnlyUpdateSelectedCheckBox->setEnabled( mVectorLayer->selectedFeatureCount() > 0 );
  }
  else
  {
    on_mCreateVirtualFieldCheckbox_stateChanged( mCreateVirtualFieldCheckbox->checkState() );
  }
}

void QgsFieldCalculator::on_mCreateVirtualFieldCheckbox_stateChanged( int state )
{
  mOnlyUpdateSelectedCheckBox->setChecked( false );
  mOnlyUpdateSelectedCheckBox->setEnabled( state != Qt::Checked && mVectorLayer->selectedFeatureCount() > 0 );

  if (( mNewFieldGroupBox->isChecked() && mCreateVirtualFieldCheckbox->isChecked() ) || mVectorLayer->isEditable() )
  {
    mEditModeAutoTurnOnLabel->setVisible( false );
  }
  else
  {
    mEditModeAutoTurnOnLabel->setVisible( true );
  }
  mInfoIcon->setVisible( mOnlyVirtualFieldsInfoLabel->isVisible() || mEditModeAutoTurnOnLabel->isVisible() );
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
