/***************************************************************************
                          qgsmapcanvas.h  -  description
                             -------------------
    begin                : Sun Jun 30 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id */

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
class QgsMapLayerInterface;
class QMouseEvent;
class QgsLegend;
class QgsLegendView;
class QColor;
class QgsPoint;

/*! \class QgsMapCanvas
 * \brief Map canvas class for displaying all GIS data types.
 */

class QgsMapCanvas:public QWidget
{
  Q_OBJECT public:
    //! Constructor
    QgsMapCanvas(QWidget * parent = 0, const char *name = 0);
    //! Destructor
     ~QgsMapCanvas();
    //! Set the legend control to be used with this canvas
    void setLegend(QgsLegend *legend);
	//! Get a pointer to the legend control used with this canvas
	QgsLegend * getLegend();
    /*! Adds a layer to the map canvas.
     * @param lyr Pointer to a layer derived from QgsMapLayer
     */
    void addLayer(QgsMapLayer * lyr);
    /*! \brief Add a layer from a map layer interface defined in a plugin. 
     * This is not currently implemented
     */
    void addLayer(QgsMapLayerInterface * lyr);
    //! Draw the map using the symbology set for each layer
    void render();
    //! Clear the map canvas
    void clear();
    //! Returns the mupp (map units per pixel) for the canvas
    double mupp();
    //! Returns the exent for all layers on the map canvased
    QgsRect extent();
    //! Set the extent of the map canvas
    void setExtent(QgsRect);
    //! Zoom to the full extent of all layers
    void zoomFullExtent();
    //! Zoom to the previous extent (view)
    void zoomPreviousExtent();
    /**Zooms to the extend of the selected features*/
    void zoomToSelected();
  /** \brief Sets the map tool currently being used on the canvas */
    void setMapTool(int tool);
  /** Write property of QColor bgColor. */
    virtual void setbgColor(const QColor & _newVal);
  /** Updates the full extent to include the mbr of the rectangle r */
    void updateFullExtent(QgsRect r);
    //! return the map layer at postion index in the layer stack
    QgsMapLayer *getZpos(int index);
    //! return the layer by name
    QgsMapLayer *layerByName(QString n);
    //! return number of layers on the map
    int layerCount();
    /*! Freeze/thaw the map canvas. This is used to prevent the canvas from
     * responding to events while layers are being added/removed etc.
     * @param frz Boolean specifying if the canvas should be frozen (true) or
     * thawed (false). Default is true. 
     */
    void freeze(bool frz = true);
    //! remove the layer defined by key
    void remove(QString key);
    //! remove all layers from the map 
    void removeAll();
    //! Flag the canvas as dirty and needed a refresh
    void setDirty(bool _dirty);
    //! Declare the legend class as a friend of the map canvas
    friend class QgsLegend;
    public slots:
        /**Sets dirty=true and calls render2()*/
  void refresh();
	void render2();
	//! This slot is connected to the visibility change of one or more layers
	void layerStateChange();
	//! sets z order based on order of layers in the legend
	void setZOrderFromLegend(QgsLegendView *lv);
 
 signals: 
 void xyCoordinates(QgsPoint & p);
  private:
    //! Overridden mouse move event
    void mouseMoveEvent(QMouseEvent * e);
    //! Overridden mouse press event
    void mousePressEvent(QMouseEvent * e);
    //! Overridden mouse release event
    void mouseReleaseEvent(QMouseEvent * e);
    //! Overridden resize event
    void resizeEvent(QResizeEvent * e);
    //! Overridden paint event
    void paintEvent(QPaintEvent * pe);
    //! map containing the layers by name
    std::map < QString, QgsMapLayer * >layers;
    //! list containing the names of layers in zorder
    std::list < QString > zOrder;
    //! Full extent of the map canvas
    QgsRect fullExtent;
    //! Current extent
    QgsRect currentExtent;
    //! Previous view extent
    QgsRect previousExtent;
    //! Map window rectangle
    QRect *mapWindow;
    //! Pointer to the map legend
    QgsLegend *mapLegend;
    //! Pointer to the coordinate transform object used to transform coordinates
    // from real world to device coordinates
    QgsCoordinateTransform *coordXForm;
  /**
  * \brief Currently selected map tool.
  * @see QGis::MapTools enum for valid values
  */
    int mapTool;
  //!Flag to indicate status of mouse button 
    bool mouseButtonDown;
    //! Map units per pixel
    double m_mupp;
  //! Rubber band box for dynamic zoom 
    QRect zoomBox;
  //! Beginning point of a rubber band box 
    QPoint boxStartPoint;
    //! Pixmap used for restoring the canvas.
    QPixmap *pmCanvas;
  //! Background color for the map canvas
    QColor bgColor;
  //! Flag to indicate a map canvas drag operation is taking place 
    bool dragging;
    //! Vector containing the inital color for a layer
    std::vector < QColor > initialColor;
    //! Increments the z order index
    void incrementZpos();
    //! Updates the z order for layers on the map
    void updateZpos();
    //! Flag indicating a map refresh is in progress
    bool drawing;
    //! Flag indicating if the map canvas is frozen.
    bool frozen;
    /*! \brief Flag to track the state of the Map canvas. 
    *
    * The canvas is
     * flagged as dirty by any operation that changes the state of
     * the layers or the view extent. If the canvas is not dirty, paint
     * events are handled by bit-blitting the stored canvas bitmap to
     * the canvas. This improves performance by not reading the data source
     * when no real change has occurred
     */
    bool dirty;
};

#endif
