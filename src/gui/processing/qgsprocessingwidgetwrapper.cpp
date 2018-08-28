/***************************************************************************
                         qgsprocessingwidgetwrapper.cpp
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


#include "qgsprocessingwidgetwrapper.h"
#include "qgsprocessingparameters.h"
#include "qgsprocessingmodelerwidget.h"
#include <QLabel>


QgsAbstractProcessingParameterWidgetWrapper::QgsAbstractProcessingParameterWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QObject *parent )
  : QObject( parent )
  , mType( type )
  , mParameterDefinition( parameter )
{
}

QgsProcessingGui::WidgetType QgsAbstractProcessingParameterWidgetWrapper::type() const
{
  return mType;
}

QWidget *QgsAbstractProcessingParameterWidgetWrapper::createWrappedWidget( const QgsProcessingContext &context )
{
  if ( mWidget )
    return mWidget;

  mWidget = createWidget();
  setWidgetValue( mParameterDefinition->defaultValue(), context );

  return mWidget;
}

QLabel *QgsAbstractProcessingParameterWidgetWrapper::createWrappedLabel()
{
  if ( mLabel )
    return mLabel;

  mLabel = createLabel();
  return mLabel;
}

QWidget *QgsAbstractProcessingParameterWidgetWrapper::wrappedWidget()
{
  return mWidget;
}

QLabel *QgsAbstractProcessingParameterWidgetWrapper::wrappedLabel()
{
  return mLabel;
}

const QgsProcessingParameterDefinition *QgsAbstractProcessingParameterWidgetWrapper::parameterDefinition() const
{
  return mParameterDefinition;
}

QLabel *QgsAbstractProcessingParameterWidgetWrapper::createLabel()
{
  switch ( mType )
  {
    case QgsProcessingGui::Batch:
      return nullptr;

    case QgsProcessingGui::Standard:
    case QgsProcessingGui::Modeler:
    {
      std::unique_ptr< QLabel > label = qgis::make_unique< QLabel >( mParameterDefinition->description() );
      label->setToolTip( mParameterDefinition->toolTip() );
      return label.release();
    }
  }
  return nullptr;
}

void QgsAbstractProcessingParameterWidgetWrapper::postInitialize( const QList<QgsAbstractProcessingParameterWidgetWrapper *> & )
{

}

QgsProcessingModelerParameterWidget *QgsProcessingParameterWidgetFactoryInterface::createModelerWidgetWrapper( QgsProcessingModelAlgorithm *model, const QString &childId, const QgsProcessingParameterDefinition *parameter, const QgsProcessingContext &context )
{
  std::unique_ptr< QgsProcessingModelerParameterWidget > widget = qgis::make_unique< QgsProcessingModelerParameterWidget >( model, childId, parameter, context );
  widget->populateSources( compatibleParameterTypes(), compatibleOutputTypes(), compatibleDataTypes() );
  return widget.release();
}
