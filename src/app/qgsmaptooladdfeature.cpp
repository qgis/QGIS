/***************************************************************************
    qgsmaptooladdfeature.cpp
    ------------------------
    begin                : April 2007
    copyright            : (C) 2007 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptooladdfeature.h"
#include "qgsadvanceddigitizingdockwidget.h"
#include "qgsapplication.h"
#include "qgsattributedialog.h"
#include "qgsexception.h"
#include "qgscurvepolygon.h"
#include "qgsfields.h"
#include "qgsgeometry.h"
#include "qgslinestring.h"
#include "qgsmultipoint.h"
#include "qgsmapcanvas.h"
#include "qgsmapmouseevent.h"
#include "qgspolygon.h"
#include "qgsproject.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgslogger.h"
#include "qgsfeatureaction.h"
#include "qgisapp.h"

#include <QMouseEvent>
#include <QSettings>

QgsMapToolAddFeature::QgsMapToolAddFeature( QgsMapCanvas *canvas, CaptureMode mode )
  : QgsMapToolDigitizeFeature( canvas, canvas->currentLayer(), mode )
  , mCheckGeometryType( true )
{
  mToolName = tr( "Add feature" );
  connect( QgisApp::instance(), &QgisApp::newProject, this, &QgsMapToolAddFeature::stopCapturing );
  connect( QgisApp::instance(), &QgisApp::projectRead, this, &QgsMapToolAddFeature::stopCapturing );
}

bool QgsMapToolAddFeature::addFeature( QgsVectorLayer *vlayer, QgsFeature *f, bool showModal )
{
  QgsExpressionContextScope *scope = QgsExpressionContextUtils::mapToolCaptureScope( snappingMatches() );
  QgsFeatureAction *action = new QgsFeatureAction( tr( "add feature" ), *f, vlayer, QString(), -1, this );
  bool res = action->addFeature( QgsAttributeMap(), showModal, scope );
  if ( showModal )
    delete action;
  return res;
}

void QgsMapToolAddFeature::digitized( QgsFeature &f )
{
  QgsVectorLayer *vlayer = currentVectorLayer();
  bool res = addFeature( vlayer, &f, false );

  if ( res && ( mode() == CaptureLine || mode() == CapturePolygon ) )
  {
    //add points to other features to keep topology up-to-date
    bool topologicalEditing = QgsProject::instance()->topologicalEditing();

    //use always topological editing for avoidIntersection.
    //Otherwise, no way to guarantee the geometries don't have a small gap in between.
    QList<QgsVectorLayer *> intersectionLayers = QgsProject::instance()->avoidIntersectionsLayers();
    bool avoidIntersection = !intersectionLayers.isEmpty();
    if ( avoidIntersection ) //try to add topological points also to background layers
    {
      Q_FOREACH ( QgsVectorLayer *vl, intersectionLayers )
      {
        //can only add topological points if background layer is editable...
        if ( vl->geometryType() == QgsWkbTypes::PolygonGeometry && vl->isEditable() )
        {
          vl->addTopologicalPoints( f.geometry() );
        }
      }
    }
    else if ( topologicalEditing )
    {
      vlayer->addTopologicalPoints( f.geometry() );
    }
  }
}
