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
// QgsProcessingNumericWidgetWrapper
//

QgsProcessingNumericWidgetWrapper::QgsProcessingNumericWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{

}

QWidget *QgsProcessingNumericWidgetWrapper::createWidget()
{
  const QgsProcessingParameterNumber *numberDef = static_cast< const QgsProcessingParameterNumber * >( parameterDefinition() );
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
          mDoubleSpinBox->setDecimals( 6 );

          // guess reasonable step value for double spin boxes
          if ( !qgsDoubleNear( numberDef->maximum(), std::numeric_limits<double>::max() ) &&
               !qgsDoubleNear( numberDef->minimum(), std::numeric_limits<double>::lowest() + 1 ) )
          {
            const double singleStep = calculateStep( numberDef->minimum(), numberDef->maximum() );
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


///@endcond PRIVATE
