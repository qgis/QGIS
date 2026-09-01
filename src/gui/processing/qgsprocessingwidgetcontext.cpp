/***************************************************************************
                         qgsprocessingwidgetcontext.cpp
                         ---------------------
    begin                : August 2026
    copyright            : (C) 2026 by Nyall Dawson
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


#include "qgsprocessingwidgetcontext.h"

#include "qgsbrowserguimodel.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsmessagebar.h"
#include "qgsmodeldesignerdialog.h"
#include "qgsproject.h"

//
// QgsProcessingParameterWidgetContext
//

void QgsProcessingParameterWidgetContext::setMapCanvas( QgsMapCanvas *canvas )
{
  mMapCanvas = canvas;
}

QgsMapCanvas *QgsProcessingParameterWidgetContext::mapCanvas() const
{
  return mMapCanvas;
}

void QgsProcessingParameterWidgetContext::setMessageBar( QgsMessageBar *bar )
{
  mMessageBar = bar;
}

QgsMessageBar *QgsProcessingParameterWidgetContext::messageBar() const
{
  return mMessageBar;
}

void QgsProcessingParameterWidgetContext::setBrowserModel( QgsBrowserGuiModel *model )
{
  mBrowserModel = model;
}

QgsBrowserGuiModel *QgsProcessingParameterWidgetContext::browserModel() const
{
  return mBrowserModel;
}

void QgsProcessingParameterWidgetContext::setProject( QgsProject *project )
{
  mProject = project;
}

QgsProject *QgsProcessingParameterWidgetContext::project() const
{
  return mProject;
}

QString QgsProcessingParameterWidgetContext::modelChildAlgorithmId() const
{
  return mModelChildAlgorithmId;
}

void QgsProcessingParameterWidgetContext::setModelChildAlgorithmId( const QString &modelChildAlgorithmId )
{
  mModelChildAlgorithmId = modelChildAlgorithmId;
}

QgsMapLayer *QgsProcessingParameterWidgetContext::activeLayer() const
{
  return mActiveLayer;
}

void QgsProcessingParameterWidgetContext::setActiveLayer( QgsMapLayer *activeLayer )
{
  mActiveLayer = activeLayer;
}

void QgsProcessingParameterWidgetContext::registerProcessingContextGenerator( QgsProcessingContextGenerator *generator )
{
  mProcessingContextGenerator = generator;
}

const QgsProcessingContextGenerator *QgsProcessingParameterWidgetContext::processingContextGenerator() const
{
  return mProcessingContextGenerator;
}

QgsModelDesignerDialog *QgsProcessingParameterWidgetContext::modelDesignerDialog() const
{
  return mModelDialog;
}

void QgsProcessingParameterWidgetContext::setModelDesignerDialog( QgsModelDesignerDialog *dialog )
{
  mModelDialog = dialog;
}

QgsProcessingModelAlgorithm *QgsProcessingParameterWidgetContext::model() const
{
  return mModel;
}

void QgsProcessingParameterWidgetContext::setModel( QgsProcessingModelAlgorithm *model )
{
  mModel = model;
}
