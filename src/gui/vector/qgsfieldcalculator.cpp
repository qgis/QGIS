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

#include <QMessageBox>


#include "qgsfieldcalculator.h"
#include "moc_qgsfieldcalculator.cpp"
#include "qgsdistancearea.h"
#include "qgsexpression.h"
#include "qgsfeatureiterator.h"
#include "qgsproject.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsexpressioncontext.h"
#include "qgsgeometry.h"
#include "qgsgui.h"
#include "qgsguiutils.h"
#include "qgsproxyprogresstask.h"
#include "qgsexpressioncontextutils.h"
#include "qgsvectorlayerjoinbuffer.h"
#include "qgsvariantutils.h"
#include "qgsfields.h"
#include "qgsmessagebar.h"


// FTC = FieldTypeCombo
constexpr int FTC_TYPE_ROLE_IDX = 0;
constexpr int FTC_TYPE_NAME_IDX = 1;
constexpr int FTC_MINLEN_IDX = 2;
constexpr int FTC_MAXLEN_IDX = 3;
constexpr int FTC_MINPREC_IDX = 4;
constexpr int FTC_MAXPREC_IDX = 5;
constexpr int FTC_SUBTYPE_IDX = 6;

QgsFieldCalculator::QgsFieldCalculator( QgsVectorLayer *vl, QWidget *parent )
  : QDialog( parent )
  , mVectorLayer( vl )
  , mAttributeId( -1 )
{
  setupUi( this );
  connect( mNewFieldGroupBox, &QGroupBox::toggled, this, &QgsFieldCalculator::mNewFieldGroupBox_toggled );
  connect( mUpdateExistingGroupBox, &QGroupBox::toggled, this, &QgsFieldCalculator::mUpdateExistingGroupBox_toggled );
  connect( mCreateVirtualFieldCheckbox, &QCheckBox::stateChanged, this, &QgsFieldCalculator::mCreateVirtualFieldCheckbox_stateChanged );
  connect( mOutputFieldNameLineEdit, &QLineEdit::textChanged, this, &QgsFieldCalculator::mOutputFieldNameLineEdit_textChanged );
  connect( mOutputFieldTypeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::activated ), this, &QgsFieldCalculator::mOutputFieldTypeComboBox_activated );
  connect( mExistingFieldComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsFieldCalculator::mExistingFieldComboBox_currentIndexChanged );

  QgsGui::enableAutoGeometryRestore( this );

  if ( !vl )
    return;
  QgsVectorDataProvider *dataProvider = vl->dataProvider();
  if ( !dataProvider )
    return;

  const Qgis::VectorProviderCapabilities caps = dataProvider->capabilities();
  mCanAddAttribute = caps & Qgis::VectorProviderCapability::AddAttributes;
  mCanChangeAttributeValue = caps & Qgis::VectorProviderCapability::ChangeAttributeValues;

  QgsExpressionContext expContext( QgsExpressionContextUtils::globalProjectLayerScopes( mVectorLayer ) );

  expContext.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "row_number" ), 1, true ) );
  expContext.setHighlightedVariables( QStringList() << QStringLiteral( "row_number" ) );

  populateFields();
  populateOutputFieldTypes();

  connect( builder, &QgsExpressionBuilderWidget::expressionParsed, this, &QgsFieldCalculator::setDialogButtonState );
  connect( mOutputFieldWidthSpinBox, &QAbstractSpinBox::editingFinished, this, &QgsFieldCalculator::setPrecisionMinMax );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, &QgsFieldCalculator::showHelp );
  connect( mButtonBox->button( QDialogButtonBox::Apply ), &QAbstractButton::clicked, this, &QgsFieldCalculator::calculate );

  QgsDistanceArea myDa;
  myDa.setSourceCrs( vl->crs(), QgsProject::instance()->transformContext() );
  myDa.setEllipsoid( QgsProject::instance()->ellipsoid() );
  builder->setGeomCalculator( myDa );

  //default values for field width and precision
  mOutputFieldWidthSpinBox->setValue( 10 );
  mOutputFieldWidthSpinBox->setClearValue( 10 );
  mOutputFieldPrecisionSpinBox->setValue( 3 );
  mOutputFieldPrecisionSpinBox->setClearValue( 3 );
  setPrecisionMinMax();

  if ( vl->providerType() == QLatin1String( "ogr" ) && vl->storageType() == QLatin1String( "ESRI Shapefile" ) )
  {
    mOutputFieldNameLineEdit->setMaxLength( 10 );
  }

  if ( !mCanAddAttribute )
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

  if ( !mCanChangeAttributeValue )
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

  if ( ( mNewFieldGroupBox->isChecked() && mCreateVirtualFieldCheckbox->isChecked() ) || mVectorLayer->isEditable() )
  {
    mEditModeAutoTurnOnLabel->setVisible( false );
    mInfoIcon->setVisible( false );
  }
  else
  {
    mInfoIcon->setVisible( true );
  }

  const bool hasselection = vl->selectedFeatureCount() > 0;
  mOnlyUpdateSelectedCheckBox->setChecked( mCanChangeAttributeValue && hasselection );
  mOnlyUpdateSelectedCheckBox->setEnabled( mCanChangeAttributeValue && hasselection );
  mOnlyUpdateSelectedCheckBox->setText( tr( "Only update %n selected feature(s)", nullptr, vl->selectedFeatureCount() ) );

  builder->initWithLayer( vl, expContext, QStringLiteral( "fieldcalc" ) );

  mInfoIcon->setPixmap( style()->standardPixmap( QStyle::SP_MessageBoxInformation ) );

  setWindowTitle( tr( "%1 — Field Calculator" ).arg( mVectorLayer->name() ) );

  // Init the message bar instance
  mMsgBar = new QgsMessageBar( this );
  mMsgBar->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
  this->vLayout->insertWidget( 0, mMsgBar );

  setDialogButtonState();
}

void QgsFieldCalculator::accept()
{
  calculate();
  QDialog::accept();
}

void QgsFieldCalculator::calculate()
{
  builder->expressionTree()->saveToRecent( builder->expressionText(), QStringLiteral( "fieldcalc" ) );

  if ( !mVectorLayer )
    return;

  // Set up QgsDistanceArea each time we (re-)calculate
  QgsDistanceArea myDa;

  myDa.setSourceCrs( mVectorLayer->crs(), QgsProject::instance()->transformContext() );
  myDa.setEllipsoid( QgsProject::instance()->ellipsoid() );

  const QString calcString = builder->expressionText();
  QgsExpression exp( calcString );
  exp.setGeomCalculator( &myDa );
  exp.setDistanceUnits( QgsProject::instance()->distanceUnits() );
  exp.setAreaUnits( QgsProject::instance()->areaUnits() );

  QgsExpressionContext expContext( QgsExpressionContextUtils::globalProjectLayerScopes( mVectorLayer ) );

  if ( !exp.prepare( &expContext ) )
  {
    QMessageBox::critical( nullptr, tr( "Evaluation Error" ), exp.evalErrorString() );
    return;
  }

  bool updatingGeom = false;

  // Test for creating expression field based on ! mUpdateExistingGroupBox checked rather
  // than on mNewFieldGroupBox checked, as if the provider does not support adding attributes
  // then mUpdateExistingGroupBox is set to not checkable, and hence is not checked.  This
  // is a minimum fix to resolve this - better would be some GUI redesign...
  if ( !mUpdateExistingGroupBox->isChecked() && mCreateVirtualFieldCheckbox->isChecked() )
  {
    mVectorLayer->addExpressionField( calcString, fieldDefinition() );
  }
  else
  {
    if ( !mVectorLayer->isEditable() )
      mVectorLayer->startEditing();

    QgsTemporaryCursorOverride cursorOverride( Qt::WaitCursor );

    mVectorLayer->beginEditCommand( QStringLiteral( "Field calculator" ) );

    //update existing field
    if ( mUpdateExistingGroupBox->isChecked() || !mNewFieldGroupBox->isEnabled() )
    {
      if ( mExistingFieldComboBox->currentData().toString() == QLatin1String( "geom" ) )
      {
        //update geometry
        mAttributeId = -1;
        updatingGeom = true;
      }
      else
      {
        bool ok = false;
        const int id = mExistingFieldComboBox->currentData().toInt( &ok );
        if ( ok )
          mAttributeId = id;
      }
    }
    else
    {
      //create new field
      const QgsField newField = fieldDefinition();

      if ( !mVectorLayer->addAttribute( newField ) )
      {
        cursorOverride.release();
        QMessageBox::critical( nullptr, tr( "Create New Field" ), tr( "Could not add the new field to the provider." ) );
        mVectorLayer->destroyEditCommand();
        return;
      }

      //get index of the new field
      const QgsFields &fields = mVectorLayer->fields();

      for ( int idx = 0; idx < fields.count(); ++idx )
      {
        if ( fields.at( idx ).name() == mOutputFieldNameLineEdit->text() )
        {
          mAttributeId = idx;
          break;
        }
      }

      //update expression context with new fields
      expContext.setFields( mVectorLayer->fields() );
      if ( !exp.prepare( &expContext ) )
      {
        cursorOverride.release();
        QMessageBox::critical( nullptr, tr( "Evaluation Error" ), exp.evalErrorString() );
        return;
      }
    }

    if ( mAttributeId == -1 && !updatingGeom )
    {
      mVectorLayer->destroyEditCommand();
      return;
    }

    //go through all the features and change the new attribute
    QgsFeature feature;
    bool calculationSuccess = true;
    QString error;

    const bool useGeometry = exp.needsGeometry();
    int rownum = 1;

    const QgsField field = !updatingGeom ? mVectorLayer->fields().at( mAttributeId ) : QgsField();

    const bool newField = !mUpdateExistingGroupBox->isChecked();
    QVariant emptyAttribute;
    if ( newField )
      emptyAttribute = QgsVariantUtils::createNullVariant( field.type() );

    QgsFeatureRequest req = QgsFeatureRequest().setFlags( useGeometry ? Qgis::FeatureRequestFlag::NoFlags : Qgis::FeatureRequestFlag::NoGeometry );
    QSet<QString> referencedColumns = exp.referencedColumns();
    referencedColumns.insert( field.name() ); // need existing column value to store old attribute when changing field values
    req.setSubsetOfAttributes( referencedColumns, mVectorLayer->fields() );
    if ( mOnlyUpdateSelectedCheckBox->isChecked() )
    {
      req.setFilterFids( mVectorLayer->selectedFeatureIds() );
    }
    QgsFeatureIterator fit = mVectorLayer->getFeatures( req );

    std::unique_ptr<QgsScopedProxyProgressTask> task = std::make_unique<QgsScopedProxyProgressTask>( tr( "Calculating field" ) );
    const long long count = mOnlyUpdateSelectedCheckBox->isChecked() ? mVectorLayer->selectedFeatureCount() : mVectorLayer->featureCount();
    long long i = 0;
    while ( fit.nextFeature( feature ) )
    {
      i++;
      task->setProgress( i / static_cast<double>( count ) * 100 );

      expContext.setFeature( feature );
      expContext.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "row_number" ), rownum, true ) );

      QVariant value = exp.evaluate( &expContext );
      if ( exp.hasEvalError() )
      {
        calculationSuccess = false;
        error = exp.evalErrorString();
        break;
      }
      else if ( updatingGeom )
      {
        if ( value.userType() == qMetaTypeId<QgsGeometry>() )
        {
          QgsGeometry geom = value.value<QgsGeometry>();
          mVectorLayer->changeGeometry( feature.id(), geom );
        }
      }
      else
      {
        ( void ) field.convertCompatible( value );
        mVectorLayer->changeAttributeValue( feature.id(), mAttributeId, value, newField ? emptyAttribute : feature.attributes().value( mAttributeId ) );
      }

      rownum++;
    }

    if ( !calculationSuccess )
    {
      cursorOverride.release();
      task.reset();
      QMessageBox::critical( nullptr, tr( "Evaluation Error" ), tr( "An error occurred while evaluating the calculation string:\n%1" ).arg( error ) );
      mVectorLayer->destroyEditCommand();
      return;
    }

    mVectorLayer->endEditCommand();
    if ( mNewFieldGroupBox->isChecked() )
    {
      pushMessage( tr( "Field \"%1\" created successfully" ).arg( mOutputFieldNameLineEdit->text() ) );
    }
    else if ( mUpdateExistingGroupBox->isChecked() )
    {
      pushMessage( tr( "Field \"%1\" updated successfully" ).arg( mExistingFieldComboBox->currentText() ) );
    }
  }
}

void QgsFieldCalculator::populateOutputFieldTypes()
{
  if ( !mVectorLayer )
  {
    return;
  }

  QgsVectorDataProvider *provider = mVectorLayer->dataProvider();
  if ( !provider )
  {
    return;
  }

  const int oldDataType = mOutputFieldTypeComboBox->currentData( Qt::UserRole + FTC_TYPE_ROLE_IDX ).toInt();

  mOutputFieldTypeComboBox->blockSignals( true );

  // Standard subset of fields in case of virtual
  const QList<QgsVectorDataProvider::NativeType> &typelist = mCreateVirtualFieldCheckbox->isChecked() ? ( QList<QgsVectorDataProvider::NativeType>()
                                                                                                          << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::Int ), QStringLiteral( "integer" ), QMetaType::Type::Int, 0, 10 )
                                                                                                          << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::Double ), QStringLiteral( "double precision" ), QMetaType::Type::Double, -1, -1, -1, -1 )
                                                                                                          << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QString ), QStringLiteral( "string" ), QMetaType::Type::QString )
                                                                                                          // date time
                                                                                                          << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QDate ), QStringLiteral( "date" ), QMetaType::Type::QDate, -1, -1, -1, -1 )
                                                                                                          << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QTime ), QStringLiteral( "time" ), QMetaType::Type::QTime, -1, -1, -1, -1 )
                                                                                                          << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QDateTime ), QStringLiteral( "datetime" ), QMetaType::Type::QDateTime, -1, -1, -1, -1 )
                                                                                                          // string types
                                                                                                          << QgsVectorDataProvider::NativeType( tr( "Text, unlimited length (text)" ), QStringLiteral( "text" ), QMetaType::Type::QString, -1, -1, -1, -1 )
                                                                                                          // boolean
                                                                                                          << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::Bool ), QStringLiteral( "bool" ), QMetaType::Type::Bool )
                                                                                                          // blob
                                                                                                          << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QByteArray ), QStringLiteral( "binary" ), QMetaType::Type::QByteArray ) )
                                                                                                      : provider->nativeTypes();

  mOutputFieldTypeComboBox->clear();
  for ( int i = 0; i < typelist.size(); i++ )
  {
    mOutputFieldTypeComboBox->addItem( QgsFields::iconForFieldType( typelist[i].mType, typelist[i].mSubType, typelist[i].mTypeName ), typelist[i].mTypeDesc );
    mOutputFieldTypeComboBox->setItemData( i, static_cast<int>( typelist[i].mType ), Qt::UserRole + FTC_TYPE_ROLE_IDX );
    mOutputFieldTypeComboBox->setItemData( i, typelist[i].mTypeName, Qt::UserRole + FTC_TYPE_NAME_IDX );
    mOutputFieldTypeComboBox->setItemData( i, typelist[i].mMinLen, Qt::UserRole + FTC_MINLEN_IDX );
    mOutputFieldTypeComboBox->setItemData( i, typelist[i].mMaxLen, Qt::UserRole + FTC_MAXLEN_IDX );
    mOutputFieldTypeComboBox->setItemData( i, typelist[i].mMinPrec, Qt::UserRole + FTC_MINPREC_IDX );
    mOutputFieldTypeComboBox->setItemData( i, typelist[i].mMaxPrec, Qt::UserRole + FTC_MAXPREC_IDX );
    mOutputFieldTypeComboBox->setItemData( i, static_cast<int>( typelist[i].mSubType ), Qt::UserRole + FTC_SUBTYPE_IDX );
  }
  mOutputFieldTypeComboBox->blockSignals( false );

  const int idx = mOutputFieldTypeComboBox->findData( oldDataType, Qt::UserRole + FTC_TYPE_ROLE_IDX );
  if ( idx != -1 )
  {
    mOutputFieldTypeComboBox->setCurrentIndex( idx );
    mOutputFieldTypeComboBox_activated( idx );
  }
  else
  {
    mOutputFieldTypeComboBox->setCurrentIndex( 0 );
    mOutputFieldTypeComboBox_activated( 0 );
  }
}

void QgsFieldCalculator::mNewFieldGroupBox_toggled( bool on )
{
  mUpdateExistingGroupBox->setChecked( !on );
  if ( on && !mCanAddAttribute )
  {
    mOnlyVirtualFieldsInfoLabel->setVisible( true );
  }
  else
  {
    mOnlyVirtualFieldsInfoLabel->setVisible( false );
  }

  if ( ( mNewFieldGroupBox->isChecked() && mCreateVirtualFieldCheckbox->isChecked() ) || mVectorLayer->isEditable() )
  {
    mEditModeAutoTurnOnLabel->setVisible( false );
  }
  else
  {
    mEditModeAutoTurnOnLabel->setVisible( true );
  }

  mInfoIcon->setVisible( mOnlyVirtualFieldsInfoLabel->isVisible() || mEditModeAutoTurnOnLabel->isVisible() );
}

void QgsFieldCalculator::mUpdateExistingGroupBox_toggled( bool on )
{
  mNewFieldGroupBox->setChecked( !on );
  setDialogButtonState();

  if ( on )
  {
    mOnlyUpdateSelectedCheckBox->setEnabled( mVectorLayer->selectedFeatureCount() > 0 );
  }
  else
  {
    mCreateVirtualFieldCheckbox_stateChanged( mCreateVirtualFieldCheckbox->checkState() );
  }
}

void QgsFieldCalculator::mCreateVirtualFieldCheckbox_stateChanged( int state )
{
  mOnlyUpdateSelectedCheckBox->setChecked( false );
  mOnlyUpdateSelectedCheckBox->setEnabled( state != Qt::Checked && mVectorLayer->selectedFeatureCount() > 0 );

  if ( ( mNewFieldGroupBox->isChecked() && mCreateVirtualFieldCheckbox->isChecked() ) || mVectorLayer->isEditable() )
  {
    mEditModeAutoTurnOnLabel->setVisible( false );
  }
  else
  {
    mEditModeAutoTurnOnLabel->setVisible( true );
  }
  populateOutputFieldTypes();
  mInfoIcon->setVisible( mOnlyVirtualFieldsInfoLabel->isVisible() || mEditModeAutoTurnOnLabel->isVisible() );
}


void QgsFieldCalculator::mOutputFieldNameLineEdit_textChanged( const QString &text )
{
  Q_UNUSED( text )
  setDialogButtonState();
}


void QgsFieldCalculator::mOutputFieldTypeComboBox_activated( int index )
{
  mOutputFieldWidthSpinBox->setMinimum( mOutputFieldTypeComboBox->itemData( index, Qt::UserRole + FTC_MINLEN_IDX ).toInt() );
  mOutputFieldWidthSpinBox->setMaximum( mOutputFieldTypeComboBox->itemData( index, Qt::UserRole + FTC_MAXLEN_IDX ).toInt() );
  mOutputFieldWidthSpinBox->setEnabled( mOutputFieldWidthSpinBox->minimum() < mOutputFieldWidthSpinBox->maximum() );
  if ( mOutputFieldWidthSpinBox->value() < mOutputFieldWidthSpinBox->minimum() )
    mOutputFieldWidthSpinBox->setValue( mOutputFieldWidthSpinBox->minimum() );
  if ( mOutputFieldWidthSpinBox->value() > mOutputFieldWidthSpinBox->maximum() )
    mOutputFieldWidthSpinBox->setValue( mOutputFieldWidthSpinBox->maximum() );

  setPrecisionMinMax();
}

void QgsFieldCalculator::mExistingFieldComboBox_currentIndexChanged( const int index )
{
  Q_UNUSED( index )
  setDialogButtonState();
}

void QgsFieldCalculator::populateFields()
{
  if ( !mVectorLayer )
    return;

  const QgsFields &fields = mVectorLayer->fields();
  for ( int idx = 0; idx < fields.count(); ++idx )
  {
    switch ( fields.fieldOrigin( idx ) )
    {
      case Qgis::FieldOrigin::Expression:
      case Qgis::FieldOrigin::Unknown:

        continue; // can't be edited

      case Qgis::FieldOrigin::Provider:
      case Qgis::FieldOrigin::Edit:
        break; // can always be edited

      case Qgis::FieldOrigin::Join:
      {
        // show joined fields (e.g. auxiliary fields) only if they have a non-hidden editor widget.
        // This enables them to be bulk field-calculated when a user needs to, but hides them by default
        // (since there's often MANY of these, e.g. after using the label properties tool on a layer)
        if ( fields.at( idx ).editorWidgetSetup().type() == QLatin1String( "Hidden" ) )
          continue;

        // only show editable joins
        int srcFieldIndex;
        const QgsVectorLayerJoinInfo *info = mVectorLayer->joinBuffer()->joinForFieldIndex( idx, fields, srcFieldIndex );

        if ( !info || !info->isEditable() )
          continue; // join is not editable

        break;
      }
    }

    const QString fieldName = fields.at( idx ).name();

    //insert into field combo box
    mExistingFieldComboBox->addItem( fields.iconForField( idx ), fieldName, idx );
  }

  if ( mVectorLayer->geometryType() != Qgis::GeometryType::Null )
  {
    mExistingFieldComboBox->addItem( tr( "<geometry>" ), "geom" );

    QFont font = mExistingFieldComboBox->itemData( mExistingFieldComboBox->count() - 1, Qt::FontRole ).value<QFont>();
    font.setItalic( true );
    mExistingFieldComboBox->setItemData( mExistingFieldComboBox->count() - 1, font, Qt::FontRole );
  }
  mExistingFieldComboBox->setCurrentIndex( -1 );
}

void QgsFieldCalculator::setDialogButtonState()
{
  QList<QPushButton *> buttons = {
    mButtonBox->button( QDialogButtonBox::Ok ),
    mButtonBox->button( QDialogButtonBox::Apply )
  };

  bool enableButtons = true;
  QString tooltip;

  if ( ( mNewFieldGroupBox->isChecked() || !mUpdateExistingGroupBox->isEnabled() )
       && mOutputFieldNameLineEdit->text().isEmpty() )
  {
    tooltip = tr( "Please enter a field name" );
    enableButtons = false;
  }
  else if ( ( mUpdateExistingGroupBox->isChecked() || !mNewFieldGroupBox->isEnabled() )
            && mExistingFieldComboBox->currentIndex() == -1 )
  {
    tooltip = tr( "Please select a field" );
    enableButtons = false;
  }
  else if ( builder->expressionText().isEmpty() )
  {
    tooltip = tr( "Please insert an expression" );
    enableButtons = false;
  }
  else if ( !builder->isExpressionValid() )
  {
    tooltip = tr( "The expression is invalid. See \"(more info)\" for details" );
    enableButtons = false;
  }

  for ( QPushButton *button : buttons )
  {
    if ( button )
    {
      button->setEnabled( enableButtons );
      button->setToolTip( tooltip );
    }
  }
}

void QgsFieldCalculator::setPrecisionMinMax()
{
  const int idx = mOutputFieldTypeComboBox->currentIndex();
  const int minPrecType = mOutputFieldTypeComboBox->itemData( idx, Qt::UserRole + FTC_MINPREC_IDX ).toInt();
  const int maxPrecType = mOutputFieldTypeComboBox->itemData( idx, Qt::UserRole + FTC_MAXPREC_IDX ).toInt();
  const bool precisionIsEnabled = minPrecType < maxPrecType;
  mOutputFieldPrecisionSpinBox->setEnabled( precisionIsEnabled );
  // Do not set min/max if it's disabled or we'll loose the default value,
  // see https://github.com/qgis/QGIS/issues/26880 - QGIS saves integer field when
  // I create a new real field through field calculator (Update field works as intended)
  if ( precisionIsEnabled )
  {
    mOutputFieldPrecisionSpinBox->setMinimum( minPrecType );
    mOutputFieldPrecisionSpinBox->setMaximum( std::max( minPrecType, std::min( maxPrecType, mOutputFieldWidthSpinBox->value() ) ) );
  }
}

void QgsFieldCalculator::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_vector/attribute_table.html#editing-attribute-values" ) );
}

QgsField QgsFieldCalculator::fieldDefinition()
{
  return QgsField( mOutputFieldNameLineEdit->text(), static_cast<QMetaType::Type>( mOutputFieldTypeComboBox->currentData( Qt::UserRole + FTC_TYPE_ROLE_IDX ).toInt() ), mOutputFieldTypeComboBox->currentData( Qt::UserRole + FTC_TYPE_NAME_IDX ).toString(), mOutputFieldWidthSpinBox->value(), mOutputFieldPrecisionSpinBox->isEnabled() ? mOutputFieldPrecisionSpinBox->value() : 0, QString(), static_cast<QMetaType::Type>( mOutputFieldTypeComboBox->currentData( Qt::UserRole + FTC_SUBTYPE_IDX ).toInt() ) );
}

void QgsFieldCalculator::pushMessage( const QString &text, Qgis::MessageLevel level, int duration )
{
  mMsgBar->pushMessage( text, level, duration );
}
