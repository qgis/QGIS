/***************************************************************************
                          qgslegend.h  -  description
                             -------------------
    begin                : Sun Jul 28 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc dot com
               Romans 3:23=>Romans 6:23=>Romans 10:9,10=>Romans 12
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

#ifndef QGSLEGEND_H
#define QGSLEGEND_H

#include <qlistview.h>


class QgsMapCanvas;
class QgsMapLayer;
class QListView;
class QPainter;


/**
   \class QgsLegend
   \brief Map legend

  @author Gary E.Sherman
*/
class QgsLegend : public QListView
{
    Q_OBJECT;
	
 public:

  /*! Constructor.
   * @param parent Parent widget
   * @param name Name of the widget
   */
  QgsLegend(QWidget * parent = 0, const char *name = 0);

  //! Destructor
   ~QgsLegend();

  //! Set the pointer to the map canvas
  void setMapCanvas(QgsMapCanvas * canvas);

  //! Update the legend
  //void update();

  /** the name of the currently selected layer */
  QString currentLayerName();

  /// current, selected layer
  QgsMapLayer *currentLayer();

public slots:

  /** used to add given layer to legend */
  void addLayer( QgsMapLayer * layer );

  /** used to remove given layer to legend 

  @param layer_key is unique layer identification
  */
  void removeLayer( QString layer_key );

protected:
  // override these to handle layer order manipulation
  void contentsMouseMoveEvent(QMouseEvent * e);
  void contentsMousePressEvent(QMouseEvent * e);
  void contentsMouseReleaseEvent(QMouseEvent * e);

private:
  /// the map canvas this legend refers to
  QgsMapCanvas * map;

  /// location of mouse press
  QPoint presspos;

  /// keep track of if the mouse is pressed or not
  bool mousePressed;

  /// keep track of the Item being dragged
  QListViewItem *movingItem;

  /// keep track of the original position of the Item being dragged
  int movingItemOrigPos;

  /// return position of item in the list
  int getItemPos(QListViewItem * item);

signals:

  // broadcast that the stacking order has changed
  // so the map canvas can be redrawn
  void zOrderChanged(QgsLegend * lv);

  // emit when a legend item is deleted (i.e., a map layer is deleted via the
  // legend)
  void layerRemoved( QString layer_key );

};
#endif
