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

#include <deque>
#include <map>
#include <QTreeWidget>

class QgisApp;
class QgsLegendItem;
class QgsMapLayer;
class QgsMapCanvas;
class QDomDocument;
class QDomNode;
class QMouseEvent;
class QTreeWidgetItem;
class Q3PopupMenu;

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

class QgsLegend : public QTreeWidget
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

  /*!Returns the current layer if the current item is a QgsLegendLayerFile.
   If the current item is a QgsLegendLayer, its first maplayer is returned.
  Else, 0 is returned.*/
  QgsMapLayer* currentLayer();

  /**Writes the content of the legend to a project file*/
  bool writeXML(QDomNode & layer_node, QDomDocument & document);

  /**Restores the legend from a project file*/
  bool readXML(QDomNode& legendnode);

  /**Returns true, if the y-coordinate is >= the center of the item*/
  bool yCoordAboveCenter(QgsLegendItem* it, int ycoord);

  /**Returns the first item in the hierarchy*/
  QTreeWidgetItem* firstItem();

  /**Returns the next item (next sibling or next item on level above)*/
  QTreeWidgetItem* nextItem(QTreeWidgetItem* item);

  /**Returns the next sibling of an item or 0 if there is none*/
  QTreeWidgetItem* nextSibling(QTreeWidgetItem* item);

  /**Returns the previous sibling of an item or 0 if there is none*/
  QTreeWidgetItem* previousSibling(QTreeWidgetItem* item);

  /**Moves an item after another one*/
  void moveItem(QTreeWidgetItem* move, QTreeWidgetItem* after);

  /**Removes an item from the legend. This is e.g. necessary before shifting it to another place*/
  void removeItem(QTreeWidgetItem* item);

  /**Returns the ids of the layers contained in this legend. The order is bottom->top*/
  std::deque<QString> layerIDs();

public slots:

    /*!Adds a new layer group with the maplayer to the canvas*/
    void addLayer( QgsMapLayer * layer );

    void setMapCanvas(QgsMapCanvas * canvas){mMapCanvas = canvas;}


 void updateLegendItem( QTreeWidgetItem* li );
 
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
  void addGroup();
  void removeLayer(QString);

  /**Removes the current LegendLayer and all its LegendLayerFiles*/
  void legendLayerRemove();

protected:

  /*!Event handler for mouse movements.
   * Mainly intended so handle cases where user is dragging and dropping
   * items into or out of groups, or is reordering layers.
   * @note Overrides method of the same name in the QListView class.
   * @return void
   */ 
  void mouseMoveEvent(QMouseEvent * e);

  /*!
   * Event handler for buton mouse presses.
   * Mainly intended so handle cases where user is dragging and dropping
   * items into or out of groups, or is reordering layers.
   * @note Overrides method of the same name in the QListView class.
   * @return void
   */ 
  void mousePressEvent(QMouseEvent * e);

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
  void mouseReleaseEvent(QMouseEvent * e);
  void mouseDoubleClickEvent(QMouseEvent* e);

    /**Stores the necessary information about the position of an item in the hierarchy. Afterwards, 
this item may be moved back to the original position with resetToInitialPosition()*/
  void storeInitialPosition(QTreeWidgetItem* li);
  
  /**Moves an item back to the position where storeInitialPosition has been called*/
  void resetToInitialPosition(QTreeWidgetItem* li);

  private slots:

  /**Calls 'handleRightClickEvent' on the item*/
  void handleRightClickEvent(QTreeWidgetItem* item, const QPoint& position);
  /**Removes the current legend group*/
  void legendGroupRemove();
  /**Adds all the legend layer files of the current legend layer to overview*/
  void legendLayerAddToOverview();
  /**Removes all the legend layer files of the current legend layer from overview*/
  void legendLayerRemoveFromOverview();
  /**Shows the property dialog of the first legend layer file in a legend layer*/
  void legendLayerShowProperties();
   /**Sets all listview items to open*/
  void expandAll();
  /**Sets all listview items to closed*/
  void collapseAll();
  /**Just for a test*/
  void handleItemChange(QTreeWidgetItem* item, int row);
  /**Calls openPersistentEditor for the current item*/
  void openEditor();
  /**Removes the current item and inserts it as a toplevel item at the end of the legend*/
  void makeToTopLevelItem();

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
  QTreeWidgetItem* mItemBeingMoved;

  /*!
   * Position in the list of the item being moved as it was at the start of a drag event.
   * An item at the top of the list will be 0 and each successive item below it
   * will be 1,2 3 etc... regardless of nesting level.
   */
  int mItemBeingMovedOrigPos;

  /**Information needed by 'storeInitialPosition' and 'resetToInitialPosition'*/
  enum HIERARCHY_POSITION_TYPE
    {
      FIRST_ITEM,
      FIRST_CHILD,
      YOUNGER_SIBLING
    };
  HIERARCHY_POSITION_TYPE mRestoreInformation;
  QTreeWidgetItem* mRestoreItem;

  /*!
   * A fuction sed to determin how far down in the list an item is (starting with one for the first Item.
   *If the item is not in the legend, -1 is returned
   * @see mItemBeingMovedOrigPos
   */
  int getItemPos(QTreeWidgetItem* item);

  /*!
   * A QPopupMenu that will be displayed when the right mouse button is clicked.
   */
  Q3PopupMenu * mPopupMenu;

  /**Pointer to the main canvas. Used for requiring repaints in case of legend changes*/
  QgsMapCanvas* mMapCanvas;

  /**Map that keeps track of which checkboxes are in which check state. This is necessary because QTreeView does not emit 
     a signal for check state changes*/
  std::map<QTreeWidgetItem*, Qt::CheckState> mStateOfCheckBoxes;

signals:
  void zOrderChanged(QgsLegend * lv);

};
#endif
