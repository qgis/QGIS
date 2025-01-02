/***************************************************************************
                         qgsprocessingalgorithmconfig.cpp
                         --------------------------
    begin                : April 2018
    copyright            : (C) 2018 by Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingalgorithmconfigurationwidget.h"
#include "moc_qgsprocessingalgorithmconfigurationwidget.cpp"

QgsProcessingAlgorithmConfigurationWidget::QgsProcessingAlgorithmConfigurationWidget( QWidget *parent )
  : QWidget( parent )
{
}

void QgsProcessingAlgorithmConfigurationWidget::setWidgetContext( const QgsProcessingParameterWidgetContext &context )
{
  mWidgetContext = context;
}

const QgsProcessingParameterWidgetContext &QgsProcessingAlgorithmConfigurationWidget::widgetContext() const
{
  return mWidgetContext;
}

void QgsProcessingAlgorithmConfigurationWidget::setAlgorithm( const QgsProcessingAlgorithm *algorithm )
{
  mAlgorithm = algorithm;
}

void QgsProcessingAlgorithmConfigurationWidget::registerProcessingContextGenerator( QgsProcessingContextGenerator *generator )
{
  mContextGenerator = generator;
}

QgsExpressionContext QgsProcessingAlgorithmConfigurationWidget::createExpressionContext() const
{
  return QgsProcessingGuiUtils::createExpressionContext( mContextGenerator, mWidgetContext, mAlgorithm, nullptr );
}
