/***************************************************************************
  qgsappmaptools.h
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

#ifndef QGSAPPMAPTOOLS_H
#define QGSAPPMAPTOOLS_H

#include <QList>
#include <QHash>
#include <QPointer>


class QgsMapTool;
class QgsMapToolCapture;

class QgsMapCanvas;
class QgsAdvancedDigitizingDockWidget;

class QgsAppMapTools
{
  public:
    enum Tool
    {
      ZoomIn,
      ZoomOut,
      Pan,
      Identify,
      FeatureAction,
      MeasureDistance,
      MeasureArea,
      MeasureAngle,
      MeasureBearing,
      AddFeature,
      MoveFeature,
      MoveFeatureCopy,
      OffsetCurve,
      ReshapeFeatures,
      SplitFeatures,
      SplitParts,
      SelectFeatures,
      SelectPolygon,
      SelectFreehand,
      SelectRadius,
      VertexAdd,
      VertexMove,
      VertexDelete,
      AddRing,
      FillRing,
      AddPart,
      SimplifyFeature,
      DeleteRing,
      DeletePart,
      VertexTool,
      VertexToolActiveLayer,
      RotatePointSymbolsTool,
      OffsetPointSymbolTool,
      Annotation,
      FormAnnotation,
      HtmlAnnotation,
      SvgAnnotation,
      TextAnnotation,
      PinLabels,
      ShowHideLabels,
      MoveLabel,
      RotateFeature,
      ScaleFeature,
      RotateLabel,
      ChangeLabelProperties,
      ReverseLine,
      TrimExtendFeature,
      EditMeshFrame,
      AnnotationEdit
    };

    QgsAppMapTools( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDock );
    ~QgsAppMapTools();

    /**
     * Returns the specified \a tool.
     */
    QgsMapTool *mapTool( Tool tool );

    /**
     * Returns the specified \a tool.
     */
    template <class ToolType> ToolType *mapTool( Tool tool )
    {
      QgsMapTool *t = mapTool( tool );
      return qobject_cast< ToolType * >( t );
    }

    /**
     * Returns a list of all QgsMapToolCapture derived tools.
     */
    QList< QgsMapToolCapture * > captureTools() const;

  private:

    QHash< Tool, QPointer< QgsMapTool > > mTools;

    // Disable copying as we have pointer members.
    QgsAppMapTools( const QgsAppMapTools & ) = delete;
    QgsAppMapTools &operator= ( const QgsAppMapTools & ) = delete;
};

#endif // QGSAPPMAPTOOLS_H
