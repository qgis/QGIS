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
#include <vector>
#include <list>
#include <qwidget.h>

#include <qevent.h>
#include "qgsrect.h"
#include "qgspoint.h"
class QRect;
class QgsCoordinateTransform;
class QgsMapLayer;
class QMouseEvent;
class QgsLegend;
class QColor;
class QgsPoint;

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
    
    void clear();
    double mupp();
    QgsRect extent();
    void setExtent(QgsRect );
    void zoomFullExtent();
    void zoomPreviousExtent();
  /** Sets the map tool currently being used on the canvas */
  void setMapTool(int tool);
  /** Write property of QColor bgColor. */
  virtual void setbgColor( const QColor& _newVal);
  /** Updates the full extent to include the mbr of the rectangle r */
  void updateFullExtent(QgsRect r);
  //! return the map layer at postion index in the layer stack
   QgsMapLayer * getZpos(int index);
   //! return the layer by name
   QgsMapLayer * layerByName(QString n);
   //! return number of layers on the map
   int layerCount();
   void freeze(bool frz=true);
	//! remove the layer defined by key
	void remove(QString key);
	friend class QgsLegend;
public slots:
	void render2();
	//! This slot is connected to the visibility change of one or more layers
	void layerStateChange();
 
 signals:
 	void xyCoordinates(QgsPoint &p);
 private:
   void mouseMoveEvent(QMouseEvent *e);
   void mousePressEvent(QMouseEvent *e);
   void mouseReleaseEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent *pe);
    //! map containing the layers by name
    std::map<QString,QgsMapLayer *>layers;
    //! vector containing the names of layers in zorder
//    std::vector<QString> zOrder;
    std::list<QString> zOrder;
    //! Full extent of the map canvas
    QgsRect fullExtent;
    //! Current extent
    QgsRect currentExtent;
    //! Previous view extent
    QgsRect previousExtent;
    QRect *mapWindow;
    QgsCoordinateTransform *coordXForm;
  /**  */
  int mapTool;
  /** Flag to indicate status of mouse button */
  bool mouseButtonDown;
  //! Map units per pixel
    double m_mupp;
  /** Rubber band box for dynamic zoom */
  QRect zoomBox;
  /** Beginning point of a rubber band box */
  QPoint boxStartPoint;
  /** Pixmap snapshot used for panning */
  QPixmap * tempPanImage;
  /** Pixmap used for filling the background when panning */
  QPixmap *backgroundFill;
  /** Background color for the map canvas */
  QColor bgColor;
  /** Flag to indicate a map canvas drag operation is taking place */
  bool dragging;
  std::vector<QColor> initialColor;
  void incrementZpos();
  void updateZpos();
  //! Flag indicating a map refresh is in progress
  bool drawing;
  bool frozen;

};

#endif
