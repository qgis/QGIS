/***************************************************************************
                             qgsprocessingparameterswidget.cpp
                             ------------------------------------
    Date                 : March 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingparameterswidget.h"
#include "qgsprocessingalgorithm.h"
#include "qgsprocessingparameters.h"

///@cond NOT_STABLE

QgsProcessingParametersWidget::QgsProcessingParametersWidget( const QgsProcessingAlgorithm *algorithm, QWidget *parent )
  : QgsPanelWidget( parent )
  , mAlgorithm( algorithm )
{
  Q_ASSERT( mAlgorithm );

  setupUi( this );

  grpAdvanced->hide();
  scrollAreaWidgetContents->setContentsMargins( 4, 4, 4, 4 );
}

const QgsProcessingAlgorithm *QgsProcessingParametersWidget::algorithm() const
{
  return mAlgorithm;
}

void QgsProcessingParametersWidget::initWidgets()
{
  // if there are advanced parameters - show corresponding groupbox
  const QgsProcessingParameterDefinitions defs = mAlgorithm->parameterDefinitions();
  for ( const QgsProcessingParameterDefinition *param : defs )
  {
    if ( param->flags() & QgsProcessingParameterDefinition::FlagAdvanced )
    {
      grpAdvanced->show();
      break;
    }
  }
}

void QgsProcessingParametersWidget::addParameterWidget( const QgsProcessingParameterDefinition *parameter, QWidget *widget, int stretch )
{
  if ( parameter->flags() & QgsProcessingParameterDefinition::FlagAdvanced )
    mAdvancedGroupLayout->addWidget( widget, stretch );
  else
    mScrollAreaLayout->insertWidget( mScrollAreaLayout->count() - 2, widget, stretch );
}

void QgsProcessingParametersWidget::addParameterLabel( const QgsProcessingParameterDefinition *parameter, QWidget *label )
{
  if ( parameter->flags() & QgsProcessingParameterDefinition::FlagAdvanced )
    mAdvancedGroupLayout->addWidget( label );
  else
    mScrollAreaLayout->insertWidget( mScrollAreaLayout->count() - 2, label );
}

void QgsProcessingParametersWidget::addOutputLabel( QWidget *label )
{
  mScrollAreaLayout->insertWidget( mScrollAreaLayout->count() - 1, label );
}

void QgsProcessingParametersWidget::addOutputWidget( QWidget *widget, int stretch )
{
  mScrollAreaLayout->insertWidget( mScrollAreaLayout->count() - 1, widget, stretch );
}

void QgsProcessingParametersWidget::addExtraWidget( QWidget *widget )
{
  mScrollAreaLayout->addWidget( widget );
}

///@endcond
