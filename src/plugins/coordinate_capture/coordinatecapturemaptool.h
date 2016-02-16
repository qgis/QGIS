/***************************************************************************
    coordinatecapturemaptool.h  -  map tool for getting map coordinates
    ---------------------
    begin                : August 2008
    copyright            : (C) 2008 by Tim Sutton
    email                : tim at linfiniti.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef COORDINATECAPTUREMAPTOOL_H
#define COORDINATECAPTUREMAPTOOL_H

#include "qgsmaptool.h"
#include "qgspoint.h"

#include <QObject>
#include <QPointer>

class QgsRubberBand;

/**
  \brief Map tool for capturing mouse clicks to clipboard
*/
class CoordinateCaptureMapTool : public QgsMapTool
{
    Q_OBJECT

  public:
    explicit CoordinateCaptureMapTool( QgsMapCanvas* thepCanvas );

    ~CoordinateCaptureMapTool();

    //! Overridden mouse move event
    virtual void canvasMoveEvent( QgsMapMouseEvent* e ) override;

    //! Overridden mouse press event
    virtual void canvasPressEvent( QgsMapMouseEvent* e ) override;

    //! Overridden mouse release event
    virtual void canvasReleaseEvent( QgsMapMouseEvent* e ) override;

    //! called when map tool is being deactivated
    virtual void deactivate() override;

  public slots:

  signals:
    void mouseMoved( const QgsPoint& );
    void mouseClicked( const QgsPoint& );
  private:

    //! Rubber band for highlighting identified feature
    QgsRubberBand * mpRubberBand;
    QPointer<QgsMapCanvas> mpMapCanvas;

};

#endif
