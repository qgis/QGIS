/***************************************************************************
  qgsappmaptools.cpp
  --------------------------------------
  Date                 : March 2021
  Copyright            : (C) 2021 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsappmaptools.h"
#include "qgisapp.h"
#include "qgsmaptool.h"
#include "qgsmaptoolselect.h"
#include "qgsmaptoolidentifyaction.h"
#include "qgsmaptooladdfeature.h"
#include "qgsmaptoolzoom.h"
#include "qgsmaptoolpan.h"
#include "qgsmaptoolfeatureaction.h"
#include "qgsmeasuretool.h"
#include "qgsmaptooltextannotation.h"
#include "qgsmaptoolhtmlannotation.h"
#include "qgsmaptoolannotation.h"
#include "qgsmaptoolmeasureangle.h"
#include "qgsmaptoolmeasurebearing.h"
#include "qgsmaptoolformannotation.h"
#include "qgsmaptoolsvgannotation.h"
#include "qgsmaptoolrotatefeature.h"
#include "qgsmaptoolscalefeature.h"
#include "qgsmaptoolmovefeature.h"
#include "qgsmaptooloffsetcurve.h"
#include "qgsmaptoolreshape.h"
#include "qgsmaptoolsplitfeatures.h"
#include "qgsmaptoolsplitparts.h"
#include "qgsmaptoolreverseline.h"
#include "qgsmaptooladdring.h"
#include "qgsmaptoolfillring.h"
#include "qgsmaptoolsimplify.h"
#include "qgsmaptooldeletepart.h"
#include "qgsmaptooldeletering.h"
#include "qgsmaptooladdpart.h"
#include "vertextool/qgsvertextool.h"
#include "qgsmaptoolrotatepointsymbols.h"
#include "qgsmaptoolrotatelabel.h"
#include "qgsmaptooltrimextendfeature.h"
#include "qgsmaptoolshowhidelabels.h"
#include "qgsmaptoolmovelabel.h"
#include "qgsmaptoolchangelabelproperties.h"
#include "qgsmaptoolpinlabels.h"
#include "qgsmaptooloffsetpointsymbol.h"
#include "qgsmaptooleditmeshframe.h"
#include "qgssettingsregistrycore.h"
#include "qgsmaptoolmodifyannotation.h"

//
// QgsAppMapTools
//

QgsAppMapTools::QgsAppMapTools( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDock )
{
  mTools.insert( Tool::ZoomIn, new QgsMapToolZoom( canvas, false /* zoomIn */ ) );
  mTools.insert( Tool::ZoomOut, new QgsMapToolZoom( canvas, true /* zoomOut */ ) );
  mTools.insert( Tool::Pan, new QgsMapToolPan( canvas ) );
  mTools.insert( Tool::Identify, new QgsMapToolIdentifyAction( canvas ) );
  mTools.insert( Tool::FeatureAction, new QgsMapToolFeatureAction( canvas ) );
  mTools.insert( Tool::MeasureDistance, new QgsMeasureTool( canvas, false /* area */ ) );
  mTools.insert( Tool::MeasureArea, new QgsMeasureTool( canvas, true /* area */ ) );
  mTools.insert( Tool::MeasureAngle, new QgsMapToolMeasureAngle( canvas ) );
  mTools.insert( Tool::MeasureBearing, new QgsMapToolMeasureBearing( canvas ) );
  mTools.insert( Tool::TextAnnotation, new QgsMapToolTextAnnotation( canvas ) );
  mTools.insert( Tool::FormAnnotation, new QgsMapToolFormAnnotation( canvas ) );
  mTools.insert( Tool::HtmlAnnotation, new QgsMapToolHtmlAnnotation( canvas ) );
  mTools.insert( Tool::SvgAnnotation, new QgsMapToolSvgAnnotation( canvas ) );
  mTools.insert( Tool::Annotation, new QgsMapToolAnnotation( canvas ) );
  mTools.insert( Tool::AddFeature, new QgsMapToolAddFeature( canvas, cadDock, QgsMapToolCapture::CaptureNone ) );
  mTools.insert( Tool::MoveFeature, new QgsMapToolMoveFeature( canvas, QgsMapToolMoveFeature::Move ) );
  mTools.insert( Tool::MoveFeatureCopy, new QgsMapToolMoveFeature( canvas, QgsMapToolMoveFeature::CopyMove ) );
  mTools.insert( Tool::RotateFeature, new QgsMapToolRotateFeature( canvas ) );
  mTools.insert( Tool::ScaleFeature, new QgsMapToolScaleFeature( canvas ) );
  mTools.insert( Tool::OffsetCurve, new QgsMapToolOffsetCurve( canvas ) );
  mTools.insert( Tool::ReshapeFeatures, new QgsMapToolReshape( canvas ) );
  mTools.insert( Tool::ReverseLine, new QgsMapToolReverseLine( canvas ) );
  mTools.insert( Tool::SplitFeatures, new QgsMapToolSplitFeatures( canvas ) );
  mTools.insert( Tool::SplitParts, new QgsMapToolSplitParts( canvas ) );
  mTools.insert( Tool::SelectFeatures, new QgsMapToolSelect( canvas ) );
  mTools.insert( Tool::SelectPolygon, new QgsMapToolSelect( canvas ) );
  mTools.insert( Tool::SelectFreehand, new QgsMapToolSelect( canvas ) );
  mTools.insert( Tool::SelectRadius, new QgsMapToolSelect( canvas ) );
  mTools.insert( Tool::AddRing, new QgsMapToolAddRing( canvas ) );
  mTools.insert( Tool::FillRing, new QgsMapToolFillRing( canvas ) );
  mTools.insert( Tool::AddPart, new QgsMapToolAddPart( canvas ) );
  mTools.insert( Tool::SimplifyFeature, new QgsMapToolSimplify( canvas ) );
  mTools.insert( Tool::DeleteRing, new QgsMapToolDeleteRing( canvas ) );
  mTools.insert( Tool::DeletePart, new QgsMapToolDeletePart( canvas ) );
  mTools.insert( Tool::VertexTool, new QgsVertexTool( canvas, cadDock ) );
  mTools.insert( Tool::VertexToolActiveLayer, new QgsVertexTool( canvas, cadDock, QgsVertexTool::ActiveLayer ) );
  mTools.insert( Tool::RotatePointSymbolsTool, new QgsMapToolRotatePointSymbols( canvas ) );
  mTools.insert( Tool::OffsetPointSymbolTool, new QgsMapToolOffsetPointSymbol( canvas ) );
  mTools.insert( Tool::TrimExtendFeature, new QgsMapToolTrimExtendFeature( canvas ) );
  mTools.insert( Tool::PinLabels, new QgsMapToolPinLabels( canvas, cadDock ) );
  mTools.insert( Tool::ShowHideLabels, new QgsMapToolShowHideLabels( canvas, cadDock ) );
  mTools.insert( Tool::MoveLabel, new QgsMapToolMoveLabel( canvas, cadDock ) );
  mTools.insert( Tool::RotateLabel, new QgsMapToolRotateLabel( canvas, cadDock ) );
  mTools.insert( Tool::ChangeLabelProperties, new QgsMapToolChangeLabelProperties( canvas, cadDock ) );
  mTools.insert( Tool::EditMeshFrame, new QgsMapToolEditMeshFrame( canvas ) );
  mTools.insert( Tool::AnnotationEdit, new QgsMapToolModifyAnnotation( canvas, cadDock ) );
}

QgsAppMapTools::~QgsAppMapTools()
{
  for ( auto it = mTools.constBegin(); it != mTools.constEnd(); ++it )
  {
    delete it.value();
  }
}

QgsMapTool *QgsAppMapTools::mapTool( QgsAppMapTools::Tool tool )
{
  return mTools.value( tool );
}


QList<QgsMapToolCapture *> QgsAppMapTools::captureTools() const
{
  QList< QgsMapToolCapture * > res;
  for ( auto it = mTools.constBegin(); it != mTools.constEnd(); ++it )
  {
    if ( QgsMapToolCapture *captureTool = qobject_cast< QgsMapToolCapture * >( it.value() ) )
      res << captureTool;
  }
  return res;
}


