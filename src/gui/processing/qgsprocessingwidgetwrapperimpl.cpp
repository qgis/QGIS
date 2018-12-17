/***************************************************************************
                         qgsprocessingwidgetwrapperimpl.cpp
                         ---------------------
    begin                : August 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingwidgetwrapperimpl.h"
#include "qgsprocessingparameters.h"
#include "qgsprocessingoutputs.h"
#include "qgsprojectionselectionwidget.h"
#include "qgsspinbox.h"
#include "qgsdoublespinbox.h"
#include "qgsprocessingcontext.h"
#include "qgsauthconfigselect.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPlainTextEdit>

///@cond PRIVATE

//
// QgsProcessingBooleanWidgetWrapper
//

QgsProcessingBooleanWidgetWrapper::QgsProcessingBooleanWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{

}

QWidget *QgsProcessingBooleanWidgetWrapper::createWidget()
{
  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    {
      QString description = parameterDefinition()->description();
      if ( parameterDefinition()->flags() & QgsProcessingParameterDefinition::FlagOptional )
        description = QObject::tr( "%1 [optional]" ).arg( description );

      mCheckBox = new QCheckBox( description );
      mCheckBox->setToolTip( parameterDefinition()->toolTip() );

      connect( mCheckBox, &QCheckBox::toggled, this, [ = ]
      {
        emit widgetValueHasChanged( this );
      } );
      return mCheckBox;
    };

    case QgsProcessingGui::Batch:
    case QgsProcessingGui::Modeler:
    {
      mComboBox = new QComboBox();
      mComboBox->addItem( tr( "Yes" ), true );
      mComboBox->addItem( tr( "No" ), false );
      mComboBox->setToolTip( parameterDefinition()->toolTip() );

      connect( mComboBox, qgis::overload< int>::of( &QComboBox::currentIndexChanged ), this, [ = ]
      {
        emit widgetValueHasChanged( this );
      } );

      return mComboBox;
    }
  }
  return nullptr;
}

QLabel *QgsProcessingBooleanWidgetWrapper::createLabel()
{
  // avoid creating labels in standard dialogs
  if ( type() == QgsProcessingGui::Standard )
    return nullptr;
  else
    return QgsAbstractProcessingParameterWidgetWrapper::createLabel();
}

void QgsProcessingBooleanWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext &context )
{
  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    {
      const bool v = QgsProcessingParameters::parameterAsBool( parameterDefinition(), value, context );
      mCheckBox->setChecked( v );
      break;
    }

    case QgsProcessingGui::Batch:
    case QgsProcessingGui::Modeler:
    {
      const bool v = QgsProcessingParameters::parameterAsBool( parameterDefinition(), value, context );
      mComboBox->setCurrentIndex( mComboBox->findData( v ) );
      break;
    }
  }
}

QVariant QgsProcessingBooleanWidgetWrapper::widgetValue() const
{
  switch ( type() )
  {
    case QgsProcessingGui::Standard:
      return mCheckBox->isChecked();

    case QgsProcessingGui::Batch:
    case QgsProcessingGui::Modeler:
      return mComboBox->currentData();
  }
  return QVariant();
}

QStringList QgsProcessingBooleanWidgetWrapper::compatibleParameterTypes() const
{
  //pretty much everything is compatible here and can be converted to a bool!
  return QStringList() << QgsProcessingParameterBoolean::typeName()
         << QgsProcessingParameterString::typeName()
         << QgsProcessingParameterNumber::typeName()
         << QgsProcessingParameterDistance::typeName()
         << QgsProcessingParameterFile::typeName()
         << QgsProcessingParameterField::typeName()
         << QgsProcessingParameterFeatureSource::typeName()
         << QgsProcessingParameterMapLayer::typeName()
         << QgsProcessingParameterRasterLayer::typeName()
         << QgsProcessingParameterVectorLayer::typeName()
         << QgsProcessingParameterMeshLayer::typeName()
         << QgsProcessingParameterExpression::typeName();
}

QStringList QgsProcessingBooleanWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList() << QgsProcessingOutputNumber::typeName()
         << QgsProcessingOutputMapLayer::typeName()
         << QgsProcessingOutputFile::typeName()
         << QgsProcessingOutputRasterLayer::typeName()
         << QgsProcessingOutputVectorLayer::typeName()
         << QgsProcessingOutputString::typeName();
}

QList<int> QgsProcessingBooleanWidgetWrapper::compatibleDataTypes() const
{
  return QList< int >();
}

QString QgsProcessingBooleanWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterBoolean::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingBooleanWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingBooleanWidgetWrapper( parameter, type );
}


//
// QgsProcessingCrsWidgetWrapper
//

QgsProcessingCrsWidgetWrapper::QgsProcessingCrsWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{

}

QWidget *QgsProcessingCrsWidgetWrapper::createWidget()
{
  mProjectionSelectionWidget = new QgsProjectionSelectionWidget();
  mProjectionSelectionWidget->setToolTip( parameterDefinition()->toolTip() );

  if ( parameterDefinition()->flags() & QgsProcessingParameterDefinition::FlagOptional )
    mProjectionSelectionWidget->setOptionVisible( QgsProjectionSelectionWidget::CrsNotSet, true );
  else
    mProjectionSelectionWidget->setOptionVisible( QgsProjectionSelectionWidget::CrsNotSet, false );

  connect( mProjectionSelectionWidget, &QgsProjectionSelectionWidget::crsChanged, this, [ = ]
  {
    emit widgetValueHasChanged( this );
  } );

  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    case QgsProcessingGui::Batch:
    {
      return mProjectionSelectionWidget;
    };

    case QgsProcessingGui::Modeler:
    {
      QWidget *w = new QWidget();
      w->setToolTip( parameterDefinition()->toolTip() );

      QVBoxLayout *vl = new QVBoxLayout();
      vl->setMargin( 0 );
      vl->setContentsMargins( 0, 0, 0, 0 );
      w->setLayout( vl );

      mUseProjectCrsCheckBox = new QCheckBox( tr( "Use project CRS" ) );
      mUseProjectCrsCheckBox->setToolTip( tr( "Always use the current project CRS when running the model" ) );
      vl->addWidget( mUseProjectCrsCheckBox );
      connect( mUseProjectCrsCheckBox, &QCheckBox::toggled, mProjectionSelectionWidget, &QgsProjectionSelectionWidget::setDisabled );
      connect( mUseProjectCrsCheckBox, &QCheckBox::toggled, this, [ = ]
      {
        emit widgetValueHasChanged( this );
      } );

      vl->addWidget( mProjectionSelectionWidget );

      return w;
    }
  }
  return nullptr;
}

void QgsProcessingCrsWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext &context )
{
  if ( mUseProjectCrsCheckBox )
  {
    if ( value.toString().compare( QLatin1String( "ProjectCrs" ), Qt::CaseInsensitive ) == 0 )
    {
      mUseProjectCrsCheckBox->setChecked( true );
      return;
    }
    else
    {
      mUseProjectCrsCheckBox->setChecked( false );
    }
  }

  const QgsCoordinateReferenceSystem v = QgsProcessingParameters::parameterAsCrs( parameterDefinition(), value, context );
  if ( mProjectionSelectionWidget )
    mProjectionSelectionWidget->setCrs( v );
}

QVariant QgsProcessingCrsWidgetWrapper::widgetValue() const
{
  if ( mUseProjectCrsCheckBox && mUseProjectCrsCheckBox->isChecked() )
    return QStringLiteral( "ProjectCrs" );
  else if ( mProjectionSelectionWidget )
    return mProjectionSelectionWidget->crs().isValid() ? mProjectionSelectionWidget->crs() : QVariant();
  else
    return QVariant();
}

QStringList QgsProcessingCrsWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList()
         << QgsProcessingParameterCrs::typeName()
         << QgsProcessingParameterExpression::typeName()
         << QgsProcessingParameterString::typeName()
         << QgsProcessingParameterRasterLayer::typeName()
         << QgsProcessingParameterVectorLayer::typeName()
         << QgsProcessingParameterMeshLayer::typeName()
         << QgsProcessingParameterFeatureSource::typeName();
}

QStringList QgsProcessingCrsWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList() << QgsProcessingOutputVectorLayer::typeName()
         << QgsProcessingOutputRasterLayer::typeName()
         << QgsProcessingOutputMapLayer::typeName()
         << QgsProcessingOutputString::typeName();
}

QList<int> QgsProcessingCrsWidgetWrapper::compatibleDataTypes() const
{
  return QList< int >();
}

QString QgsProcessingCrsWidgetWrapper::modelerExpressionFormatString() const
{
  return tr( "string as EPSG code, WKT or PROJ format, or a string identifying a map layer" );
}

QString QgsProcessingCrsWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterCrs::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingCrsWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingCrsWidgetWrapper( parameter, type );
}



//
// QgsProcessingStringWidgetWrapper
//

QgsProcessingStringWidgetWrapper::QgsProcessingStringWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{

}

QWidget *QgsProcessingStringWidgetWrapper::createWidget()
{
  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    case QgsProcessingGui::Modeler:
    {
      if ( static_cast< const QgsProcessingParameterString * >( parameterDefinition() )->multiLine() )
      {
        mPlainTextEdit = new QPlainTextEdit();
        mPlainTextEdit->setToolTip( parameterDefinition()->toolTip() );

        connect( mPlainTextEdit, &QPlainTextEdit::textChanged, this, [ = ]
        {
          emit widgetValueHasChanged( this );
        } );
        return mPlainTextEdit;
      }
      else
      {
        mLineEdit = new QLineEdit();
        mLineEdit->setToolTip( parameterDefinition()->toolTip() );

        connect( mLineEdit, &QLineEdit::textChanged, this, [ = ]
        {
          emit widgetValueHasChanged( this );
        } );
        return mLineEdit;
      }
    };

    case QgsProcessingGui::Batch:
    {
      mLineEdit = new QLineEdit();
      mLineEdit->setToolTip( parameterDefinition()->toolTip() );

      connect( mLineEdit, &QLineEdit::textChanged, this, [ = ]
      {
        emit widgetValueHasChanged( this );
      } );
      return mLineEdit;
    }
  }
  return nullptr;
}

void QgsProcessingStringWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext &context )
{
  const QString v = QgsProcessingParameters::parameterAsString( parameterDefinition(), value, context );
  if ( mLineEdit )
    mLineEdit->setText( v );
  if ( mPlainTextEdit )
    mPlainTextEdit->setPlainText( v );
}

QVariant QgsProcessingStringWidgetWrapper::widgetValue() const
{
  if ( mLineEdit )
    return mLineEdit->text();
  else if ( mPlainTextEdit )
    return mPlainTextEdit->toPlainText();
  else
    return QVariant();
}

QStringList QgsProcessingStringWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList()
         << QgsProcessingParameterString::typeName()
         << QgsProcessingParameterAuthConfig::typeName()
         << QgsProcessingParameterNumber::typeName()
         << QgsProcessingParameterDistance::typeName()
         << QgsProcessingParameterFile::typeName()
         << QgsProcessingParameterField::typeName()
         << QgsProcessingParameterExpression::typeName();
}

QStringList QgsProcessingStringWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList() << QgsProcessingOutputNumber::typeName()
         << QgsProcessingOutputFile::typeName()
         << QgsProcessingOutputString::typeName();
}

QList<int> QgsProcessingStringWidgetWrapper::compatibleDataTypes() const
{
  return QList< int >();
}

QString QgsProcessingStringWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterString::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingStringWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingStringWidgetWrapper( parameter, type );
}



//
// QgsProcessingAuthConfigWidgetWrapper
//

QgsProcessingAuthConfigWidgetWrapper::QgsProcessingAuthConfigWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{

}

QWidget *QgsProcessingAuthConfigWidgetWrapper::createWidget()
{
  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    case QgsProcessingGui::Modeler:
    case QgsProcessingGui::Batch:
    {
      mAuthConfigSelect = new QgsAuthConfigSelect();
      mAuthConfigSelect->setToolTip( parameterDefinition()->toolTip() );

      connect( mAuthConfigSelect, &QgsAuthConfigSelect::selectedConfigIdChanged, this, [ = ]
      {
        emit widgetValueHasChanged( this );
      } );
      return mAuthConfigSelect;
    };
  }
  return nullptr;
}

void QgsProcessingAuthConfigWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext &context )
{
  const QString v = QgsProcessingParameters::parameterAsString( parameterDefinition(), value, context );
  if ( mAuthConfigSelect )
    mAuthConfigSelect->setConfigId( v );
}

QVariant QgsProcessingAuthConfigWidgetWrapper::widgetValue() const
{
  if ( mAuthConfigSelect )
    return mAuthConfigSelect->configId();
  else
    return QVariant();
}

QStringList QgsProcessingAuthConfigWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList()
         << QgsProcessingParameterAuthConfig::typeName()
         << QgsProcessingParameterString::typeName()
         << QgsProcessingParameterExpression::typeName();
}

QStringList QgsProcessingAuthConfigWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList() << QgsProcessingOutputString::typeName();
}

QList<int> QgsProcessingAuthConfigWidgetWrapper::compatibleDataTypes() const
{
  return QList< int >();
}

QString QgsProcessingAuthConfigWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterAuthConfig::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingAuthConfigWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingAuthConfigWidgetWrapper( parameter, type );
}

//
// QgsProcessingNumericWidgetWrapper
//

QgsProcessingNumericWidgetWrapper::QgsProcessingNumericWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{

}

QWidget *QgsProcessingNumericWidgetWrapper::createWidget()
{
  const QgsProcessingParameterNumber *numberDef = static_cast< const QgsProcessingParameterNumber * >( parameterDefinition() );
  const QVariantMap metadata = numberDef->metadata();
  const int decimals = metadata.value( QStringLiteral( "widget_wrapper" ) ).toMap().value( QStringLiteral( "decimals" ), 6 ).toInt();
  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    case QgsProcessingGui::Modeler:
    case QgsProcessingGui::Batch:
    {
      // lots of duplicate code here -- but there's no common interface between QSpinBox/QDoubleSpinBox which would allow us to avoid this
      QAbstractSpinBox *spinBox = nullptr;
      switch ( numberDef->dataType() )
      {
        case QgsProcessingParameterNumber::Double:
          mDoubleSpinBox = new QgsDoubleSpinBox();
          mDoubleSpinBox->setExpressionsEnabled( true );
          mDoubleSpinBox->setDecimals( decimals );

          // guess reasonable step value for double spin boxes
          if ( !qgsDoubleNear( numberDef->maximum(), std::numeric_limits<double>::max() ) &&
               !qgsDoubleNear( numberDef->minimum(), std::numeric_limits<double>::lowest() + 1 ) )
          {
            double singleStep = calculateStep( numberDef->minimum(), numberDef->maximum() );
            singleStep = std::max( singleStep, std::pow( 10, -decimals ) );
            mDoubleSpinBox->setSingleStep( singleStep );
          }

          spinBox = mDoubleSpinBox;
          break;

        case QgsProcessingParameterNumber::Integer:
          mSpinBox = new QgsSpinBox();
          mSpinBox->setExpressionsEnabled( true );
          spinBox = mSpinBox;
          break;
      }
      spinBox->setToolTip( parameterDefinition()->toolTip() );

      double max = 999999999;
      if ( !qgsDoubleNear( numberDef->maximum(), std::numeric_limits<double>::max() ) )
      {
        max = numberDef->maximum();
      }
      double min = -999999999;
      if ( !qgsDoubleNear( numberDef->minimum(), std::numeric_limits<double>::lowest() ) )
      {
        min = numberDef->minimum();
      }
      if ( mDoubleSpinBox )
      {
        mDoubleSpinBox->setMinimum( min );
        mDoubleSpinBox->setMaximum( max );
      }
      else
      {
        mSpinBox->setMinimum( static_cast< int >( min ) );
        mSpinBox->setMaximum( static_cast< int >( max ) );
      }

      if ( numberDef->flags() & QgsProcessingParameterDefinition::FlagOptional )
      {
        mAllowingNull = true;
        if ( mDoubleSpinBox )
        {
          mDoubleSpinBox->setShowClearButton( true );
          const double min = mDoubleSpinBox->minimum() - 1;
          mDoubleSpinBox->setMinimum( min );
          mDoubleSpinBox->setValue( min );
        }
        else
        {
          mSpinBox->setShowClearButton( true );
          const int min = mSpinBox->minimum() - 1;
          mSpinBox->setMinimum( min );
          mSpinBox->setValue( min );
        }
        spinBox->setSpecialValueText( tr( "Not set" ) );
      }
      else
      {
        if ( numberDef->defaultValue().isValid() )
        {
          // if default value for parameter, we clear to that
          bool ok = false;
          if ( mDoubleSpinBox )
          {
            double defaultVal = numberDef->defaultValue().toDouble( &ok );
            if ( ok )
              mDoubleSpinBox->setClearValue( defaultVal );
          }
          else
          {
            int intVal = numberDef->defaultValue().toInt( &ok );
            if ( ok )
              mSpinBox->setClearValue( intVal );
          }
        }
        else if ( !qgsDoubleNear( numberDef->minimum(), std::numeric_limits<double>::lowest() ) )
        {
          // otherwise we clear to the minimum, if it's set
          if ( mDoubleSpinBox )
            mDoubleSpinBox->setClearValue( numberDef->minimum() );
          else
            mSpinBox->setClearValue( static_cast< int >( numberDef->minimum() ) );
        }
        else
        {
          // last resort, we clear to 0
          if ( mDoubleSpinBox )
          {
            mDoubleSpinBox->setValue( 0 );
            mDoubleSpinBox->setClearValue( 0 );
          }
          else
          {
            mSpinBox->setValue( 0 );
            mSpinBox->setClearValue( 0 );
          }
        }
      }

      if ( mDoubleSpinBox )
        connect( mDoubleSpinBox, qgis::overload<double>::of( &QgsDoubleSpinBox::valueChanged ), this, [ = ] { emit widgetValueHasChanged( this ); } );
      else if ( mSpinBox )
        connect( mSpinBox, qgis::overload<int>::of( &QgsSpinBox::valueChanged ), this, [ = ] { emit widgetValueHasChanged( this ); } );

      return spinBox;
    };
  }
  return nullptr;
}

void QgsProcessingNumericWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext &context )
{
  if ( mDoubleSpinBox )
  {
    if ( mAllowingNull && !value.isValid() )
      mDoubleSpinBox->clear();
    else
    {
      const double v = QgsProcessingParameters::parameterAsDouble( parameterDefinition(), value, context );
      mDoubleSpinBox->setValue( v );
    }
  }
  else if ( mSpinBox )
  {
    if ( mAllowingNull && !value.isValid() )
      mSpinBox->clear();
    else
    {
      const int v = QgsProcessingParameters::parameterAsInt( parameterDefinition(), value, context );
      mSpinBox->setValue( v );
    }
  }
}

QVariant QgsProcessingNumericWidgetWrapper::widgetValue() const
{
  if ( mDoubleSpinBox )
  {
    if ( mAllowingNull && qgsDoubleNear( mDoubleSpinBox->value(), mDoubleSpinBox->minimum() ) )
      return QVariant();
    else
      return mDoubleSpinBox->value();
  }
  else if ( mSpinBox )
  {
    if ( mAllowingNull && mSpinBox->value() == mSpinBox->minimum() )
      return QVariant();
    else
      return mSpinBox->value();
  }
  else
    return QVariant();
}

QStringList QgsProcessingNumericWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList()
         << QgsProcessingParameterString::typeName()
         << QgsProcessingParameterNumber::typeName()
         << QgsProcessingParameterDistance::typeName();
}

QStringList QgsProcessingNumericWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList() << QgsProcessingOutputNumber::typeName()
         << QgsProcessingOutputString::typeName();
}

QList<int> QgsProcessingNumericWidgetWrapper::compatibleDataTypes() const
{
  return QList< int >();
}

double QgsProcessingNumericWidgetWrapper::calculateStep( const double minimum, const double maximum )
{
  const double valueRange = maximum - minimum;
  if ( valueRange <= 1.0 )
  {
    const double step = valueRange / 10.0;
    // round to 1 significant figure
    return qgsRound( step, -std::floor( std::log( step ) ) );
  }
  else
  {
    return 1.0;
  }
}

QString QgsProcessingNumericWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterNumber::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingNumericWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingNumericWidgetWrapper( parameter, type );
}

//
// QgsProcessingDistanceWidgetWrapper
//

QgsProcessingDistanceWidgetWrapper::QgsProcessingDistanceWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsProcessingNumericWidgetWrapper( parameter, type, parent )
{

}

QString QgsProcessingDistanceWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterDistance::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingDistanceWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingDistanceWidgetWrapper( parameter, type );
}

QWidget *QgsProcessingDistanceWidgetWrapper::createWidget()
{
  const QgsProcessingParameterDistance *distanceDef = static_cast< const QgsProcessingParameterDistance * >( parameterDefinition() );

  QWidget *spin = QgsProcessingNumericWidgetWrapper::createWidget();
  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    {
      mLabel = new QLabel();
      mUnitsCombo = new QComboBox();

      mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::DistanceMeters ), QgsUnitTypes::DistanceMeters );
      mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::DistanceKilometers ), QgsUnitTypes::DistanceKilometers );
      mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::DistanceFeet ), QgsUnitTypes::DistanceFeet );
      mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::DistanceMiles ), QgsUnitTypes::DistanceMiles );
      mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::DistanceYards ), QgsUnitTypes::DistanceYards );

      const int labelMargin = static_cast< int >( std::round( mUnitsCombo->fontMetrics().width( 'X' ) ) );
      QHBoxLayout *layout = new QHBoxLayout();
      layout->addWidget( spin, 1 );
      layout->insertSpacing( 1, labelMargin / 2 );
      layout->insertWidget( 2, mLabel );
      layout->insertWidget( 3, mUnitsCombo );

      // bit of fiddlyness here -- we want the initial spacing to only be visible
      // when the warning label is shown, so it's embedded inside mWarningLabel
      // instead of outside it
      mWarningLabel = new QWidget();
      QHBoxLayout *warningLayout = new QHBoxLayout();
      warningLayout->setMargin( 0 );
      warningLayout->setContentsMargins( 0, 0, 0, 0 );
      QLabel *warning = new QLabel();
      QIcon icon = QgsApplication::getThemeIcon( QStringLiteral( "mIconWarning.svg" ) );
      const int size = static_cast< int >( std::max( 24.0, spin->minimumSize().height() * 0.5 ) );
      warning->setPixmap( icon.pixmap( icon.actualSize( QSize( size, size ) ) ) );
      warning->setToolTip( tr( "Distance is in geographic degrees. Consider reprojecting to a projected local coordinate system for accurate results." ) );
      warningLayout->insertSpacing( 0, labelMargin / 2 );
      warningLayout->insertWidget( 1, warning );
      mWarningLabel->setLayout( warningLayout );
      layout->insertWidget( 4, mWarningLabel );

      setUnits( distanceDef->defaultUnit() );

      QWidget *w = new QWidget();
      layout->setMargin( 0 );
      layout->setContentsMargins( 0, 0, 0, 0 );
      w->setLayout( layout );
      return w;
    }

    case QgsProcessingGui::Batch:
    case QgsProcessingGui::Modeler:
      return spin;

  }
  return nullptr;
}

void QgsProcessingDistanceWidgetWrapper::postInitialize( const QList<QgsAbstractProcessingParameterWidgetWrapper *> &wrappers )
{
  QgsProcessingNumericWidgetWrapper::postInitialize( wrappers );
  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    {
      for ( const QgsAbstractProcessingParameterWidgetWrapper *wrapper : wrappers )
      {
        if ( wrapper->parameterDefinition()->name() == static_cast< const QgsProcessingParameterDistance * >( parameterDefinition() )->parentParameterName() )
        {
          setUnitParameterValue( wrapper->parameterValue() );
          connect( wrapper, &QgsAbstractProcessingParameterWidgetWrapper::widgetValueHasChanged, this, [ = ]
          {
            setUnitParameterValue( wrapper->parameterValue() );
          } );
          break;
        }
      }
      break;
    }

    case QgsProcessingGui::Batch:
    case QgsProcessingGui::Modeler:
      break;
  }
}

void QgsProcessingDistanceWidgetWrapper::setUnitParameterValue( const QVariant &value )
{
  QgsUnitTypes::DistanceUnit units = QgsUnitTypes::DistanceUnknownUnit;

  // evaluate value to layer
  QgsProcessingContext *context = nullptr;
  std::unique_ptr< QgsProcessingContext > tmpContext;
  if ( mProcessingContextGenerator )
    context = mProcessingContextGenerator->processingContext();

  if ( !context )
  {
    tmpContext = qgis::make_unique< QgsProcessingContext >();
    context = tmpContext.get();
  }

  QgsCoordinateReferenceSystem crs = QgsProcessingParameters::parameterAsCrs( parameterDefinition(), value, *context );
  if ( crs.isValid() )
  {
    units = crs.mapUnits();
  }

  setUnits( units );
}

void QgsProcessingDistanceWidgetWrapper::setUnits( const QgsUnitTypes::DistanceUnit units )
{
  mLabel->setText( QgsUnitTypes::toString( units ) );
  if ( QgsUnitTypes::unitType( units ) != QgsUnitTypes::Standard )
  {
    mUnitsCombo->hide();
    mLabel->show();
  }
  else
  {
    mUnitsCombo->setCurrentIndex( mUnitsCombo->findData( units ) );
    mUnitsCombo->show();
    mLabel->hide();
  }
  mWarningLabel->setVisible( units == QgsUnitTypes::DistanceDegrees );
  mBaseUnit = units;
}

QVariant QgsProcessingDistanceWidgetWrapper::widgetValue() const
{
  const QVariant val = QgsProcessingNumericWidgetWrapper::widgetValue();
  if ( val.type() == QVariant::Double && mUnitsCombo && mUnitsCombo->isVisible() )
  {
    QgsUnitTypes::DistanceUnit displayUnit = static_cast<QgsUnitTypes::DistanceUnit >( mUnitsCombo->currentData().toInt() );
    return val.toDouble() * QgsUnitTypes::fromUnitToUnitFactor( displayUnit, mBaseUnit );
  }
  else
  {
    return val;
  }
}


//
// QgsProcessingRangeWidgetWrapper
//

QgsProcessingRangeWidgetWrapper::QgsProcessingRangeWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{

}

QWidget *QgsProcessingRangeWidgetWrapper::createWidget()
{
  const QgsProcessingParameterRange *rangeDef = static_cast< const QgsProcessingParameterRange * >( parameterDefinition() );
  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    case QgsProcessingGui::Modeler:
    case QgsProcessingGui::Batch:
    {
      QHBoxLayout *layout = new QHBoxLayout();

      mMinSpinBox = new QgsDoubleSpinBox();
      mMaxSpinBox = new QgsDoubleSpinBox();

      mMinSpinBox->setExpressionsEnabled( true );
      mMinSpinBox->setShowClearButton( false );
      mMaxSpinBox->setExpressionsEnabled( true );
      mMaxSpinBox->setShowClearButton( false );

      QLabel *minLabel = new QLabel( tr( "Min" ) );
      layout->addWidget( minLabel );
      layout->addWidget( mMinSpinBox, 1 );

      QLabel *maxLabel = new QLabel( tr( "Max" ) );
      layout->addWidget( maxLabel );
      layout->addWidget( mMaxSpinBox, 1 );

      QWidget *w = new QWidget();
      layout->setMargin( 0 );
      layout->setContentsMargins( 0, 0, 0, 0 );
      w->setLayout( layout );

      if ( rangeDef->dataType() == QgsProcessingParameterNumber::Double )
      {
        mMinSpinBox->setDecimals( 6 );
        mMaxSpinBox->setDecimals( 6 );
      }
      else
      {
        mMinSpinBox->setDecimals( 0 );
        mMaxSpinBox->setDecimals( 0 );
      }

      mMinSpinBox->setMinimum( -99999999.999999 );
      mMaxSpinBox->setMinimum( -99999999.999999 );
      mMinSpinBox->setMaximum( 99999999.999999 );
      mMaxSpinBox->setMaximum( 99999999.999999 );

      w->setToolTip( parameterDefinition()->toolTip() );

      connect( mMinSpinBox, qgis::overload<double>::of( &QgsDoubleSpinBox::valueChanged ), this, [ = ]( const double v )
      {
        mBlockChangedSignal++;
        if ( v > mMaxSpinBox->value() )
          mMaxSpinBox->setValue( v );
        mBlockChangedSignal--;

        if ( !mBlockChangedSignal )
          emit widgetValueHasChanged( this );
      } );
      connect( mMaxSpinBox, qgis::overload<double>::of( &QgsDoubleSpinBox::valueChanged ), this, [ = ]( const double v )
      {
        mBlockChangedSignal++;
        if ( v < mMinSpinBox->value() )
          mMinSpinBox->setValue( v );
        mBlockChangedSignal--;

        if ( !mBlockChangedSignal )
          emit widgetValueHasChanged( this );
      } );

      return w;
    };
  }
  return nullptr;
}

void QgsProcessingRangeWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext &context )
{
  const QList< double > v = QgsProcessingParameters::parameterAsRange( parameterDefinition(), value, context );
  if ( v.empty() )
    return;

  mBlockChangedSignal++;
  mMinSpinBox->setValue( v.at( 0 ) );
  if ( v.count() >= 2 )
    mMaxSpinBox->setValue( v.at( 1 ) );
  mBlockChangedSignal--;

  if ( !mBlockChangedSignal )
    emit widgetValueHasChanged( this );
}

QVariant QgsProcessingRangeWidgetWrapper::widgetValue() const
{
  return QStringLiteral( "%1,%2" ).arg( mMinSpinBox->value() ).arg( mMaxSpinBox->value() );
}

QStringList QgsProcessingRangeWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList()
         << QgsProcessingParameterRange::typeName()
         << QgsProcessingParameterString::typeName();
}

QStringList QgsProcessingRangeWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList() << QgsProcessingOutputString::typeName();
}

QList<int> QgsProcessingRangeWidgetWrapper::compatibleDataTypes() const
{
  return QList< int >();
}

QString QgsProcessingRangeWidgetWrapper::modelerExpressionFormatString() const
{
  return tr( "string as two comma delimited floats, e.g. '1,10'" );
}

QString QgsProcessingRangeWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterRange::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingRangeWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingRangeWidgetWrapper( parameter, type );
}


///@endcond PRIVATE

