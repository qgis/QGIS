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
#include "qgsexpressioncontext.h"
#include "qgsgeometry.h"

#include <QMessageBox>
#include <QSettings>

QgsFieldCalculator::QgsFieldCalculator( QgsVectorLayer* vl, QWidget* parent )
    : QDialog( parent )
    , mVectorLayer( vl )
    , mAttributeId( -1 )
{
  setupUi( this );

  if ( !vl )
    return;


  QgsExpressionContext expContext;
  expContext << QgsExpressionContextUtils::globalScope()
  << QgsExpressionContextUtils::projectScope()
  << QgsExpressionContextUtils::layerScope( mVectorLayer );

  expContext.lastScope()->setVariable( "row_number", 1 );
  expContext.setHighlightedVariables( QStringList() << "row_number" );

  builder->setLayer( vl );
  builder->loadFieldNames();
  builder->setExpressionContext( expContext );

  populateFields();
  populateOutputFieldTypes();

  connect( builder, SIGNAL( expressionParsed( bool ) ), this, SLOT( setOkButtonState() ) );
  connect( mOutputFieldWidthSpinBox, SIGNAL( editingFinished() ), this, SLOT( setPrecisionMinMax() ) );

  QgsDistanceArea myDa;
  myDa.setSourceCrs( vl->crs().srsid() );
  myDa.setEllipsoidalMode( QgisApp::instance()->mapCanvas()->mapSettings().hasCrsTransformEnabled() );
  myDa.setEllipsoid( QgsProject::instance()->readEntry( "Measure", "/Ellipsoid", GEO_NONE ) );
  builder->setGeomCalculator( myDa );

  //default values for field width and precision
  mOutputFieldWidthSpinBox->setValue( 10 );
  mOutputFieldPrecisionSpinBox->setValue( 3 );
  setPrecisionMinMax();

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
  exp.setDistanceUnits( QgsProject::instance()->distanceUnits() );
  exp.setAreaUnits( QgsProject::instance()->areaUnits() );

  QgsExpressionContext expContext;
  expContext << QgsExpressionContextUtils::globalScope()
  << QgsExpressionContextUtils::projectScope()
  << QgsExpressionContextUtils::layerScope( mVectorLayer );

  if ( !exp.prepare( &expContext ) )
  {
    QMessageBox::critical( nullptr, tr( "Evaluation error" ), exp.evalErrorString() );
    return;
  }

  bool updatingGeom = false;

  // Test for creating expression field based on ! mUpdateExistingGroupBox checked rather
  // than on mNewFieldGroupBox checked, as if the provider does not support adding attributes
  // then mUpdateExistingGroupBox is set to not checkable, and hence is not checked.  This
  // is a minimum fix to resolve this - better would be some GUI redesign...
  if ( ! mUpdateExistingGroupBox->isChecked() && mCreateVirtualFieldCheckbox->isChecked() )
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
      if ( mExistingFieldComboBox->itemData( mExistingFieldComboBox->currentIndex() ).toString() == "geom" )
      {
        //update geometry
        mAttributeId = -1;
        updatingGeom = true;
      }
      else
      {
        QMap<QString, int>::const_iterator fieldIt = mFieldMap.constFind( mExistingFieldComboBox->currentText() );
        if ( fieldIt != mFieldMap.constEnd() )
        {
          mAttributeId = fieldIt.value();
        }
      }
    }
    else
    {
      //create new field
      const QgsField newField = fieldDefinition();

      if ( !mVectorLayer->addAttribute( newField ) )
      {
        QApplication::restoreOverrideCursor();
        QMessageBox::critical( nullptr, tr( "Provider error" ), tr( "Could not add the new field to the provider." ) );
        mVectorLayer->destroyEditCommand();
        return;
      }

      //get index of the new field
      const QgsFields& fields = mVectorLayer->fields();

      for ( int idx = 0; idx < fields.count(); ++idx )
      {
        if ( fields[idx].name() == mOutputFieldNameLineEdit->text() )
        {
          mAttributeId = idx;
          break;
        }
      }

      //update expression context with new fields
      expContext.setFields( mVectorLayer->fields() );
      if ( ! exp.prepare( &expContext ) )
      {
        QApplication::restoreOverrideCursor();
        QMessageBox::critical( nullptr, tr( "Evaluation error" ), exp.evalErrorString() );
        return;
      }
    }

    if ( mAttributeId == -1 && !updatingGeom )
    {
      mVectorLayer->destroyEditCommand();
      QApplication::restoreOverrideCursor();
      return;
    }

    //go through all the features and change the new attribute
    QgsFeature feature;
    bool calculationSuccess = true;
    QString error;

    bool useGeometry = exp.needsGeometry();
    int rownum = 1;

    QgsField field = !updatingGeom ? mVectorLayer->fields().at( mAttributeId ) : QgsField();

    bool newField = !mUpdateExistingGroupBox->isChecked();
    QVariant emptyAttribute;
    if ( newField )
      emptyAttribute = QVariant( field.type() );

    QgsFeatureRequest req = QgsFeatureRequest().setFlags( useGeometry ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry );
    if ( mOnlyUpdateSelectedCheckBox->isChecked() )
    {
      req.setFilterFids( mVectorLayer->selectedFeaturesIds() );
    }
    QgsFeatureIterator fit = mVectorLayer->getFeatures( req );
    while ( fit.nextFeature( feature ) )
    {
      expContext.setFeature( feature );
      expContext.lastScope()->setVariable( QString( "row_number" ), rownum );

      QVariant value = exp.evaluate( &expContext );
      if ( exp.hasEvalError() )
      {
        calculationSuccess = false;
        error = exp.evalErrorString();
        break;
      }
      else if ( updatingGeom )
      {
        if ( value.canConvert< QgsGeometry >() )
        {
          QgsGeometry geom = value.value< QgsGeometry >();
          mVectorLayer->changeGeometry( feature.id(), &geom );
        }
      }
      else
      {
        field.convertCompatible( value );
        mVectorLayer->changeAttributeValue( feature.id(), mAttributeId, value, newField ? emptyAttribute : feature.attributes().value( mAttributeId ) );
      }

      rownum++;
    }

    QApplication::restoreOverrideCursor();

    if ( !calculationSuccess )
    {
      QMessageBox::critical( nullptr, tr( "Error" ), tr( "An error occurred while evaluating the calculation string:\n%1" ).arg( error ) );
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

  setPrecisionMinMax();
}

void QgsFieldCalculator::populateFields()
{
  if ( !mVectorLayer )
    return;

  const QgsFields& fields = mVectorLayer->fields();
  for ( int idx = 0; idx < fields.count(); ++idx )
  {
    if ( fields.fieldOrigin( idx ) != QgsFields::OriginExpression && fields.fieldOrigin( idx ) != QgsFields::OriginJoin )
    {
      QString fieldName = fields.at( idx ).name();

      //insert into field list and field combo box
      mFieldMap.insert( fieldName, idx );
      mExistingFieldComboBox->addItem( fieldName );
    }
  }

  if ( mVectorLayer->geometryType() != QGis::NoGeometry )
  {
    mExistingFieldComboBox->addItem( tr( "<geometry>" ), "geom" );

    QFont font = mExistingFieldComboBox->itemData( mExistingFieldComboBox->count() - 1, Qt::FontRole ).value<QFont>();
    font.setItalic( true );
    mExistingFieldComboBox->setItemData( mExistingFieldComboBox->count() - 1, font, Qt::FontRole );
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

void QgsFieldCalculator::setPrecisionMinMax()
{
  int idx = mOutputFieldTypeComboBox->currentIndex();
  int minPrecType = mOutputFieldTypeComboBox->itemData( idx, Qt::UserRole + 4 ).toInt();
  int maxPrecType = mOutputFieldTypeComboBox->itemData( idx, Qt::UserRole + 5 ).toInt();
  mOutputFieldPrecisionSpinBox->setEnabled( minPrecType < maxPrecType );
  mOutputFieldPrecisionSpinBox->setMinimum( minPrecType );
  mOutputFieldPrecisionSpinBox->setMaximum( qMax( minPrecType, qMin( maxPrecType, mOutputFieldWidthSpinBox->value() ) ) );
}
