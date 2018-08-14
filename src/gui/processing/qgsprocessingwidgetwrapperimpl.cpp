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
#include <QLabel>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QComboBox>

///@cond PRIVATE

//
// QgsProcessingBooleanWidgetWrapper
//

QgsProcessingBooleanWidgetWrapper::QgsProcessingBooleanWidgetWrapper( const QgsProcessingParameterDefinition *parameter, WidgetType type, QWidget *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{

}

QWidget *QgsProcessingBooleanWidgetWrapper::createWidget()
{
  switch ( type() )
  {
    case Standard:
    {
      QString description = parameterDefinition()->description();
      if ( parameterDefinition()->flags() & QgsProcessingParameterDefinition::FlagOptional )
        description = QObject::tr( "%1 [optional]" ).arg( description );

      mCheckBox = new QCheckBox( description );
      mCheckBox->setToolTip( parameterDefinition()->toolTip() );
      return mCheckBox;
    };

    case Batch:
    {
      mComboBox = new QComboBox();
      mComboBox->addItem( tr( "Yes" ), true );
      mComboBox->addItem( tr( "No" ), false );
      mComboBox->setToolTip( parameterDefinition()->toolTip() );
      return mComboBox;
    }
  }
  return nullptr;
}

QLabel *QgsProcessingBooleanWidgetWrapper::createLabel()
{
  // avoid creating labels in standard dialogs
  if ( type() == QgsAbstractProcessingParameterWidgetWrapper::Standard )
    return nullptr;
  else
    return QgsAbstractProcessingParameterWidgetWrapper::createLabel();
}

void QgsProcessingBooleanWidgetWrapper::setWidgetValue( const QVariant &value, const QgsProcessingContext &context )
{
  switch ( type() )
  {
    case Standard:
    {
      const bool v = QgsProcessingParameters::parameterAsBool( parameterDefinition(), value, context );
      mCheckBox->setChecked( v );
      break;
    }

    case Batch:
    {
      const bool v = QgsProcessingParameters::parameterAsBool( parameterDefinition(), value, context );
      mComboBox->setCurrentIndex( mComboBox->findData( v ) );
      break;
    }
  }

}

QVariant QgsProcessingBooleanWidgetWrapper::value() const
{
  switch ( type() )
  {
    case Standard:
      return mCheckBox->isChecked();

    case Batch:
      return mComboBox->currentData();

  }
}

QString QgsProcessingBooleanWidgetWrapper::parameterType() const
{
  return QStringLiteral( "boolean" );
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingBooleanWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsAbstractProcessingParameterWidgetWrapper::WidgetType type )
{
  return new QgsProcessingBooleanWidgetWrapper( parameter, type );
}

///@endcond PRIVATE
