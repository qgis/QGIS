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
#include "qgsprocessingmatrixparameterdialog.h"
#include "qgsspinbox.h"
#include "qgsdoublespinbox.h"
#include "qgsprocessingcontext.h"
#include "qgsauthconfigselect.h"
#include "qgsapplication.h"
#include "qgsfilewidget.h"
#include "qgssettings.h"
#include "qgsexpressionlineedit.h"
#include "qgsfieldexpressionwidget.h"
#include "qgsprocessingmultipleselectiondialog.h"
#include "qgslayoutmanager.h"
#include "qgsproject.h"
#include "qgslayoutcombobox.h"
#include "qgslayoutitemcombobox.h"
#include "qgsprintlayout.h"
#include "qgsscalewidget.h"
#include "qgssnapindicator.h"
#include "qgsmapmouseevent.h"
#include "qgsfilterlineedit.h"
#include "qgsmapcanvas.h"
#include <QToolButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QRadioButton>
#include <QButtonGroup>
#include <QMenu>

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
         << QgsProcessingParameterScale::typeName()
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
         << QgsProcessingParameterScale::typeName()
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
         << QgsProcessingParameterDistance::typeName()
         << QgsProcessingParameterScale::typeName();
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
// QgsProcessingScaleWidgetWrapper
//

QgsProcessingScaleWidgetWrapper::QgsProcessingScaleWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsProcessingNumericWidgetWrapper( parameter, type, parent )
{

}

QString QgsProcessingScaleWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterScale::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingScaleWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingScaleWidgetWrapper( parameter, type );
}

QWidget *QgsProcessingScaleWidgetWrapper::createWidget()
{
  const QgsProcessingParameterScale *scaleDef = static_cast< const QgsProcessingParameterScale * >( parameterDefinition() );

  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    case QgsProcessingGui::Batch:
    case QgsProcessingGui::Modeler:
    {
      mScaleWidget = new QgsScaleWidget( nullptr );
      if ( scaleDef->flags() & QgsProcessingParameterDefinition::FlagOptional )
        mScaleWidget->setAllowNull( true );

      mScaleWidget->setMapCanvas( widgetContext().mapCanvas() );
      mScaleWidget->setShowCurrentScaleButton( true );

      mScaleWidget->setToolTip( parameterDefinition()->toolTip() );
      connect( mScaleWidget, &QgsScaleWidget::scaleChanged, this, [ = ]( double )
      {
        emit widgetValueHasChanged( this );
      } );
      return mScaleWidget;
    }
  }
  return nullptr;
}

void QgsProcessingScaleWidgetWrapper::setWidgetContext( const QgsProcessingParameterWidgetContext &context )
{
  if ( mScaleWidget )
    mScaleWidget->setMapCanvas( context.mapCanvas() );
  QgsAbstractProcessingParameterWidgetWrapper::setWidgetContext( context );
}


QVariant QgsProcessingScaleWidgetWrapper::widgetValue() const
{
  return mScaleWidget && !mScaleWidget->isNull() ? QVariant( mScaleWidget->scale() ) : QVariant();
}

void QgsProcessingScaleWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext &context )
{
  if ( mScaleWidget )
  {
    if ( mScaleWidget->allowNull() && !value.isValid() )
      mScaleWidget->setNull();
    else
    {
      const double v = QgsProcessingParameters::parameterAsDouble( parameterDefinition(), value, context );
      mScaleWidget->setScale( v );
    }
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



//
// QgsProcessingMatrixWidgetWrapper
//

QgsProcessingMatrixWidgetWrapper::QgsProcessingMatrixWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{

}

QWidget *QgsProcessingMatrixWidgetWrapper::createWidget()
{
  mMatrixWidget = new QgsProcessingMatrixParameterPanel( nullptr, dynamic_cast< const QgsProcessingParameterMatrix *>( parameterDefinition() ) );
  mMatrixWidget->setToolTip( parameterDefinition()->toolTip() );

  connect( mMatrixWidget, &QgsProcessingMatrixParameterPanel::changed, this, [ = ]
  {
    emit widgetValueHasChanged( this );
  } );

  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    case QgsProcessingGui::Batch:
    case QgsProcessingGui::Modeler:
    {
      return mMatrixWidget;
    };
  }
  return nullptr;
}

void QgsProcessingMatrixWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext &context )
{
  const QVariantList v = QgsProcessingParameters::parameterAsMatrix( parameterDefinition(), value, context );
  if ( mMatrixWidget )
    mMatrixWidget->setValue( v );
}

QVariant QgsProcessingMatrixWidgetWrapper::widgetValue() const
{
  if ( mMatrixWidget )
    return mMatrixWidget->value().isEmpty() ? QVariant() : mMatrixWidget->value();
  else
    return QVariant();
}

QStringList QgsProcessingMatrixWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList()
         << QgsProcessingParameterMatrix::typeName();
}

QStringList QgsProcessingMatrixWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList();
}

QList<int> QgsProcessingMatrixWidgetWrapper::compatibleDataTypes() const
{
  return QList< int >();
}

QString QgsProcessingMatrixWidgetWrapper::modelerExpressionFormatString() const
{
  return tr( "comma delimited string of values, or an array of values" );
}

QString QgsProcessingMatrixWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterMatrix::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingMatrixWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingMatrixWidgetWrapper( parameter, type );
}




//
// QgsProcessingFileWidgetWrapper
//

QgsProcessingFileWidgetWrapper::QgsProcessingFileWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{

}

QWidget *QgsProcessingFileWidgetWrapper::createWidget()
{
  const QgsProcessingParameterFile *fileParam = dynamic_cast< const QgsProcessingParameterFile *>( parameterDefinition() );
  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    case QgsProcessingGui::Modeler:
    case QgsProcessingGui::Batch:
    {
      mFileWidget = new QgsFileWidget();
      mFileWidget->setToolTip( parameterDefinition()->toolTip() );
      mFileWidget->setDialogTitle( parameterDefinition()->description() );

      mFileWidget->setDefaultRoot( QgsSettings().value( QStringLiteral( "/Processing/LastInputPath" ), QDir::homePath() ).toString() );

      switch ( fileParam->behavior() )
      {
        case QgsProcessingParameterFile::File:
          mFileWidget->setStorageMode( QgsFileWidget::GetFile );
          if ( !fileParam->extension().isEmpty() )
            mFileWidget->setFilter( tr( "%1 files" ).arg( fileParam->extension().toUpper() ) + QStringLiteral( " (*." ) + fileParam->extension().toLower() + ')' );
          break;

        case QgsProcessingParameterFile::Folder:
          mFileWidget->setStorageMode( QgsFileWidget::GetDirectory );
          break;
      }

      connect( mFileWidget, &QgsFileWidget::fileChanged, this, [ = ]( const QString & path )
      {
        QgsSettings().setValue( QStringLiteral( "/Processing/LastInputPath" ), QFileInfo( path ).canonicalPath() );
        emit widgetValueHasChanged( this );
      } );
      return mFileWidget;
    };
  }
  return nullptr;
}

void QgsProcessingFileWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext &context )
{
  const QString v = QgsProcessingParameters::parameterAsString( parameterDefinition(), value, context );
  if ( mFileWidget )
    mFileWidget->setFilePath( v );
}

QVariant QgsProcessingFileWidgetWrapper::widgetValue() const
{
  if ( mFileWidget )
    return mFileWidget->filePath();
  else
    return QVariant();
}

QStringList QgsProcessingFileWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList()
         << QgsProcessingParameterString::typeName()
         << QgsProcessingParameterFile::typeName();
}

QStringList QgsProcessingFileWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList() << QgsProcessingOutputFile::typeName()
         << QgsProcessingOutputString::typeName()
         << QgsProcessingOutputRasterLayer::typeName()
         << QgsProcessingOutputVectorLayer::typeName()
         << QgsProcessingOutputMapLayer::typeName();
}

QList<int> QgsProcessingFileWidgetWrapper::compatibleDataTypes() const
{
  return QList< int >();
}

QString QgsProcessingFileWidgetWrapper::modelerExpressionFormatString() const
{
  return tr( "string representing a path to a file or folder" );
}

QString QgsProcessingFileWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterFile::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingFileWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingFileWidgetWrapper( parameter, type );
}




//
// QgsProcessingExpressionWidgetWrapper
//

QgsProcessingExpressionWidgetWrapper::QgsProcessingExpressionWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{

}

QWidget *QgsProcessingExpressionWidgetWrapper::createWidget()
{
  const QgsProcessingParameterExpression *expParam = dynamic_cast< const QgsProcessingParameterExpression *>( parameterDefinition() );
  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    case QgsProcessingGui::Modeler:
    case QgsProcessingGui::Batch:
    {
      if ( expParam->parentLayerParameterName().isEmpty() )
      {
        mExpLineEdit = new QgsExpressionLineEdit();
        mExpLineEdit->setToolTip( parameterDefinition()->toolTip() );
        mExpLineEdit->setExpressionDialogTitle( parameterDefinition()->description() );
        mExpLineEdit->registerExpressionContextGenerator( this );
        connect( mExpLineEdit, &QgsExpressionLineEdit::expressionChanged, this, [ = ]( const QString & )
        {
          emit widgetValueHasChanged( this );
        } );
        return mExpLineEdit;
      }
      else
      {
        mFieldExpWidget = new QgsFieldExpressionWidget();
        mFieldExpWidget->setToolTip( parameterDefinition()->toolTip() );
        mFieldExpWidget->setExpressionDialogTitle( parameterDefinition()->description() );
        mFieldExpWidget->registerExpressionContextGenerator( this );
        connect( mFieldExpWidget, static_cast < void ( QgsFieldExpressionWidget::* )( const QString & ) >( &QgsFieldExpressionWidget::fieldChanged ), this, [ = ]( const QString & )
        {
          emit widgetValueHasChanged( this );
        } );
        return mFieldExpWidget;
      }
    };
  }
  return nullptr;
}

void QgsProcessingExpressionWidgetWrapper::postInitialize( const QList<QgsAbstractProcessingParameterWidgetWrapper *> &wrappers )
{
  QgsAbstractProcessingParameterWidgetWrapper::postInitialize( wrappers );
  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    case QgsProcessingGui::Batch:
    {
      for ( const QgsAbstractProcessingParameterWidgetWrapper *wrapper : wrappers )
      {
        if ( wrapper->parameterDefinition()->name() == static_cast< const QgsProcessingParameterExpression * >( parameterDefinition() )->parentLayerParameterName() )
        {
          setParentLayerWrapperValue( wrapper );
          connect( wrapper, &QgsAbstractProcessingParameterWidgetWrapper::widgetValueHasChanged, this, [ = ]
          {
            setParentLayerWrapperValue( wrapper );
          } );
          break;
        }
      }
      break;
    }

    case QgsProcessingGui::Modeler:
      break;
  }
}

void QgsProcessingExpressionWidgetWrapper::setParentLayerWrapperValue( const QgsAbstractProcessingParameterWidgetWrapper *parentWrapper )
{
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

  QgsVectorLayer *layer = QgsProcessingParameters::parameterAsVectorLayer( parentWrapper->parameterDefinition(), parentWrapper->parameterValue(), *context );
  if ( !layer )
  {
    if ( mFieldExpWidget )
      mFieldExpWidget->setLayer( nullptr );
    else if ( mExpLineEdit )
      mExpLineEdit->setLayer( nullptr );
    return;
  }

  // need to grab ownership of layer if required - otherwise layer may be deleted when context
  // goes out of scope
  std::unique_ptr< QgsMapLayer > ownedLayer( context->takeResultLayer( layer->id() ) );
  if ( ownedLayer && ownedLayer->type() == QgsMapLayer::VectorLayer )
  {
    mParentLayer.reset( qobject_cast< QgsVectorLayer * >( ownedLayer.release() ) );
    layer = mParentLayer.get();
  }
  else
  {
    // don't need ownership of this layer - it wasn't owned by context (so e.g. is owned by the project)
  }

  if ( mFieldExpWidget )
    mFieldExpWidget->setLayer( layer );
  else if ( mExpLineEdit )
    mExpLineEdit->setLayer( layer );
}

void QgsProcessingExpressionWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext &context )
{
  const QString v = QgsProcessingParameters::parameterAsString( parameterDefinition(), value, context );
  if ( mFieldExpWidget )
    mFieldExpWidget->setExpression( v );
  else if ( mExpLineEdit )
    mExpLineEdit->setExpression( v );
}

QVariant QgsProcessingExpressionWidgetWrapper::widgetValue() const
{
  if ( mFieldExpWidget )
    return mFieldExpWidget->expression();
  else if ( mExpLineEdit )
    return mExpLineEdit->expression();
  else
    return QVariant();
}

QStringList QgsProcessingExpressionWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList()
         << QgsProcessingParameterExpression::typeName()
         << QgsProcessingParameterString::typeName()
         << QgsProcessingParameterNumber::typeName()
         << QgsProcessingParameterDistance::typeName()
         << QgsProcessingParameterScale::typeName();
}

QStringList QgsProcessingExpressionWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList()
         << QgsProcessingOutputString::typeName()
         << QgsProcessingOutputNumber::typeName();
}

QList<int> QgsProcessingExpressionWidgetWrapper::compatibleDataTypes() const
{
  return QList< int >();
}

QString QgsProcessingExpressionWidgetWrapper::modelerExpressionFormatString() const
{
  return tr( "string representation of an expression" );
}

const QgsVectorLayer *QgsProcessingExpressionWidgetWrapper::linkedVectorLayer() const
{
  if ( mFieldExpWidget && mFieldExpWidget->layer() )
    return mFieldExpWidget->layer();

  return QgsAbstractProcessingParameterWidgetWrapper::linkedVectorLayer();
}

QString QgsProcessingExpressionWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterExpression::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingExpressionWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingExpressionWidgetWrapper( parameter, type );
}



//
// QgsProcessingEnumPanelWidget
//

QgsProcessingEnumPanelWidget::QgsProcessingEnumPanelWidget( QWidget *parent, const QgsProcessingParameterEnum *param )
  : QWidget( parent )
  , mParam( param )
{
  QHBoxLayout *hl = new QHBoxLayout();
  hl->setMargin( 0 );
  hl->setContentsMargins( 0, 0, 0, 0 );

  mLineEdit = new QLineEdit();
  mLineEdit->setEnabled( false );
  hl->addWidget( mLineEdit, 1 );

  mToolButton = new QToolButton();
  mToolButton->setText( tr( "…" ) );
  hl->addWidget( mToolButton );

  setLayout( hl );

  if ( mParam )
  {
    mLineEdit->setText( tr( "%1 options selected" ).arg( 0 ) );
  }

  connect( mToolButton, &QToolButton::clicked, this, &QgsProcessingEnumPanelWidget::showDialog );
}

void QgsProcessingEnumPanelWidget::setValue( const QVariant &value )
{
  if ( value.isValid() )
    mValue = value.type() == QVariant::List ? value.toList() : QVariantList() << value;
  else
    mValue.clear();

  updateSummaryText();
  emit changed();
}

void QgsProcessingEnumPanelWidget::showDialog()
{
  QVariantList availableOptions;
  if ( mParam )
  {
    availableOptions.reserve( mParam->options().size() );
    for ( int i = 0; i < mParam->options().count(); ++i )
      availableOptions << i;
  }

  QgsProcessingMultipleSelectionDialog dlg( availableOptions, mValue, this, nullptr );
  const QStringList options = mParam ? mParam->options() : QStringList();
  dlg.setValueFormatter( [options]( const QVariant & v ) -> QString
  {
    const int i = v.toInt();
    return options.size() > i ? options.at( i ) : QString();
  } );
  if ( dlg.exec() )
  {
    setValue( dlg.selectedOptions() );
  }
}

void QgsProcessingEnumPanelWidget::updateSummaryText()
{
  if ( mParam )
    mLineEdit->setText( tr( "%1 options selected" ).arg( mValue.count() ) );
}


//
// QgsProcessingEnumCheckboxPanelWidget
//
QgsProcessingEnumCheckboxPanelWidget::QgsProcessingEnumCheckboxPanelWidget( QWidget *parent, const QgsProcessingParameterEnum *param, int columns )
  : QWidget( parent )
  , mParam( param )
  , mButtonGroup( new QButtonGroup( this ) )
  , mColumns( columns )
{
  mButtonGroup->setExclusive( !mParam->allowMultiple() );

  QGridLayout *l = new QGridLayout();
  l->setContentsMargins( 0, 0, 0, 0 );
  l->setMargin( 0 );

  int rows = static_cast< int >( std::ceil( mParam->options().count() / static_cast< double >( mColumns ) ) );
  for ( int i = 0; i < mParam->options().count(); ++i )
  {
    QAbstractButton *button = nullptr;
    if ( mParam->allowMultiple() )
      button = new QCheckBox( mParam->options().at( i ) );
    else
      button = new QRadioButton( mParam->options().at( i ) );

    connect( button, &QAbstractButton::toggled, this, [ = ]
    {
      if ( !mBlockChangedSignal )
        emit changed();
    } );

    mButtons.insert( i, button );
    mButtonGroup->addButton( button, i );
    l->addWidget( button, i % rows, i / rows );
  }
  l->addItem( new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum ), 0, mColumns );
  setLayout( l );

  if ( mParam->allowMultiple() )
  {
    setContextMenuPolicy( Qt::CustomContextMenu );
    connect( this, &QWidget::customContextMenuRequested, this, &QgsProcessingEnumCheckboxPanelWidget::showPopupMenu );
  }
}

QVariant QgsProcessingEnumCheckboxPanelWidget::value() const
{
  if ( mParam->allowMultiple() )
  {
    QVariantList value;
    for ( auto it = mButtons.constBegin(); it != mButtons.constEnd(); ++it )
    {
      if ( it.value()->isChecked() )
        value.append( it.key() );
    }
    return value;
  }
  else
  {
    return mButtonGroup->checkedId() >= 0 ? mButtonGroup->checkedId() : QVariant();
  }
}

void QgsProcessingEnumCheckboxPanelWidget::setValue( const QVariant &value )
{
  mBlockChangedSignal = true;
  if ( mParam->allowMultiple() )
  {
    QVariantList selected;
    if ( value.isValid() )
      selected = value.type() == QVariant::List ? value.toList() : QVariantList() << value;
    for ( auto it = mButtons.constBegin(); it != mButtons.constEnd(); ++it )
    {
      it.value()->setChecked( selected.contains( it.key() ) );
    }
  }
  else
  {
    QVariant v = value;
    if ( v.type() == QVariant::List )
      v = v.toList().value( 0 );
    if ( mButtons.contains( v ) )
      mButtons.value( v )->setChecked( true );
  }
  mBlockChangedSignal = false;
  emit changed();
}

void QgsProcessingEnumCheckboxPanelWidget::showPopupMenu()
{
  QMenu popupMenu;
  QAction *selectAllAction = new QAction( tr( "Select All" ), &popupMenu );
  connect( selectAllAction, &QAction::triggered, this, &QgsProcessingEnumCheckboxPanelWidget::selectAll );
  QAction *clearAllAction = new QAction( tr( "Clear Selection" ), &popupMenu );
  connect( clearAllAction, &QAction::triggered, this, &QgsProcessingEnumCheckboxPanelWidget::deselectAll );
  popupMenu.addAction( selectAllAction );
  popupMenu.addAction( clearAllAction );
  popupMenu.exec( QCursor::pos() );
}

void QgsProcessingEnumCheckboxPanelWidget::selectAll()
{
  mBlockChangedSignal = true;
  for ( auto it = mButtons.constBegin(); it != mButtons.constEnd(); ++it )
    it.value()->setChecked( true );
  mBlockChangedSignal = false;
  emit changed();
}

void QgsProcessingEnumCheckboxPanelWidget::deselectAll()
{
  mBlockChangedSignal = true;
  for ( auto it = mButtons.constBegin(); it != mButtons.constEnd(); ++it )
    it.value()->setChecked( false );
  mBlockChangedSignal = false;
  emit changed();
}


//
// QgsProcessingEnumWidgetWrapper
//

QgsProcessingEnumWidgetWrapper::QgsProcessingEnumWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{

}

QWidget *QgsProcessingEnumWidgetWrapper::createWidget()
{
  const QgsProcessingParameterEnum *expParam = dynamic_cast< const QgsProcessingParameterEnum *>( parameterDefinition() );
  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    {
      // checkbox panel only for use outside in standard gui!
      if ( expParam->metadata().value( QStringLiteral( "widget_wrapper" ) ).toMap().value( QStringLiteral( "useCheckBoxes" ), false ).toBool() )
      {
        const int columns = expParam->metadata().value( QStringLiteral( "widget_wrapper" ) ).toMap().value( QStringLiteral( "columns" ), 2 ).toInt();
        mCheckboxPanel = new QgsProcessingEnumCheckboxPanelWidget( nullptr, expParam, columns );
        mCheckboxPanel->setToolTip( parameterDefinition()->toolTip() );
        connect( mCheckboxPanel, &QgsProcessingEnumCheckboxPanelWidget::changed, this, [ = ]
        {
          emit widgetValueHasChanged( this );
        } );
        return mCheckboxPanel;
      }
    }
    FALLTHROUGH
    case QgsProcessingGui::Modeler:
    case QgsProcessingGui::Batch:
    {
      if ( expParam->allowMultiple() )
      {
        mPanel = new QgsProcessingEnumPanelWidget( nullptr, expParam );
        mPanel->setToolTip( parameterDefinition()->toolTip() );
        connect( mPanel, &QgsProcessingEnumPanelWidget::changed, this, [ = ]
        {
          emit widgetValueHasChanged( this );
        } );
        return mPanel;
      }
      else
      {
        mComboBox = new QComboBox();

        if ( expParam->flags() & QgsProcessingParameterDefinition::FlagOptional )
          mComboBox->addItem( tr( "[Not selected]" ), QVariant() );
        const QStringList options = expParam->options();
        for ( int i = 0; i < options.count(); ++i )
          mComboBox->addItem( options.at( i ), i );

        mComboBox->setToolTip( parameterDefinition()->toolTip() );
        connect( mComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ), this, [ = ]( int )
        {
          emit widgetValueHasChanged( this );
        } );
        return mComboBox;
      }
    };
  }
  return nullptr;
}

void QgsProcessingEnumWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext &context )
{
  if ( mComboBox )
  {
    if ( !value.isValid() )
      mComboBox->setCurrentIndex( mComboBox->findData( QVariant() ) );
    else
    {
      const int v = QgsProcessingParameters::parameterAsEnum( parameterDefinition(), value, context );
      mComboBox->setCurrentIndex( mComboBox->findData( v ) );
    }
  }
  else if ( mPanel || mCheckboxPanel )
  {
    QVariantList opts;
    if ( value.isValid() )
    {
      const QList< int > v = QgsProcessingParameters::parameterAsEnums( parameterDefinition(), value, context );
      opts.reserve( v.size() );
      for ( int i : v )
        opts << i;
    }
    if ( mPanel )
      mPanel->setValue( opts );
    else if ( mCheckboxPanel )
      mCheckboxPanel->setValue( opts );
  }
}

QVariant QgsProcessingEnumWidgetWrapper::widgetValue() const
{
  if ( mComboBox )
    return mComboBox->currentData();
  else if ( mPanel )
    return mPanel->value();
  else if ( mCheckboxPanel )
    return mCheckboxPanel->value();
  else
    return QVariant();
}

QStringList QgsProcessingEnumWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList()
         << QgsProcessingParameterEnum::typeName()
         << QgsProcessingParameterString::typeName()
         << QgsProcessingParameterNumber::typeName();
}

QStringList QgsProcessingEnumWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList()
         << QgsProcessingOutputString::typeName()
         << QgsProcessingOutputNumber::typeName();
}

QList<int> QgsProcessingEnumWidgetWrapper::compatibleDataTypes() const
{
  return QList<int>();
}

QString QgsProcessingEnumWidgetWrapper::modelerExpressionFormatString() const
{
  return tr( "selected option index (starting from 0), array of indices, or comma separated string of options (e.g. '1,3')" );
}

QString QgsProcessingEnumWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterEnum::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingEnumWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingEnumWidgetWrapper( parameter, type );
}



//
// QgsProcessingLayoutWidgetWrapper
//

QgsProcessingLayoutWidgetWrapper::QgsProcessingLayoutWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{

}

QWidget *QgsProcessingLayoutWidgetWrapper::createWidget()
{
  const QgsProcessingParameterLayout *layoutParam = dynamic_cast< const QgsProcessingParameterLayout *>( parameterDefinition() );
  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    case QgsProcessingGui::Batch:
    {
      // combobox only for use outside modeler!
      mComboBox = new QgsLayoutComboBox( nullptr, widgetContext().project() ? widgetContext().project()->layoutManager() : nullptr );
      if ( layoutParam->flags() & QgsProcessingParameterDefinition::FlagOptional )
        mComboBox->setAllowEmptyLayout( true );
      mComboBox->setFilters( QgsLayoutManagerProxyModel::FilterPrintLayouts );

      mComboBox->setToolTip( parameterDefinition()->toolTip() );
      connect( mComboBox, &QgsLayoutComboBox::layoutChanged, this, [ = ]( QgsMasterLayoutInterface * )
      {
        emit widgetValueHasChanged( this );
      } );
      return mComboBox;
    }

    case QgsProcessingGui::Modeler:
    {
      mLineEdit = new QLineEdit();
      mLineEdit->setToolTip( tr( "Name of an existing print layout" ) );
      connect( mLineEdit, &QLineEdit::textChanged, this, [ = ]( const QString & )
      {
        emit widgetValueHasChanged( this );
      } );
      return mLineEdit;
    }
  }
  return nullptr;
}

void QgsProcessingLayoutWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext &context )
{
  if ( mComboBox )
  {
    if ( !value.isValid() )
      mComboBox->setCurrentLayout( nullptr );
    else
    {
      if ( QgsPrintLayout *l = QgsProcessingParameters::parameterAsLayout( parameterDefinition(), value, context ) )
        mComboBox->setCurrentLayout( l );
      else
        mComboBox->setCurrentLayout( nullptr );
    }
  }
  else if ( mLineEdit )
  {
    const QString v = QgsProcessingParameters::parameterAsString( parameterDefinition(), value, context );
    mLineEdit->setText( v );
  }
}

QVariant QgsProcessingLayoutWidgetWrapper::widgetValue() const
{
  if ( mComboBox )
  {
    const QgsMasterLayoutInterface *l = mComboBox->currentLayout();
    return l ? l->name() : QVariant();
  }
  else if ( mLineEdit )
    return mLineEdit->text().isEmpty() ? QVariant() : mLineEdit->text();
  else
    return QVariant();
}

QStringList QgsProcessingLayoutWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList()
         << QgsProcessingParameterLayout::typeName()
         << QgsProcessingParameterString::typeName();
}

QStringList QgsProcessingLayoutWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList()
         << QgsProcessingOutputString::typeName();
}

QList<int> QgsProcessingLayoutWidgetWrapper::compatibleDataTypes() const
{
  return QList<int>();
}

QString QgsProcessingLayoutWidgetWrapper::modelerExpressionFormatString() const
{
  return tr( "string representing the name of an existing print layout" );
}

QString QgsProcessingLayoutWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterLayout::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingLayoutWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingLayoutWidgetWrapper( parameter, type );
}




//
// QgsProcessingLayoutItemWidgetWrapper
//

QgsProcessingLayoutItemWidgetWrapper::QgsProcessingLayoutItemWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{

}

QWidget *QgsProcessingLayoutItemWidgetWrapper::createWidget()
{
  const QgsProcessingParameterLayoutItem *layoutParam = dynamic_cast< const QgsProcessingParameterLayoutItem *>( parameterDefinition() );
  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    case QgsProcessingGui::Batch:
    {
      // combobox only for use outside modeler!
      mComboBox = new QgsLayoutItemComboBox( nullptr, nullptr );
      if ( layoutParam->flags() & QgsProcessingParameterDefinition::FlagOptional )
        mComboBox->setAllowEmptyItem( true );
      if ( layoutParam->itemType() >= 0 )
        mComboBox->setItemType( static_cast< QgsLayoutItemRegistry::ItemType >( layoutParam->itemType() ) );

      mComboBox->setToolTip( parameterDefinition()->toolTip() );
      connect( mComboBox, &QgsLayoutItemComboBox::itemChanged, this, [ = ]( QgsLayoutItem * )
      {
        emit widgetValueHasChanged( this );
      } );
      return mComboBox;
    }

    case QgsProcessingGui::Modeler:
    {
      mLineEdit = new QLineEdit();
      mLineEdit->setToolTip( tr( "UUID or ID of an existing print layout item" ) );
      connect( mLineEdit, &QLineEdit::textChanged, this, [ = ]( const QString & )
      {
        emit widgetValueHasChanged( this );
      } );
      return mLineEdit;
    }
  }
  return nullptr;
}

void QgsProcessingLayoutItemWidgetWrapper::postInitialize( const QList<QgsAbstractProcessingParameterWidgetWrapper *> &wrappers )
{
  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    case QgsProcessingGui::Batch:
    {
      for ( const QgsAbstractProcessingParameterWidgetWrapper *wrapper : wrappers )
      {
        if ( wrapper->parameterDefinition()->name() == static_cast< const QgsProcessingParameterLayoutItem * >( parameterDefinition() )->parentLayoutParameterName() )
        {
          setLayoutParameterValue( wrapper->parameterValue() );
          connect( wrapper, &QgsAbstractProcessingParameterWidgetWrapper::widgetValueHasChanged, this, [ = ]
          {
            setLayoutParameterValue( wrapper->parameterValue() );
          } );
          break;
        }
      }
      break;
    }

    case QgsProcessingGui::Modeler:
      break;
  }
}

void QgsProcessingLayoutItemWidgetWrapper::setLayoutParameterValue( const QVariant &value )
{
  QgsPrintLayout *layout = nullptr;

  // evaluate value to layout
  QgsProcessingContext *context = nullptr;
  std::unique_ptr< QgsProcessingContext > tmpContext;
  if ( mProcessingContextGenerator )
    context = mProcessingContextGenerator->processingContext();

  if ( !context )
  {
    tmpContext = qgis::make_unique< QgsProcessingContext >();
    context = tmpContext.get();
  }

  layout = QgsProcessingParameters::parameterAsLayout( parameterDefinition(), value, *context );
  setLayout( layout );
}

void QgsProcessingLayoutItemWidgetWrapper::setLayout( QgsPrintLayout *layout )
{
  if ( mComboBox )
    mComboBox->setCurrentLayout( layout );
}

void QgsProcessingLayoutItemWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext &context )
{
  if ( mComboBox )
  {
    if ( !value.isValid() )
      mComboBox->setItem( nullptr );
    else
    {
      QgsLayoutItem *item = QgsProcessingParameters::parameterAsLayoutItem( parameterDefinition(), value, context, qobject_cast< QgsPrintLayout * >( mComboBox->currentLayout() ) );
      mComboBox->setItem( item );
    }
  }
  else if ( mLineEdit )
  {
    const QString v = QgsProcessingParameters::parameterAsString( parameterDefinition(), value, context );
    mLineEdit->setText( v );
  }
}

QVariant QgsProcessingLayoutItemWidgetWrapper::widgetValue() const
{
  if ( mComboBox )
  {
    const QgsLayoutItem *i = mComboBox->currentItem();
    return i ? i->uuid() : QVariant();
  }
  else if ( mLineEdit )
    return mLineEdit->text().isEmpty() ? QVariant() : mLineEdit->text();
  else
    return QVariant();
}

QStringList QgsProcessingLayoutItemWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList()
         << QgsProcessingParameterLayoutItem::typeName()
         << QgsProcessingParameterString::typeName();
}

QStringList QgsProcessingLayoutItemWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList()
         << QgsProcessingOutputString::typeName();
}

QList<int> QgsProcessingLayoutItemWidgetWrapper::compatibleDataTypes() const
{
  return QList<int>();
}

QString QgsProcessingLayoutItemWidgetWrapper::modelerExpressionFormatString() const
{
  return tr( "string representing the UUID or ID of an existing print layout item" );
}

QString QgsProcessingLayoutItemWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterLayoutItem::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingLayoutItemWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingLayoutItemWidgetWrapper( parameter, type );
}

//
// QgsProcessingPointMapTool
//

QgsProcessingPointMapTool::QgsProcessingPointMapTool( QgsMapCanvas *canvas )
  : QgsMapTool( canvas )
{
  setCursor( QgsApplication::getThemeCursor( QgsApplication::Cursor::CapturePoint ) );
  mSnapIndicator.reset( new QgsSnapIndicator( canvas ) );
}

QgsProcessingPointMapTool::~QgsProcessingPointMapTool() = default;

void QgsProcessingPointMapTool::deactivate()
{
  mSnapIndicator->setMatch( QgsPointLocator::Match() );
  QgsMapTool::deactivate();
}

void QgsProcessingPointMapTool::canvasMoveEvent( QgsMapMouseEvent *e )
{
  e->snapPoint();
  mSnapIndicator->setMatch( e->mapPointMatch() );
}

void QgsProcessingPointMapTool::canvasPressEvent( QgsMapMouseEvent *e )
{
  if ( e->button() == Qt::LeftButton )
  {
    QgsPointXY point = e->snapPoint();
    emit clicked( point );
    emit complete();
  }
}

void QgsProcessingPointMapTool::keyPressEvent( QKeyEvent *e )
{
  if ( e->key() == Qt::Key_Escape )
  {

    // Override default shortcut management in MapCanvas
    e->ignore();
    emit complete();
  }
}



//
// QgsProcessingPointPanel
//

QgsProcessingPointPanel::QgsProcessingPointPanel( QWidget *parent )
  : QWidget( parent )
{
  QHBoxLayout *l = new QHBoxLayout();
  l->setContentsMargins( 0, 0, 0, 0 );
  l->setMargin( 0 );
  mLineEdit = new QgsFilterLineEdit( );
  mLineEdit->setShowClearButton( false );
  l->addWidget( mLineEdit, 1 );
  mButton = new QToolButton();
  mButton->setText( QStringLiteral( "…" ) );
  l->addWidget( mButton );
  setLayout( l );

  connect( mLineEdit, &QLineEdit::textChanged, this, &QgsProcessingPointPanel::changed );
  connect( mButton, &QToolButton::clicked, this, &QgsProcessingPointPanel::selectOnCanvas );
  mButton->setVisible( false );
}

void QgsProcessingPointPanel::setMapCanvas( QgsMapCanvas *canvas )
{
  mCanvas = canvas;
  mButton->setVisible( true );

  mCrs = canvas->mapSettings().destinationCrs();
  mTool = qgis::make_unique< QgsProcessingPointMapTool >( mCanvas );
  connect( mTool.get(), &QgsProcessingPointMapTool::clicked, this, &QgsProcessingPointPanel::updatePoint );
  connect( mTool.get(), &QgsProcessingPointMapTool::complete, this, &QgsProcessingPointPanel::pointPicked );
}

void QgsProcessingPointPanel::setAllowNull( bool allowNull )
{
  mLineEdit->setShowClearButton( allowNull );
}

QVariant QgsProcessingPointPanel::value() const
{
  return mLineEdit->showClearButton() && mLineEdit->text().trimmed().isEmpty() ? QVariant() : QVariant( mLineEdit->text() );
}

void QgsProcessingPointPanel::clear()
{
  mLineEdit->clear();
}

void QgsProcessingPointPanel::setValue( const QgsPointXY &point, const QgsCoordinateReferenceSystem &crs )
{
  QString newText = QStringLiteral( "%1,%2" ).arg( point.x() ).arg( point.y() );
  mCrs = crs;
  if ( mCrs.isValid() )
  {
    newText += QStringLiteral( " [%1]" ).arg( mCrs.authid() );
  }
  mLineEdit->setText( newText );
}

void QgsProcessingPointPanel::selectOnCanvas()
{
  if ( !mCanvas )
    return;

  mPrevTool = mCanvas->mapTool();
  mCanvas->setMapTool( mTool.get() );

  emit toggleDialogVisibility( false );
}

void QgsProcessingPointPanel::updatePoint( const QgsPointXY &point )
{
  setValue( point, mCanvas->mapSettings().destinationCrs() );
}

void QgsProcessingPointPanel::pointPicked()
{
  if ( !mCanvas )
    return;

  mCanvas->setMapTool( mPrevTool );

  emit toggleDialogVisibility( true );
}




//
// QgsProcessingPointWidgetWrapper
//

QgsProcessingPointWidgetWrapper::QgsProcessingPointWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{

}

QWidget *QgsProcessingPointWidgetWrapper::createWidget()
{
  const QgsProcessingParameterPoint *pointParam = dynamic_cast< const QgsProcessingParameterPoint *>( parameterDefinition() );
  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    case QgsProcessingGui::Batch:
    {
      mPanel = new QgsProcessingPointPanel( nullptr );
      if ( widgetContext().mapCanvas() )
        mPanel->setMapCanvas( widgetContext().mapCanvas() );

      if ( pointParam->flags() & QgsProcessingParameterDefinition::FlagOptional )
        mPanel->setAllowNull( true );

      mPanel->setToolTip( parameterDefinition()->toolTip() );

      connect( mPanel, &QgsProcessingPointPanel::changed, this, [ = ]
      {
        emit widgetValueHasChanged( this );
      } );

      if ( mDialog )
        setDialog( mDialog ); // setup connections to panel - dialog was previously set before the widget was created
      return mPanel;
    }

    case QgsProcessingGui::Modeler:
    {
      mLineEdit = new QLineEdit();
      mLineEdit->setToolTip( tr( "Point as 'x,y'" ) );
      connect( mLineEdit, &QLineEdit::textChanged, this, [ = ]( const QString & )
      {
        emit widgetValueHasChanged( this );
      } );
      return mLineEdit;
    }
  }
  return nullptr;
}

void QgsProcessingPointWidgetWrapper::setWidgetContext( const QgsProcessingParameterWidgetContext &context )
{
  QgsAbstractProcessingParameterWidgetWrapper::setWidgetContext( context );
  if ( mPanel && context.mapCanvas() )
    mPanel->setMapCanvas( context.mapCanvas() );
}

void QgsProcessingPointWidgetWrapper::setDialog( QDialog *dialog )
{
  mDialog = dialog;
  if ( mPanel )
  {
    connect( mPanel, &QgsProcessingPointPanel::toggleDialogVisibility, mDialog, [ = ]( bool visible )
    {
      if ( !visible )
        mDialog->showMinimized();
      else
      {
        mDialog->showNormal();
        mDialog->raise();
        mDialog->activateWindow();
      }
    } );
  }
  QgsAbstractProcessingParameterWidgetWrapper::setDialog( dialog );
}

void QgsProcessingPointWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext &context )
{
  if ( mPanel )
  {
    if ( !value.isValid() || ( value.type() == QVariant::String && value.toString().isEmpty() ) )
      mPanel->clear();
    else
    {
      QgsPointXY p = QgsProcessingParameters::parameterAsPoint( parameterDefinition(), value, context );
      QgsCoordinateReferenceSystem crs = QgsProcessingParameters::parameterAsPointCrs( parameterDefinition(), value, context );
      mPanel->setValue( p, crs );
    }
  }
  else if ( mLineEdit )
  {
    const QString v = QgsProcessingParameters::parameterAsString( parameterDefinition(), value, context );
    mLineEdit->setText( v );
  }
}

QVariant QgsProcessingPointWidgetWrapper::widgetValue() const
{
  if ( mPanel )
  {
    return mPanel->value();
  }
  else if ( mLineEdit )
    return mLineEdit->text().isEmpty() ? QVariant() : mLineEdit->text();
  else
    return QVariant();
}

QStringList QgsProcessingPointWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList()
         << QgsProcessingParameterPoint::typeName()
         << QgsProcessingParameterString::typeName();
}

QStringList QgsProcessingPointWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList()
         << QgsProcessingOutputString::typeName();
}

QList<int> QgsProcessingPointWidgetWrapper::compatibleDataTypes() const
{
  return QList<int>();
}

QString QgsProcessingPointWidgetWrapper::modelerExpressionFormatString() const
{
  return tr( "string of the format 'x,y'" );
}

QString QgsProcessingPointWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterPoint::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingPointWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingPointWidgetWrapper( parameter, type );
}

///@endcond PRIVATE
