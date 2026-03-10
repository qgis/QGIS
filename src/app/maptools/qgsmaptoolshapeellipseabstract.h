/***************************************************************************
    qgsmaptoolshapeellipseabstract.h  -  map tool for adding ellipse
    ---------------------
    begin                : July 2017
    copyright            : (C) 2017
    email                : lituus at free dot fr
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLSHAPEELLIPSEABSTRACT_H
#define QGSMAPTOOLSHAPEELLIPSEABSTRACT_H

#include "qgis_app.h"
#include "qgsellipse.h"
#include "qgsmaptoolshapeabstract.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingsregistrycore.h"

class QgsGeometryRubberBand;
class QgsSnapIndicator;

class APP_EXPORT QgsMapToolShapeEllipseAbstract : public QgsMapToolShapeAbstract
{
    Q_OBJECT
  public:
    QgsMapToolShapeEllipseAbstract( const QString &id, QgsMapToolCapture *parentTool )
      : QgsMapToolShapeAbstract( id, parentTool )
    {}

    void clean() override;

  protected:
    void addEllipseToParentTool();

    //! Ellipse
    QgsEllipse mEllipse;

    //! convenient method to return the number of segments
    unsigned int segments() { return QgsSettingsRegistryCore::settingsDigitizingOffsetQuadSeg->value() * 12; }
};

#endif // QGSMAPTOOLSHAPEELLIPSEABSTRACT_H
