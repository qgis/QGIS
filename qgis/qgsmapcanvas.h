/***************************************************************************
                          qgsmapcanvas.h  -  description
                             -------------------
    begin                : Sun Jun 30 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman@mrcc.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPCANVAS_H
#define QGSMAPCANVAS_H
#include <map>
#include <qwidget.h>
#include <qevent.h>
#include "qgsrect.h"
class QRect;
class QgsCoordinateTransform;
class QgsMapLayer;

/*! \class QgsMapCanvas
 * \brief Map canvas class for displaying all GIS data types.
 */

class QgsMapCanvas : public QWidget  {
    Q_OBJECT
public: 
    //! Constructor
    QgsMapCanvas(QWidget *parent=0, const char *name=0);
    //! Destructor
    ~QgsMapCanvas();
    /*! Adds a layer to the map canvas.
     * @param lyr Pointer to a layer derived from QgsMapLayer
     */
    void addLayer(QgsMapLayer *lyr);
    /*! Draw the map using the symbology set for each layer
     */
    void render();
    void render2();
    void clear();
    double mupp();
    QgsRect extent();
    void setExtent(QgsRect );
    void zoomFullExtent();
 private:
    void paintEvent(QPaintEvent *pe);
    //! map containing the layers by name
    map<QString,QgsMapLayer *>layers;
    //! Full extent of the map canvas
    QgsRect fullExtent;
    //! Current extent
    QgsRect currentExtent;
    QRect *mapWindow;
    QgsCoordinateTransform *coordXForm;
    double m_mupp;

};

#endif
