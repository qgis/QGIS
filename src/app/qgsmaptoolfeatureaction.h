/***************************************************************************
    qgsmaptoolfeatureaction.h  -  map tool for running feature actions
    ---------------------
    begin                : January 2012
    copyright            : (C) 2012 by Giuseppe Sucameli
    email                : brush.tyler at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLFEATUREACTION_H
#define QGSMAPTOOLFEATUREACTION_H

#include "qgis.h"
#include "qgsmaptool.h"

#include <QObject>
#include <QPointer>

class QgsVectorLayer;

/**
  \brief Map tool for running feature actions on the current layer
*/
class APP_EXPORT QgsMapToolFeatureAction : public QgsMapTool
{
    Q_OBJECT

  public:
    QgsMapToolFeatureAction( QgsMapCanvas* canvas );

    ~QgsMapToolFeatureAction();

    virtual Flags flags() const override { return QgsMapTool::AllowZoomRect; }

    //! Overridden mouse move event
    virtual void canvasMoveEvent( QgsMapMouseEvent* e ) override;

    //! Overridden mouse press event
    virtual void canvasPressEvent( QgsMapMouseEvent* e ) override;

    //! Overridden mouse release event
    virtual void canvasReleaseEvent( QgsMapMouseEvent* e ) override;

    virtual void activate() override;

    virtual void deactivate() override;

  private:
    bool doAction( QgsVectorLayer *layer, int x, int y );
};

#endif
