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
#include "processing/models/qgsprocessingmodelalgorithm.h"
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
#include "qgsexpressionbuilderwidget.h"
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
#include "qgsmessagebar.h"
#include "qgscolorbutton.h"
#include "qgscoordinateoperationwidget.h"
#include "qgsdatumtransformdialog.h"
#include "qgsfieldcombobox.h"
#include "qgsmapthemecollection.h"
#include "qgsdatetimeedit.h"
#include "qgsproviderconnectioncombobox.h"
#include "qgsdatabaseschemacombobox.h"
#include "qgsdatabasetablecombobox.h"
#include "qgsextentwidget.h"
#include "qgsprocessingenummodelerwidget.h"
#include "qgsprocessingmatrixmodelerwidget.h"
#include "qgsprocessingmaplayercombobox.h"
#include "qgsrasterbandcombobox.h"
#include "qgsprocessingoutputdestinationwidget.h"
#include "qgscheckablecombobox.h"
#include "qgsexpressioncontext.h"
#include "qgsexpressioncontextutils.h"
#include "qgsdoublevalidator.h"
#include "qgsmaplayercombobox.h"
#include "qgsannotationlayer.h"
#include <QToolButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QRadioButton>
#include <QButtonGroup>
#include <QMenu>
#include <QFileDialog>

///@cond PRIVATE

//
// QgsProcessingBooleanWidgetWrapper
//


QgsProcessingBooleanParameterDefinitionWidget::QgsProcessingBooleanParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm, QWidget *parent )
  : QgsProcessingAbstractParameterDefinitionWidget( context, widgetContext, definition, algorithm, parent )
{
  QVBoxLayout *vlayout = new QVBoxLayout();
  vlayout->setContentsMargins( 0, 0, 0, 0 );

  mDefaultCheckBox = new QCheckBox( tr( "Checked" ) );
  if ( const QgsProcessingParameterBoolean *boolParam = dynamic_cast<const QgsProcessingParameterBoolean *>( definition ) )
    mDefaultCheckBox->setChecked( QgsProcessingParameters::parameterAsBool( boolParam, boolParam->defaultValueForGui(), context ) );
  else
    mDefaultCheckBox->setChecked( false );
  vlayout->addWidget( mDefaultCheckBox );
  setLayout( vlayout );
}

QgsProcessingParameterDefinition *QgsProcessingBooleanParameterDefinitionWidget::createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const
{
  auto param = std::make_unique< QgsProcessingParameterBoolean >( name, description, mDefaultCheckBox->isChecked() );
  param->setFlags( flags );
  return param.release();
}


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
    }

    case QgsProcessingGui::Batch:
    case QgsProcessingGui::Modeler:
    {
      mComboBox = new QComboBox();
      mComboBox->addItem( tr( "Yes" ), true );
      mComboBox->addItem( tr( "No" ), false );
      mComboBox->setToolTip( parameterDefinition()->toolTip() );

      connect( mComboBox, qOverload< int>( &QComboBox::currentIndexChanged ), this, [ = ]
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
         << QgsProcessingParameterDuration::typeName()
         << QgsProcessingParameterScale::typeName()
         << QgsProcessingParameterFile::typeName()
         << QgsProcessingParameterField::typeName()
         << QgsProcessingParameterFeatureSource::typeName()
         << QgsProcessingParameterMapLayer::typeName()
         << QgsProcessingParameterRasterLayer::typeName()
         << QgsProcessingParameterVectorLayer::typeName()
         << QgsProcessingParameterMeshLayer::typeName()
         << QgsProcessingParameterExpression::typeName()
         << QgsProcessingParameterProviderConnection::typeName()
         << QgsProcessingParameterPointCloudLayer::typeName()
         << QgsProcessingParameterAnnotationLayer::typeName();
}

QStringList QgsProcessingBooleanWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList() << QgsProcessingOutputNumber::typeName()
         << QgsProcessingOutputMapLayer::typeName()
         << QgsProcessingOutputFile::typeName()
         << QgsProcessingOutputRasterLayer::typeName()
         << QgsProcessingOutputVectorLayer::typeName()
         << QgsProcessingOutputString::typeName()
         << QgsProcessingOutputBoolean::typeName();
}

QString QgsProcessingBooleanWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterBoolean::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingBooleanWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingBooleanWidgetWrapper( parameter, type );
}

QgsProcessingAbstractParameterDefinitionWidget *QgsProcessingBooleanWidgetWrapper::createParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm )
{
  return new QgsProcessingBooleanParameterDefinitionWidget( context, widgetContext, definition, algorithm );
}


//
// QgsProcessingCrsWidgetWrapper
//

QgsProcessingCrsParameterDefinitionWidget::QgsProcessingCrsParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm, QWidget *parent )
  : QgsProcessingAbstractParameterDefinitionWidget( context, widgetContext, definition, algorithm, parent )
{
  QVBoxLayout *vlayout = new QVBoxLayout();
  vlayout->setContentsMargins( 0, 0, 0, 0 );

  vlayout->addWidget( new QLabel( tr( "Default value" ) ) );

  mCrsSelector = new QgsProjectionSelectionWidget();

  // possibly we should expose this for parameter by parameter control
  mCrsSelector->setShowAccuracyWarnings( true );

  if ( const QgsProcessingParameterCrs *crsParam = dynamic_cast<const QgsProcessingParameterCrs *>( definition ) )
    mCrsSelector->setCrs( QgsProcessingParameters::parameterAsCrs( crsParam, crsParam->defaultValueForGui(), context ) );
  else
    mCrsSelector->setCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );

  vlayout->addWidget( mCrsSelector );
  setLayout( vlayout );
}

QgsProcessingParameterDefinition *QgsProcessingCrsParameterDefinitionWidget::createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const
{
  auto param = std::make_unique< QgsProcessingParameterCrs >( name, description, mCrsSelector->crs().authid() );
  param->setFlags( flags );
  return param.release();
}

QgsProcessingCrsWidgetWrapper::QgsProcessingCrsWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{

}

QWidget *QgsProcessingCrsWidgetWrapper::createWidget()
{
  Q_ASSERT( mProjectionSelectionWidget == nullptr );
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
    }

    case QgsProcessingGui::Modeler:
    {
      QWidget *w = new QWidget();
      w->setToolTip( parameterDefinition()->toolTip() );

      QVBoxLayout *vl = new QVBoxLayout();
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
         << QgsProcessingParameterFeatureSource::typeName()
         << QgsProcessingParameterPointCloudLayer::typeName()
         << QgsProcessingParameterAnnotationLayer::typeName();
}

QStringList QgsProcessingCrsWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList() << QgsProcessingOutputVectorLayer::typeName()
         << QgsProcessingOutputRasterLayer::typeName()
         << QgsProcessingOutputMapLayer::typeName()
         << QgsProcessingOutputString::typeName();
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

QgsProcessingAbstractParameterDefinitionWidget *QgsProcessingCrsWidgetWrapper::createParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm )
{
  return new QgsProcessingCrsParameterDefinitionWidget( context, widgetContext, definition, algorithm );
}



//
// QgsProcessingStringWidgetWrapper
//


QgsProcessingStringParameterDefinitionWidget::QgsProcessingStringParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm, QWidget *parent )
  : QgsProcessingAbstractParameterDefinitionWidget( context, widgetContext, definition, algorithm, parent )
{
  QVBoxLayout *vlayout = new QVBoxLayout();
  vlayout->setContentsMargins( 0, 0, 0, 0 );

  vlayout->addWidget( new QLabel( tr( "Default value" ) ) );

  mDefaultLineEdit = new QLineEdit();
  if ( const QgsProcessingParameterString *stringParam = dynamic_cast<const QgsProcessingParameterString *>( definition ) )
    mDefaultLineEdit->setText( QgsProcessingParameters::parameterAsString( stringParam, stringParam->defaultValueForGui(), context ) );
  vlayout->addWidget( mDefaultLineEdit );

  mMultiLineCheckBox = new QCheckBox( tr( "Multiline input" ) );
  if ( const QgsProcessingParameterString *stringParam = dynamic_cast<const QgsProcessingParameterString *>( definition ) )
    mMultiLineCheckBox->setChecked( stringParam->multiLine() );
  vlayout->addWidget( mMultiLineCheckBox );

  setLayout( vlayout );
}

QgsProcessingParameterDefinition *QgsProcessingStringParameterDefinitionWidget::createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const
{
  auto param = std::make_unique< QgsProcessingParameterString >( name, description, mDefaultLineEdit->text(), mMultiLineCheckBox->isChecked() );
  param->setFlags( flags );
  return param.release();
}



QgsProcessingStringWidgetWrapper::QgsProcessingStringWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{

}

QWidget *QgsProcessingStringWidgetWrapper::createWidget()
{
  const QVariantMap metadata = parameterDefinition()->metadata();
  const QVariant valueHintsVariant = metadata.value( QStringLiteral( "widget_wrapper" ) ).toMap().value( QStringLiteral( "value_hints" ) );

  if ( valueHintsVariant.isValid() )
  {
    const QVariantList valueList = valueHintsVariant.toList();
    mComboBox = new QComboBox();
    mComboBox->setToolTip( parameterDefinition()->toolTip() );

    if ( parameterDefinition()->flags() & QgsProcessingParameterDefinition::FlagOptional )
    {
      mComboBox->addItem( QString() );
    }
    for ( const QVariant &entry : valueList )
    {
      mComboBox->addItem( entry.toString(), entry.toString() );
    }
    mComboBox->setCurrentIndex( 0 );

    connect( mComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, [ = ]( int )
    {
      emit widgetValueHasChanged( this );
    } );
    return mComboBox;
  }
  else
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
      }

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
  if ( mComboBox )
  {
    int index = -1;
    if ( !value.isValid() )
      index = mComboBox->findData( QVariant() );
    else
      index = mComboBox->findData( v );

    if ( index >= 0 )
      mComboBox->setCurrentIndex( index );
    else
      mComboBox->setCurrentIndex( 0 );
  }
}

QVariant QgsProcessingStringWidgetWrapper::widgetValue() const
{
  if ( mLineEdit )
    return mLineEdit->text();
  else if ( mPlainTextEdit )
    return mPlainTextEdit->toPlainText();
  else if ( mComboBox )
    return mComboBox->currentData();
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
         << QgsProcessingParameterDuration::typeName()
         << QgsProcessingParameterScale::typeName()
         << QgsProcessingParameterFile::typeName()
         << QgsProcessingParameterField::typeName()
         << QgsProcessingParameterExpression::typeName()
         << QgsProcessingParameterCoordinateOperation::typeName()
         << QgsProcessingParameterProviderConnection::typeName();
}

QStringList QgsProcessingStringWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList() << QgsProcessingOutputNumber::typeName()
         << QgsProcessingOutputFile::typeName()
         << QgsProcessingOutputFolder::typeName()
         << QgsProcessingOutputString::typeName();
}

QString QgsProcessingStringWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterString::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingStringWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingStringWidgetWrapper( parameter, type );
}

QgsProcessingAbstractParameterDefinitionWidget *QgsProcessingStringWidgetWrapper::createParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm )
{
  return new QgsProcessingStringParameterDefinitionWidget( context, widgetContext, definition, algorithm );
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
    }
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

QgsProcessingNumberParameterDefinitionWidget::QgsProcessingNumberParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm, QWidget *parent )
  : QgsProcessingAbstractParameterDefinitionWidget( context, widgetContext, definition, algorithm, parent )
{
  QVBoxLayout *vlayout = new QVBoxLayout();
  vlayout->setContentsMargins( 0, 0, 0, 0 );

  vlayout->addWidget( new QLabel( tr( "Number type" ) ) );

  mTypeComboBox = new QComboBox();
  mTypeComboBox->addItem( tr( "Float" ), QgsProcessingParameterNumber::Double );
  mTypeComboBox->addItem( tr( "Integer" ), QgsProcessingParameterNumber::Integer );
  vlayout->addWidget( mTypeComboBox );

  vlayout->addWidget( new QLabel( tr( "Minimum value" ) ) );
  mMinLineEdit = new QLineEdit();
  vlayout->addWidget( mMinLineEdit );

  vlayout->addWidget( new QLabel( tr( "Maximum value" ) ) );
  mMaxLineEdit = new QLineEdit();
  vlayout->addWidget( mMaxLineEdit );

  vlayout->addWidget( new QLabel( tr( "Default value" ) ) );
  mDefaultLineEdit = new QLineEdit();
  vlayout->addWidget( mDefaultLineEdit );

  if ( const QgsProcessingParameterNumber *numberParam = dynamic_cast<const QgsProcessingParameterNumber *>( definition ) )
  {
    mTypeComboBox->setCurrentIndex( mTypeComboBox->findData( numberParam->dataType() ) );

    if ( !qgsDoubleNear( numberParam->maximum(), std::numeric_limits<double>::max() ) )
    {
      mMaxLineEdit->setText( QLocale().toString( numberParam->maximum() ) );
    }
    else
    {
      mMaxLineEdit->clear();
    }

    if ( !qgsDoubleNear( numberParam->minimum(), std::numeric_limits<double>::lowest() ) )
    {
      mMinLineEdit->setText( QLocale().toString( numberParam->minimum() ) );
    }
    else
    {
      mMinLineEdit->clear();
    }

    mDefaultLineEdit->setText( numberParam->defaultValueForGui().toString() );
  }

  setLayout( vlayout );
}

QgsProcessingParameterDefinition *QgsProcessingNumberParameterDefinitionWidget::createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const
{
  bool ok;
  double val = QgsDoubleValidator::toDouble( mDefaultLineEdit->text(), &ok );

  QgsProcessingParameterNumber::Type dataType = static_cast< QgsProcessingParameterNumber::Type >( mTypeComboBox->currentData().toInt() );
  auto param = std::make_unique< QgsProcessingParameterNumber >( name, description, dataType, ok ? val : QVariant() );

  if ( !mMinLineEdit->text().trimmed().isEmpty() )
  {
    val = QgsDoubleValidator::toDouble( mMinLineEdit->text( ), &ok );
    if ( ok )
    {
      param->setMinimum( val );
    }
  }

  if ( !mMaxLineEdit->text().trimmed().isEmpty() )
  {
    val = QgsDoubleValidator::toDouble( mMaxLineEdit->text(), &ok );
    if ( ok )
    {
      param->setMaximum( val );
    }
  }

  param->setFlags( flags );
  return param.release();
}

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
          const double min = mDoubleSpinBox->minimum() - mDoubleSpinBox->singleStep();
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
        if ( numberDef->defaultValueForGui().isValid() )
        {
          // if default value for parameter, we clear to that
          bool ok = false;
          if ( mDoubleSpinBox )
          {
            double defaultVal = numberDef->defaultValueForGui().toDouble( &ok );
            if ( ok )
              mDoubleSpinBox->setClearValue( defaultVal );
          }
          else
          {
            int intVal = numberDef->defaultValueForGui().toInt( &ok );
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
        connect( mDoubleSpinBox, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, [ = ] { emit widgetValueHasChanged( this ); } );
      else if ( mSpinBox )
        connect( mSpinBox, qOverload<int>( &QgsSpinBox::valueChanged ), this, [ = ] { emit widgetValueHasChanged( this ); } );

      return spinBox;
    }
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
         << QgsProcessingParameterDuration::typeName()
         << QgsProcessingParameterScale::typeName();
}

QStringList QgsProcessingNumericWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList() << QgsProcessingOutputNumber::typeName()
         << QgsProcessingOutputString::typeName();
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

QgsProcessingAbstractParameterDefinitionWidget *QgsProcessingNumericWidgetWrapper::createParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm )
{
  return new QgsProcessingNumberParameterDefinitionWidget( context, widgetContext, definition, algorithm );
}

//
// QgsProcessingDistanceWidgetWrapper
//

QgsProcessingDistanceParameterDefinitionWidget::QgsProcessingDistanceParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm, QWidget *parent )
  : QgsProcessingAbstractParameterDefinitionWidget( context, widgetContext, definition, algorithm, parent )
{
  QVBoxLayout *vlayout = new QVBoxLayout();
  vlayout->setContentsMargins( 0, 0, 0, 0 );

  vlayout->addWidget( new QLabel( tr( "Linked input" ) ) );

  mParentLayerComboBox = new QComboBox();

  QString initialParent;
  if ( const QgsProcessingParameterDistance *distParam = dynamic_cast<const QgsProcessingParameterDistance *>( definition ) )
    initialParent = distParam->parentParameterName();

  if ( auto *lModel = widgetContext.model() )
  {
    // populate combo box with other model input choices
    const QMap<QString, QgsProcessingModelParameter> components = lModel->parameterComponents();
    for ( auto it = components.constBegin(); it != components.constEnd(); ++it )
    {
      if ( const QgsProcessingParameterFeatureSource *definition = dynamic_cast< const QgsProcessingParameterFeatureSource * >( lModel->parameterDefinition( it.value().parameterName() ) ) )
      {
        mParentLayerComboBox-> addItem( definition->description(), definition->name() );
        if ( !initialParent.isEmpty() && initialParent == definition->name() )
        {
          mParentLayerComboBox->setCurrentIndex( mParentLayerComboBox->count() - 1 );
        }
      }
      else if ( const QgsProcessingParameterVectorLayer *definition = dynamic_cast< const QgsProcessingParameterVectorLayer * >( lModel->parameterDefinition( it.value().parameterName() ) ) )
      {
        mParentLayerComboBox-> addItem( definition->description(), definition->name() );
        if ( !initialParent.isEmpty() && initialParent == definition->name() )
        {
          mParentLayerComboBox->setCurrentIndex( mParentLayerComboBox->count() - 1 );
        }
      }
      else if ( const QgsProcessingParameterMapLayer *definition = dynamic_cast< const QgsProcessingParameterMapLayer * >( lModel->parameterDefinition( it.value().parameterName() ) ) )
      {
        mParentLayerComboBox-> addItem( definition->description(), definition->name() );
        if ( !initialParent.isEmpty() && initialParent == definition->name() )
        {
          mParentLayerComboBox->setCurrentIndex( mParentLayerComboBox->count() - 1 );
        }
      }
      else if ( const QgsProcessingParameterCrs *definition = dynamic_cast< const QgsProcessingParameterCrs * >( lModel->parameterDefinition( it.value().parameterName() ) ) )
      {
        mParentLayerComboBox-> addItem( definition->description(), definition->name() );
        if ( !initialParent.isEmpty() && initialParent == definition->name() )
        {
          mParentLayerComboBox->setCurrentIndex( mParentLayerComboBox->count() - 1 );
        }
      }
    }
  }

  if ( mParentLayerComboBox->count() == 0 && !initialParent.isEmpty() )
  {
    // if no parent candidates found, we just add the existing one as a placeholder
    mParentLayerComboBox->addItem( initialParent, initialParent );
    mParentLayerComboBox->setCurrentIndex( mParentLayerComboBox->count() - 1 );
  }

  vlayout->addWidget( mParentLayerComboBox );

  vlayout->addWidget( new QLabel( tr( "Minimum value" ) ) );
  mMinLineEdit = new QLineEdit();
  vlayout->addWidget( mMinLineEdit );

  vlayout->addWidget( new QLabel( tr( "Maximum value" ) ) );
  mMaxLineEdit = new QLineEdit();
  vlayout->addWidget( mMaxLineEdit );

  vlayout->addWidget( new QLabel( tr( "Default value" ) ) );
  mDefaultLineEdit = new QLineEdit();
  vlayout->addWidget( mDefaultLineEdit );

  if ( const QgsProcessingParameterDistance *distParam = dynamic_cast<const QgsProcessingParameterDistance *>( definition ) )
  {
    mMinLineEdit->setText( QLocale().toString( distParam->minimum() ) );
    mMaxLineEdit->setText( QLocale().toString( distParam->maximum() ) );
    mDefaultLineEdit->setText( distParam->defaultValueForGui().toString() );
  }

  setLayout( vlayout );
}

QgsProcessingParameterDefinition *QgsProcessingDistanceParameterDefinitionWidget::createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const
{
  bool ok;
  double val = QgsDoubleValidator::toDouble( mDefaultLineEdit->text(), &ok );

  auto param = std::make_unique< QgsProcessingParameterDistance >( name, description, ok ? val : QVariant(), mParentLayerComboBox->currentData().toString() );

  val = QgsDoubleValidator::toDouble( mMinLineEdit->text(), &ok );
  if ( ok )
  {
    param->setMinimum( val );
  }

  val = QgsDoubleValidator::toDouble( mMaxLineEdit->text(), &ok );
  if ( ok )
  {
    param->setMaximum( val );
  }

  param->setFlags( flags );
  return param.release();
}

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

      const int labelMargin = static_cast< int >( std::round( mUnitsCombo->fontMetrics().horizontalAdvance( 'X' ) ) );
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

      QWidget *w = new QWidget();
      layout->setContentsMargins( 0, 0, 0, 0 );
      w->setLayout( layout );

      setUnits( distanceDef->defaultUnit() );

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
    tmpContext = std::make_unique< QgsProcessingContext >();
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

QgsProcessingAbstractParameterDefinitionWidget *QgsProcessingDistanceWidgetWrapper::createParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm )
{
  return new QgsProcessingDistanceParameterDefinitionWidget( context, widgetContext, definition, algorithm );
}


//
// QgsProcessingDurationWidgetWrapper
//

QgsProcessingDurationParameterDefinitionWidget::QgsProcessingDurationParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm, QWidget *parent )
  : QgsProcessingAbstractParameterDefinitionWidget( context, widgetContext, definition, algorithm, parent )
{
  QVBoxLayout *vlayout = new QVBoxLayout();
  vlayout->setContentsMargins( 0, 0, 0, 0 );

  vlayout->addWidget( new QLabel( tr( "Minimum value" ) ) );
  mMinLineEdit = new QLineEdit();
  vlayout->addWidget( mMinLineEdit );

  vlayout->addWidget( new QLabel( tr( "Maximum value" ) ) );
  mMaxLineEdit = new QLineEdit();
  vlayout->addWidget( mMaxLineEdit );

  vlayout->addWidget( new QLabel( tr( "Default value" ) ) );
  mDefaultLineEdit = new QLineEdit();
  vlayout->addWidget( mDefaultLineEdit );

  vlayout->addWidget( new QLabel( tr( "Default unit type" ) ) );

  mUnitsCombo = new QComboBox();
  mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::TemporalMilliseconds ), QgsUnitTypes::TemporalMilliseconds );
  mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::TemporalSeconds ), QgsUnitTypes::TemporalSeconds );
  mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::TemporalMinutes ), QgsUnitTypes::TemporalMinutes );
  mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::TemporalHours ), QgsUnitTypes::TemporalHours );
  mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::TemporalDays ), QgsUnitTypes::TemporalDays );
  mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::TemporalWeeks ), QgsUnitTypes::TemporalWeeks );
  mUnitsCombo->addItem( tr( "years (365.25 days)" ), QgsUnitTypes::TemporalYears );
  mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::TemporalDecades ), QgsUnitTypes::TemporalDecades );
  mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::TemporalCenturies ), QgsUnitTypes::TemporalCenturies );
  vlayout->addWidget( mUnitsCombo );

  if ( const QgsProcessingParameterDuration *durationParam = dynamic_cast<const QgsProcessingParameterDuration *>( definition ) )
  {
    mMinLineEdit->setText( QLocale().toString( durationParam->minimum() ) );
    mMaxLineEdit->setText( QLocale().toString( durationParam->maximum() ) );
    mDefaultLineEdit->setText( durationParam->defaultValueForGui().toString() );
    mUnitsCombo->setCurrentIndex( durationParam->defaultUnit() );
  }

  setLayout( vlayout );
}

QgsProcessingParameterDefinition *QgsProcessingDurationParameterDefinitionWidget::createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const
{
  bool ok;
  double val = QgsDoubleValidator::toDouble( mDefaultLineEdit->text(), &ok );

  auto param = std::make_unique< QgsProcessingParameterDuration >( name, description, ok ? val : QVariant() );

  val = QgsDoubleValidator::toDouble( mMinLineEdit->text(), &ok );
  if ( ok )
  {
    param->setMinimum( val );
  }

  val = QgsDoubleValidator::toDouble( mMaxLineEdit->text(), &ok );
  if ( ok )
  {
    param->setMaximum( val );
  }

  param->setDefaultUnit( static_cast<QgsUnitTypes::TemporalUnit >( mUnitsCombo->currentData().toInt() ) );

  param->setFlags( flags );
  return param.release();
}

QgsProcessingDurationWidgetWrapper::QgsProcessingDurationWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsProcessingNumericWidgetWrapper( parameter, type, parent )
{

}

QString QgsProcessingDurationWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterDuration::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingDurationWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingDurationWidgetWrapper( parameter, type );
}

QWidget *QgsProcessingDurationWidgetWrapper::createWidget()
{
  const QgsProcessingParameterDuration *durationDef = static_cast< const QgsProcessingParameterDuration * >( parameterDefinition() );

  QWidget *spin = QgsProcessingNumericWidgetWrapper::createWidget();
  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    {
      mUnitsCombo = new QComboBox();

      mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::TemporalMilliseconds ), QgsUnitTypes::TemporalMilliseconds );
      mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::TemporalSeconds ), QgsUnitTypes::TemporalSeconds );
      mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::TemporalMinutes ), QgsUnitTypes::TemporalMinutes );
      mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::TemporalHours ), QgsUnitTypes::TemporalHours );
      mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::TemporalDays ), QgsUnitTypes::TemporalDays );
      mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::TemporalWeeks ), QgsUnitTypes::TemporalWeeks );
      mUnitsCombo->addItem( tr( "years (365.25 days)" ), QgsUnitTypes::TemporalYears );
      mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::TemporalDecades ), QgsUnitTypes::TemporalDecades );
      mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::TemporalCenturies ), QgsUnitTypes::TemporalCenturies );

      QHBoxLayout *layout = new QHBoxLayout();
      layout->addWidget( spin, 1 );
      layout->insertWidget( 1, mUnitsCombo );

      QWidget *w = new QWidget();
      layout->setContentsMargins( 0, 0, 0, 0 );
      w->setLayout( layout );

      mUnitsCombo->setCurrentIndex( mUnitsCombo->findData( durationDef->defaultUnit() ) );
      mUnitsCombo->show();

      return w;
    }

    case QgsProcessingGui::Batch:
    case QgsProcessingGui::Modeler:
      return spin;

  }
  return nullptr;
}

QLabel *QgsProcessingDurationWidgetWrapper::createLabel()
{
  QLabel *label = QgsAbstractProcessingParameterWidgetWrapper::createLabel();

  if ( type() == QgsProcessingGui::Modeler )
  {
    label->setText( QStringLiteral( "%1 [%2]" ).arg( label->text(), QgsUnitTypes::toString( mBaseUnit ) ) );
  }

  return label;
}

QVariant QgsProcessingDurationWidgetWrapper::widgetValue() const
{
  const QVariant val = QgsProcessingNumericWidgetWrapper::widgetValue();
  if ( val.type() == QVariant::Double && mUnitsCombo )
  {
    QgsUnitTypes::TemporalUnit displayUnit = static_cast<QgsUnitTypes::TemporalUnit >( mUnitsCombo->currentData().toInt() );
    return val.toDouble() * QgsUnitTypes::fromUnitToUnitFactor( displayUnit, mBaseUnit );
  }
  else
  {
    return val;
  }
}

void QgsProcessingDurationWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext &context )
{
  if ( mUnitsCombo )
  {
    QgsUnitTypes::TemporalUnit displayUnit = static_cast<QgsUnitTypes::TemporalUnit >( mUnitsCombo->currentData().toInt() );
    const QVariant val = value.toDouble() * QgsUnitTypes::fromUnitToUnitFactor( mBaseUnit, displayUnit );
    QgsProcessingNumericWidgetWrapper::setWidgetValue( val, context );
  }
  else
  {
    QgsProcessingNumericWidgetWrapper::setWidgetValue( value, context );
  }
}

QgsProcessingAbstractParameterDefinitionWidget *QgsProcessingDurationWidgetWrapper::createParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm )
{
  return new QgsProcessingDurationParameterDefinitionWidget( context, widgetContext, definition, algorithm );
}

//
// QgsProcessingScaleWidgetWrapper
//

QgsProcessingScaleParameterDefinitionWidget::QgsProcessingScaleParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm, QWidget *parent )
  : QgsProcessingAbstractParameterDefinitionWidget( context, widgetContext, definition, algorithm, parent )
{
  QVBoxLayout *vlayout = new QVBoxLayout();
  vlayout->setContentsMargins( 0, 0, 0, 0 );

  vlayout->addWidget( new QLabel( tr( "Default value" ) ) );

  mDefaultLineEdit = new QLineEdit();

  if ( const QgsProcessingParameterScale *scaleParam = dynamic_cast<const QgsProcessingParameterScale *>( definition ) )
  {
    mDefaultLineEdit->setText( scaleParam->defaultValueForGui().toString() );
  }

  vlayout->addWidget( mDefaultLineEdit );

  setLayout( vlayout );
}

QgsProcessingParameterDefinition *QgsProcessingScaleParameterDefinitionWidget::createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const
{
  bool ok;
  double val = mDefaultLineEdit->text().toDouble( &ok );
  auto param = std::make_unique< QgsProcessingParameterScale >( name, description, ok ? val : QVariant() );
  param->setFlags( flags );
  return param.release();
}

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

QgsProcessingAbstractParameterDefinitionWidget *QgsProcessingScaleWidgetWrapper::createParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm )
{
  return new QgsProcessingScaleParameterDefinitionWidget( context, widgetContext, definition, algorithm );
}


//
// QgsProcessingRangeWidgetWrapper
//

QgsProcessingRangeParameterDefinitionWidget::QgsProcessingRangeParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm, QWidget *parent )
  : QgsProcessingAbstractParameterDefinitionWidget( context, widgetContext, definition, algorithm, parent )
{
  QVBoxLayout *vlayout = new QVBoxLayout();
  vlayout->setContentsMargins( 0, 0, 0, 0 );

  vlayout->addWidget( new QLabel( tr( "Number type" ) ) );

  mTypeComboBox = new QComboBox();
  mTypeComboBox->addItem( tr( "Float" ), QgsProcessingParameterNumber::Double );
  mTypeComboBox->addItem( tr( "Integer" ), QgsProcessingParameterNumber::Integer );
  vlayout->addWidget( mTypeComboBox );

  vlayout->addWidget( new QLabel( tr( "Minimum value" ) ) );
  mMinLineEdit = new QLineEdit();
  vlayout->addWidget( mMinLineEdit );

  vlayout->addWidget( new QLabel( tr( "Maximum value" ) ) );
  mMaxLineEdit = new QLineEdit();
  vlayout->addWidget( mMaxLineEdit );

  if ( const QgsProcessingParameterRange *rangeParam = dynamic_cast<const QgsProcessingParameterRange *>( definition ) )
  {
    mTypeComboBox->setCurrentIndex( mTypeComboBox->findData( rangeParam->dataType() ) );
    const QList< double > range = QgsProcessingParameters::parameterAsRange( rangeParam, rangeParam->defaultValueForGui(), context );
    mMinLineEdit->setText( QLocale().toString( range.at( 0 ) ) );
    mMaxLineEdit->setText( QLocale().toString( range.at( 1 ) ) );
  }

  setLayout( vlayout );
}

QgsProcessingParameterDefinition *QgsProcessingRangeParameterDefinitionWidget::createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const
{
  QString defaultValue;
  if ( mMinLineEdit->text().isEmpty() )
  {
    defaultValue = QStringLiteral( "None" );
  }
  else
  {
    bool ok;
    defaultValue = QString::number( QgsDoubleValidator::toDouble( mMinLineEdit->text(), &ok ) );
    if ( ! ok )
    {
      defaultValue = QStringLiteral( "None" );
    }
  }

  if ( mMaxLineEdit->text().isEmpty() )
  {
    defaultValue += QLatin1String( ",None" );
  }
  else
  {
    bool ok;
    const double val { QgsDoubleValidator::toDouble( mMaxLineEdit->text(), &ok ) };
    defaultValue += QStringLiteral( ",%1" ).arg( ok ? QString::number( val ) : QLatin1String( "None" ) );
  }

  QgsProcessingParameterNumber::Type dataType = static_cast< QgsProcessingParameterNumber::Type >( mTypeComboBox->currentData().toInt() );
  auto param = std::make_unique< QgsProcessingParameterRange >( name, description, dataType, defaultValue );
  param->setFlags( flags );
  return param.release();
}


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

      if ( rangeDef->flags() & QgsProcessingParameterDefinition::FlagOptional )
      {
        mAllowingNull = true;

        const double min = mMinSpinBox->minimum() - 1;
        mMinSpinBox->setMinimum( min );
        mMaxSpinBox->setMinimum( min );
        mMinSpinBox->setValue( min );
        mMaxSpinBox->setValue( min );

        mMinSpinBox->setShowClearButton( true );
        mMaxSpinBox->setShowClearButton( true );
        mMinSpinBox->setSpecialValueText( tr( "Not set" ) );
        mMaxSpinBox->setSpecialValueText( tr( "Not set" ) );
      }

      w->setToolTip( parameterDefinition()->toolTip() );

      connect( mMinSpinBox, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, [ = ]( const double v )
      {
        mBlockChangedSignal++;
        if ( !mAllowingNull && v > mMaxSpinBox->value() )
          mMaxSpinBox->setValue( v );
        mBlockChangedSignal--;

        if ( !mBlockChangedSignal )
          emit widgetValueHasChanged( this );
      } );
      connect( mMaxSpinBox, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, [ = ]( const double v )
      {
        mBlockChangedSignal++;
        if ( !mAllowingNull && v < mMinSpinBox->value() )
          mMinSpinBox->setValue( v );
        mBlockChangedSignal--;

        if ( !mBlockChangedSignal )
          emit widgetValueHasChanged( this );
      } );

      return w;
    }
  }
  return nullptr;
}

void QgsProcessingRangeWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext &context )
{
  const QList< double > v = QgsProcessingParameters::parameterAsRange( parameterDefinition(), value, context );
  if ( mAllowingNull && v.empty() )
  {
    mMinSpinBox->clear();
    mMaxSpinBox->clear();
  }
  else
  {
    if ( v.empty() )
      return;

    if ( mAllowingNull )
    {
      mBlockChangedSignal++;
      if ( std::isnan( v.at( 0 ) ) )
        mMinSpinBox->clear();
      else
        mMinSpinBox->setValue( v.at( 0 ) );

      if ( v.count() >= 2 )
      {
        if ( std::isnan( v.at( 1 ) ) )
          mMaxSpinBox->clear();
        else
          mMaxSpinBox->setValue( v.at( 1 ) );
      }
      mBlockChangedSignal--;
    }
    else
    {
      mBlockChangedSignal++;
      mMinSpinBox->setValue( v.at( 0 ) );
      if ( v.count() >= 2 )
        mMaxSpinBox->setValue( v.at( 1 ) );
      mBlockChangedSignal--;
    }
  }

  if ( !mBlockChangedSignal )
    emit widgetValueHasChanged( this );
}

QVariant QgsProcessingRangeWidgetWrapper::widgetValue() const
{
  if ( mAllowingNull )
  {
    QString value;
    if ( qgsDoubleNear( mMinSpinBox->value(), mMinSpinBox->minimum() ) )
      value = QStringLiteral( "None" );
    else
      value = QString::number( mMinSpinBox->value() );

    if ( qgsDoubleNear( mMaxSpinBox->value(), mMaxSpinBox->minimum() ) )
      value += QLatin1String( ",None" );
    else
      value += QStringLiteral( ",%1" ).arg( mMaxSpinBox->value() );

    return value;
  }
  else
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

QgsProcessingAbstractParameterDefinitionWidget *QgsProcessingRangeWidgetWrapper::createParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm )
{
  return new QgsProcessingRangeParameterDefinitionWidget( context, widgetContext, definition, algorithm );
}


//
// QgsProcessingMatrixWidgetWrapper
//

QgsProcessingMatrixParameterDefinitionWidget::QgsProcessingMatrixParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm, QWidget *parent )
  : QgsProcessingAbstractParameterDefinitionWidget( context, widgetContext, definition, algorithm, parent )
{
  QVBoxLayout *vlayout = new QVBoxLayout();
  vlayout->setContentsMargins( 0, 0, 0, 0 );

  mMatrixWidget = new QgsProcessingMatrixModelerWidget();
  if ( const QgsProcessingParameterMatrix *matrixParam = dynamic_cast<const QgsProcessingParameterMatrix *>( definition ) )
  {
    mMatrixWidget->setValue( matrixParam->headers(), matrixParam->defaultValueForGui() );
    mMatrixWidget->setFixedRows( matrixParam->hasFixedNumberRows() );
  }
  vlayout->addWidget( mMatrixWidget );
  setLayout( vlayout );
}

QgsProcessingParameterDefinition *QgsProcessingMatrixParameterDefinitionWidget::createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const
{
  auto param = std::make_unique< QgsProcessingParameterMatrix >( name, description, 1, mMatrixWidget->fixedRows(), mMatrixWidget->headers(), mMatrixWidget->value() );
  param->setFlags( flags );
  return param.release();
}


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
    }
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

QgsProcessingAbstractParameterDefinitionWidget *QgsProcessingMatrixWidgetWrapper::createParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm )
{
  return new QgsProcessingMatrixParameterDefinitionWidget( context, widgetContext, definition, algorithm );
}


//
// QgsProcessingFileWidgetWrapper
//


QgsProcessingFileParameterDefinitionWidget::QgsProcessingFileParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm, QWidget *parent )
  : QgsProcessingAbstractParameterDefinitionWidget( context, widgetContext, definition, algorithm, parent )
{
  QVBoxLayout *vlayout = new QVBoxLayout();
  vlayout->setContentsMargins( 0, 0, 0, 0 );

  vlayout->addWidget( new QLabel( tr( "Type" ) ) );

  mTypeComboBox = new QComboBox();
  mTypeComboBox->addItem( tr( "File" ), QgsProcessingParameterFile::File );
  mTypeComboBox->addItem( tr( "Folder" ), QgsProcessingParameterFile::Folder );
  if ( const QgsProcessingParameterFile *fileParam = dynamic_cast<const QgsProcessingParameterFile *>( definition ) )
    mTypeComboBox->setCurrentIndex( mTypeComboBox->findData( fileParam->behavior() ) );
  else
    mTypeComboBox->setCurrentIndex( 0 );
  vlayout->addWidget( mTypeComboBox );

  vlayout->addWidget( new QLabel( tr( "File filter" ) ) );

  mFilterComboBox = new QComboBox();
  mFilterComboBox->setEditable( true );
  // add some standard ones -- these also act as a demonstration of the required format
  mFilterComboBox->addItem( tr( "All Files (*.*)" ) );
  mFilterComboBox->addItem( tr( "CSV Files (*.csv)" ) );
  mFilterComboBox->addItem( tr( "HTML Files (*.html *.htm)" ) );
  mFilterComboBox->addItem( tr( "Text Files (*.txt)" ) );
  if ( const QgsProcessingParameterFile *fileParam = dynamic_cast<const QgsProcessingParameterFile *>( definition ) )
    mFilterComboBox->setCurrentText( fileParam->fileFilter() );
  else
    mFilterComboBox->setCurrentIndex( 0 );
  vlayout->addWidget( mFilterComboBox );

  vlayout->addWidget( new QLabel( tr( "Default value" ) ) );

  mDefaultFileWidget = new QgsFileWidget();
  mDefaultFileWidget->lineEdit()->setShowClearButton( true );
  if ( const QgsProcessingParameterFile *fileParam = dynamic_cast<const QgsProcessingParameterFile *>( definition ) )
  {
    mDefaultFileWidget->setStorageMode( fileParam->behavior() == QgsProcessingParameterFile::File ? QgsFileWidget::GetFile : QgsFileWidget::GetDirectory );
    mDefaultFileWidget->setFilePath( fileParam->defaultValueForGui().toString() );
  }
  else
    mDefaultFileWidget->setStorageMode( QgsFileWidget::GetFile );
  vlayout->addWidget( mDefaultFileWidget );

  connect( mTypeComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, [ = ]
  {
    QgsProcessingParameterFile::Behavior behavior = static_cast< QgsProcessingParameterFile::Behavior >( mTypeComboBox->currentData().toInt() );
    mFilterComboBox->setEnabled( behavior == QgsProcessingParameterFile::File );
    mDefaultFileWidget->setStorageMode( behavior == QgsProcessingParameterFile::File ? QgsFileWidget::GetFile : QgsFileWidget::GetDirectory );
  } );
  mFilterComboBox->setEnabled( static_cast< QgsProcessingParameterFile::Behavior >( mTypeComboBox->currentData().toInt() ) == QgsProcessingParameterFile::File );


  setLayout( vlayout );
}

QgsProcessingParameterDefinition *QgsProcessingFileParameterDefinitionWidget::createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const
{
  auto param = std::make_unique< QgsProcessingParameterFile >( name, description );
  param->setBehavior( static_cast< QgsProcessingParameterFile::Behavior>( mTypeComboBox->currentData().toInt() ) );
  if ( param->behavior() == QgsProcessingParameterFile::File )
    param->setFileFilter( mFilterComboBox->currentText() );
  if ( !mDefaultFileWidget->filePath().isEmpty() )
    param->setDefaultValue( mDefaultFileWidget->filePath() );
  param->setFlags( flags );
  return param.release();
}


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
          if ( !fileParam->fileFilter().isEmpty() )
            mFileWidget->setFilter( fileParam->fileFilter() );
          else if ( !fileParam->extension().isEmpty() )
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
    }
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
         << QgsProcessingOutputFolder::typeName()
         << QgsProcessingOutputString::typeName()
         << QgsProcessingOutputRasterLayer::typeName()
         << QgsProcessingOutputVectorLayer::typeName()
         << QgsProcessingOutputMapLayer::typeName();
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

QgsProcessingAbstractParameterDefinitionWidget *QgsProcessingFileWidgetWrapper::createParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm )
{
  return new QgsProcessingFileParameterDefinitionWidget( context, widgetContext, definition, algorithm );
}



//
// QgsProcessingExpressionWidgetWrapper
//

QgsProcessingExpressionParameterDefinitionWidget::QgsProcessingExpressionParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm, QWidget *parent )
  : QgsProcessingAbstractParameterDefinitionWidget( context, widgetContext, definition, algorithm, parent )
{
  QVBoxLayout *vlayout = new QVBoxLayout();
  vlayout->setContentsMargins( 0, 0, 0, 0 );
  vlayout->addWidget( new QLabel( tr( "Default value" ) ) );

  mDefaultLineEdit = new QgsExpressionLineEdit();
  mDefaultLineEdit->registerExpressionContextGenerator( this );

  if ( const QgsProcessingParameterExpression *expParam = dynamic_cast<const QgsProcessingParameterExpression *>( definition ) )
    mDefaultLineEdit->setExpression( QgsProcessingParameters::parameterAsExpression( expParam, expParam->defaultValueForGui(), context ) );
  vlayout->addWidget( mDefaultLineEdit );

  vlayout->addWidget( new QLabel( tr( "Parent layer" ) ) );

  mParentLayerComboBox = new QComboBox();
  mParentLayerComboBox->addItem( tr( "None" ), QVariant() );

  QString initialParent;
  if ( const QgsProcessingParameterExpression *expParam = dynamic_cast<const QgsProcessingParameterExpression *>( definition ) )
    initialParent = expParam->parentLayerParameterName();

  if ( QgsProcessingModelAlgorithm *model = widgetContext.model() )
  {
    // populate combo box with other model input choices
    const QMap<QString, QgsProcessingModelParameter> components = model->parameterComponents();
    for ( auto it = components.constBegin(); it != components.constEnd(); ++it )
    {
      if ( const QgsProcessingParameterFeatureSource *definition = dynamic_cast< const QgsProcessingParameterFeatureSource * >( model->parameterDefinition( it.value().parameterName() ) ) )
      {
        mParentLayerComboBox-> addItem( definition->description(), definition->name() );
        if ( !initialParent.isEmpty() && initialParent == definition->name() )
        {
          mParentLayerComboBox->setCurrentIndex( mParentLayerComboBox->count() - 1 );
        }
      }
      else if ( const QgsProcessingParameterVectorLayer *definition = dynamic_cast< const QgsProcessingParameterVectorLayer * >( model->parameterDefinition( it.value().parameterName() ) ) )
      {
        mParentLayerComboBox-> addItem( definition->description(), definition->name() );
        if ( !initialParent.isEmpty() && initialParent == definition->name() )
        {
          mParentLayerComboBox->setCurrentIndex( mParentLayerComboBox->count() - 1 );
        }
      }
    }
  }

  if ( mParentLayerComboBox->count() == 1 && !initialParent.isEmpty() )
  {
    // if no parent candidates found, we just add the existing one as a placeholder
    mParentLayerComboBox->addItem( initialParent, initialParent );
    mParentLayerComboBox->setCurrentIndex( mParentLayerComboBox->count() - 1 );
  }

  vlayout->addWidget( mParentLayerComboBox );
  setLayout( vlayout );
}

QgsProcessingParameterDefinition *QgsProcessingExpressionParameterDefinitionWidget::createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const
{
  auto param = std::make_unique< QgsProcessingParameterExpression >( name, description, mDefaultLineEdit->expression(), mParentLayerComboBox->currentData().toString() );
  param->setFlags( flags );
  return param.release();
}

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
        if ( expParam->metadata().value( QStringLiteral( "inlineEditor" ) ).toBool() )
        {
          mExpBuilderWidget = new QgsExpressionBuilderWidget();
          mExpBuilderWidget->setToolTip( parameterDefinition()->toolTip() );
          mExpBuilderWidget->init( createExpressionContext() );
          connect( mExpBuilderWidget, &QgsExpressionBuilderWidget::expressionParsed, this, [ = ]( bool changed )
          {
            Q_UNUSED( changed );
            emit widgetValueHasChanged( this );
          } );
          return mExpBuilderWidget;
        }
        else
        {
          mFieldExpWidget = new QgsFieldExpressionWidget();
          mFieldExpWidget->setToolTip( parameterDefinition()->toolTip() );
          mFieldExpWidget->setExpressionDialogTitle( parameterDefinition()->description() );
          mFieldExpWidget->registerExpressionContextGenerator( this );
          if ( expParam->flags() & QgsProcessingParameterDefinition::FlagOptional )
            mFieldExpWidget->setAllowEmptyFieldName( true );

          connect( mFieldExpWidget, static_cast < void ( QgsFieldExpressionWidget::* )( const QString & ) >( &QgsFieldExpressionWidget::fieldChanged ), this, [ = ]( const QString & )
          {
            emit widgetValueHasChanged( this );
          } );
          return mFieldExpWidget;
        }
      }
    }
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

void QgsProcessingExpressionWidgetWrapper::registerProcessingContextGenerator( QgsProcessingContextGenerator *generator )
{
  QgsAbstractProcessingParameterWidgetWrapper::registerProcessingContextGenerator( generator );
  if ( mExpBuilderWidget )
  {
    // we need to regenerate the expression context for use by this widget -- it doesn't fetch automatically on demand
    mExpBuilderWidget->setExpressionContext( createExpressionContext() );
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
    tmpContext = std::make_unique< QgsProcessingContext >();
    context = tmpContext.get();
  }

  QVariant val = parentWrapper->parameterValue();
  if ( val.canConvert<QgsProcessingFeatureSourceDefinition>() )
  {
    // input is a QgsProcessingFeatureSourceDefinition - get extra properties from it
    QgsProcessingFeatureSourceDefinition fromVar = qvariant_cast<QgsProcessingFeatureSourceDefinition>( val );
    val = fromVar.source;
  }

  QgsVectorLayer *layer = QgsProcessingParameters::parameterAsVectorLayer( parentWrapper->parameterDefinition(), val, *context );
  if ( !layer )
  {
    if ( mFieldExpWidget )
      mFieldExpWidget->setLayer( nullptr );
    else if ( mExpBuilderWidget )
      mExpBuilderWidget->setLayer( nullptr );
    else if ( mExpLineEdit )
      mExpLineEdit->setLayer( nullptr );
    return;
  }

  // need to grab ownership of layer if required - otherwise layer may be deleted when context
  // goes out of scope
  std::unique_ptr< QgsMapLayer > ownedLayer( context->takeResultLayer( layer->id() ) );
  if ( ownedLayer && ownedLayer->type() == QgsMapLayerType::VectorLayer )
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
  if ( mExpBuilderWidget )
    mExpBuilderWidget->setLayer( layer );
  else if ( mExpLineEdit )
    mExpLineEdit->setLayer( layer );
}

void QgsProcessingExpressionWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext &context )
{
  const QString v = QgsProcessingParameters::parameterAsString( parameterDefinition(), value, context );
  if ( mFieldExpWidget )
    mFieldExpWidget->setExpression( v );
  else if ( mExpBuilderWidget )
    mExpBuilderWidget->setExpressionText( v );
  else if ( mExpLineEdit )
    mExpLineEdit->setExpression( v );
}

QVariant QgsProcessingExpressionWidgetWrapper::widgetValue() const
{
  if ( mFieldExpWidget )
    return mFieldExpWidget->expression();
  if ( mExpBuilderWidget )
    return mExpBuilderWidget->expressionText();
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
         << QgsProcessingParameterScale::typeName()
         << QgsProcessingParameterProviderConnection::typeName();
}

QStringList QgsProcessingExpressionWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList()
         << QgsProcessingOutputString::typeName()
         << QgsProcessingOutputNumber::typeName();
}

QString QgsProcessingExpressionWidgetWrapper::modelerExpressionFormatString() const
{
  return tr( "string representation of an expression" );
}

const QgsVectorLayer *QgsProcessingExpressionWidgetWrapper::linkedVectorLayer() const
{
  if ( mFieldExpWidget && mFieldExpWidget->layer() )
    return mFieldExpWidget->layer();

  if ( mExpBuilderWidget && mExpBuilderWidget->layer() )
    return mExpBuilderWidget->layer();

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

QgsProcessingAbstractParameterDefinitionWidget *QgsProcessingExpressionWidgetWrapper::createParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm )
{
  return new QgsProcessingExpressionParameterDefinitionWidget( context, widgetContext, definition, algorithm );
}



//
// QgsProcessingEnumPanelWidget
//

QgsProcessingEnumPanelWidget::QgsProcessingEnumPanelWidget( QWidget *parent, const QgsProcessingParameterEnum *param )
  : QWidget( parent )
  , mParam( param )
{
  QHBoxLayout *hl = new QHBoxLayout();
  hl->setContentsMargins( 0, 0, 0, 0 );

  mLineEdit = new QLineEdit();
  mLineEdit->setEnabled( false );
  hl->addWidget( mLineEdit, 1 );

  mToolButton = new QToolButton();
  mToolButton->setText( QString( QChar( 0x2026 ) ) );
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
  {
    mValue = value.type() == QVariant::List ? value.toList() : QVariantList() << value;

    if ( mParam->usesStaticStrings() && mValue.count() == 1 && mValue.at( 0 ).toString().isEmpty() )
      mValue.clear();
  }
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

  const QStringList options = mParam ? mParam->options() : QStringList();
  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );
  if ( panel && panel->dockMode() )
  {
    QgsProcessingMultipleSelectionPanelWidget *widget = new QgsProcessingMultipleSelectionPanelWidget( availableOptions, mValue );
    widget->setPanelTitle( mParam->description() );

    if ( mParam->usesStaticStrings() )
    {
      widget->setValueFormatter( [options]( const QVariant & v ) -> QString
      {
        const QString i = v.toString();
        return options.contains( i ) ? i : QString();
      } );
    }
    else
    {
      widget->setValueFormatter( [options]( const QVariant & v ) -> QString
      {
        const int i = v.toInt();
        return options.size() > i ? options.at( i ) : QString();
      } );
    }

    connect( widget, &QgsProcessingMultipleSelectionPanelWidget::selectionChanged, this, [ = ]()
    {
      setValue( widget->selectedOptions() );
    } );
    connect( widget, &QgsProcessingMultipleSelectionPanelWidget::acceptClicked, widget, &QgsPanelWidget::acceptPanel );
    panel->openPanel( widget );
  }
  else
  {
    QgsProcessingMultipleSelectionDialog dlg( availableOptions, mValue, this, Qt::WindowFlags() );

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
}

void QgsProcessingEnumPanelWidget::updateSummaryText()
{
  if ( !mParam )
    return;

  if ( mValue.empty() )
  {
    mLineEdit->setText( tr( "%1 options selected" ).arg( 0 ) );
  }
  else
  {
    QStringList values;
    values.reserve( mValue.size() );
    if ( mParam->usesStaticStrings() )
    {
      for ( const QVariant &val : std::as_const( mValue ) )
      {
        values << val.toString();
      }
    }
    else
    {
      const QStringList options = mParam->options();
      for ( const QVariant &val : std::as_const( mValue ) )
      {
        const int i = val.toInt();
        values << ( options.size() > i ? options.at( i ) : QString() );
      }
    }

    const QString concatenated = values.join( tr( "," ) );
    if ( concatenated.length() < 100 )
      mLineEdit->setText( concatenated );
    else
      mLineEdit->setText( tr( "%n option(s) selected", nullptr, mValue.count() ) );
  }
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
        value.append( mParam->usesStaticStrings() ? mParam->options().at( it.key().toInt() ) : it.key() );
    }
    return value;
  }
  else
  {
    if ( mParam->usesStaticStrings() )
      return mButtonGroup->checkedId() >= 0 ? mParam->options().at( mButtonGroup->checkedId() ) : QVariant();
    else
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
      QVariant v = mParam->usesStaticStrings() ? mParam->options().at( it.key().toInt() ) : it.key();
      it.value()->setChecked( selected.contains( v ) );
    }
  }
  else
  {
    QVariant v = value;
    if ( v.type() == QVariant::List )
      v = v.toList().value( 0 );

    v = mParam->usesStaticStrings() ? mParam->options().indexOf( v.toString() ) : v;
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

QgsProcessingEnumParameterDefinitionWidget::QgsProcessingEnumParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm, QWidget *parent )
  : QgsProcessingAbstractParameterDefinitionWidget( context, widgetContext, definition, algorithm, parent )
{
  QVBoxLayout *vlayout = new QVBoxLayout();
  vlayout->setContentsMargins( 0, 0, 0, 0 );

  mEnumWidget = new QgsProcessingEnumModelerWidget();
  if ( const QgsProcessingParameterEnum *enumParam = dynamic_cast<const QgsProcessingParameterEnum *>( definition ) )
  {
    mEnumWidget->setAllowMultiple( enumParam->allowMultiple() );
    mEnumWidget->setOptions( enumParam->options() );
    mEnumWidget->setDefaultOptions( enumParam->defaultValueForGui() );
  }
  vlayout->addWidget( mEnumWidget );
  setLayout( vlayout );
}

QgsProcessingParameterDefinition *QgsProcessingEnumParameterDefinitionWidget::createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const
{
  auto param = std::make_unique< QgsProcessingParameterEnum >( name, description, mEnumWidget->options(), mEnumWidget->allowMultiple(), mEnumWidget->defaultOptions() );
  param->setFlags( flags );
  return param.release();
}


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
        const QVariantList iconList = expParam->metadata().value( QStringLiteral( "widget_wrapper" ) ).toMap().value( QStringLiteral( "icons" ) ).toList();
        for ( int i = 0; i < options.count(); ++i )
        {
          const QIcon icon = iconList.value( i ).value< QIcon >();

          if ( expParam->usesStaticStrings() )
            mComboBox->addItem( icon, options.at( i ), options.at( i ) );
          else
            mComboBox->addItem( icon, options.at( i ), i );
        }

        mComboBox->setToolTip( parameterDefinition()->toolTip() );
        connect( mComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, [ = ]( int )
        {
          emit widgetValueHasChanged( this );
        } );
        return mComboBox;
      }
    }
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
      const QgsProcessingParameterEnum *enumDef = dynamic_cast< const QgsProcessingParameterEnum *>( parameterDefinition() );
      if ( enumDef->usesStaticStrings() )
      {
        const QString v = QgsProcessingParameters::parameterAsEnumString( parameterDefinition(), value, context );
        mComboBox->setCurrentIndex( mComboBox->findData( v ) );
      }
      else
      {
        const int v = QgsProcessingParameters::parameterAsEnum( parameterDefinition(), value, context );
        mComboBox->setCurrentIndex( mComboBox->findData( v ) );
      }
    }
  }
  else if ( mPanel || mCheckboxPanel )
  {
    QVariantList opts;
    if ( value.isValid() )
    {
      const QgsProcessingParameterEnum *enumDef = dynamic_cast< const QgsProcessingParameterEnum *>( parameterDefinition() );
      if ( enumDef->usesStaticStrings() )
      {
        const QStringList v = QgsProcessingParameters::parameterAsEnumStrings( parameterDefinition(), value, context );
        opts.reserve( v.size() );
        for ( QString i : v )
          opts << i;
      }
      else
      {
        const QList< int > v = QgsProcessingParameters::parameterAsEnums( parameterDefinition(), value, context );
        opts.reserve( v.size() );
        for ( int i : v )
          opts << i;
      }
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

QgsProcessingAbstractParameterDefinitionWidget *QgsProcessingEnumWidgetWrapper::createParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm )
{
  return new QgsProcessingEnumParameterDefinitionWidget( context, widgetContext, definition, algorithm );
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
      mPlainComboBox = new QComboBox();
      mPlainComboBox->setEditable( true );
      mPlainComboBox->setToolTip( tr( "Name of an existing print layout" ) );
      if ( widgetContext().project() )
      {
        const QList< QgsPrintLayout * > layouts = widgetContext().project()->layoutManager()->printLayouts();
        for ( const QgsPrintLayout *layout : layouts )
          mPlainComboBox->addItem( layout->name() );
      }

      connect( mPlainComboBox, &QComboBox::currentTextChanged, this, [ = ]( const QString & )
      {
        emit widgetValueHasChanged( this );
      } );
      return mPlainComboBox;
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
  else if ( mPlainComboBox )
  {
    const QString v = QgsProcessingParameters::parameterAsString( parameterDefinition(), value, context );
    mPlainComboBox->setCurrentText( v );
  }
}

QVariant QgsProcessingLayoutWidgetWrapper::widgetValue() const
{
  if ( mComboBox )
  {
    const QgsMasterLayoutInterface *l = mComboBox->currentLayout();
    return l ? l->name() : QVariant();
  }
  else if ( mPlainComboBox )
    return mPlainComboBox->currentText().isEmpty() ? QVariant() : mPlainComboBox->currentText();
  else
    return QVariant();
}

void QgsProcessingLayoutWidgetWrapper::setWidgetContext( const QgsProcessingParameterWidgetContext &context )
{
  QgsAbstractProcessingParameterWidgetWrapper::setWidgetContext( context );
  if ( mPlainComboBox && context.project() )
  {
    const QList< QgsPrintLayout * > layouts = widgetContext().project()->layoutManager()->printLayouts();
    for ( const QgsPrintLayout *layout : layouts )
      mPlainComboBox->addItem( layout->name() );
  }
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


QgsProcessingLayoutItemParameterDefinitionWidget::QgsProcessingLayoutItemParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm, QWidget *parent )
  : QgsProcessingAbstractParameterDefinitionWidget( context, widgetContext, definition, algorithm, parent )
{
  QVBoxLayout *vlayout = new QVBoxLayout();
  vlayout->setContentsMargins( 0, 0, 0, 0 );

  vlayout->addWidget( new QLabel( tr( "Parent layout" ) ) );

  mParentLayoutComboBox = new QComboBox();
  QString initialParent;
  if ( const QgsProcessingParameterLayoutItem *itemParam = dynamic_cast<const QgsProcessingParameterLayoutItem *>( definition ) )
    initialParent = itemParam->parentLayoutParameterName();

  if ( auto *lModel = widgetContext.model() )
  {
    // populate combo box with other model input choices
    const QMap<QString, QgsProcessingModelParameter> components = lModel->parameterComponents();
    for ( auto it = components.constBegin(); it != components.constEnd(); ++it )
    {
      if ( const QgsProcessingParameterLayout *definition = dynamic_cast< const QgsProcessingParameterLayout * >( lModel->parameterDefinition( it.value().parameterName() ) ) )
      {
        mParentLayoutComboBox-> addItem( definition->description(), definition->name() );
        if ( !initialParent.isEmpty() && initialParent == definition->name() )
        {
          mParentLayoutComboBox->setCurrentIndex( mParentLayoutComboBox->count() - 1 );
        }
      }
    }
  }

  if ( mParentLayoutComboBox->count() == 0 && !initialParent.isEmpty() )
  {
    // if no parent candidates found, we just add the existing one as a placeholder
    mParentLayoutComboBox->addItem( initialParent, initialParent );
    mParentLayoutComboBox->setCurrentIndex( mParentLayoutComboBox->count() - 1 );
  }

  vlayout->addWidget( mParentLayoutComboBox );
  setLayout( vlayout );
}
QgsProcessingParameterDefinition *QgsProcessingLayoutItemParameterDefinitionWidget::createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const
{
  auto param = std::make_unique< QgsProcessingParameterLayoutItem >( name, description, QVariant(), mParentLayoutComboBox->currentData().toString() );
  param->setFlags( flags );
  return param.release();
}


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
  QgsAbstractProcessingParameterWidgetWrapper::postInitialize( wrappers );
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
    tmpContext = std::make_unique< QgsProcessingContext >();
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

QgsProcessingAbstractParameterDefinitionWidget *QgsProcessingLayoutItemWidgetWrapper::createParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm )
{
  return new QgsProcessingLayoutItemParameterDefinitionWidget( context, widgetContext, definition, algorithm );
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
  mLineEdit = new QgsFilterLineEdit( );
  mLineEdit->setShowClearButton( false );
  l->addWidget( mLineEdit, 1 );
  mButton = new QToolButton();
  mButton->setText( QString( QChar( 0x2026 ) ) );
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
  mTool = std::make_unique< QgsProcessingPointMapTool >( mCanvas );
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
  QString newText = QStringLiteral( "%1,%2" )
                    .arg( QString::number( point.x(), 'f' ),
                          QString::number( point.y(), 'f' ) );

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

QgsProcessingPointParameterDefinitionWidget::QgsProcessingPointParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm, QWidget *parent )
  : QgsProcessingAbstractParameterDefinitionWidget( context, widgetContext, definition, algorithm, parent )
{
  QVBoxLayout *vlayout = new QVBoxLayout();
  vlayout->setContentsMargins( 0, 0, 0, 0 );

  vlayout->addWidget( new QLabel( tr( "Default value" ) ) );

  mDefaultLineEdit = new QLineEdit();
  mDefaultLineEdit->setToolTip( tr( "Point as 'x,y'" ) );
  mDefaultLineEdit->setPlaceholderText( tr( "Point as 'x,y'" ) );
  if ( const QgsProcessingParameterPoint *pointParam = dynamic_cast<const QgsProcessingParameterPoint *>( definition ) )
  {
    QgsPointXY point = QgsProcessingParameters::parameterAsPoint( pointParam, pointParam->defaultValueForGui(), context );
    mDefaultLineEdit->setText( QStringLiteral( "%1,%2" ).arg( QString::number( point.x(), 'f' ), QString::number( point.y(), 'f' ) ) );
  }

  vlayout->addWidget( mDefaultLineEdit );
  setLayout( vlayout );
}

QgsProcessingParameterDefinition *QgsProcessingPointParameterDefinitionWidget::createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const
{
  auto param = std::make_unique< QgsProcessingParameterPoint >( name, description, mDefaultLineEdit->text() );
  param->setFlags( flags );
  return param.release();
}

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

QString QgsProcessingPointWidgetWrapper::modelerExpressionFormatString() const
{
  return tr( "string of the format 'x,y' or a geometry value (centroid is used)" );
}

QString QgsProcessingPointWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterPoint::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingPointWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingPointWidgetWrapper( parameter, type );
}

QgsProcessingAbstractParameterDefinitionWidget *QgsProcessingPointWidgetWrapper::createParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm )
{
  return new QgsProcessingPointParameterDefinitionWidget( context, widgetContext, definition, algorithm );
}


//
// QgsProcessingGeometryWidgetWrapper
//


QgsProcessingGeometryParameterDefinitionWidget::QgsProcessingGeometryParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm, QWidget *parent )
  : QgsProcessingAbstractParameterDefinitionWidget( context, widgetContext, definition, algorithm, parent )
{
  QVBoxLayout *vlayout = new QVBoxLayout();
  vlayout->setContentsMargins( 0, 0, 0, 0 );

  vlayout->addWidget( new QLabel( tr( "Default value" ) ) );

  mDefaultLineEdit = new QLineEdit();
  mDefaultLineEdit->setToolTip( tr( "Geometry as WKT" ) );
  mDefaultLineEdit->setPlaceholderText( tr( "Geometry as WKT" ) );
  if ( const QgsProcessingParameterGeometry *geometryParam = dynamic_cast<const QgsProcessingParameterGeometry *>( definition ) )
  {
    QgsGeometry g = QgsProcessingParameters::parameterAsGeometry( geometryParam, geometryParam->defaultValueForGui(), context );
    if ( !g.isNull() )
      mDefaultLineEdit->setText( g.asWkt() );
  }

  vlayout->addWidget( mDefaultLineEdit );
  setLayout( vlayout );
}

QgsProcessingParameterDefinition *QgsProcessingGeometryParameterDefinitionWidget::createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const
{
  auto param = std::make_unique< QgsProcessingParameterGeometry >( name, description, mDefaultLineEdit->text() );
  param->setFlags( flags );
  return param.release();
}

QgsProcessingGeometryWidgetWrapper::QgsProcessingGeometryWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{

}

QWidget *QgsProcessingGeometryWidgetWrapper::createWidget()
{
  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    case QgsProcessingGui::Modeler:
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

void QgsProcessingGeometryWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext &context )
{
  if ( mLineEdit )
  {
    QgsGeometry g = QgsProcessingParameters::parameterAsGeometry( parameterDefinition(), value, context );
    if ( !g.isNull() )
      mLineEdit->setText( g.asWkt() );
    else
      mLineEdit->clear();
  }
}

QVariant QgsProcessingGeometryWidgetWrapper::widgetValue() const
{
  if ( mLineEdit )
    return mLineEdit->text().isEmpty() ? QVariant() : mLineEdit->text();
  else
    return QVariant();
}

QStringList QgsProcessingGeometryWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList()
         << QgsProcessingParameterGeometry::typeName()
         << QgsProcessingParameterString::typeName()
         << QgsProcessingParameterPoint::typeName()
         << QgsProcessingParameterExtent::typeName();
}

QStringList QgsProcessingGeometryWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList()
         << QgsProcessingOutputString::typeName();
}

QString QgsProcessingGeometryWidgetWrapper::modelerExpressionFormatString() const
{
  return tr( "string in the Well-Known-Text format or a geometry value" );
}

QString QgsProcessingGeometryWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterGeometry::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingGeometryWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingGeometryWidgetWrapper( parameter, type );
}

QgsProcessingAbstractParameterDefinitionWidget *QgsProcessingGeometryWidgetWrapper::createParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm )
{
  return new QgsProcessingGeometryParameterDefinitionWidget( context, widgetContext, definition, algorithm );
}


//
// QgsProcessingColorWidgetWrapper
//


QgsProcessingColorParameterDefinitionWidget::QgsProcessingColorParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm, QWidget *parent )
  : QgsProcessingAbstractParameterDefinitionWidget( context, widgetContext, definition, algorithm, parent )
{
  QVBoxLayout *vlayout = new QVBoxLayout();
  vlayout->setContentsMargins( 0, 0, 0, 0 );

  vlayout->addWidget( new QLabel( tr( "Default value" ) ) );

  mDefaultColorButton = new QgsColorButton();
  mDefaultColorButton->setShowNull( true );
  mAllowOpacity = new QCheckBox( tr( "Allow opacity control" ) );

  if ( const QgsProcessingParameterColor *colorParam = dynamic_cast<const QgsProcessingParameterColor *>( definition ) )
  {
    const QColor c = QgsProcessingParameters::parameterAsColor( colorParam, colorParam->defaultValueForGui(), context );
    if ( !c.isValid() )
      mDefaultColorButton->setToNull();
    else
      mDefaultColorButton->setColor( c );
    mAllowOpacity->setChecked( colorParam->opacityEnabled() );
  }
  else
  {
    mDefaultColorButton->setToNull();
    mAllowOpacity->setChecked( true );
  }

  vlayout->addWidget( mDefaultColorButton );
  vlayout->addWidget( mAllowOpacity );
  setLayout( vlayout );
}

QgsProcessingParameterDefinition *QgsProcessingColorParameterDefinitionWidget::createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const
{
  auto param = std::make_unique< QgsProcessingParameterColor >( name, description, mDefaultColorButton->color(), mAllowOpacity->isChecked() );
  param->setFlags( flags );
  return param.release();
}

QgsProcessingColorWidgetWrapper::QgsProcessingColorWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{

}

QWidget *QgsProcessingColorWidgetWrapper::createWidget()
{
  const QgsProcessingParameterColor *colorParam = dynamic_cast< const QgsProcessingParameterColor *>( parameterDefinition() );
  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    case QgsProcessingGui::Batch:
    case QgsProcessingGui::Modeler:
    {
      mColorButton = new QgsColorButton( nullptr );
      mColorButton->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );

      if ( colorParam->flags() & QgsProcessingParameterDefinition::FlagOptional )
        mColorButton->setShowNull( true );

      mColorButton->setAllowOpacity( colorParam->opacityEnabled() );
      mColorButton->setToolTip( parameterDefinition()->toolTip() );
      mColorButton->setColorDialogTitle( parameterDefinition()->description() );
      if ( colorParam->defaultValueForGui().value< QColor >().isValid() )
      {
        mColorButton->setDefaultColor( colorParam->defaultValueForGui().value< QColor >() );
      }

      connect( mColorButton, &QgsColorButton::colorChanged, this, [ = ]
      {
        emit widgetValueHasChanged( this );
      } );

      return mColorButton;
    }
  }
  return nullptr;
}

void QgsProcessingColorWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext &context )
{
  if ( mColorButton )
  {
    if ( !value.isValid() ||
         ( value.type() == QVariant::String && value.toString().isEmpty() )
         || ( value.type() == QVariant::Color && !value.value< QColor >().isValid() ) )
      mColorButton->setToNull();
    else
    {
      const QColor c = QgsProcessingParameters::parameterAsColor( parameterDefinition(), value, context );
      if ( !c.isValid() && mColorButton->showNull() )
        mColorButton->setToNull();
      else
        mColorButton->setColor( c );
    }
  }
}

QVariant QgsProcessingColorWidgetWrapper::widgetValue() const
{
  if ( mColorButton )
    return mColorButton->isNull() ? QVariant() : mColorButton->color();
  else
    return QVariant();
}

QStringList QgsProcessingColorWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList()
         << QgsProcessingParameterColor::typeName()
         << QgsProcessingParameterString::typeName();
}

QStringList QgsProcessingColorWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList()
         << QgsProcessingOutputString::typeName();
}

QString QgsProcessingColorWidgetWrapper::modelerExpressionFormatString() const
{
  return tr( "color style string, e.g. #ff0000 or 255,0,0" );
}

QString QgsProcessingColorWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterColor::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingColorWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingColorWidgetWrapper( parameter, type );
}

QgsProcessingAbstractParameterDefinitionWidget *QgsProcessingColorWidgetWrapper::createParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm )
{
  return new QgsProcessingColorParameterDefinitionWidget( context, widgetContext, definition, algorithm );
}


//
// QgsProcessingCoordinateOperationWidgetWrapper
//

QgsProcessingCoordinateOperationParameterDefinitionWidget::QgsProcessingCoordinateOperationParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm, QWidget *parent )
  : QgsProcessingAbstractParameterDefinitionWidget( context, widgetContext, definition, algorithm, parent )
{
  QVBoxLayout *vlayout = new QVBoxLayout();
  vlayout->setContentsMargins( 0, 0, 0, 0 );

  vlayout->addWidget( new QLabel( tr( "Default value" ) ) );

  mDefaultLineEdit = new QLineEdit();
  if ( const QgsProcessingParameterCoordinateOperation *coordParam = dynamic_cast<const QgsProcessingParameterCoordinateOperation *>( definition ) )
    mDefaultLineEdit->setText( QgsProcessingParameters::parameterAsString( coordParam, coordParam->defaultValueForGui(), context ) );
  vlayout->addWidget( mDefaultLineEdit );

  mSourceParamComboBox = new QComboBox();
  mDestParamComboBox = new QComboBox();
  QString initialSource;
  QString initialDest;
  QgsCoordinateReferenceSystem sourceCrs;
  QgsCoordinateReferenceSystem destCrs;
  if ( const QgsProcessingParameterCoordinateOperation *itemParam = dynamic_cast<const QgsProcessingParameterCoordinateOperation *>( definition ) )
  {
    initialSource = itemParam->sourceCrsParameterName();
    initialDest = itemParam->destinationCrsParameterName();
    sourceCrs = QgsProcessingUtils::variantToCrs( itemParam->sourceCrs(), context );
    destCrs = QgsProcessingUtils::variantToCrs( itemParam->destinationCrs(), context );
  }

  mSourceParamComboBox->addItem( QString(), QString() );
  mDestParamComboBox->addItem( QString(), QString() );
  if ( auto *lModel = widgetContext.model() )
  {
    // populate combo box with other model input choices
    const QMap<QString, QgsProcessingModelParameter> components = lModel->parameterComponents();
    for ( auto it = components.constBegin(); it != components.constEnd(); ++it )
    {
      if ( definition && it->parameterName() == definition->name() )
        continue;

      // TODO - we should probably filter this list?
      mSourceParamComboBox->addItem( it->parameterName(), it->parameterName() );
      mDestParamComboBox->addItem( it->parameterName(), it->parameterName() );
      if ( !initialSource.isEmpty() && initialSource == it->parameterName() )
      {
        mSourceParamComboBox->setCurrentIndex( mSourceParamComboBox->count() - 1 );
      }
      if ( !initialDest.isEmpty() && initialDest == it->parameterName() )
      {
        mDestParamComboBox->setCurrentIndex( mDestParamComboBox->count() - 1 );
      }
    }
  }

  if ( mSourceParamComboBox->count() == 1 && !initialSource.isEmpty() )
  {
    // if no source candidates found, we just add the existing one as a placeholder
    mSourceParamComboBox->addItem( initialSource, initialSource );
    mSourceParamComboBox->setCurrentIndex( mSourceParamComboBox->count() - 1 );
  }
  if ( mDestParamComboBox->count() == 1 && !initialDest.isEmpty() )
  {
    // if no dest candidates found, we just add the existing one as a placeholder
    mDestParamComboBox->addItem( initialDest, initialDest );
    mDestParamComboBox->setCurrentIndex( mDestParamComboBox->count() - 1 );
  }

  vlayout->addWidget( new QLabel( tr( "Source CRS parameter" ) ) );
  vlayout->addWidget( mSourceParamComboBox );
  vlayout->addWidget( new QLabel( tr( "Destination CRS parameter" ) ) );
  vlayout->addWidget( mDestParamComboBox );

  mStaticSourceWidget = new QgsProjectionSelectionWidget();
  mStaticSourceWidget->setOptionVisible( QgsProjectionSelectionWidget::CrsNotSet, true );
  mStaticSourceWidget->setCrs( sourceCrs );
  mStaticDestWidget = new QgsProjectionSelectionWidget();
  mStaticDestWidget->setOptionVisible( QgsProjectionSelectionWidget::CrsNotSet, true );
  mStaticDestWidget->setCrs( destCrs );

  vlayout->addWidget( new QLabel( tr( "Static source CRS" ) ) );
  vlayout->addWidget( mStaticSourceWidget );
  vlayout->addWidget( new QLabel( tr( "Static destination CRS" ) ) );
  vlayout->addWidget( mStaticDestWidget );

  setLayout( vlayout );
}

QgsProcessingParameterDefinition *QgsProcessingCoordinateOperationParameterDefinitionWidget::createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const
{
  auto param = std::make_unique< QgsProcessingParameterCoordinateOperation >( name, description, mDefaultLineEdit->text(),
               mSourceParamComboBox->currentText(),
               mDestParamComboBox->currentText(),
               mStaticSourceWidget->crs().isValid() ? QVariant::fromValue( mStaticSourceWidget->crs() ) : QVariant(),
               mStaticDestWidget->crs().isValid() ? QVariant::fromValue( mStaticDestWidget->crs() ) : QVariant() );
  param->setFlags( flags );
  return param.release();
}

QgsProcessingCoordinateOperationWidgetWrapper::QgsProcessingCoordinateOperationWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{

}

QWidget *QgsProcessingCoordinateOperationWidgetWrapper::createWidget()
{
  const QgsProcessingParameterCoordinateOperation *coordParam = dynamic_cast< const QgsProcessingParameterCoordinateOperation *>( parameterDefinition() );
  QgsProcessingContext c;
  mSourceCrs = QgsProcessingUtils::variantToCrs( coordParam->sourceCrs(), c );
  mDestCrs = QgsProcessingUtils::variantToCrs( coordParam->destinationCrs(), c );
  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    {
      mOperationWidget = new QgsCoordinateOperationWidget( nullptr );
      mOperationWidget->setShowMakeDefault( false );
      mOperationWidget->setShowFallbackOption( false );
      mOperationWidget->setToolTip( parameterDefinition()->toolTip() );
      mOperationWidget->setSourceCrs( mSourceCrs );
      mOperationWidget->setDestinationCrs( mDestCrs );
      mOperationWidget->setMapCanvas( mCanvas );
      if ( !coordParam->defaultValueForGui().toString().isEmpty() )
      {
        QgsCoordinateOperationWidget::OperationDetails deets;
        deets.proj = coordParam->defaultValueForGui().toString();
        mOperationWidget->setSelectedOperation( deets );
      }

      connect( mOperationWidget, &QgsCoordinateOperationWidget::operationChanged, this, [ = ]
      {
        emit widgetValueHasChanged( this );
      } );

      return mOperationWidget;
    }

    case QgsProcessingGui::Batch:
    case QgsProcessingGui::Modeler:
    {
      mLineEdit = new QLineEdit();
      QHBoxLayout *layout = new QHBoxLayout();
      layout->addWidget( mLineEdit, 1 );
      connect( mLineEdit, &QLineEdit::textChanged, this, [ = ]
      {
        emit widgetValueHasChanged( this );
      } );

      QToolButton *button = new QToolButton();
      button->setText( QString( QChar( 0x2026 ) ) );
      connect( button, &QToolButton::clicked, this, [ = ]
      {
        QgsDatumTransformDialog dlg( mSourceCrs, mDestCrs, false, false, false, qMakePair( -1, -1 ), button, Qt::WindowFlags(), mLineEdit->text(), mCanvas );
        if ( dlg.exec() )
        {
          mLineEdit->setText( dlg.selectedDatumTransform().proj );
          emit widgetValueHasChanged( this );
        }
      } );
      layout->addWidget( button );

      QWidget *w = new QWidget();
      layout->setContentsMargins( 0, 0, 0, 0 );
      w->setLayout( layout );
      return w;
    }

  }
  return nullptr;
}

void QgsProcessingCoordinateOperationWidgetWrapper::postInitialize( const QList<QgsAbstractProcessingParameterWidgetWrapper *> &wrappers )
{
  QgsAbstractProcessingParameterWidgetWrapper::postInitialize( wrappers );
  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    case QgsProcessingGui::Batch:
    {
      for ( const QgsAbstractProcessingParameterWidgetWrapper *wrapper : wrappers )
      {
        if ( wrapper->parameterDefinition()->name() == static_cast< const QgsProcessingParameterCoordinateOperation * >( parameterDefinition() )->sourceCrsParameterName() )
        {
          setSourceCrsParameterValue( wrapper->parameterValue() );
          connect( wrapper, &QgsAbstractProcessingParameterWidgetWrapper::widgetValueHasChanged, this, [ = ]
          {
            setSourceCrsParameterValue( wrapper->parameterValue() );
          } );
        }
        if ( wrapper->parameterDefinition()->name() == static_cast< const QgsProcessingParameterCoordinateOperation * >( parameterDefinition() )->destinationCrsParameterName() )
        {
          setDestinationCrsParameterValue( wrapper->parameterValue() );
          connect( wrapper, &QgsAbstractProcessingParameterWidgetWrapper::widgetValueHasChanged, this, [ = ]
          {
            setDestinationCrsParameterValue( wrapper->parameterValue() );
          } );
        }
      }
      break;
    }

    case QgsProcessingGui::Modeler:
      break;
  }
}

void QgsProcessingCoordinateOperationWidgetWrapper::setWidgetContext( const QgsProcessingParameterWidgetContext &context )
{
  mCanvas = context.mapCanvas();
  if ( mOperationWidget )
    mOperationWidget->setMapCanvas( context.mapCanvas() );
}

void QgsProcessingCoordinateOperationWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext & )
{
  if ( mOperationWidget )
  {
    if ( !value.isValid() ||
         ( value.type() == QVariant::String ) )
    {
      QgsCoordinateOperationWidget::OperationDetails deets;
      deets.proj = value.toString();
      mOperationWidget->setSelectedOperation( deets );
    }
  }
  if ( mLineEdit )
  {
    if ( !value.isValid() ||
         ( value.type() == QVariant::String ) )
    {
      mLineEdit->setText( value.toString() );
    }
  }
}

QVariant QgsProcessingCoordinateOperationWidgetWrapper::widgetValue() const
{
  if ( mOperationWidget )
    return mOperationWidget->selectedOperation().proj;
  else if ( mLineEdit )
    return mLineEdit->text();
  else
    return QVariant();
}

QStringList QgsProcessingCoordinateOperationWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList()
         << QgsProcessingParameterCoordinateOperation::typeName()
         << QgsProcessingParameterString::typeName();
}

QStringList QgsProcessingCoordinateOperationWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList()
         << QgsProcessingOutputString::typeName();
}

QString QgsProcessingCoordinateOperationWidgetWrapper::modelerExpressionFormatString() const
{
  return tr( "Proj coordinate operation string, e.g. '+proj=pipeline +step +inv...'" );
}

void QgsProcessingCoordinateOperationWidgetWrapper::setSourceCrsParameterValue( const QVariant &value )
{
  QgsProcessingContext *context = nullptr;
  std::unique_ptr< QgsProcessingContext > tmpContext;
  if ( mProcessingContextGenerator )
    context = mProcessingContextGenerator->processingContext();

  if ( !context )
  {
    tmpContext = std::make_unique< QgsProcessingContext >();
    context = tmpContext.get();
  }

  mSourceCrs = QgsProcessingUtils::variantToCrs( value, *context );
  if ( mOperationWidget )
  {
    mOperationWidget->setSourceCrs( mSourceCrs );
    mOperationWidget->setSelectedOperationUsingContext( context->transformContext() );
  }
}

void QgsProcessingCoordinateOperationWidgetWrapper::setDestinationCrsParameterValue( const QVariant &value )
{
  QgsProcessingContext *context = nullptr;
  std::unique_ptr< QgsProcessingContext > tmpContext;
  if ( mProcessingContextGenerator )
    context = mProcessingContextGenerator->processingContext();

  if ( !context )
  {
    tmpContext = std::make_unique< QgsProcessingContext >();
    context = tmpContext.get();
  }

  mDestCrs = QgsProcessingUtils::variantToCrs( value, *context );
  if ( mOperationWidget )
  {
    mOperationWidget->setDestinationCrs( mDestCrs );
    mOperationWidget->setSelectedOperationUsingContext( context->transformContext() );
  }
}

QString QgsProcessingCoordinateOperationWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterCoordinateOperation::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingCoordinateOperationWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingCoordinateOperationWidgetWrapper( parameter, type );
}

QgsProcessingAbstractParameterDefinitionWidget *QgsProcessingCoordinateOperationWidgetWrapper::createParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm )
{
  return new QgsProcessingCoordinateOperationParameterDefinitionWidget( context, widgetContext, definition, algorithm );
}



//
// QgsProcessingFieldPanelWidget
//

QgsProcessingFieldPanelWidget::QgsProcessingFieldPanelWidget( QWidget *parent, const QgsProcessingParameterField *param )
  : QWidget( parent )
  , mParam( param )
{
  QHBoxLayout *hl = new QHBoxLayout();
  hl->setContentsMargins( 0, 0, 0, 0 );

  mLineEdit = new QLineEdit();
  mLineEdit->setEnabled( false );
  hl->addWidget( mLineEdit, 1 );

  mToolButton = new QToolButton();
  mToolButton->setText( QString( QChar( 0x2026 ) ) );
  hl->addWidget( mToolButton );

  setLayout( hl );

  if ( mParam )
  {
    mLineEdit->setText( tr( "%n field(s) selected", nullptr, 0 ) );
  }

  connect( mToolButton, &QToolButton::clicked, this, &QgsProcessingFieldPanelWidget::showDialog );
}

void QgsProcessingFieldPanelWidget::setFields( const QgsFields &fields )
{
  mFields = fields;
}

void QgsProcessingFieldPanelWidget::setValue( const QVariant &value )
{
  if ( value.isValid() )
    mValue = value.type() == QVariant::List ? value.toList() : QVariantList() << value;
  else
    mValue.clear();

  updateSummaryText();
  emit changed();
}

void QgsProcessingFieldPanelWidget::showDialog()
{
  QVariantList availableOptions;
  QStringList fieldNames;
  availableOptions.reserve( mFields.size() );
  for ( const QgsField &field : std::as_const( mFields ) )
  {
    availableOptions << field.name();
  }

  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );
  if ( panel && panel->dockMode() )
  {
    QgsProcessingMultipleSelectionPanelWidget *widget = new QgsProcessingMultipleSelectionPanelWidget( availableOptions, mValue );
    widget->setPanelTitle( mParam->description() );

    widget->setValueFormatter( []( const QVariant & v ) -> QString
    {
      return v.toString();
    } );

    connect( widget, &QgsProcessingMultipleSelectionPanelWidget::selectionChanged, this, [ = ]()
    {
      setValue( widget->selectedOptions() );
    } );
    connect( widget, &QgsProcessingMultipleSelectionPanelWidget::acceptClicked, widget, &QgsPanelWidget::acceptPanel );
    panel->openPanel( widget );
  }
  else
  {
    QgsProcessingMultipleSelectionDialog dlg( availableOptions, mValue, this, Qt::WindowFlags() );

    dlg.setValueFormatter( []( const QVariant & v ) -> QString
    {
      return v.toString();
    } );
    if ( dlg.exec() )
    {
      setValue( dlg.selectedOptions() );
    }
  }
}

void QgsProcessingFieldPanelWidget::updateSummaryText()
{
  if ( !mParam )
    return;

  if ( mValue.empty() )
  {
    mLineEdit->setText( tr( "%n field(s) selected", nullptr, 0 ) );
  }
  else
  {
    QStringList values;
    values.reserve( mValue.size() );
    for ( const QVariant &val : std::as_const( mValue ) )
    {
      values << val.toString();
    }

    const QString concatenated = values.join( tr( "," ) );
    if ( concatenated.length() < 100 )
      mLineEdit->setText( concatenated );
    else
      mLineEdit->setText( tr( "%n field(s) selected", nullptr, mValue.count() ) );
  }
}


//
// QgsProcessingFieldWidgetWrapper
//

QgsProcessingFieldParameterDefinitionWidget::QgsProcessingFieldParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm, QWidget *parent )
  : QgsProcessingAbstractParameterDefinitionWidget( context, widgetContext, definition, algorithm, parent )
{
  QVBoxLayout *vlayout = new QVBoxLayout();
  vlayout->setContentsMargins( 0, 0, 0, 0 );

  vlayout->addWidget( new QLabel( tr( "Parent layer" ) ) );
  mParentLayerComboBox = new QComboBox();

  QString initialParent;
  if ( const QgsProcessingParameterField *fieldParam = dynamic_cast<const QgsProcessingParameterField *>( definition ) )
    initialParent = fieldParam->parentLayerParameterName();

  if ( auto *lModel = widgetContext.model() )
  {
    // populate combo box with other model input choices
    const QMap<QString, QgsProcessingModelParameter> components = lModel->parameterComponents();
    for ( auto it = components.constBegin(); it != components.constEnd(); ++it )
    {
      if ( const QgsProcessingParameterFeatureSource *definition = dynamic_cast< const QgsProcessingParameterFeatureSource * >( lModel->parameterDefinition( it.value().parameterName() ) ) )
      {
        mParentLayerComboBox-> addItem( definition->description(), definition->name() );
        if ( !initialParent.isEmpty() && initialParent == definition->name() )
        {
          mParentLayerComboBox->setCurrentIndex( mParentLayerComboBox->count() - 1 );
        }
      }
      else if ( const QgsProcessingParameterVectorLayer *definition = dynamic_cast< const QgsProcessingParameterVectorLayer * >( lModel->parameterDefinition( it.value().parameterName() ) ) )
      {
        mParentLayerComboBox-> addItem( definition->description(), definition->name() );
        if ( !initialParent.isEmpty() && initialParent == definition->name() )
        {
          mParentLayerComboBox->setCurrentIndex( mParentLayerComboBox->count() - 1 );
        }
      }
      else if ( const QgsProcessingParameterMultipleLayers *definition = dynamic_cast< const QgsProcessingParameterMultipleLayers * >( lModel->parameterDefinition( it.value().parameterName() ) ) )
      {
        if ( definition->layerType() == QgsProcessing::TypeVector )
        {
          mParentLayerComboBox-> addItem( definition->description(), definition->name() );
          if ( !initialParent.isEmpty() && initialParent == definition->name() )
          {
            mParentLayerComboBox->setCurrentIndex( mParentLayerComboBox->count() - 1 );
          }
        }
      }
    }
  }

  if ( mParentLayerComboBox->count() == 0 && !initialParent.isEmpty() )
  {
    // if no parent candidates found, we just add the existing one as a placeholder
    mParentLayerComboBox->addItem( initialParent, initialParent );
    mParentLayerComboBox->setCurrentIndex( mParentLayerComboBox->count() - 1 );
  }

  vlayout->addWidget( mParentLayerComboBox );

  vlayout->addWidget( new QLabel( tr( "Allowed data type" ) ) );
  mDataTypeComboBox = new QComboBox();
  mDataTypeComboBox->addItem( tr( "Any" ), QgsProcessingParameterField::Any );
  mDataTypeComboBox->addItem( tr( "Number" ), QgsProcessingParameterField::Numeric );
  mDataTypeComboBox->addItem( tr( "String" ), QgsProcessingParameterField::String );
  mDataTypeComboBox->addItem( tr( "Date/time" ), QgsProcessingParameterField::DateTime );
  if ( const QgsProcessingParameterField *fieldParam = dynamic_cast<const QgsProcessingParameterField *>( definition ) )
    mDataTypeComboBox->setCurrentIndex( mDataTypeComboBox->findData( fieldParam->dataType() ) );

  vlayout->addWidget( mDataTypeComboBox );

  mAllowMultipleCheckBox = new QCheckBox( tr( "Accept multiple fields" ) );
  if ( const QgsProcessingParameterField *fieldParam = dynamic_cast<const QgsProcessingParameterField *>( definition ) )
    mAllowMultipleCheckBox->setChecked( fieldParam->allowMultiple() );

  vlayout->addWidget( mAllowMultipleCheckBox );

  mDefaultToAllCheckBox = new QCheckBox( tr( "Select all fields by default" ) );
  mDefaultToAllCheckBox->setEnabled( mAllowMultipleCheckBox->isChecked() );
  if ( const QgsProcessingParameterField *fieldParam = dynamic_cast<const QgsProcessingParameterField *>( definition ) )
    mDefaultToAllCheckBox->setChecked( fieldParam->defaultToAllFields() );

  vlayout->addWidget( mDefaultToAllCheckBox );

  connect( mAllowMultipleCheckBox, &QCheckBox::stateChanged, this, [ = ]
  {
    mDefaultToAllCheckBox->setEnabled( mAllowMultipleCheckBox->isChecked() );
  } );

  vlayout->addWidget( new QLabel( tr( "Default value" ) ) );

  mDefaultLineEdit = new QLineEdit();
  mDefaultLineEdit->setToolTip( tr( "Default field name, or ; separated list of field names for multiple field parameters" ) );
  if ( const QgsProcessingParameterField *fieldParam = dynamic_cast<const QgsProcessingParameterField *>( definition ) )
  {
    const QStringList fields = QgsProcessingParameters::parameterAsFields( fieldParam, fieldParam->defaultValueForGui(), context );
    mDefaultLineEdit->setText( fields.join( ';' ) );
  }
  vlayout->addWidget( mDefaultLineEdit );

  setLayout( vlayout );
}

QgsProcessingParameterDefinition *QgsProcessingFieldParameterDefinitionWidget::createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const
{
  QgsProcessingParameterField::DataType dataType = static_cast< QgsProcessingParameterField::DataType >( mDataTypeComboBox->currentData().toInt() );

  QVariant defaultValue;
  if ( !mDefaultLineEdit->text().trimmed().isEmpty() )
  {
    defaultValue = mDefaultLineEdit->text();
  }
  auto param = std::make_unique< QgsProcessingParameterField >( name, description, defaultValue, mParentLayerComboBox->currentData().toString(), dataType, mAllowMultipleCheckBox->isChecked(), false, mDefaultToAllCheckBox->isChecked() );
  param->setFlags( flags );
  return param.release();
}

QgsProcessingFieldWidgetWrapper::QgsProcessingFieldWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{

}

QWidget *QgsProcessingFieldWidgetWrapper::createWidget()
{
  const QgsProcessingParameterField *fieldParam = dynamic_cast< const QgsProcessingParameterField *>( parameterDefinition() );
  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    case QgsProcessingGui::Batch:
    {
      if ( fieldParam->allowMultiple() )
      {
        mPanel = new QgsProcessingFieldPanelWidget( nullptr, fieldParam );
        mPanel->setToolTip( parameterDefinition()->toolTip() );
        connect( mPanel, &QgsProcessingFieldPanelWidget::changed, this, [ = ]
        {
          emit widgetValueHasChanged( this );
        } );
        return mPanel;
      }
      else
      {
        mComboBox = new QgsFieldComboBox();
        mComboBox->setAllowEmptyFieldName( fieldParam->flags() & QgsProcessingParameterDefinition::FlagOptional );

        if ( fieldParam->dataType() == QgsProcessingParameterField::Numeric )
          mComboBox->setFilters( QgsFieldProxyModel::Numeric );
        else if ( fieldParam->dataType() == QgsProcessingParameterField::String )
          mComboBox->setFilters( QgsFieldProxyModel::String );
        else if ( fieldParam->dataType() == QgsProcessingParameterField::DateTime )
          mComboBox->setFilters( QgsFieldProxyModel::Date | QgsFieldProxyModel::Time | QgsFieldProxyModel::DateTime );

        mComboBox->setToolTip( parameterDefinition()->toolTip() );
        connect( mComboBox, &QgsFieldComboBox::fieldChanged, this, [ = ]( const QString & )
        {
          emit widgetValueHasChanged( this );
        } );
        return mComboBox;
      }
    }

    case QgsProcessingGui::Modeler:
    {
      mLineEdit = new QLineEdit();
      mLineEdit->setToolTip( QObject::tr( "Name of field (separate field names with ; for multiple field parameters)" ) );
      connect( mLineEdit, &QLineEdit::textChanged, this, [ = ]
      {
        emit widgetValueHasChanged( this );
      } );
      return mLineEdit;
    }

  }
  return nullptr;
}

void QgsProcessingFieldWidgetWrapper::postInitialize( const QList<QgsAbstractProcessingParameterWidgetWrapper *> &wrappers )
{
  QgsAbstractProcessingParameterWidgetWrapper::postInitialize( wrappers );
  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    case QgsProcessingGui::Batch:
    {
      for ( const QgsAbstractProcessingParameterWidgetWrapper *wrapper : wrappers )
      {
        if ( wrapper->parameterDefinition()->name() == static_cast< const QgsProcessingParameterField * >( parameterDefinition() )->parentLayerParameterName() )
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

void QgsProcessingFieldWidgetWrapper::setParentLayerWrapperValue( const QgsAbstractProcessingParameterWidgetWrapper *parentWrapper )
{
  // evaluate value to layer
  QgsProcessingContext *context = nullptr;
  std::unique_ptr< QgsProcessingContext > tmpContext;
  if ( mProcessingContextGenerator )
    context = mProcessingContextGenerator->processingContext();

  if ( !context )
  {
    tmpContext = std::make_unique< QgsProcessingContext >();
    context = tmpContext.get();
  }

  QVariant value = parentWrapper->parameterValue();

  if ( value.canConvert<QgsProcessingFeatureSourceDefinition>() )
  {
    // input is a QgsProcessingFeatureSourceDefinition - source from it.
    // this is normally discouraged, and algorithms should NEVER do this -- but in this case we can make
    // certain assumptions due to the fact that we're calling this outside of algorithm/model execution and all sources
    // should be real map layers at this stage
    QgsProcessingFeatureSourceDefinition fromVar = qvariant_cast<QgsProcessingFeatureSourceDefinition>( value );
    value = fromVar.source;
  }

  bool valueSet = false;
  const QList< QgsMapLayer * > layers = QgsProcessingParameters::parameterAsLayerList( parentWrapper->parameterDefinition(), value, *context );

  // several layers, populate with intersection of layers fields
  if ( layers.count() > 1 )
  {
    QgsVectorLayer *vlayer = qobject_cast< QgsVectorLayer * >( layers.at( 0 ) );
    QgsFields fields = vlayer && vlayer->isValid() ? vlayer->fields() : QgsFields();
    const  QList< QgsMapLayer * > remainingLayers = layers.mid( 1 );
    for ( QgsMapLayer *layer : remainingLayers )
    {
      if ( fields.isEmpty() )
        break;

      QgsVectorLayer *vlayer = qobject_cast< QgsVectorLayer * >( layer );
      if ( !vlayer || !vlayer->isValid() )
      {
        fields = QgsFields();
        break;
      }

      for ( int fieldIdx = fields.count() - 1; fieldIdx >= 0; fieldIdx-- )
      {
        if ( vlayer->fields().lookupField( fields.at( fieldIdx ).name() ) < 0 )
          fields.remove( fieldIdx );
      }
    }

    if ( mComboBox )
      mComboBox->setFields( fields );
    else if ( mPanel )
      mPanel->setFields( filterFields( fields ) );

    valueSet = true;
  }

  // only one layer
  if ( !valueSet && !layers.isEmpty() && layers.at( 0 )->isValid() )
  {
    QgsVectorLayer *layer = qobject_cast< QgsVectorLayer * >( layers.at( 0 ) );

    // need to grab ownership of layer if required - otherwise layer may be deleted when context
    // goes out of scope
    std::unique_ptr< QgsMapLayer > ownedLayer( context->takeResultLayer( layer->id() ) );
    if ( ownedLayer && ownedLayer->type() == QgsMapLayerType::VectorLayer )
    {
      mParentLayer.reset( qobject_cast< QgsVectorLayer * >( ownedLayer.release() ) );
      layer = mParentLayer.get();
    }
    else
    {
      // don't need ownership of this layer - it wasn't owned by context (so e.g. is owned by the project)
    }

    if ( mComboBox )
      mComboBox->setLayer( layer );
    else if ( mPanel )
      mPanel->setFields( filterFields( layer->fields() ) );

    valueSet = true;
  }

  if ( !valueSet )
  {
    std::unique_ptr< QgsProcessingFeatureSource > source( QgsProcessingParameters::parameterAsSource( parentWrapper->parameterDefinition(), value, *context ) );
    if ( source )
    {
      const QgsFields fields = source->fields();
      if ( mComboBox )
        mComboBox->setFields( fields );
      else if ( mPanel )
        mPanel->setFields( filterFields( fields ) );

      valueSet = true;
    }
  }

  if ( !valueSet )
  {
    if ( mComboBox )
      mComboBox->setLayer( nullptr );
    else if ( mPanel )
      mPanel->setFields( QgsFields() );

    if ( value.isValid() && widgetContext().messageBar() )
    {
      widgetContext().messageBar()->clearWidgets();
      widgetContext().messageBar()->pushMessage( QString(), QObject::tr( "Could not load selected layer/table. Dependent field could not be populated" ),
          Qgis::MessageLevel::Info );
    }
    return;
  }

  const QgsProcessingParameterField *fieldParam = static_cast< const QgsProcessingParameterField * >( parameterDefinition() );
  if ( mPanel && fieldParam->defaultToAllFields() )
  {
    QVariantList val;
    val.reserve( mPanel->fields().size() );
    for ( const QgsField &field : mPanel->fields() )
      val << field.name();
    setWidgetValue( val, *context );
  }
  else if ( fieldParam->defaultValueForGui().isValid() )
    setWidgetValue( parameterDefinition()->defaultValueForGui(), *context );
}

void QgsProcessingFieldWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext &context )
{
  if ( mComboBox )
  {
    if ( !value.isValid() )
      mComboBox->setField( QString() );
    else
    {
      const QString v = QgsProcessingParameters::parameterAsString( parameterDefinition(), value, context );
      mComboBox->setField( v );
    }
  }
  else if ( mPanel )
  {
    QVariantList opts;
    if ( value.isValid() )
    {
      const QStringList v = QgsProcessingParameters::parameterAsFields( parameterDefinition(), value, context );
      opts.reserve( v.size() );
      for ( const QString &i : v )
        opts << i;
    }
    if ( mPanel )
      mPanel->setValue( opts );
  }
  else if ( mLineEdit )
  {
    const QgsProcessingParameterField *fieldParam = static_cast< const QgsProcessingParameterField * >( parameterDefinition() );
    if ( fieldParam->allowMultiple() )
    {
      const QStringList v = QgsProcessingParameters::parameterAsFields( parameterDefinition(), value, context );
      mLineEdit->setText( v.join( ';' ) );
    }
    else
    {
      mLineEdit->setText( QgsProcessingParameters::parameterAsString( parameterDefinition(), value, context ) );
    }
  }
}

QVariant QgsProcessingFieldWidgetWrapper::widgetValue() const
{
  if ( mComboBox )
    return mComboBox->currentField();
  else if ( mPanel )
    return mPanel->value();
  else if ( mLineEdit )
  {
    const QgsProcessingParameterField *fieldParam = static_cast< const QgsProcessingParameterField * >( parameterDefinition() );
    if ( fieldParam->allowMultiple() )
    {
      return mLineEdit->text().split( ';' );
    }
    else
      return mLineEdit->text();
  }
  else
    return QVariant();
}

QStringList QgsProcessingFieldWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList()
         << QgsProcessingParameterField::typeName()
         << QgsProcessingParameterString::typeName();
}

QStringList QgsProcessingFieldWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList()
         << QgsProcessingOutputString::typeName();
}

QString QgsProcessingFieldWidgetWrapper::modelerExpressionFormatString() const
{
  return tr( "selected field names as an array of names, or semicolon separated string of options (e.g. 'fid;place_name')" );
}

const QgsVectorLayer *QgsProcessingFieldWidgetWrapper::linkedVectorLayer() const
{
  if ( mComboBox && mComboBox->layer() )
    return mComboBox->layer();

  return QgsAbstractProcessingParameterWidgetWrapper::linkedVectorLayer();
}

QgsFields QgsProcessingFieldWidgetWrapper::filterFields( const QgsFields &fields ) const
{
  const QgsProcessingParameterField *fieldParam = static_cast< const QgsProcessingParameterField * >( parameterDefinition() );
  QgsFields res;
  for ( const QgsField &f : fields )
  {
    switch ( fieldParam->dataType() )
    {
      case QgsProcessingParameterField::Any:
        res.append( f );
        break;

      case QgsProcessingParameterField::Numeric:
        if ( f.isNumeric() )
          res.append( f );
        break;

      case QgsProcessingParameterField::String:
        if ( f.type() == QVariant::String )
          res.append( f );
        break;

      case QgsProcessingParameterField::DateTime:
        if ( f.type() == QVariant::Date || f.type() == QVariant::Time || f.type() == QVariant::DateTime )
          res.append( f );
        break;
    }
  }

  return res;
}

QString QgsProcessingFieldWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterField::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingFieldWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingFieldWidgetWrapper( parameter, type );
}

QgsProcessingAbstractParameterDefinitionWidget *QgsProcessingFieldWidgetWrapper::createParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm )
{
  return new QgsProcessingFieldParameterDefinitionWidget( context, widgetContext, definition, algorithm );
}

//
// QgsProcessingMapThemeWidgetWrapper
//


QgsProcessingMapThemeParameterDefinitionWidget::QgsProcessingMapThemeParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm, QWidget *parent )
  : QgsProcessingAbstractParameterDefinitionWidget( context, widgetContext, definition, algorithm, parent )
{
  QVBoxLayout *vlayout = new QVBoxLayout();
  vlayout->setContentsMargins( 0, 0, 0, 0 );

  vlayout->addWidget( new QLabel( tr( "Default value" ) ) );

  mDefaultComboBox = new QComboBox();
  mDefaultComboBox->addItem( QString(), QVariant( -1 ) );

  const QStringList mapThemes = widgetContext.project() ? widgetContext.project()->mapThemeCollection()->mapThemes() : QgsProject::instance()->mapThemeCollection()->mapThemes();
  for ( const QString &theme : mapThemes )
  {
    mDefaultComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mActionShowAllLayers.svg" ) ), theme, theme );
  }
  mDefaultComboBox->setEditable( true );

  if ( const QgsProcessingParameterMapTheme *themeParam = dynamic_cast<const QgsProcessingParameterMapTheme *>( definition ) )
  {
    if ( themeParam->defaultValueForGui().isValid() )
      mDefaultComboBox->setCurrentText( QgsProcessingParameters::parameterAsString( themeParam, themeParam->defaultValueForGui(), context ) );
    else
      mDefaultComboBox->setCurrentIndex( mDefaultComboBox->findData( -1 ) );
  }
  else
    mDefaultComboBox->setCurrentIndex( mDefaultComboBox->findData( -1 ) );

  vlayout->addWidget( mDefaultComboBox );

  setLayout( vlayout );
}

QgsProcessingParameterDefinition *QgsProcessingMapThemeParameterDefinitionWidget::createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const
{
  QVariant defaultVal;
  if ( mDefaultComboBox->currentText().isEmpty() )
    defaultVal = QVariant();
  else
    defaultVal = mDefaultComboBox->currentText();
  auto param = std::make_unique< QgsProcessingParameterMapTheme>( name, description, defaultVal );
  param->setFlags( flags );
  return param.release();
}


QgsProcessingMapThemeWidgetWrapper::QgsProcessingMapThemeWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{

}

QWidget *QgsProcessingMapThemeWidgetWrapper::createWidget()
{
  const QgsProcessingParameterMapTheme *themeParam = dynamic_cast< const QgsProcessingParameterMapTheme *>( parameterDefinition() );

  mComboBox = new QComboBox();

  if ( themeParam->flags() & QgsProcessingParameterDefinition::FlagOptional )
    mComboBox->addItem( tr( "[Not selected]" ), QVariant( -1 ) );

  const QStringList mapThemes = widgetContext().project() ? widgetContext().project()->mapThemeCollection()->mapThemes() : QgsProject::instance()->mapThemeCollection()->mapThemes();
  for ( const QString &theme : mapThemes )
  {
    mComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mActionShowAllLayers.svg" ) ), theme, theme );
  }

  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    case QgsProcessingGui::Batch:
      break;

    case QgsProcessingGui::Modeler:
      mComboBox->setEditable( true );
      break;
  }

  mComboBox->setToolTip( parameterDefinition()->toolTip() );
  connect( mComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, [ = ]( int )
  {
    emit widgetValueHasChanged( this );
  } );

  return mComboBox;
}

void QgsProcessingMapThemeWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext &context )
{
  const QString v = QgsProcessingParameters::parameterAsString( parameterDefinition(), value, context );

  if ( !value.isValid() )
    mComboBox->setCurrentIndex( mComboBox->findData( QVariant( -1 ) ) );
  else
  {
    if ( mComboBox->isEditable() && mComboBox->findData( v ) == -1 )
    {
      const QString prev = mComboBox->currentText();
      mComboBox->setCurrentText( v );
      if ( prev != v )
        emit widgetValueHasChanged( this );
    }
    else
      mComboBox->setCurrentIndex( mComboBox->findData( v ) );
  }
}

QVariant QgsProcessingMapThemeWidgetWrapper::widgetValue() const
{
  if ( mComboBox )
    return mComboBox->currentData().toInt() == -1 ? QVariant() :
           !mComboBox->currentData().isValid() && mComboBox->isEditable() ? mComboBox->currentText().isEmpty() ? QVariant() : QVariant( mComboBox->currentText() )
           : mComboBox->currentData();
  else
    return QVariant();
}

QStringList QgsProcessingMapThemeWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList()
         << QgsProcessingParameterString::typeName()
         << QgsProcessingParameterExpression::typeName();
}

QStringList QgsProcessingMapThemeWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList()
         << QgsProcessingOutputString::typeName();
}

QString QgsProcessingMapThemeWidgetWrapper::modelerExpressionFormatString() const
{
  return tr( "map theme as a string value (e.g. 'base maps')" );
}

QString QgsProcessingMapThemeWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterMapTheme::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingMapThemeWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingMapThemeWidgetWrapper( parameter, type );
}

QgsProcessingAbstractParameterDefinitionWidget *QgsProcessingMapThemeWidgetWrapper::createParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm )
{
  return new QgsProcessingMapThemeParameterDefinitionWidget( context, widgetContext, definition, algorithm );
}



//
// QgsProcessingDateTimeWidgetWrapper
//


QgsProcessingDateTimeParameterDefinitionWidget::QgsProcessingDateTimeParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm, QWidget *parent )
  : QgsProcessingAbstractParameterDefinitionWidget( context, widgetContext, definition, algorithm, parent )
{
  QVBoxLayout *vlayout = new QVBoxLayout();
  vlayout->setContentsMargins( 0, 0, 0, 0 );

  vlayout->addWidget( new QLabel( tr( "Type" ) ) );

  mTypeComboBox = new QComboBox();
  mTypeComboBox->addItem( tr( "Date and Time" ), QgsProcessingParameterDateTime::DateTime );
  mTypeComboBox->addItem( tr( "Date" ), QgsProcessingParameterDateTime::Date );
  mTypeComboBox->addItem( tr( "Time" ), QgsProcessingParameterDateTime::Time );
  if ( const QgsProcessingParameterDateTime *datetimeParam = dynamic_cast<const QgsProcessingParameterDateTime *>( definition ) )
    mTypeComboBox->setCurrentIndex( mTypeComboBox->findData( datetimeParam->dataType() ) );
  else
    mTypeComboBox->setCurrentIndex( 0 );
  vlayout->addWidget( mTypeComboBox );

  setLayout( vlayout );
}

QgsProcessingParameterDefinition *QgsProcessingDateTimeParameterDefinitionWidget::createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const
{
  auto param = std::make_unique< QgsProcessingParameterDateTime >( name, description );
  param->setDataType( static_cast< QgsProcessingParameterDateTime::Type >( mTypeComboBox->currentData().toInt() ) );
  param->setFlags( flags );
  return param.release();
}


QgsProcessingDateTimeWidgetWrapper::QgsProcessingDateTimeWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{

}

QWidget *QgsProcessingDateTimeWidgetWrapper::createWidget()
{
  const QgsProcessingParameterDateTime *dateTimeParam = dynamic_cast< const QgsProcessingParameterDateTime *>( parameterDefinition() );

  QgsDateTimeEdit *widget = nullptr;
  switch ( dateTimeParam->dataType() )
  {
    case QgsProcessingParameterDateTime::DateTime:
      mDateTimeEdit = new QgsDateTimeEdit();
      widget = mDateTimeEdit;
      break;

    case QgsProcessingParameterDateTime::Date:
      mDateEdit = new QgsDateEdit();
      widget = mDateEdit;
      break;

    case QgsProcessingParameterDateTime::Time:
      mTimeEdit = new QgsTimeEdit();
      widget = mTimeEdit;
      break;
  }

  if ( dateTimeParam->flags() & QgsProcessingParameterDefinition::FlagOptional )
  {
    widget->setNullRepresentation( tr( "[Not selected]" ) );
    widget->setAllowNull( true );
  }
  else
  {
    widget->setAllowNull( false );
  }
  widget->setToolTip( parameterDefinition()->toolTip() );

  if ( mDateTimeEdit )
  {
    connect( mDateTimeEdit, &QgsDateTimeEdit::valueChanged, this, [ = ]( const QDateTime & )
    {
      emit widgetValueHasChanged( this );
    } );
  }
  else if ( mDateEdit )
  {
    connect( mDateEdit, &QgsDateEdit::dateValueChanged, this, [ = ]( const QDate & )
    {
      emit widgetValueHasChanged( this );
    } );
  }
  else if ( mTimeEdit )
  {
    connect( mTimeEdit, &QgsTimeEdit::timeValueChanged, this, [ = ]( const QTime & )
    {
      emit widgetValueHasChanged( this );
    } );
  }

  return widget;
}

QgsProcessingAbstractParameterDefinitionWidget *QgsProcessingDateTimeWidgetWrapper::createParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm )
{
  return new QgsProcessingDateTimeParameterDefinitionWidget( context, widgetContext, definition, algorithm );
}

void QgsProcessingDateTimeWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext &context )
{
  if ( mDateTimeEdit )
  {
    mDateTimeEdit->setDateTime( QgsProcessingParameters::parameterAsDateTime( parameterDefinition(), value, context ) );
  }
  else if ( mDateEdit )
  {
    mDateEdit->setDate( QgsProcessingParameters::parameterAsDate( parameterDefinition(), value, context ) );
  }
  else if ( mTimeEdit )
  {
    mTimeEdit->setTime( QgsProcessingParameters::parameterAsTime( parameterDefinition(), value, context ) );
  }
}

QVariant QgsProcessingDateTimeWidgetWrapper::widgetValue() const
{
  if ( mDateTimeEdit )
    return !mDateTimeEdit->dateTime().isNull() && mDateTimeEdit->dateTime().isValid() ? QVariant( mDateTimeEdit->dateTime() ) : QVariant();
  else if ( mDateEdit )
    return !mDateEdit->date().isNull() && mDateEdit->date().isValid() ? QVariant( mDateEdit->date() ) : QVariant();
  else if ( mTimeEdit )
    return !mTimeEdit->time().isNull() && mTimeEdit->time().isValid() ? QVariant( mTimeEdit->time() ) : QVariant();
  else
    return QVariant();
}

QStringList QgsProcessingDateTimeWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList()
         << QgsProcessingParameterDateTime::typeName()
         << QgsProcessingParameterString::typeName();
}

QStringList QgsProcessingDateTimeWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList()
         << QgsProcessingOutputString::typeName();
}

QString QgsProcessingDateTimeWidgetWrapper::modelerExpressionFormatString() const
{
  const QgsProcessingParameterDateTime *dateTimeParam = dynamic_cast< const QgsProcessingParameterDateTime *>( parameterDefinition() );
  if ( dateTimeParam )
  {
    switch ( dateTimeParam->dataType() )
    {
      case QgsProcessingParameterDateTime::DateTime:
        return tr( "datetime value, or a ISO string representation of a datetime" );

      case QgsProcessingParameterDateTime::Date:
        return tr( "date value, or a ISO string representation of a date" );

      case QgsProcessingParameterDateTime::Time:
        return tr( "time value, or a ISO string representation of a time" );
    }
  }
  return QString();
}

QString QgsProcessingDateTimeWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterDateTime::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingDateTimeWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingDateTimeWidgetWrapper( parameter, type );
}



//
// QgsProcessingProviderConnectionWidgetWrapper
//

QgsProcessingProviderConnectionParameterDefinitionWidget::QgsProcessingProviderConnectionParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm, QWidget *parent )
  : QgsProcessingAbstractParameterDefinitionWidget( context, widgetContext, definition, algorithm, parent )
{
  const QgsProcessingParameterProviderConnection *connectionParam = dynamic_cast< const QgsProcessingParameterProviderConnection *>( definition );

  QVBoxLayout *vlayout = new QVBoxLayout();
  vlayout->setContentsMargins( 0, 0, 0, 0 );

  vlayout->addWidget( new QLabel( tr( "Provider" ) ) );
  mProviderComboBox = new QComboBox();
  mProviderComboBox->addItem( QObject::tr( "Postgres" ), QStringLiteral( "postgres" ) );
  mProviderComboBox->addItem( QObject::tr( "GeoPackage" ), QStringLiteral( "ogr" ) );
  mProviderComboBox->addItem( QObject::tr( "Spatialite" ), QStringLiteral( "spatialite" ) );

  vlayout->addWidget( mProviderComboBox );

  vlayout->addWidget( new QLabel( tr( "Default value" ) ) );

  mDefaultEdit = new QLineEdit();
  vlayout->addWidget( mDefaultEdit );
  setLayout( vlayout );

  if ( connectionParam )
  {
    mProviderComboBox->setCurrentIndex( mProviderComboBox->findData( connectionParam->providerId() ) );
    mDefaultEdit->setText( connectionParam->defaultValueForGui().toString() );
  }
}

QgsProcessingParameterDefinition *QgsProcessingProviderConnectionParameterDefinitionWidget::createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const
{
  QVariant defaultVal;
  if ( mDefaultEdit->text().isEmpty() )
    defaultVal = QVariant();
  else
    defaultVal = mDefaultEdit->text();
  auto param = std::make_unique< QgsProcessingParameterProviderConnection>( name, description, mProviderComboBox->currentData().toString(), defaultVal );
  param->setFlags( flags );
  return param.release();
}


QgsProcessingProviderConnectionWidgetWrapper::QgsProcessingProviderConnectionWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{

}

QWidget *QgsProcessingProviderConnectionWidgetWrapper::createWidget()
{
  const QgsProcessingParameterProviderConnection *connectionParam = dynamic_cast< const QgsProcessingParameterProviderConnection *>( parameterDefinition() );

  mProviderComboBox = new QgsProviderConnectionComboBox( connectionParam->providerId() );
  if ( connectionParam->flags() & QgsProcessingParameterDefinition::FlagOptional )
    mProviderComboBox->setAllowEmptyConnection( true );

  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    case QgsProcessingGui::Batch:
      break;
    case QgsProcessingGui::Modeler:
      mProviderComboBox->setEditable( true );
      break;
  }

  mProviderComboBox->setToolTip( parameterDefinition()->toolTip() );
  connect( mProviderComboBox, &QgsProviderConnectionComboBox::currentTextChanged, this, [ = ]( const QString & )
  {
    if ( mBlockSignals )
      return;

    emit widgetValueHasChanged( this );
  } );

  return mProviderComboBox;
}

QgsProcessingAbstractParameterDefinitionWidget *QgsProcessingProviderConnectionWidgetWrapper::createParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm )
{
  return new QgsProcessingProviderConnectionParameterDefinitionWidget( context, widgetContext, definition, algorithm );
}

void QgsProcessingProviderConnectionWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext &context )
{
  const QString v = QgsProcessingParameters::parameterAsConnectionName( parameterDefinition(), value, context );

  if ( !value.isValid() )
    mProviderComboBox->setCurrentIndex( -1 );
  else
  {
    if ( mProviderComboBox->isEditable() )
    {
      const QString prev = mProviderComboBox->currentText();
      mBlockSignals++;
      mProviderComboBox->setConnection( v );
      mProviderComboBox->setCurrentText( v );

      mBlockSignals--;
      if ( prev != v )
        emit widgetValueHasChanged( this );
    }
    else
      mProviderComboBox->setConnection( v );
  }
}

QVariant QgsProcessingProviderConnectionWidgetWrapper::widgetValue() const
{
  if ( mProviderComboBox )
    if ( mProviderComboBox->isEditable() )
      return mProviderComboBox->currentText().isEmpty() ? QVariant() : QVariant( mProviderComboBox->currentText() );
    else
      return mProviderComboBox->currentConnection().isEmpty() ? QVariant() : QVariant( mProviderComboBox->currentConnection() );
  else
    return QVariant();
}

QStringList QgsProcessingProviderConnectionWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList()
         << QgsProcessingParameterProviderConnection::typeName()
         << QgsProcessingParameterString::typeName()
         << QgsProcessingParameterExpression::typeName();
}

QStringList QgsProcessingProviderConnectionWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList()
         << QgsProcessingOutputString::typeName();
}

QString QgsProcessingProviderConnectionWidgetWrapper::modelerExpressionFormatString() const
{
  return tr( "connection name as a string value" );
}

QString QgsProcessingProviderConnectionWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterProviderConnection::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingProviderConnectionWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingProviderConnectionWidgetWrapper( parameter, type );
}




//
// QgsProcessingDatabaseSchemaWidgetWrapper
//

QgsProcessingDatabaseSchemaParameterDefinitionWidget::QgsProcessingDatabaseSchemaParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm, QWidget *parent )
  : QgsProcessingAbstractParameterDefinitionWidget( context, widgetContext, definition, algorithm, parent )
{
  const QgsProcessingParameterDatabaseSchema *schemaParam = dynamic_cast< const QgsProcessingParameterDatabaseSchema *>( definition );

  QVBoxLayout *vlayout = new QVBoxLayout();
  vlayout->setContentsMargins( 0, 0, 0, 0 );

  mConnectionParamComboBox = new QComboBox();
  QString initialConnection;
  if ( schemaParam )
  {
    initialConnection = schemaParam->parentConnectionParameterName();
  }

  if ( auto *lModel = widgetContext.model() )
  {
    // populate combo box with other model input choices
    const QMap<QString, QgsProcessingModelParameter> components = lModel->parameterComponents();
    for ( auto it = components.constBegin(); it != components.constEnd(); ++it )
    {
      if ( definition && it->parameterName() == definition->name() )
        continue;

      if ( !dynamic_cast< const QgsProcessingParameterProviderConnection * >( lModel->parameterDefinition( it->parameterName() ) ) )
        continue;

      mConnectionParamComboBox->addItem( it->parameterName(), it->parameterName() );
      if ( !initialConnection.isEmpty() && initialConnection == it->parameterName() )
      {
        mConnectionParamComboBox->setCurrentIndex( mConnectionParamComboBox->count() - 1 );
      }
    }
  }

  if ( mConnectionParamComboBox->count() == 0 && !initialConnection.isEmpty() )
  {
    // if no candidates found, we just add the existing one as a placeholder
    mConnectionParamComboBox->addItem( initialConnection, initialConnection );
    mConnectionParamComboBox->setCurrentIndex( mConnectionParamComboBox->count() - 1 );
  }

  vlayout->addWidget( new QLabel( tr( "Provider connection parameter" ) ) );
  vlayout->addWidget( mConnectionParamComboBox );

  vlayout->addWidget( new QLabel( tr( "Default value" ) ) );

  mDefaultEdit = new QLineEdit();
  vlayout->addWidget( mDefaultEdit );
  setLayout( vlayout );

  if ( schemaParam )
  {
    mDefaultEdit->setText( schemaParam->defaultValueForGui().toString() );
  }
}

QgsProcessingParameterDefinition *QgsProcessingDatabaseSchemaParameterDefinitionWidget::createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const
{
  QVariant defaultVal;
  if ( mDefaultEdit->text().isEmpty() )
    defaultVal = QVariant();
  else
    defaultVal = mDefaultEdit->text();
  auto param = std::make_unique< QgsProcessingParameterDatabaseSchema>( name, description, mConnectionParamComboBox->currentData().toString(), defaultVal );
  param->setFlags( flags );
  return param.release();
}


QgsProcessingDatabaseSchemaWidgetWrapper::QgsProcessingDatabaseSchemaWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{

}

QWidget *QgsProcessingDatabaseSchemaWidgetWrapper::createWidget()
{
  const QgsProcessingParameterDatabaseSchema *schemaParam = dynamic_cast< const QgsProcessingParameterDatabaseSchema *>( parameterDefinition() );

  mSchemaComboBox = new QgsDatabaseSchemaComboBox( QString(), QString() );
  if ( schemaParam->flags() & QgsProcessingParameterDefinition::FlagOptional )
    mSchemaComboBox->setAllowEmptySchema( true );

  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    case QgsProcessingGui::Batch:
      break;
    case QgsProcessingGui::Modeler:
      mSchemaComboBox->comboBox()->setEditable( true );
      break;
  }

  mSchemaComboBox->setToolTip( parameterDefinition()->toolTip() );
  connect( mSchemaComboBox->comboBox(), &QComboBox::currentTextChanged, this, [ = ]( const QString & )
  {
    if ( mBlockSignals )
      return;

    emit widgetValueHasChanged( this );
  } );

  return mSchemaComboBox;
}

QgsProcessingAbstractParameterDefinitionWidget *QgsProcessingDatabaseSchemaWidgetWrapper::createParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm )
{
  return new QgsProcessingDatabaseSchemaParameterDefinitionWidget( context, widgetContext, definition, algorithm );
}

void QgsProcessingDatabaseSchemaWidgetWrapper::setParentConnectionWrapperValue( const QgsAbstractProcessingParameterWidgetWrapper *parentWrapper )
{
  // evaluate value to connection
  QgsProcessingContext *context = nullptr;
  std::unique_ptr< QgsProcessingContext > tmpContext;
  if ( mProcessingContextGenerator )
    context = mProcessingContextGenerator->processingContext();

  if ( !context )
  {
    tmpContext = std::make_unique< QgsProcessingContext >();
    context = tmpContext.get();
  }

  const QVariant value = parentWrapper->parameterValue();
  const QString connection = value.isValid() ? QgsProcessingParameters::parameterAsConnectionName( parentWrapper->parameterDefinition(), value, *context ) : QString();

  if ( mSchemaComboBox )
    mSchemaComboBox->setConnectionName( connection, qgis::down_cast< const QgsProcessingParameterProviderConnection * >( parentWrapper->parameterDefinition() )->providerId() );

  const QgsProcessingParameterDatabaseSchema *schemaParam = qgis::down_cast< const QgsProcessingParameterDatabaseSchema * >( parameterDefinition() );
  if ( schemaParam->defaultValueForGui().isValid() )
    setWidgetValue( parameterDefinition()->defaultValueForGui(), *context );
}

void QgsProcessingDatabaseSchemaWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext &context )
{
  const QString v = QgsProcessingParameters::parameterAsSchema( parameterDefinition(), value, context );

  if ( !value.isValid() )
    mSchemaComboBox->comboBox()->setCurrentIndex( -1 );
  else
  {
    if ( mSchemaComboBox->comboBox()->isEditable() )
    {
      const QString prev = mSchemaComboBox->comboBox()->currentText();
      mBlockSignals++;
      mSchemaComboBox->setSchema( v );
      mSchemaComboBox->comboBox()->setCurrentText( v );

      mBlockSignals--;
      if ( prev != v )
        emit widgetValueHasChanged( this );
    }
    else
      mSchemaComboBox->setSchema( v );
  }
}

QVariant QgsProcessingDatabaseSchemaWidgetWrapper::widgetValue() const
{
  if ( mSchemaComboBox )
    if ( mSchemaComboBox->comboBox()->isEditable() )
      return mSchemaComboBox->comboBox()->currentText().isEmpty() ? QVariant() : QVariant( mSchemaComboBox->comboBox()->currentText() );
    else
      return mSchemaComboBox->currentSchema().isEmpty() ? QVariant() : QVariant( mSchemaComboBox->currentSchema() );
  else
    return QVariant();
}

QStringList QgsProcessingDatabaseSchemaWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList()
         << QgsProcessingParameterProviderConnection::typeName()
         << QgsProcessingParameterString::typeName()
         << QgsProcessingParameterExpression::typeName();
}

QStringList QgsProcessingDatabaseSchemaWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList()
         << QgsProcessingOutputString::typeName();
}

QString QgsProcessingDatabaseSchemaWidgetWrapper::modelerExpressionFormatString() const
{
  return tr( "database schema name as a string value" );
}

QString QgsProcessingDatabaseSchemaWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterDatabaseSchema::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingDatabaseSchemaWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingDatabaseSchemaWidgetWrapper( parameter, type );
}

void QgsProcessingDatabaseSchemaWidgetWrapper::postInitialize( const QList<QgsAbstractProcessingParameterWidgetWrapper *> &wrappers )
{
  QgsAbstractProcessingParameterWidgetWrapper::postInitialize( wrappers );
  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    case QgsProcessingGui::Batch:
    {
      for ( const QgsAbstractProcessingParameterWidgetWrapper *wrapper : wrappers )
      {
        if ( wrapper->parameterDefinition()->name() == static_cast< const QgsProcessingParameterDatabaseSchema * >( parameterDefinition() )->parentConnectionParameterName() )
        {
          setParentConnectionWrapperValue( wrapper );
          connect( wrapper, &QgsAbstractProcessingParameterWidgetWrapper::widgetValueHasChanged, this, [ = ]
          {
            setParentConnectionWrapperValue( wrapper );
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



//
// QgsProcessingDatabaseTableWidgetWrapper
//

QgsProcessingDatabaseTableParameterDefinitionWidget::QgsProcessingDatabaseTableParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm, QWidget *parent )
  : QgsProcessingAbstractParameterDefinitionWidget( context, widgetContext, definition, algorithm, parent )
{
  const QgsProcessingParameterDatabaseTable *tableParam = dynamic_cast< const QgsProcessingParameterDatabaseTable *>( definition );

  QVBoxLayout *vlayout = new QVBoxLayout();
  vlayout->setContentsMargins( 0, 0, 0, 0 );

  mConnectionParamComboBox = new QComboBox();
  mSchemaParamComboBox = new QComboBox();
  QString initialConnection;
  QString initialSchema;
  if ( tableParam )
  {
    initialConnection = tableParam->parentConnectionParameterName();
    initialSchema = tableParam->parentSchemaParameterName();
  }

  if ( auto *lModel = widgetContext.model() )
  {
    // populate combo box with other model input choices
    const QMap<QString, QgsProcessingModelParameter> components = lModel->parameterComponents();
    for ( auto it = components.constBegin(); it != components.constEnd(); ++it )
    {
      if ( definition && it->parameterName() == definition->name() )
        continue;

      if ( dynamic_cast< const QgsProcessingParameterProviderConnection * >( lModel->parameterDefinition( it->parameterName() ) ) )
      {
        mConnectionParamComboBox->addItem( it->parameterName(), it->parameterName() );
        if ( !initialConnection.isEmpty() && initialConnection == it->parameterName() )
        {
          mConnectionParamComboBox->setCurrentIndex( mConnectionParamComboBox->count() - 1 );
        }
      }
      else if ( dynamic_cast< const QgsProcessingParameterDatabaseSchema * >( lModel->parameterDefinition( it->parameterName() ) ) )
      {
        mSchemaParamComboBox->addItem( it->parameterName(), it->parameterName() );
        if ( !initialConnection.isEmpty() && initialConnection == it->parameterName() )
        {
          mSchemaParamComboBox->setCurrentIndex( mSchemaParamComboBox->count() - 1 );
        }
      }
    }
  }

  if ( mConnectionParamComboBox->count() == 0 && !initialConnection.isEmpty() )
  {
    // if no candidates found, we just add the existing one as a placeholder
    mConnectionParamComboBox->addItem( initialConnection, initialConnection );
    mConnectionParamComboBox->setCurrentIndex( mConnectionParamComboBox->count() - 1 );
  }

  if ( mSchemaParamComboBox->count() == 0 && !initialSchema.isEmpty() )
  {
    // if no candidates found, we just add the existing one as a placeholder
    mSchemaParamComboBox->addItem( initialSchema, initialSchema );
    mSchemaParamComboBox->setCurrentIndex( mSchemaParamComboBox->count() - 1 );
  }

  vlayout->addWidget( new QLabel( tr( "Provider connection parameter" ) ) );
  vlayout->addWidget( mConnectionParamComboBox );

  vlayout->addWidget( new QLabel( tr( "Database schema parameter" ) ) );
  vlayout->addWidget( mSchemaParamComboBox );

  vlayout->addWidget( new QLabel( tr( "Default value" ) ) );

  mDefaultEdit = new QLineEdit();
  vlayout->addWidget( mDefaultEdit );
  setLayout( vlayout );

  if ( tableParam )
  {
    mDefaultEdit->setText( tableParam->defaultValueForGui().toString() );
  }
}

QgsProcessingParameterDefinition *QgsProcessingDatabaseTableParameterDefinitionWidget::createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const
{
  QVariant defaultVal;
  if ( mDefaultEdit->text().isEmpty() )
    defaultVal = QVariant();
  else
    defaultVal = mDefaultEdit->text();
  auto param = std::make_unique< QgsProcessingParameterDatabaseTable>( name, description,
               mConnectionParamComboBox->currentData().toString(),
               mSchemaParamComboBox->currentData().toString(),
               defaultVal );
  param->setFlags( flags );
  return param.release();
}


QgsProcessingDatabaseTableWidgetWrapper::QgsProcessingDatabaseTableWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{

}

QWidget *QgsProcessingDatabaseTableWidgetWrapper::createWidget()
{
  const QgsProcessingParameterDatabaseTable *tableParam = dynamic_cast< const QgsProcessingParameterDatabaseTable *>( parameterDefinition() );

  mTableComboBox = new QgsDatabaseTableComboBox( QString(), QString() );
  if ( tableParam->flags() & QgsProcessingParameterDefinition::FlagOptional )
    mTableComboBox->setAllowEmptyTable( true );

  if ( type() == QgsProcessingGui::Modeler || tableParam->allowNewTableNames() )
    mTableComboBox->comboBox()->setEditable( true );

  mTableComboBox->setToolTip( parameterDefinition()->toolTip() );
  connect( mTableComboBox->comboBox(), &QComboBox::currentTextChanged, this, [ = ]( const QString & )
  {
    if ( mBlockSignals )
      return;

    emit widgetValueHasChanged( this );
  } );

  return mTableComboBox;
}

QgsProcessingAbstractParameterDefinitionWidget *QgsProcessingDatabaseTableWidgetWrapper::createParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm )
{
  return new QgsProcessingDatabaseTableParameterDefinitionWidget( context, widgetContext, definition, algorithm );
}

void QgsProcessingDatabaseTableWidgetWrapper::setParentConnectionWrapperValue( const QgsAbstractProcessingParameterWidgetWrapper *parentWrapper )
{
  // evaluate value to connection
  QgsProcessingContext *context = nullptr;
  std::unique_ptr< QgsProcessingContext > tmpContext;
  if ( mProcessingContextGenerator )
    context = mProcessingContextGenerator->processingContext();

  if ( !context )
  {
    tmpContext = std::make_unique< QgsProcessingContext >();
    context = tmpContext.get();
  }

  QVariant value = parentWrapper->parameterValue();
  mConnection = value.isValid() ? QgsProcessingParameters::parameterAsConnectionName( parentWrapper->parameterDefinition(), value, *context ) : QString();
  mProvider = qgis::down_cast< const QgsProcessingParameterProviderConnection * >( parentWrapper->parameterDefinition() )->providerId();
  if ( mTableComboBox && !mSchema.isEmpty() )
  {
    mTableComboBox->setSchema( mSchema );
    mTableComboBox->setConnectionName( mConnection, mProvider );

    const QgsProcessingParameterDatabaseTable *tableParam = qgis::down_cast< const QgsProcessingParameterDatabaseTable * >( parameterDefinition() );
    if ( tableParam->defaultValueForGui().isValid() )
      setWidgetValue( parameterDefinition()->defaultValueForGui(), *context );
  }
}

void QgsProcessingDatabaseTableWidgetWrapper::setParentSchemaWrapperValue( const QgsAbstractProcessingParameterWidgetWrapper *parentWrapper )
{
  // evaluate value to schema
  QgsProcessingContext *context = nullptr;
  std::unique_ptr< QgsProcessingContext > tmpContext;
  if ( mProcessingContextGenerator )
    context = mProcessingContextGenerator->processingContext();

  if ( !context )
  {
    tmpContext = std::make_unique< QgsProcessingContext >();
    context = tmpContext.get();
  }

  QVariant value = parentWrapper->parameterValue();
  mSchema = value.isValid() ? QgsProcessingParameters::parameterAsSchema( parentWrapper->parameterDefinition(), value, *context ) : QString();

  if ( mTableComboBox && !mSchema.isEmpty() && !mConnection.isEmpty() )
  {
    mTableComboBox->setSchema( mSchema );
    mTableComboBox->setConnectionName( mConnection, mProvider );

    const QgsProcessingParameterDatabaseTable *tableParam = static_cast< const QgsProcessingParameterDatabaseTable * >( parameterDefinition() );
    if ( tableParam->defaultValueForGui().isValid() )
      setWidgetValue( parameterDefinition()->defaultValueForGui(), *context );
  }

}

void QgsProcessingDatabaseTableWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext &context )
{
  const QString v = QgsProcessingParameters::parameterAsDatabaseTableName( parameterDefinition(), value, context );

  if ( !value.isValid() )
    mTableComboBox->comboBox()->setCurrentIndex( -1 );
  else
  {
    if ( mTableComboBox->comboBox()->isEditable() )
    {
      const QString prev = mTableComboBox->comboBox()->currentText();
      mBlockSignals++;
      mTableComboBox->setTable( v );
      mTableComboBox->comboBox()->setCurrentText( v );

      mBlockSignals--;
      if ( prev != v )
        emit widgetValueHasChanged( this );
    }
    else
      mTableComboBox->setTable( v );
  }
}

QVariant QgsProcessingDatabaseTableWidgetWrapper::widgetValue() const
{
  if ( mTableComboBox )
    if ( mTableComboBox->comboBox()->isEditable() )
      return mTableComboBox->comboBox()->currentText().isEmpty() ? QVariant() : QVariant( mTableComboBox->comboBox()->currentText() );
    else
      return mTableComboBox->currentTable().isEmpty() ? QVariant() : QVariant( mTableComboBox->currentTable() );
  else
    return QVariant();
}

QStringList QgsProcessingDatabaseTableWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList()
         << QgsProcessingParameterProviderConnection::typeName()
         << QgsProcessingParameterString::typeName()
         << QgsProcessingParameterExpression::typeName();
}

QStringList QgsProcessingDatabaseTableWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList()
         << QgsProcessingOutputString::typeName();
}

QString QgsProcessingDatabaseTableWidgetWrapper::modelerExpressionFormatString() const
{
  return tr( "database table name as a string value" );
}

QString QgsProcessingDatabaseTableWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterDatabaseTable::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingDatabaseTableWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingDatabaseTableWidgetWrapper( parameter, type );
}

void QgsProcessingDatabaseTableWidgetWrapper::postInitialize( const QList<QgsAbstractProcessingParameterWidgetWrapper *> &wrappers )
{
  QgsAbstractProcessingParameterWidgetWrapper::postInitialize( wrappers );
  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    case QgsProcessingGui::Batch:
    {
      for ( const QgsAbstractProcessingParameterWidgetWrapper *wrapper : wrappers )
      {
        if ( wrapper->parameterDefinition()->name() == static_cast< const QgsProcessingParameterDatabaseTable * >( parameterDefinition() )->parentConnectionParameterName() )
        {
          setParentConnectionWrapperValue( wrapper );
          connect( wrapper, &QgsAbstractProcessingParameterWidgetWrapper::widgetValueHasChanged, this, [ = ]
          {
            setParentConnectionWrapperValue( wrapper );
          } );
        }
        else if ( wrapper->parameterDefinition()->name() == static_cast< const QgsProcessingParameterDatabaseTable * >( parameterDefinition() )->parentSchemaParameterName() )
        {
          setParentSchemaWrapperValue( wrapper );
          connect( wrapper, &QgsAbstractProcessingParameterWidgetWrapper::widgetValueHasChanged, this, [ = ]
          {
            setParentSchemaWrapperValue( wrapper );
          } );
        }
      }
      break;
    }

    case QgsProcessingGui::Modeler:
      break;
  }
}


//
// QgsProcessingExtentWidgetWrapper
//

QgsProcessingExtentParameterDefinitionWidget::QgsProcessingExtentParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm, QWidget *parent )
  : QgsProcessingAbstractParameterDefinitionWidget( context, widgetContext, definition, algorithm, parent )
{
  QVBoxLayout *vlayout = new QVBoxLayout();
  vlayout->setContentsMargins( 0, 0, 0, 0 );

  vlayout->addWidget( new QLabel( tr( "Default value" ) ) );

  mDefaultWidget = new QgsExtentWidget();
  mDefaultWidget->setNullValueAllowed( true, tr( "Not set" ) );
  if ( const QgsProcessingParameterExtent *extentParam = dynamic_cast<const QgsProcessingParameterExtent *>( definition ) )
  {
    if ( extentParam->defaultValueForGui().isValid() )
    {
      QgsRectangle rect = QgsProcessingParameters::parameterAsExtent( extentParam, extentParam->defaultValueForGui(), context );
      QgsCoordinateReferenceSystem crs = QgsProcessingParameters::parameterAsExtentCrs( extentParam, extentParam->defaultValueForGui(), context );
      mDefaultWidget->setCurrentExtent( rect, crs );
      mDefaultWidget->setOutputExtentFromCurrent();
    }
    else
    {
      mDefaultWidget->clear();
    }
  }

  vlayout->addWidget( mDefaultWidget );
  setLayout( vlayout );
}

QgsProcessingParameterDefinition *QgsProcessingExtentParameterDefinitionWidget::createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const
{
  const QString defaultVal = mDefaultWidget->isValid() ? QStringLiteral( "%1,%2,%3,%4%5" ).arg(
                               QString::number( mDefaultWidget->outputExtent().xMinimum(), 'f', 9 ),
                               QString::number( mDefaultWidget->outputExtent().xMaximum(), 'f', 9 ),
                               QString::number( mDefaultWidget->outputExtent().yMinimum(), 'f', 9 ),
                               QString::number( mDefaultWidget->outputExtent().yMaximum(), 'f', 9 ),
                               mDefaultWidget->outputCrs().isValid() ? QStringLiteral( " [%1]" ).arg( mDefaultWidget->outputCrs().authid() ) : QString()
                             ) : QString();
  auto param = std::make_unique< QgsProcessingParameterExtent >( name, description, !defaultVal.isEmpty() ? QVariant( defaultVal ) : QVariant() );
  param->setFlags( flags );
  return param.release();
}



QgsProcessingExtentWidgetWrapper::QgsProcessingExtentWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{

}

QWidget *QgsProcessingExtentWidgetWrapper::createWidget()
{
  const QgsProcessingParameterExtent *extentParam = dynamic_cast< const QgsProcessingParameterExtent *>( parameterDefinition() );
  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    case QgsProcessingGui::Batch:
    case QgsProcessingGui::Modeler:
    {
      mExtentWidget = new QgsExtentWidget( nullptr );
      if ( widgetContext().mapCanvas() )
        mExtentWidget->setMapCanvas( widgetContext().mapCanvas() );

      if ( extentParam->flags() & QgsProcessingParameterDefinition::FlagOptional )
        mExtentWidget->setNullValueAllowed( true, tr( "Not set" ) );

      mExtentWidget->setToolTip( parameterDefinition()->toolTip() );

      connect( mExtentWidget, &QgsExtentWidget::extentChanged, this, [ = ]
      {
        emit widgetValueHasChanged( this );
      } );

      if ( mDialog && type() != QgsProcessingGui::Modeler )
        setDialog( mDialog ); // setup connections to panel - dialog was previously set before the widget was created

      return mExtentWidget;
    }
  }
  return nullptr;
}

void QgsProcessingExtentWidgetWrapper::setWidgetContext( const QgsProcessingParameterWidgetContext &context )
{
  QgsAbstractProcessingParameterWidgetWrapper::setWidgetContext( context );
  if ( mExtentWidget && context.mapCanvas() && type() != QgsProcessingGui::Modeler )
    mExtentWidget->setMapCanvas( context.mapCanvas() );
}

void QgsProcessingExtentWidgetWrapper::setDialog( QDialog *dialog )
{
  mDialog = dialog;
  if ( mExtentWidget && mDialog && type() != QgsProcessingGui::Modeler )
  {
    connect( mExtentWidget, &QgsExtentWidget::toggleDialogVisibility, mDialog, [ = ]( bool visible )
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

void QgsProcessingExtentWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext &context )
{
  if ( mExtentWidget )
  {
    if ( !value.isValid() || ( value.type() == QVariant::String && value.toString().isEmpty() ) )
      mExtentWidget->clear();
    else
    {
      QgsRectangle r = QgsProcessingParameters::parameterAsExtent( parameterDefinition(), value, context );
      QgsCoordinateReferenceSystem crs = QgsProcessingParameters::parameterAsPointCrs( parameterDefinition(), value, context );
      mExtentWidget->setCurrentExtent( r, crs );
      mExtentWidget->setOutputExtentFromUser( r, crs );
    }
  }
}

QVariant QgsProcessingExtentWidgetWrapper::widgetValue() const
{
  if ( mExtentWidget )
  {
    const QString val = mExtentWidget->isValid() ? QStringLiteral( "%1,%2,%3,%4%5" ).arg(
                          QString::number( mExtentWidget->outputExtent().xMinimum(), 'f', 9 ),
                          QString::number( mExtentWidget->outputExtent().xMaximum(), 'f', 9 ),
                          QString::number( mExtentWidget->outputExtent().yMinimum(), 'f', 9 ),
                          QString::number( mExtentWidget->outputExtent().yMaximum(), 'f', 9 ),
                          mExtentWidget->outputCrs().isValid() ? QStringLiteral( " [%1]" ).arg( mExtentWidget->outputCrs().authid() ) : QString()
                        ) : QString();

    return val.isEmpty() ? QVariant() : QVariant( val );
  }
  else
    return QVariant();
}

QStringList QgsProcessingExtentWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList()
         << QgsProcessingParameterExtent::typeName()
         << QgsProcessingParameterString::typeName()
         << QgsProcessingParameterMapLayer::typeName()
         << QgsProcessingParameterFeatureSource::typeName()
         << QgsProcessingParameterRasterLayer::typeName()
         << QgsProcessingParameterVectorLayer::typeName()
         << QgsProcessingParameterMeshLayer::typeName()
         << QgsProcessingParameterPointCloudLayer::typeName()
         << QgsProcessingParameterAnnotationLayer::typeName();
}

QStringList QgsProcessingExtentWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList()
         << QgsProcessingOutputString::typeName()
         << QgsProcessingOutputRasterLayer::typeName()
         << QgsProcessingOutputVectorLayer::typeName()
         << QgsProcessingOutputMapLayer::typeName();
}

QString QgsProcessingExtentWidgetWrapper::modelerExpressionFormatString() const
{
  return tr( "string of the format 'x min,x max,y min,y max' or a geometry value (bounding box is used)" );
}

QString QgsProcessingExtentWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterExtent::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingExtentWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingExtentWidgetWrapper( parameter, type );
}

QgsProcessingAbstractParameterDefinitionWidget *QgsProcessingExtentWidgetWrapper::createParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm )
{
  return new QgsProcessingExtentParameterDefinitionWidget( context, widgetContext, definition, algorithm );
}



//
// QgsProcessingMapLayerWidgetWrapper
//

QgsProcessingMapLayerParameterDefinitionWidget::QgsProcessingMapLayerParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm, QWidget *parent )
  : QgsProcessingAbstractParameterDefinitionWidget( context, widgetContext, definition, algorithm, parent )
{
  QVBoxLayout *vlayout = new QVBoxLayout();
  vlayout->setContentsMargins( 0, 0, 0, 0 );

  vlayout->addWidget( new QLabel( tr( "Layer type" ) ) );
  mLayerTypeComboBox = new QgsCheckableComboBox();
  mLayerTypeComboBox->addItem( tr( "Any Map Layer" ), QgsProcessing::TypeMapLayer );
  mLayerTypeComboBox->addItem( tr( "Vector (Point)" ), QgsProcessing::TypeVectorPoint );
  mLayerTypeComboBox->addItem( tr( "Vector (Line)" ), QgsProcessing::TypeVectorLine );
  mLayerTypeComboBox->addItem( tr( "Vector (Polygon)" ), QgsProcessing::TypeVectorPolygon );
  mLayerTypeComboBox->addItem( tr( "Vector (Any Geometry Type)" ), QgsProcessing::TypeVectorAnyGeometry );
  mLayerTypeComboBox->addItem( tr( "Raster" ), QgsProcessing::TypeRaster );
  mLayerTypeComboBox->addItem( tr( "Mesh" ), QgsProcessing::TypeMesh );
  mLayerTypeComboBox->addItem( tr( "Plugin" ), QgsProcessing::TypePlugin );
  mLayerTypeComboBox->addItem( tr( "Point Cloud" ), QgsProcessing::TypePointCloud );
  mLayerTypeComboBox->addItem( tr( "Annotation" ), QgsProcessing::TypeAnnotation );

  if ( const QgsProcessingParameterMapLayer *layerParam = dynamic_cast<const QgsProcessingParameterMapLayer *>( definition ) )
  {
    for ( int i : layerParam->dataTypes() )
    {
      mLayerTypeComboBox->setItemCheckState( mLayerTypeComboBox->findData( i ), Qt::Checked );
    }
  }

  vlayout->addWidget( mLayerTypeComboBox );

  setLayout( vlayout );
}

QgsProcessingParameterDefinition *QgsProcessingMapLayerParameterDefinitionWidget::createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const
{
  QList< int > dataTypes;
  for ( const QVariant &v : mLayerTypeComboBox->checkedItemsData() )
    dataTypes << v.toInt();

  auto param = std::make_unique< QgsProcessingParameterMapLayer >( name, description );
  param->setDataTypes( dataTypes );
  param->setFlags( flags );
  return param.release();
}

QgsProcessingMapLayerWidgetWrapper::QgsProcessingMapLayerWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{

}

QWidget *QgsProcessingMapLayerWidgetWrapper::createWidget()
{
  mComboBox = new QgsProcessingMapLayerComboBox( parameterDefinition(), type() );

  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    case QgsProcessingGui::Batch:
      break;
    case QgsProcessingGui::Modeler:
      mComboBox->setEditable( true );
      break;
  }

  mComboBox->setToolTip( parameterDefinition()->toolTip() );

  connect( mComboBox, &QgsProcessingMapLayerComboBox::valueChanged, this, [ = ]()
  {
    if ( mBlockSignals )
      return;

    emit widgetValueHasChanged( this );
  } );

  setWidgetContext( widgetContext() );
  return mComboBox;
}

void QgsProcessingMapLayerWidgetWrapper::setWidgetContext( const QgsProcessingParameterWidgetContext &context )
{
  QgsAbstractProcessingParameterWidgetWrapper::setWidgetContext( context );
  if ( mComboBox )
  {
    mComboBox->setWidgetContext( context );

    if ( !( parameterDefinition()->flags() & QgsProcessingParameterDefinition::FlagOptional ) )
    {
      // non optional parameter -- if no default value set, default to active layer
      if ( !parameterDefinition()->defaultValueForGui().isValid() )
        mComboBox->setLayer( context.activeLayer() );
    }
  }
}

void QgsProcessingMapLayerWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext &context )
{
  if ( mComboBox )
    mComboBox->setValue( value, context );
}

QVariant QgsProcessingMapLayerWidgetWrapper::widgetValue() const
{
  return mComboBox ? mComboBox->value() : QVariant();
}

QStringList QgsProcessingMapLayerWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList()
         << QgsProcessingParameterRasterLayer::typeName()
         << QgsProcessingParameterMeshLayer::typeName()
         << QgsProcessingParameterVectorLayer::typeName()
         << QgsProcessingParameterMapLayer::typeName()
         << QgsProcessingParameterPointCloudLayer::typeName()
         << QgsProcessingParameterAnnotationLayer::typeName()
         << QgsProcessingParameterString::typeName()
         << QgsProcessingParameterExpression::typeName();
}

QStringList QgsProcessingMapLayerWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList()
         << QgsProcessingOutputString::typeName()
         << QgsProcessingOutputRasterLayer::typeName()
         << QgsProcessingOutputVectorLayer::typeName()
         << QgsProcessingOutputMapLayer::typeName()
         << QgsProcessingOutputFile::typeName();
}

QString QgsProcessingMapLayerWidgetWrapper::modelerExpressionFormatString() const
{
  return tr( "path to a map layer" );
}

QgsProcessingModelChildParameterSource::Source QgsProcessingMapLayerWidgetWrapper::defaultModelSource( const QgsProcessingParameterDefinition * ) const
{
  return QgsProcessingModelChildParameterSource::ModelParameter;
}

QString QgsProcessingMapLayerWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterMapLayer::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingMapLayerWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingMapLayerWidgetWrapper( parameter, type );
}

QgsProcessingAbstractParameterDefinitionWidget *QgsProcessingMapLayerWidgetWrapper::createParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm )
{
  return new QgsProcessingMapLayerParameterDefinitionWidget( context, widgetContext, definition, algorithm );
}


//
// QgsProcessingRasterLayerWidgetWrapper
//

QgsProcessingRasterLayerWidgetWrapper::QgsProcessingRasterLayerWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsProcessingMapLayerWidgetWrapper( parameter, type, parent )
{

}

QStringList QgsProcessingRasterLayerWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList()
         << QgsProcessingParameterRasterLayer::typeName()
         << QgsProcessingParameterMapLayer::typeName()
         << QgsProcessingParameterString::typeName()
         << QgsProcessingParameterExpression::typeName();
}

QStringList QgsProcessingRasterLayerWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList()
         << QgsProcessingOutputString::typeName()
         << QgsProcessingOutputRasterLayer::typeName()
         << QgsProcessingOutputMapLayer::typeName()
         << QgsProcessingOutputFile::typeName()
         << QgsProcessingOutputFolder::typeName();
}

QString QgsProcessingRasterLayerWidgetWrapper::modelerExpressionFormatString() const
{
  return tr( "path to a raster layer" );
}

QString QgsProcessingRasterLayerWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterRasterLayer::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingRasterLayerWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingRasterLayerWidgetWrapper( parameter, type );
}

QgsProcessingAbstractParameterDefinitionWidget *QgsProcessingRasterLayerWidgetWrapper::createParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm )
{
  Q_UNUSED( context );
  Q_UNUSED( widgetContext );
  Q_UNUSED( definition );
  Q_UNUSED( algorithm );

  return nullptr;
}


//
// QgsProcessingVectorLayerWidgetWrapper
//

QgsProcessingVectorLayerParameterDefinitionWidget::QgsProcessingVectorLayerParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm, QWidget *parent )
  : QgsProcessingAbstractParameterDefinitionWidget( context, widgetContext, definition, algorithm, parent )
{
  QVBoxLayout *vlayout = new QVBoxLayout();
  vlayout->setContentsMargins( 0, 0, 0, 0 );

  vlayout->addWidget( new QLabel( tr( "Geometry type" ) ) );
  mGeometryTypeComboBox = new QgsCheckableComboBox();
  mGeometryTypeComboBox->addItem( tr( "Geometry Not Required" ), QgsProcessing::TypeVector );
  mGeometryTypeComboBox->addItem( tr( "Point" ), QgsProcessing::TypeVectorPoint );
  mGeometryTypeComboBox->addItem( tr( "Line" ), QgsProcessing::TypeVectorLine );
  mGeometryTypeComboBox->addItem( tr( "Polygon" ), QgsProcessing::TypeVectorPolygon );
  mGeometryTypeComboBox->addItem( tr( "Any Geometry Type" ), QgsProcessing::TypeVectorAnyGeometry );

  if ( const QgsProcessingParameterVectorLayer *vectorParam = dynamic_cast<const QgsProcessingParameterVectorLayer *>( definition ) )
  {
    for ( int i : vectorParam->dataTypes() )
    {
      mGeometryTypeComboBox->setItemCheckState( mGeometryTypeComboBox->findData( i ), Qt::Checked );
    }
  }

  vlayout->addWidget( mGeometryTypeComboBox );

  setLayout( vlayout );
}

QgsProcessingParameterDefinition *QgsProcessingVectorLayerParameterDefinitionWidget::createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const
{
  QList< int > dataTypes;
  for ( const QVariant &v : mGeometryTypeComboBox->checkedItemsData() )
    dataTypes << v.toInt();

  auto param = std::make_unique< QgsProcessingParameterVectorLayer >( name, description, dataTypes );
  param->setFlags( flags );
  return param.release();
}


QgsProcessingVectorLayerWidgetWrapper::QgsProcessingVectorLayerWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsProcessingMapLayerWidgetWrapper( parameter, type, parent )
{

}

QStringList QgsProcessingVectorLayerWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList()
         << QgsProcessingParameterVectorLayer::typeName()
         << QgsProcessingParameterMapLayer::typeName()
         << QgsProcessingParameterString::typeName()
         << QgsProcessingParameterExpression::typeName();
}

QStringList QgsProcessingVectorLayerWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList()
         << QgsProcessingOutputString::typeName()
         << QgsProcessingOutputVectorLayer::typeName()
         << QgsProcessingOutputMapLayer::typeName()
         << QgsProcessingOutputFile::typeName()
         << QgsProcessingOutputFolder::typeName();
}

QString QgsProcessingVectorLayerWidgetWrapper::modelerExpressionFormatString() const
{
  return tr( "path to a vector layer" );
}

QList<int> QgsProcessingVectorLayerWidgetWrapper::compatibleDataTypes( const QgsProcessingParameterDefinition *parameter ) const
{
  if ( const QgsProcessingParameterVectorLayer *param = dynamic_cast< const QgsProcessingParameterVectorLayer *>( parameter ) )
    return param->dataTypes();
  else
    return QList< int >();
}

QString QgsProcessingVectorLayerWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterVectorLayer::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingVectorLayerWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingVectorLayerWidgetWrapper( parameter, type );
}

QgsProcessingAbstractParameterDefinitionWidget *QgsProcessingVectorLayerWidgetWrapper::createParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm )
{
  return new QgsProcessingVectorLayerParameterDefinitionWidget( context, widgetContext, definition, algorithm );
}



//
// QgsProcessingFeatureSourceLayerWidgetWrapper
//

QgsProcessingFeatureSourceParameterDefinitionWidget::QgsProcessingFeatureSourceParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm, QWidget *parent )
  : QgsProcessingAbstractParameterDefinitionWidget( context, widgetContext, definition, algorithm, parent )
{
  QVBoxLayout *vlayout = new QVBoxLayout();
  vlayout->setContentsMargins( 0, 0, 0, 0 );

  vlayout->addWidget( new QLabel( tr( "Geometry type" ) ) );
  mGeometryTypeComboBox = new QgsCheckableComboBox();
  mGeometryTypeComboBox->addItem( tr( "Geometry Not Required" ), QgsProcessing::TypeVector );
  mGeometryTypeComboBox->addItem( tr( "Point" ), QgsProcessing::TypeVectorPoint );
  mGeometryTypeComboBox->addItem( tr( "Line" ), QgsProcessing::TypeVectorLine );
  mGeometryTypeComboBox->addItem( tr( "Polygon" ), QgsProcessing::TypeVectorPolygon );
  mGeometryTypeComboBox->addItem( tr( "Any Geometry Type" ), QgsProcessing::TypeVectorAnyGeometry );

  if ( const QgsProcessingParameterFeatureSource *sourceParam = dynamic_cast<const QgsProcessingParameterFeatureSource *>( definition ) )
  {
    for ( int i : sourceParam->dataTypes() )
    {
      mGeometryTypeComboBox->setItemCheckState( mGeometryTypeComboBox->findData( i ), Qt::Checked );
    }
  }
  else
  {
    mGeometryTypeComboBox->setItemCheckState( mGeometryTypeComboBox->findData( QgsProcessing::TypeVectorAnyGeometry ), Qt::Checked );
  }

  vlayout->addWidget( mGeometryTypeComboBox );

  setLayout( vlayout );
}

QgsProcessingParameterDefinition *QgsProcessingFeatureSourceParameterDefinitionWidget::createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const
{
  QList< int > dataTypes;
  for ( const QVariant &v : mGeometryTypeComboBox->checkedItemsData() )
    dataTypes << v.toInt();

  auto param = std::make_unique< QgsProcessingParameterFeatureSource >( name, description, dataTypes );
  param->setFlags( flags );
  return param.release();
}

QgsProcessingFeatureSourceWidgetWrapper::QgsProcessingFeatureSourceWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsProcessingMapLayerWidgetWrapper( parameter, type, parent )
{

}

QStringList QgsProcessingFeatureSourceWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList()
         << QgsProcessingParameterFeatureSource::typeName()
         << QgsProcessingParameterVectorLayer::typeName()
         << QgsProcessingParameterMapLayer::typeName()
         << QgsProcessingParameterString::typeName()
         << QgsProcessingParameterExpression::typeName();
}

QStringList QgsProcessingFeatureSourceWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList()
         << QgsProcessingOutputString::typeName()
         << QgsProcessingOutputVectorLayer::typeName()
         << QgsProcessingOutputMapLayer::typeName()
         << QgsProcessingOutputFile::typeName()
         << QgsProcessingOutputFolder::typeName();
}

QString QgsProcessingFeatureSourceWidgetWrapper::modelerExpressionFormatString() const
{
  return tr( "path to a vector layer" );
}

QList<int> QgsProcessingFeatureSourceWidgetWrapper::compatibleDataTypes( const QgsProcessingParameterDefinition *parameter ) const
{
  if ( const QgsProcessingParameterFeatureSource *param = dynamic_cast< const QgsProcessingParameterFeatureSource *>( parameter ) )
    return param->dataTypes();
  else
    return QList< int >();
}

QString QgsProcessingFeatureSourceWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterFeatureSource::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingFeatureSourceWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingFeatureSourceWidgetWrapper( parameter, type );
}

QgsProcessingAbstractParameterDefinitionWidget *QgsProcessingFeatureSourceWidgetWrapper::createParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm )
{
  return new QgsProcessingFeatureSourceParameterDefinitionWidget( context, widgetContext, definition, algorithm );
}

//
// QgsProcessingMeshLayerWidgetWrapper
//

QgsProcessingMeshLayerWidgetWrapper::QgsProcessingMeshLayerWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsProcessingMapLayerWidgetWrapper( parameter, type, parent )
{

}

QStringList QgsProcessingMeshLayerWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList()
         << QgsProcessingParameterMeshLayer::typeName()
         << QgsProcessingParameterMapLayer::typeName()
         << QgsProcessingParameterString::typeName()
         << QgsProcessingParameterExpression::typeName();
}

QStringList QgsProcessingMeshLayerWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList()
         << QgsProcessingOutputString::typeName()
         // TODO  << QgsProcessingOutputMeshLayer::typeName()
         << QgsProcessingOutputMapLayer::typeName()
         << QgsProcessingOutputFile::typeName()
         << QgsProcessingOutputFolder::typeName();
}

QString QgsProcessingMeshLayerWidgetWrapper::modelerExpressionFormatString() const
{
  return tr( "path to a mesh layer" );
}

QString QgsProcessingMeshLayerWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterMeshLayer::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingMeshLayerWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingMeshLayerWidgetWrapper( parameter, type );
}

QgsProcessingAbstractParameterDefinitionWidget *QgsProcessingMeshLayerWidgetWrapper::createParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm )
{
  Q_UNUSED( context );
  Q_UNUSED( widgetContext );
  Q_UNUSED( definition );
  Q_UNUSED( algorithm );

  return nullptr;
}



//
// QgsProcessingRasterBandPanelWidget
//

QgsProcessingRasterBandPanelWidget::QgsProcessingRasterBandPanelWidget( QWidget *parent, const QgsProcessingParameterBand *param )
  : QWidget( parent )
  , mParam( param )
{
  QHBoxLayout *hl = new QHBoxLayout();
  hl->setContentsMargins( 0, 0, 0, 0 );

  mLineEdit = new QLineEdit();
  mLineEdit->setEnabled( false );
  hl->addWidget( mLineEdit, 1 );

  mToolButton = new QToolButton();
  mToolButton->setText( QString( QChar( 0x2026 ) ) );
  hl->addWidget( mToolButton );

  setLayout( hl );

  if ( mParam )
  {
    mLineEdit->setText( tr( "%n band(s) selected", nullptr, 0 ) );
  }

  connect( mToolButton, &QToolButton::clicked, this, &QgsProcessingRasterBandPanelWidget::showDialog );
}

void QgsProcessingRasterBandPanelWidget::setBands( const QList< int > &bands )
{
  mBands = bands;
}

void QgsProcessingRasterBandPanelWidget::setBandNames( const QHash<int, QString> &names )
{
  mBandNames = names;
}

void QgsProcessingRasterBandPanelWidget::setValue( const QVariant &value )
{
  if ( value.isValid() )
    mValue = value.type() == QVariant::List ? value.toList() : QVariantList() << value;
  else
    mValue.clear();

  updateSummaryText();
  emit changed();
}

void QgsProcessingRasterBandPanelWidget::showDialog()
{
  QVariantList availableOptions;
  QStringList fieldNames;
  availableOptions.reserve( mBands.size() );
  for ( int band : std::as_const( mBands ) )
  {
    availableOptions << band;
  }

  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );
  if ( panel && panel->dockMode() )
  {
    QgsProcessingMultipleSelectionPanelWidget *widget = new QgsProcessingMultipleSelectionPanelWidget( availableOptions, mValue );
    widget->setPanelTitle( mParam->description() );

    widget->setValueFormatter( [this]( const QVariant & v ) -> QString
    {
      int band = v.toInt();
      return mBandNames.contains( band ) ? mBandNames.value( band ) : v.toString();
    } );

    connect( widget, &QgsProcessingMultipleSelectionPanelWidget::selectionChanged, this, [ = ]()
    {
      setValue( widget->selectedOptions() );
    } );
    connect( widget, &QgsProcessingMultipleSelectionPanelWidget::acceptClicked, widget, &QgsPanelWidget::acceptPanel );
    panel->openPanel( widget );
  }
  else
  {
    QgsProcessingMultipleSelectionDialog dlg( availableOptions, mValue, this, Qt::WindowFlags() );

    dlg.setValueFormatter( [this]( const QVariant & v ) -> QString
    {
      int band = v.toInt();
      return mBandNames.contains( band ) ? mBandNames.value( band ) : v.toString();
    } );
    if ( dlg.exec() )
    {
      setValue( dlg.selectedOptions() );
    }
  }
}

void QgsProcessingRasterBandPanelWidget::updateSummaryText()
{
  if ( mParam )
    mLineEdit->setText( tr( "%n band(s) selected", nullptr, mValue.count() ) );
}



//
// QgsProcessingBandWidgetWrapper
//

QgsProcessingBandParameterDefinitionWidget::QgsProcessingBandParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm, QWidget *parent )
  : QgsProcessingAbstractParameterDefinitionWidget( context, widgetContext, definition, algorithm, parent )
{
  QVBoxLayout *vlayout = new QVBoxLayout();
  vlayout->setContentsMargins( 0, 0, 0, 0 );

  vlayout->addWidget( new QLabel( tr( "Default value" ) ) );

  mDefaultLineEdit = new QLineEdit();
  mDefaultLineEdit->setToolTip( tr( "Band number (separate bands with ; for multiple band parameters)" ) );
  if ( const QgsProcessingParameterBand *bandParam = dynamic_cast<const QgsProcessingParameterBand *>( definition ) )
  {
    const QList< int > bands = QgsProcessingParameters::parameterAsInts( bandParam, bandParam->defaultValueForGui(), context );
    QStringList defVal;
    for ( int b : bands )
    {
      defVal << QString::number( b );
    }

    mDefaultLineEdit->setText( defVal.join( ';' ) );
  }
  vlayout->addWidget( mDefaultLineEdit );

  vlayout->addWidget( new QLabel( tr( "Parent layer" ) ) );
  mParentLayerComboBox = new QComboBox();

  QString initialParent;
  if ( const QgsProcessingParameterBand *bandParam = dynamic_cast<const QgsProcessingParameterBand *>( definition ) )
    initialParent = bandParam->parentLayerParameterName();

  if ( auto *lModel = widgetContext.model() )
  {
    // populate combo box with other model input choices
    const QMap<QString, QgsProcessingModelParameter> components = lModel->parameterComponents();
    for ( auto it = components.constBegin(); it != components.constEnd(); ++it )
    {
      if ( const QgsProcessingParameterRasterLayer *definition = dynamic_cast< const QgsProcessingParameterRasterLayer * >( lModel->parameterDefinition( it.value().parameterName() ) ) )
      {
        mParentLayerComboBox-> addItem( definition->description(), definition->name() );
        if ( !initialParent.isEmpty() && initialParent == definition->name() )
        {
          mParentLayerComboBox->setCurrentIndex( mParentLayerComboBox->count() - 1 );
        }
      }
    }
  }

  if ( mParentLayerComboBox->count() == 0 && !initialParent.isEmpty() )
  {
    // if no parent candidates found, we just add the existing one as a placeholder
    mParentLayerComboBox->addItem( initialParent, initialParent );
    mParentLayerComboBox->setCurrentIndex( mParentLayerComboBox->count() - 1 );
  }

  vlayout->addWidget( mParentLayerComboBox );

  mAllowMultipleCheckBox = new QCheckBox( tr( "Allow multiple" ) );
  if ( const QgsProcessingParameterBand *bandParam = dynamic_cast<const QgsProcessingParameterBand *>( definition ) )
    mAllowMultipleCheckBox->setChecked( bandParam->allowMultiple() );

  vlayout->addWidget( mAllowMultipleCheckBox );
  setLayout( vlayout );
}

QgsProcessingParameterDefinition *QgsProcessingBandParameterDefinitionWidget::createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const
{
  auto param = std::make_unique< QgsProcessingParameterBand >( name, description, mDefaultLineEdit->text().split( ';' ), mParentLayerComboBox->currentData().toString(), false, mAllowMultipleCheckBox->isChecked() );
  param->setFlags( flags );
  return param.release();
}

QgsProcessingBandWidgetWrapper::QgsProcessingBandWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{

}

QWidget *QgsProcessingBandWidgetWrapper::createWidget()
{
  const QgsProcessingParameterBand *bandParam = dynamic_cast< const QgsProcessingParameterBand *>( parameterDefinition() );
  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    case QgsProcessingGui::Batch:
    {
      if ( bandParam->allowMultiple() )
      {
        mPanel = new QgsProcessingRasterBandPanelWidget( nullptr, bandParam );
        mPanel->setToolTip( parameterDefinition()->toolTip() );
        connect( mPanel, &QgsProcessingRasterBandPanelWidget::changed, this, [ = ]
        {
          emit widgetValueHasChanged( this );
        } );
        return mPanel;
      }
      else
      {
        mComboBox = new QgsRasterBandComboBox();
        mComboBox->setShowNotSetOption( bandParam->flags() & QgsProcessingParameterDefinition::FlagOptional );

        mComboBox->setToolTip( parameterDefinition()->toolTip() );
        connect( mComboBox, &QgsRasterBandComboBox::bandChanged, this, [ = ]( int )
        {
          emit widgetValueHasChanged( this );
        } );
        return mComboBox;
      }
    }

    case QgsProcessingGui::Modeler:
    {
      mLineEdit = new QLineEdit();
      mLineEdit->setToolTip( QObject::tr( "Band number (separate bands with ; for multiple band parameters)" ) );
      connect( mLineEdit, &QLineEdit::textChanged, this, [ = ]
      {
        emit widgetValueHasChanged( this );
      } );
      return mLineEdit;
    }

  }
  return nullptr;
}

void QgsProcessingBandWidgetWrapper::postInitialize( const QList<QgsAbstractProcessingParameterWidgetWrapper *> &wrappers )
{
  QgsAbstractProcessingParameterWidgetWrapper::postInitialize( wrappers );
  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    case QgsProcessingGui::Batch:
    {
      for ( const QgsAbstractProcessingParameterWidgetWrapper *wrapper : wrappers )
      {
        if ( wrapper->parameterDefinition()->name() == static_cast< const QgsProcessingParameterBand * >( parameterDefinition() )->parentLayerParameterName() )
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

void QgsProcessingBandWidgetWrapper::setParentLayerWrapperValue( const QgsAbstractProcessingParameterWidgetWrapper *parentWrapper )
{
  // evaluate value to layer
  QgsProcessingContext *context = nullptr;
  std::unique_ptr< QgsProcessingContext > tmpContext;
  if ( mProcessingContextGenerator )
    context = mProcessingContextGenerator->processingContext();

  if ( !context )
  {
    tmpContext = std::make_unique< QgsProcessingContext >();
    context = tmpContext.get();
  }

  QVariant value = parentWrapper->parameterValue();

  QgsRasterLayer *layer = QgsProcessingParameters::parameterAsRasterLayer( parentWrapper->parameterDefinition(), value, *context );
  if ( layer && layer->isValid() )
  {
    // need to grab ownership of layer if required - otherwise layer may be deleted when context
    // goes out of scope
    std::unique_ptr< QgsMapLayer > ownedLayer( context->takeResultLayer( layer->id() ) );
    if ( ownedLayer && ownedLayer->type() == QgsMapLayerType::RasterLayer )
    {
      mParentLayer.reset( qobject_cast< QgsRasterLayer * >( ownedLayer.release() ) );
      layer = mParentLayer.get();
    }
    else
    {
      // don't need ownership of this layer - it wasn't owned by context (so e.g. is owned by the project)
    }

    if ( mComboBox )
      mComboBox->setLayer( layer );
    else if ( mPanel )
    {
      QgsRasterDataProvider *provider = layer->dataProvider();
      if ( provider && layer->isValid() )
      {
        //fill available bands
        int nBands = provider->bandCount();
        QList< int > bands;
        QHash< int, QString > bandNames;
        for ( int i = 1; i <= nBands; ++i )
        {
          bandNames.insert( i, QgsRasterBandComboBox::displayBandName( provider, i ) );
          bands << i;
        }
        mPanel->setBands( bands );
        mPanel->setBandNames( bandNames );
      }
    }
  }
  else
  {
    if ( mComboBox )
      mComboBox->setLayer( nullptr );
    else if ( mPanel )
      mPanel->setBands( QList< int >() );

    if ( value.isValid() && widgetContext().messageBar() )
    {
      widgetContext().messageBar()->clearWidgets();
      widgetContext().messageBar()->pushMessage( QString(), QObject::tr( "Could not load selected layer/table. Dependent bands could not be populated" ),
          Qgis::MessageLevel::Info );
    }
  }

  if ( parameterDefinition()->defaultValueForGui().isValid() )
    setWidgetValue( parameterDefinition()->defaultValueForGui(), *context );
}

void QgsProcessingBandWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext &context )
{
  if ( mComboBox )
  {
    if ( !value.isValid() )
      mComboBox->setBand( -1 );
    else
    {
      const int v = QgsProcessingParameters::parameterAsInt( parameterDefinition(), value, context );
      mComboBox->setBand( v );
    }
  }
  else if ( mPanel )
  {
    QVariantList opts;
    if ( value.isValid() )
    {
      const QList< int > v = QgsProcessingParameters::parameterAsInts( parameterDefinition(), value, context );
      opts.reserve( v.size() );
      for ( int i : v )
        opts << i;
    }
    if ( mPanel )
      mPanel->setValue( value.isValid() ? opts : QVariant() );
  }
  else if ( mLineEdit )
  {
    const QgsProcessingParameterBand *bandParam = static_cast< const QgsProcessingParameterBand * >( parameterDefinition() );
    if ( bandParam->allowMultiple() )
    {
      const QList< int > v = QgsProcessingParameters::parameterAsInts( parameterDefinition(), value, context );
      QStringList opts;
      opts.reserve( v.size() );
      for ( int i : v )
        opts << QString::number( i );
      mLineEdit->setText( value.isValid() && !opts.empty() ? opts.join( ';' ) : QString() );
    }
    else
    {
      if ( value.isValid() )
        mLineEdit->setText( QString::number( QgsProcessingParameters::parameterAsInt( parameterDefinition(), value, context ) ) );
      else
        mLineEdit->clear();
    }
  }
}

QVariant QgsProcessingBandWidgetWrapper::widgetValue() const
{
  if ( mComboBox )
    return mComboBox->currentBand() == -1 ? QVariant() : mComboBox->currentBand();
  else if ( mPanel )
    return !mPanel->value().toList().isEmpty() ? mPanel->value() : QVariant();
  else if ( mLineEdit )
  {
    const QgsProcessingParameterBand *bandParam = static_cast< const QgsProcessingParameterBand * >( parameterDefinition() );
    if ( bandParam->allowMultiple() )
    {
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
      const QStringList parts = mLineEdit->text().split( ';', QString::SkipEmptyParts );
#else
      const QStringList parts = mLineEdit->text().split( ';', Qt::SkipEmptyParts );
#endif
      QVariantList res;
      res.reserve( parts.count() );
      for ( const QString &s : parts )
      {
        bool ok = false;
        int band = s.toInt( &ok );
        if ( ok )
          res << band;
      }
      return res.isEmpty() ? QVariant() : res;
    }
    else
    {
      return mLineEdit->text().isEmpty() ? QVariant() : mLineEdit->text();
    }
  }
  else
    return QVariant();
}

QStringList QgsProcessingBandWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList()
         << QgsProcessingParameterBand::typeName()
         << QgsProcessingParameterNumber::typeName();
}

QStringList QgsProcessingBandWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList()
         << QgsProcessingOutputNumber::typeName();
}

QString QgsProcessingBandWidgetWrapper::modelerExpressionFormatString() const
{
  return tr( "selected band numbers as an array of numbers, or semicolon separated string of options (e.g. '1;3')" );
}

QString QgsProcessingBandWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterBand::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingBandWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingBandWidgetWrapper( parameter, type );
}

QgsProcessingAbstractParameterDefinitionWidget *QgsProcessingBandWidgetWrapper::createParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm )
{
  return new QgsProcessingBandParameterDefinitionWidget( context, widgetContext, definition, algorithm );
}



//
// QgsProcessingMultipleLayerPanelWidget
//

QgsProcessingMultipleLayerPanelWidget::QgsProcessingMultipleLayerPanelWidget( QWidget *parent, const QgsProcessingParameterMultipleLayers *param )
  : QWidget( parent )
  , mParam( param )
{
  QHBoxLayout *hl = new QHBoxLayout();
  hl->setContentsMargins( 0, 0, 0, 0 );

  mLineEdit = new QLineEdit();
  mLineEdit->setEnabled( false );
  hl->addWidget( mLineEdit, 1 );

  mToolButton = new QToolButton();
  mToolButton->setText( QString( QChar( 0x2026 ) ) );
  hl->addWidget( mToolButton );

  setLayout( hl );

  if ( mParam )
  {
    mLineEdit->setText( tr( "%n input(s) selected", nullptr, 0 ) );
  }

  connect( mToolButton, &QToolButton::clicked, this, &QgsProcessingMultipleLayerPanelWidget::showDialog );
}

void QgsProcessingMultipleLayerPanelWidget::setValue( const QVariant &value )
{
  if ( value.isValid() )
    mValue = value.type() == QVariant::List ? value.toList() : QVariantList() << value;
  else
    mValue.clear();

  updateSummaryText();
  emit changed();
}

void QgsProcessingMultipleLayerPanelWidget::setProject( QgsProject *project )
{
  mProject = project;
  if ( mProject )
  {
    connect( mProject, &QgsProject::layerRemoved, this, [&]( const QString & layerId )
    {
      if ( mValue.removeAll( layerId ) )
      {
        updateSummaryText();
        emit changed();
      }
    } );
  }
}

void QgsProcessingMultipleLayerPanelWidget::setModel( QgsProcessingModelAlgorithm *model, const QString &modelChildAlgorithmID )
{
  mModel = model;
  if ( !model )
    return;

  switch ( mParam->layerType() )
  {
    case QgsProcessing::TypeFile:
      mModelSources = model->availableSourcesForChild( modelChildAlgorithmID, QStringList() << QgsProcessingParameterFile::typeName(),
                      QStringList() <<  QgsProcessingOutputFile::typeName() );
      break;

    case QgsProcessing::TypeRaster:
    {
      mModelSources = model->availableSourcesForChild( modelChildAlgorithmID, QStringList() << QgsProcessingParameterRasterLayer::typeName()
                      << QgsProcessingParameterMultipleLayers::typeName()
                      << QgsProcessingParameterFile::typeName(),
                      QStringList() << QgsProcessingOutputFile::typeName()
                      << QgsProcessingOutputRasterLayer::typeName()
                      << QgsProcessingOutputMapLayer::typeName()
                      << QgsProcessingOutputMultipleLayers::typeName() );
      break;
    }

    case QgsProcessing::TypeMesh:
    {
      mModelSources = model->availableSourcesForChild( modelChildAlgorithmID, QStringList() << QgsProcessingParameterMeshLayer::typeName()
                      << QgsProcessingParameterMultipleLayers::typeName()
                      << QgsProcessingParameterFile::typeName(),
                      QStringList() << QgsProcessingOutputFile::typeName()
                      << QgsProcessingOutputMapLayer::typeName()
                      << QgsProcessingOutputMultipleLayers::typeName() );
      break;
    }

    case QgsProcessing::TypePlugin:
    {
      mModelSources = model->availableSourcesForChild( modelChildAlgorithmID, QStringList() << QgsProcessingParameterMapLayer::typeName()
                      << QgsProcessingParameterMultipleLayers::typeName()
                      << QgsProcessingParameterFile::typeName(),
                      QStringList() << QgsProcessingOutputFile::typeName()
                      << QgsProcessingOutputMapLayer::typeName()
                      << QgsProcessingOutputMultipleLayers::typeName() );
      break;
    }

    case QgsProcessing::TypePointCloud:
    {
      mModelSources = model->availableSourcesForChild( modelChildAlgorithmID, QStringList() << QgsProcessingParameterPointCloudLayer::typeName()
                      << QgsProcessingParameterMultipleLayers::typeName()
                      << QgsProcessingParameterFile::typeName(),
                      QStringList() << QgsProcessingOutputFile::typeName()
                      << QgsProcessingOutputMapLayer::typeName()
                      << QgsProcessingOutputMultipleLayers::typeName() );
      break;
    }

    case QgsProcessing::TypeAnnotation:
    {
      mModelSources = model->availableSourcesForChild( modelChildAlgorithmID, QStringList() << QgsProcessingParameterAnnotationLayer::typeName()
                      << QgsProcessingParameterMultipleLayers::typeName(),
                      QStringList() << QgsProcessingOutputMapLayer::typeName()
                      << QgsProcessingOutputMultipleLayers::typeName() );
      break;
    }

    case QgsProcessing::TypeVector:
    {
      mModelSources = model->availableSourcesForChild( modelChildAlgorithmID, QStringList() << QgsProcessingParameterFeatureSource::typeName()
                      << QgsProcessingParameterVectorLayer::typeName()
                      << QgsProcessingParameterFile::typeName()
                      << QgsProcessingParameterMultipleLayers::typeName(),
                      QStringList() << QgsProcessingOutputFile::typeName()
                      << QgsProcessingOutputVectorLayer::typeName()
                      << QgsProcessingOutputMapLayer::typeName()
                      << QgsProcessingOutputMultipleLayers::typeName(),
                      QList< int >() << QgsProcessing::TypeVector );
      break;
    }

    case QgsProcessing::TypeVectorAnyGeometry:
    {
      mModelSources = model->availableSourcesForChild( modelChildAlgorithmID, QStringList() << QgsProcessingParameterFeatureSource::typeName()
                      << QgsProcessingParameterVectorLayer::typeName()
                      << QgsProcessingParameterFile::typeName()
                      << QgsProcessingParameterMultipleLayers::typeName(),
                      QStringList() << QgsProcessingOutputFile::typeName()
                      << QgsProcessingOutputVectorLayer::typeName()
                      << QgsProcessingOutputMapLayer::typeName()
                      << QgsProcessingOutputMultipleLayers::typeName() );
      break;
    }

    case QgsProcessing::TypeVectorPoint:
    {
      mModelSources = model->availableSourcesForChild( modelChildAlgorithmID, QStringList() << QgsProcessingParameterFeatureSource::typeName()
                      << QgsProcessingParameterVectorLayer::typeName()
                      << QgsProcessingParameterFile::typeName()
                      << QgsProcessingParameterMultipleLayers::typeName(),
                      QStringList() << QgsProcessingOutputFile::typeName()
                      << QgsProcessingOutputVectorLayer::typeName()
                      << QgsProcessingOutputMapLayer::typeName()
                      << QgsProcessingOutputMultipleLayers::typeName(),
                      QList< int >() << QgsProcessing::TypeVectorAnyGeometry << QgsProcessing::TypeVectorPoint );
      break;
    }

    case QgsProcessing::TypeVectorLine:
    {
      mModelSources = model->availableSourcesForChild( modelChildAlgorithmID, QStringList() << QgsProcessingParameterFeatureSource::typeName()
                      << QgsProcessingParameterVectorLayer::typeName()
                      << QgsProcessingParameterFile::typeName()
                      << QgsProcessingParameterMultipleLayers::typeName(),
                      QStringList() << QgsProcessingOutputFile::typeName()
                      << QgsProcessingOutputVectorLayer::typeName()
                      << QgsProcessingOutputMapLayer::typeName()
                      << QgsProcessingOutputMultipleLayers::typeName(),
                      QList< int >() << QgsProcessing::TypeVectorAnyGeometry << QgsProcessing::TypeVectorLine );
      break;
    }

    case QgsProcessing::TypeVectorPolygon:
    {
      mModelSources = model->availableSourcesForChild( modelChildAlgorithmID, QStringList() << QgsProcessingParameterFeatureSource::typeName()
                      << QgsProcessingParameterVectorLayer::typeName()
                      << QgsProcessingParameterFile::typeName()
                      << QgsProcessingParameterMultipleLayers::typeName(),
                      QStringList() << QgsProcessingOutputFile::typeName()
                      << QgsProcessingOutputVectorLayer::typeName()
                      << QgsProcessingOutputMultipleLayers::typeName()
                      << QgsProcessingOutputMapLayer::typeName(),
                      QList< int >() << QgsProcessing::TypeVectorAnyGeometry << QgsProcessing::TypeVectorPolygon );
      break;
    }

    case QgsProcessing::TypeMapLayer:
    {
      mModelSources = model->availableSourcesForChild( modelChildAlgorithmID, QStringList() << QgsProcessingParameterFeatureSource::typeName()
                      << QgsProcessingParameterVectorLayer::typeName()
                      << QgsProcessingParameterRasterLayer::typeName()
                      << QgsProcessingParameterMeshLayer::typeName()
                      << QgsProcessingParameterFile::typeName()
                      << QgsProcessingParameterMultipleLayers::typeName(),
                      QStringList() << QgsProcessingOutputFile::typeName()
                      << QgsProcessingOutputMapLayer::typeName()
                      << QgsProcessingOutputVectorLayer::typeName()
                      << QgsProcessingOutputRasterLayer::typeName()
                      // << QgsProcessingOutputMeshLayer::typeName()
                      << QgsProcessingOutputMultipleLayers::typeName() );
      break;
    }
  }
}

void QgsProcessingMultipleLayerPanelWidget::showDialog()
{
  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );
  if ( panel && panel->dockMode() )
  {
    QgsProcessingMultipleInputPanelWidget *widget = new QgsProcessingMultipleInputPanelWidget( mParam, mValue, mModelSources, mModel );
    widget->setPanelTitle( mParam->description() );
    widget->setProject( mProject );
    connect( widget, &QgsProcessingMultipleSelectionPanelWidget::selectionChanged, this, [ = ]()
    {
      setValue( widget->selectedOptions() );
    } );
    connect( widget, &QgsProcessingMultipleSelectionPanelWidget::acceptClicked, widget, &QgsPanelWidget::acceptPanel );
    panel->openPanel( widget );
  }
  else
  {
    QgsProcessingMultipleInputDialog dlg( mParam, mValue, mModelSources, mModel, this, Qt::WindowFlags() );
    dlg.setProject( mProject );
    if ( dlg.exec() )
    {
      setValue( dlg.selectedOptions() );
    }
  }
}

void QgsProcessingMultipleLayerPanelWidget::updateSummaryText()
{
  if ( mParam )
    mLineEdit->setText( tr( "%n input(s) selected", nullptr, mValue.count() ) );
}

//
// QgsProcessingMultipleLayerWidgetWrapper
//

QgsProcessingMultipleLayerParameterDefinitionWidget::QgsProcessingMultipleLayerParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm, QWidget *parent )
  : QgsProcessingAbstractParameterDefinitionWidget( context, widgetContext, definition, algorithm, parent )
{
  QVBoxLayout *vlayout = new QVBoxLayout();
  vlayout->setContentsMargins( 0, 0, 0, 0 );

  vlayout->addWidget( new QLabel( tr( "Allowed layer type" ) ) );
  mLayerTypeComboBox = new QComboBox();
  mLayerTypeComboBox->addItem( tr( "Any Map Layer" ), QgsProcessing::TypeMapLayer );
  mLayerTypeComboBox->addItem( tr( "Vector (No Geometry Required)" ), QgsProcessing::TypeVector );
  mLayerTypeComboBox->addItem( tr( "Vector (Point)" ), QgsProcessing::TypeVectorPoint );
  mLayerTypeComboBox->addItem( tr( "Vector (Line)" ), QgsProcessing::TypeVectorLine );
  mLayerTypeComboBox->addItem( tr( "Vector (Polygon)" ), QgsProcessing::TypeVectorPolygon );
  mLayerTypeComboBox->addItem( tr( "Any Geometry Type" ), QgsProcessing::TypeVectorAnyGeometry );
  mLayerTypeComboBox->addItem( tr( "Raster" ), QgsProcessing::TypeRaster );
  mLayerTypeComboBox->addItem( tr( "File" ), QgsProcessing::TypeFile );
  mLayerTypeComboBox->addItem( tr( "Mesh" ), QgsProcessing::TypeMesh );
  mLayerTypeComboBox->addItem( tr( "Plugin" ), QgsProcessing::TypePlugin );
  mLayerTypeComboBox->addItem( tr( "Point Cloud" ), QgsProcessing::TypePointCloud );
  mLayerTypeComboBox->addItem( tr( "Annotation" ), QgsProcessing::TypeAnnotation );
  if ( const QgsProcessingParameterMultipleLayers *layersParam = dynamic_cast<const QgsProcessingParameterMultipleLayers *>( definition ) )
    mLayerTypeComboBox->setCurrentIndex( mLayerTypeComboBox->findData( layersParam->layerType() ) );

  vlayout->addWidget( mLayerTypeComboBox );
  setLayout( vlayout );
}

QgsProcessingParameterDefinition *QgsProcessingMultipleLayerParameterDefinitionWidget::createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const
{
  auto param = std::make_unique< QgsProcessingParameterMultipleLayers >( name, description, static_cast< QgsProcessing::SourceType >( mLayerTypeComboBox->currentData().toInt() ) );
  param->setFlags( flags );
  return param.release();
}

QgsProcessingMultipleLayerWidgetWrapper::QgsProcessingMultipleLayerWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{

}

QWidget *QgsProcessingMultipleLayerWidgetWrapper::createWidget()
{
  const QgsProcessingParameterMultipleLayers *layerParam = dynamic_cast< const QgsProcessingParameterMultipleLayers *>( parameterDefinition() );

  mPanel = new QgsProcessingMultipleLayerPanelWidget( nullptr, layerParam );
  mPanel->setToolTip( parameterDefinition()->toolTip() );
  mPanel->setProject( widgetContext().project() );
  if ( type() == QgsProcessingGui::Modeler )
    mPanel->setModel( widgetContext().model(), widgetContext().modelChildAlgorithmId() );
  connect( mPanel, &QgsProcessingMultipleLayerPanelWidget::changed, this, [ = ]
  {
    emit widgetValueHasChanged( this );
  } );
  return mPanel;
}

void QgsProcessingMultipleLayerWidgetWrapper::setWidgetContext( const QgsProcessingParameterWidgetContext &context )
{
  QgsAbstractProcessingParameterWidgetWrapper::setWidgetContext( context );
  if ( mPanel )
  {
    mPanel->setProject( context.project() );
    if ( type() == QgsProcessingGui::Modeler )
      mPanel->setModel( widgetContext().model(), widgetContext().modelChildAlgorithmId() );
  }
}

void QgsProcessingMultipleLayerWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext &context )
{
  if ( mPanel )
  {
    QVariantList opts;
    if ( value.isValid() )
    {
      const QList< QgsMapLayer * > v = QgsProcessingParameters::parameterAsLayerList( parameterDefinition(), value, context );
      opts.reserve( v.size() );
      for ( const QgsMapLayer *l : v )
        opts << l->source();
    }

    for ( const QVariant &v : value.toList() )
    {
      if ( v.canConvert< QgsProcessingModelChildParameterSource >() )
      {
        const QgsProcessingModelChildParameterSource source = v.value< QgsProcessingModelChildParameterSource >();
        opts << QVariant::fromValue( source );
      }
    }

    if ( mPanel )
      mPanel->setValue( value.isValid() ? opts : QVariant() );
  }
}

QVariant QgsProcessingMultipleLayerWidgetWrapper::widgetValue() const
{
  if ( mPanel )
    return !mPanel->value().toList().isEmpty() ? mPanel->value() : QVariant();
  else
    return QVariant();
}

QStringList QgsProcessingMultipleLayerWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList()
         << QgsProcessingParameterMultipleLayers::typeName()
         << QgsProcessingParameterMapLayer::typeName()
         << QgsProcessingParameterVectorLayer::typeName()
         << QgsProcessingParameterMeshLayer::typeName()
         << QgsProcessingParameterFeatureSource::typeName()
         << QgsProcessingParameterRasterLayer::typeName()
         << QgsProcessingParameterFile::typeName()
         << QgsProcessingParameterString::typeName();
}

QStringList QgsProcessingMultipleLayerWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList()
         << QgsProcessingOutputMapLayer::typeName()
         << QgsProcessingOutputRasterLayer::typeName()
         << QgsProcessingOutputVectorLayer::typeName()
         << QgsProcessingOutputMultipleLayers::typeName()
         << QgsProcessingOutputFile::typeName()
         << QgsProcessingOutputString::typeName();
}

QString QgsProcessingMultipleLayerWidgetWrapper::modelerExpressionFormatString() const
{
  return tr( "an array of layer paths, or semicolon separated string of layer paths" );
}

QString QgsProcessingMultipleLayerWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterMultipleLayers::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingMultipleLayerWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingMultipleLayerWidgetWrapper( parameter, type );
}

QgsProcessingAbstractParameterDefinitionWidget *QgsProcessingMultipleLayerWidgetWrapper::createParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm )
{
  return new QgsProcessingMultipleLayerParameterDefinitionWidget( context, widgetContext, definition, algorithm );
}


//
// QgsProcessingPointCloudLayerWidgetWrapper
//

QgsProcessingPointCloudLayerWidgetWrapper::QgsProcessingPointCloudLayerWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsProcessingMapLayerWidgetWrapper( parameter, type, parent )
{

}

QStringList QgsProcessingPointCloudLayerWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList()
         << QgsProcessingParameterPointCloudLayer::typeName()
         << QgsProcessingParameterMapLayer::typeName()
         << QgsProcessingParameterString::typeName()
         << QgsProcessingParameterExpression::typeName();
}

QStringList QgsProcessingPointCloudLayerWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList()
         << QgsProcessingOutputString::typeName()
         // TODO  << QgsProcessingOutputPointCloudLayer::typeName()
         << QgsProcessingOutputMapLayer::typeName()
         << QgsProcessingOutputFile::typeName()
         << QgsProcessingOutputFolder::typeName();
}

QString QgsProcessingPointCloudLayerWidgetWrapper::modelerExpressionFormatString() const
{
  return tr( "path to a point cloud layer" );
}

QString QgsProcessingPointCloudLayerWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterPointCloudLayer::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingPointCloudLayerWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingPointCloudLayerWidgetWrapper( parameter, type );
}

QgsProcessingAbstractParameterDefinitionWidget *QgsProcessingPointCloudLayerWidgetWrapper::createParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm )
{
  Q_UNUSED( context );
  Q_UNUSED( widgetContext );
  Q_UNUSED( definition );
  Q_UNUSED( algorithm );

  return nullptr;
}


//
// QgsProcessingAnnotationLayerWidgetWrapper
//

QgsProcessingAnnotationLayerWidgetWrapper::QgsProcessingAnnotationLayerWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{

}

QStringList QgsProcessingAnnotationLayerWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList()
         << QgsProcessingParameterAnnotationLayer::typeName()
         << QgsProcessingParameterMapLayer::typeName()
         << QgsProcessingParameterString::typeName()
         << QgsProcessingParameterExpression::typeName();
}

QStringList QgsProcessingAnnotationLayerWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList()
         << QgsProcessingOutputString::typeName()
         << QgsProcessingOutputMapLayer::typeName();
}

QString QgsProcessingAnnotationLayerWidgetWrapper::modelerExpressionFormatString() const
{
  return tr( "name of an annotation layer, or \"main\" for the main annotation layer" );
}

QString QgsProcessingAnnotationLayerWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterAnnotationLayer::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingAnnotationLayerWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingAnnotationLayerWidgetWrapper( parameter, type );
}

QgsProcessingAbstractParameterDefinitionWidget *QgsProcessingAnnotationLayerWidgetWrapper::createParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm )
{
  Q_UNUSED( context );
  Q_UNUSED( widgetContext );
  Q_UNUSED( definition );
  Q_UNUSED( algorithm );

  return nullptr;
}

void QgsProcessingAnnotationLayerWidgetWrapper::setWidgetContext( const QgsProcessingParameterWidgetContext &context )
{
  QgsAbstractProcessingParameterWidgetWrapper::setWidgetContext( context );
  if ( mComboBox )
  {
    if ( mWidgetContext.project() )
      mComboBox->setAdditionalLayers( { mWidgetContext.project()->mainAnnotationLayer() } );
  }
}

QWidget *QgsProcessingAnnotationLayerWidgetWrapper::createWidget()
{
  mComboBox = new QgsMapLayerComboBox( );
  mComboBox->setFilters( QgsMapLayerProxyModel::AnnotationLayer );

  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    case QgsProcessingGui::Batch:
      break;
    case QgsProcessingGui::Modeler:
      mComboBox->setEditable( true );
      break;
  }

  mComboBox->setToolTip( parameterDefinition()->toolTip() );

  if ( mWidgetContext.project() )
    mComboBox->setAdditionalLayers( { mWidgetContext.project()->mainAnnotationLayer() } );

  if ( parameterDefinition()->flags() & QgsProcessingParameterDefinition::FlagOptional )
    mComboBox->setAllowEmptyLayer( true );

  connect( mComboBox, &QgsMapLayerComboBox::layerChanged, this, [ = ]()
  {
    if ( mBlockSignals )
      return;

    emit widgetValueHasChanged( this );
  } );

  setWidgetContext( widgetContext() );
  return mComboBox;
}

void QgsProcessingAnnotationLayerWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext &context )
{
  if ( mComboBox )
  {
    if ( !value.isValid()  && parameterDefinition()->flags() & QgsProcessingParameterDefinition::FlagOptional )
    {
      mComboBox->setLayer( nullptr );
      return;
    }

    QVariant val = value;
    if ( val.canConvert<QgsProperty>() )
    {
      if ( val.value< QgsProperty >().propertyType() == QgsProperty::StaticProperty )
      {
        val = val.value< QgsProperty >().staticValue();
      }
      else
      {
        val = val.value< QgsProperty >().valueAsString( context.expressionContext(), parameterDefinition()->defaultValueForGui().toString() );
      }
    }

    QgsMapLayer *layer = qobject_cast< QgsMapLayer * >( val.value< QObject * >() );
    if ( !layer && val.type() == QVariant::String )
    {
      layer = QgsProcessingUtils::mapLayerFromString( val.toString(), context, false, QgsProcessingUtils::LayerHint::Annotation );
    }

    if ( layer )
    {
      mComboBox->setLayer( layer );
    }
  }
}

QVariant QgsProcessingAnnotationLayerWidgetWrapper::widgetValue() const
{
  return mComboBox && mComboBox->currentLayer() ?
         ( mWidgetContext.project() ? ( mComboBox->currentLayer() == mWidgetContext.project()->mainAnnotationLayer() ? QStringLiteral( "main" ) : mComboBox->currentLayer()->id() ) : mComboBox->currentLayer()->id() )
         : QVariant();
}



//
// QgsProcessingOutputWidgetWrapper
//

QgsProcessingOutputWidgetWrapper::QgsProcessingOutputWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{

}

QWidget *QgsProcessingOutputWidgetWrapper::createWidget()
{
  const QgsProcessingDestinationParameter *destParam = dynamic_cast< const QgsProcessingDestinationParameter * >( parameterDefinition() );
  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    case QgsProcessingGui::Modeler:
    {
      mOutputWidget = new QgsProcessingLayerOutputDestinationWidget( destParam, false );
      if ( mProcessingContextGenerator )
        mOutputWidget->setContext( mProcessingContextGenerator->processingContext() );
      if ( mParametersGenerator )
        mOutputWidget->registerProcessingParametersGenerator( mParametersGenerator );
      mOutputWidget->setToolTip( parameterDefinition()->toolTip() );

      connect( mOutputWidget, &QgsProcessingLayerOutputDestinationWidget::destinationChanged, this, [ = ]()
      {
        if ( mBlockSignals )
          return;

        emit widgetValueHasChanged( this );
      } );

      if ( type() == QgsProcessingGui::Standard
           && ( destParam->type() == QgsProcessingParameterRasterDestination::typeName() ||
                destParam->type() == QgsProcessingParameterFeatureSink::typeName() ||
                destParam->type() == QgsProcessingParameterVectorDestination::typeName() ||
                destParam->type() == QgsProcessingParameterPointCloudDestination::typeName() ) )
        mOutputWidget->addOpenAfterRunningOption();

      return mOutputWidget;
    }
    case QgsProcessingGui::Batch:
      break;
  }

  return nullptr;
}


void QgsProcessingOutputWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext & )
{
  if ( mOutputWidget )
    mOutputWidget->setValue( value );
}

QVariant QgsProcessingOutputWidgetWrapper::widgetValue() const
{
  if ( mOutputWidget )
    return mOutputWidget->value();

  return QVariant();
}

QVariantMap QgsProcessingOutputWidgetWrapper::customProperties() const
{
  QVariantMap res;
  if ( mOutputWidget )
    res.insert( QStringLiteral( "OPEN_AFTER_RUNNING" ), mOutputWidget->openAfterRunning() );
  return res;
}

QStringList QgsProcessingOutputWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList()
         << QgsProcessingParameterRasterLayer::typeName()
         << QgsProcessingParameterMeshLayer::typeName()
         << QgsProcessingParameterVectorLayer::typeName()
         << QgsProcessingParameterMapLayer::typeName()
         << QgsProcessingParameterString::typeName()
         << QgsProcessingParameterExpression::typeName();
}

QStringList QgsProcessingOutputWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList()
         << QgsProcessingOutputString::typeName()
         << QgsProcessingOutputFolder::typeName()
         << QgsProcessingOutputFile::typeName();
}

//
// QgsProcessingFeatureSinkWidgetWrapper
//

QgsProcessingFeatureSinkWidgetWrapper::QgsProcessingFeatureSinkWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsProcessingOutputWidgetWrapper( parameter, type, parent )
{

}

QString QgsProcessingFeatureSinkWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterFeatureSink::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingFeatureSinkWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingFeatureSinkWidgetWrapper( parameter, type );
}

QString QgsProcessingFeatureSinkWidgetWrapper::modelerExpressionFormatString() const
{
  return tr( "path to layer destination" );
}

//
// QgsProcessingFeatureSinkWidgetWrapper
//

QgsProcessingVectorDestinationWidgetWrapper::QgsProcessingVectorDestinationWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsProcessingOutputWidgetWrapper( parameter, type, parent )
{

}

QString QgsProcessingVectorDestinationWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterVectorDestination::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingVectorDestinationWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingVectorDestinationWidgetWrapper( parameter, type );
}

QString QgsProcessingVectorDestinationWidgetWrapper::modelerExpressionFormatString() const
{
  return tr( "path to layer destination" );
}

//
// QgsProcessingRasterDestinationWidgetWrapper
//

QgsProcessingRasterDestinationWidgetWrapper::QgsProcessingRasterDestinationWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsProcessingOutputWidgetWrapper( parameter, type, parent )
{

}

QString QgsProcessingRasterDestinationWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterRasterDestination::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingRasterDestinationWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingRasterDestinationWidgetWrapper( parameter, type );
}

QString QgsProcessingRasterDestinationWidgetWrapper::modelerExpressionFormatString() const
{
  return tr( "path to layer destination" );
}

//
// QgsProcessingPointCloudDestinationWidgetWrapper
//

QgsProcessingPointCloudDestinationWidgetWrapper::QgsProcessingPointCloudDestinationWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsProcessingOutputWidgetWrapper( parameter, type, parent )
{

}

QString QgsProcessingPointCloudDestinationWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterPointCloudDestination::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingPointCloudDestinationWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingPointCloudDestinationWidgetWrapper( parameter, type );
}

QString QgsProcessingPointCloudDestinationWidgetWrapper::modelerExpressionFormatString() const
{
  return tr( "path to layer destination" );
}

//
// QgsProcessingFileDestinationWidgetWrapper
//

QgsProcessingFileDestinationWidgetWrapper::QgsProcessingFileDestinationWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsProcessingOutputWidgetWrapper( parameter, type, parent )
{

}

QString QgsProcessingFileDestinationWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterFileDestination::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingFileDestinationWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingFileDestinationWidgetWrapper( parameter, type );
}

QString QgsProcessingFileDestinationWidgetWrapper::modelerExpressionFormatString() const
{
  return tr( "path to file destination" );
}

//
// QgsProcessingFolderDestinationWidgetWrapper
//

QgsProcessingFolderDestinationWidgetWrapper::QgsProcessingFolderDestinationWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsProcessingOutputWidgetWrapper( parameter, type, parent )
{

}

QString QgsProcessingFolderDestinationWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterFolderDestination::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingFolderDestinationWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingFolderDestinationWidgetWrapper( parameter, type );
}

QString QgsProcessingFolderDestinationWidgetWrapper::modelerExpressionFormatString() const
{
  return tr( "path to folder destination" );
}

///@endcond PRIVATE
