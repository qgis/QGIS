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


///@endcond PRIVATE
