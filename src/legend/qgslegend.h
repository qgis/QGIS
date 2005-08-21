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
/* $Id$ */

#ifndef QGSLEGEND_H
#define QGSLEGEND_H

#include "qgisapp.h"
#include "qgslegenditem.h"
#include <map>
#include <qlistview.h>
#include <qpopupmenu.h>

class QCheckBox;
class QgsMapLayer;
class QgsMapCanvas;

/**
   \class QgsLegend
   \brief A Legend treeview for QGIS
   Map legend is a specialised QListView designed to show grooups of map layers,
   map layers, and the map layer members, properties and symbols for each layer.

   The legend supports simple operations such as displaying an ordered list of 
   layers in the current canvas, and complex operations such as drag/dropping 
   layer symbologies and properties between layers. 

   There are a variety of different items that can appear in a QgsLegend. All 
   items added to a QgsLegend should be inherited from QgsLegendItem as this
   will ensure that they can perform legend specific tasks such as seeing if
   dropping of other items onto them is allowed, returning their type etc.

   The following types are defined:
   <ul>
   <li>QgsLegendGroup - a group folder for many layers (can include other groups)</li>
   <li>QgsLegendLayer - a layer that contains one or more files associated with it. 
                        allowing more than one file lets you create virtual layers where
                        1 or more files can share the same symbology and properties
                        and be treated as they are one layer for things such as hiding / 
                        showing, scale dependent visibility etc.</li>
   <li>QgsLegendSymbologyGroup - a collabsable node that contains symbology items. Can
                                only exist inside of a QgsLegendLayer</li>
   <li>QgsLegendSymbologyItem - a class break (vector) or pallette entry (raster) etc. 
                                Double clicking on a symbology item will let you change
                                the properties for only that specific item. Can only exist
                                inside a symbology group.</li>
   <li>QgsLegendPropertyGroup - a collapsable node that shows 1 or more properties. Can
                                only exist inside of a QgsLegendLayer</li>
   <li>QgsLegendPropertyItem - A list of properties related to the layer. Double clicking
                              a property item will invoke a dialog that will let you change
                              the property settings. Can only exist inside a property group</li>
   <li>QgsLegendLayerFileGroup - each QgsLegendLayer can have one or more files associated
                              with it. This is the container group for these files. Can
                              only exist inside of a QgsLegendLayer.</li>
   <li>QgsLegendLayerFile -  A file node that relates to a file on disk. Assigning multiple
                             file nodes in a file group allows you treat them as if they are
                             one entity.</li>
   </ul>
   @note Additional group types may be defined in the future to accommodate WMS, PostGIS etc layers.
   @author Gary E.Sherman, Tim Sutton, Marco Hugentobler and Jens Oberender
*/

class QgsLegend : public QListView
{
    Q_OBJECT;
	
 public:
    /*! Constructor.
   * @param qgis_app link to qgisapp   
   * @param theParent An optional parent widget
   * @param theName An optional name for the widget
   */
  QgsLegend(QgisApp* app, QWidget * parent = 0, const char *name = 0);

  //! Destructor
   ~QgsLegend();

   /*!
   * Show a context menu of things that can be done to / in a legend
   * @param theListViewItem The QListViewItem with which the context menu is associated.
   * @param thePoint The QPoint indicating wher the context menu should be displayed.
   * @return void
   */
  void showContextMenu(QListViewItem * lvi, const QPoint & pt);

  /*!Returns the current layer or 0 if the current item is not a QgsLegendLayerFile*/
  QgsMapLayer* currentLayer();

  /**Adds a checkbox and its item to mCheckBoxes*/
  void registerCheckBox(QListViewItem* item, QCheckBox* cbox);

  /**Removes a checkbox from mCheckBoxes. Does not delete the objects*/
  void unregisterCheckBox(QListViewItem* item);

public slots:

    /*!Adds a new layer group with the maplayer to the canvas*/
    void addLayer( QgsMapLayer * layer );

    void setMapCanvas(QgsMapCanvas * canvas){mMapCanvas = canvas;}


 void updateLegendItem( QListViewItem * li );
 
 /*!
   * Slot called to clear the tree of all items
   * @note Usually connected to a QgsMapCanvas that will ask its legend to clear itself.
   * @return void
   */
  void removeAll();

  /*!
   * Slot called when user wishes to add a new empty layer group to the legend.
   * The user will be prompted for the name of the newly added group.
   * @return void
   */
  void removeLayer(QString);
  void addGroup();

protected:

  /*!Event handler for mouse movements.
   * Mainly intended so handle cases where user is dragging and dropping
   * items into or out of groups, or is reordering layers.
   * @note Overrides method of the same name in the QListView class.
   * @return void
   */ 
  void contentsMouseMoveEvent(QMouseEvent * e);

  /*!
   * Event handler for buton mouse presses.
   * Mainly intended so handle cases where user is dragging and dropping
   * items into or out of groups, or is reordering layers.
   * @note Overrides method of the same name in the QListView class.
   * @return void
   */ 
  void contentsMousePressEvent(QMouseEvent * e);

  /*!
   * Event handler for mouse button releases.
   * Mainly intended so handle cases where user is dragging and dropping
   * items into or out of groups, or is reordering layers. Each sublass of
   * QgsLegendItem has an accept method that defines behaviour rules for
   * whether another QgsLegendItem child instance can be dropped onto it.
   * <h1>Behaviour rules for dropped legend items</h1>
   * <ul>
   * <li> Symbology groups, properies groups and layers groups can only be dropped
   * onto QgsLegendLayer nodes. </li> 
   * <li>Only QgsLegendGroup and QgsLegendLayer can be top level items in the view</li>
   * <li>Groups can be nested by dropping them into each other,</li>
   * <li>Each group can have one or more layers</li>
   * <li>Layers can be ordered by dragging them above or below another layer.</li>
   * <li>The order for QgsLegendSymbologyGroup, QgsLegendPropertyGroup and QgsLegendLayerGroup
   * is predefined to sort in that order.</li>
   * </ul>
   * @note Overrides method of the same name in the QListView class.
   * @return void
   */  
  void contentsMouseReleaseEvent(QMouseEvent * e);

  private slots:

  /**Calls 'handleDoubleClickEvent' on the item*/
  void distributeDoubleClickEvent(QListViewItem* item);
  /**Calls 'handleRightClickEvent' on the item*/
  void distributeRightClickEvent(QListViewItem* item, const QPoint& position);
  /**Moves all the checkboxes stored in mCheckBoxes to the right places. Needs to
   be called every time the geometry of the treeview is changed*/
  void placeCheckBoxes();

private:

  /**Pointer to QGisApp, needed for signal/slot reasons*/
  QgisApp* mApp;

   /*! Prevent the copying of QgsLegends
   * @todo See if this is really required - we may want multiple map, canvas and 
           legend support at some stage in the future.
   */
  QgsLegend( QgsLegend const & );

  /*!
   * Prevent the copying of QgsLegends
   * @todo See if this is really required - we may want multiple map, canvas and 
           legend support at some stage in the future.
   */
  QgsLegend & operator=( QgsLegend const & );

  /*!
   * Position of mouse when it is pressed at the start of a drag event.
   */
  QPoint mLastPressPos;

  /**True if the mouse is pressed*/
  bool mMousePressedFlag;

  /// keep track of the Item being dragged
  QListViewItem* mItemBeingMoved;

  /*!
   * Position in the list of the item being moved as it was at the start of a drag event.
   * An item at the top of the list will be 0 and each successive item below it
   * will be 1,2 3 etc... regardless of nesting level.
   */
  int mItemBeingMovedOrigPos;

  /*!
   * A fuction sed to determin how far down in the list an item is.
   * @see mItemBeingMovedOrigPos
   */
  int getItemPos(QListViewItem * item);

  /*!
   * A QPopupMenu that will be displayed when the right mouse button is clicked.
   */
  QPopupMenu * mPopupMenu;

  /**Pointer to the main canvas. Used for requiring repaints in case of legend changes*/
  QgsMapCanvas* mMapCanvas;

  /**QgsLegendItem is derived from QListViewItem, not QCheckBoxItem. So there must be a mechanism to allow
   QgsLegendLayerFiles to have checkboxes without deriving from QCheckBoxItem. The solution is that QgsLegend
  manages the positioning of the checkboxes if the geometry of the treeview is changed. New checkboxes can be
  registered together with their QListViewItem using registerCheckBox() and unregistered using unregisterCheckBox().
  QgsLegend then takes care of the positioning of the checkboxes*/
  std::map<QListViewItem*, QCheckBox*> mCheckBoxes;

  /**Moves a checkbox to a position next to its listview*/
  void placeCheckBox(QListViewItem* litem, QCheckBox* cbox);

signals:
  void zOrderChanged(QgsLegend * lv);

};
#endif
