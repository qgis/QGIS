/***************************************************************************
                         qgsprocessingalgorithmconfig.cpp
                         --------------------------
    begin                : April 2018
    copyright            : (C) 2018 by Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingalgorithmconfigurationwidget.h"

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
