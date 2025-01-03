/***************************************************************************
                         qgsprocessingrasteroptionswidgetwrapper.cpp
                         ---------------------
    begin                : November 2020
    copyright            : (C) 2020 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingrasteroptionswidgetwrapper.h"
#include "moc_qgsprocessingrasteroptionswidgetwrapper.cpp"
#include "qgsrasterformatsaveoptionswidget.h"
#include "qgsprocessingparameters.h"
#include "qgsprocessingoutputs.h"

#include <QComboBox>
#include <QLineEdit>

/// @cond private

QgsProcessingRasterOptionsWidgetWrapper::QgsProcessingRasterOptionsWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{
}

QString QgsProcessingRasterOptionsWidgetWrapper::parameterType() const
{
  return QStringLiteral( "rasteroptions" );
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingRasterOptionsWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingRasterOptionsWidgetWrapper( parameter, type );
}

QWidget *QgsProcessingRasterOptionsWidgetWrapper::createWidget()
{
  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    {
      mOptionsWidget = new QgsRasterFormatSaveOptionsWidget();
      mOptionsWidget->setToolTip( parameterDefinition()->toolTip() );
      connect( mOptionsWidget, &QgsRasterFormatSaveOptionsWidget::optionsChanged, this, [=] {
        emit widgetValueHasChanged( this );
      } );
      return mOptionsWidget;
    }
    case QgsProcessingGui::Batch:
    case QgsProcessingGui::Modeler:
    {
      mLineEdit = new QLineEdit();
      mLineEdit->setToolTip( parameterDefinition()->toolTip() );
      connect( mLineEdit, &QLineEdit::textChanged, this, [=] {
        emit widgetValueHasChanged( this );
      } );
      return mLineEdit;
    }
  }

  return nullptr;
}

void QgsProcessingRasterOptionsWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext &context )
{
  QString val = QgsProcessingParameters::parameterAsString( parameterDefinition(), value, context );

  if ( mOptionsWidget )
  {
    if ( value.isValid() )
      mOptionsWidget->setOptions( val.replace( '|', ' ' ) );
    else
      mOptionsWidget->setOptions( QString() );
  }
  else if ( mLineEdit )
  {
    if ( value.isValid() )
      mLineEdit->setText( val );
    else
      mLineEdit->clear();
  }
}

QVariant QgsProcessingRasterOptionsWidgetWrapper::widgetValue() const
{
  if ( mOptionsWidget )
  {
    return mOptionsWidget->options().join( '|' );
  }
  else if ( mLineEdit )
  {
    return mLineEdit->text().isEmpty() ? QVariant() : mLineEdit->text();
  }
  else
    return QVariant();
}

QStringList QgsProcessingRasterOptionsWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList()
         << QgsProcessingParameterString::typeName()
         << QgsProcessingParameterAuthConfig::typeName()
         << QgsProcessingParameterNumber::typeName()
         << QgsProcessingParameterDistance::typeName()
         << QgsProcessingParameterArea::typeName()
         << QgsProcessingParameterVolume::typeName()
         << QgsProcessingParameterDuration::typeName()
         << QgsProcessingParameterScale::typeName()
         << QgsProcessingParameterFile::typeName()
         << QgsProcessingParameterField::typeName()
         << QgsProcessingParameterExpression::typeName()
         << QgsProcessingParameterCoordinateOperation::typeName()
         << QgsProcessingParameterProviderConnection::typeName();
}

QStringList QgsProcessingRasterOptionsWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList() << QgsProcessingOutputNumber::typeName()
                       << QgsProcessingOutputVariant::typeName()
                       << QgsProcessingOutputFile::typeName()
                       << QgsProcessingOutputFolder::typeName()
                       << QgsProcessingOutputString::typeName();
}

/// @endcond
