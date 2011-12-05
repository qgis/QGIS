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

#ifndef QGSLEGEND_H
#define QGSLEGEND_H

#include <QTreeWidget>
#include <QPair>
#include <set>

class QgsLegendGroup;
class QgsLegendLayer;
class QgsLegendItem;
class QgsMapLayer;
class QgsMapCanvas;
class QDomDocument;
class QDomElement;
class QDomNode;
class QMouseEvent;
class QTreeWidgetItem;
class QgsCoordinateReferenceSystem;
class QgsMapCanvasLayer;

//Information about relationship between groups and layers
//key: group name (or null strings for single layers without groups)
//value: containter with layer ids contained in the group
typedef QPair< QString, QList<QString> > GroupLayerInfo;

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
   </ul>
   @note Additional group types may be defined in the future to accommodate WMS, PostGIS etc layers.
   @author Gary E.Sherman, Tim Sutton, Marco Hugentobler and Jens Oberender
*/

class QgsLegend : public QTreeWidget
{
    Q_OBJECT
  private:
    // Moved here to match access of declaration later in file.
    // Previous location raised a warning in msvc as the forward
    // declaration was public while the definition was private
    class QgsLegendPixmaps;

  public:
    /*! Constructor.
    * @param qgis_app link to qgisapp
    * @param theParent An optional parent widget
    * @param theName An optional name for the widget
    */
    QgsLegend( QgsMapCanvas *canvas, QWidget * parent = 0, const char *name = 0 );

    //! Destructor
    ~QgsLegend();

    /** Returns QgsLegendLayer associated with current layer */
    QgsLegendLayer* currentLegendLayer();

    /*!Returns the current layer if the current item is a QgsLegendLayer.
    If the current item is a QgsLegendLayer, its first maplayer is returned.
    Else, 0 is returned.*/
    QgsMapLayer* currentLayer();

    /*!Returns the currently selected layer QgsLegendLayers.
    Else, an empty list is returned.*/
    QList<QgsMapLayer *> selectedLayers();

    /*!Returns all layers loaded in QgsMapCanvas in drawing order
    Else, an empty list is returned.*/
    QList<QgsMapLayer *> layers();

    //!Return all layers in drawing order
    QList<QgsLegendLayer *> legendLayers();

    void setDrawingOrder( QList<QgsMapLayer *> );

    /*!set the current layer
    returns true if the layer exists, false otherwise*/
    bool setCurrentLayer( QgsMapLayer *layer );

    /**Writes the content of the legend to a project file*/
    bool writeXML( QDomNode & layer_node, QDomDocument & document );

    /**Restores the legend from a project file*/
    bool readXML( QDomNode& legendnode );

    /**Returns true, if the y-coordinate is >= the center of the item*/
    bool yCoordAboveCenter( QgsLegendItem* it, int ycoord );

    /**Returns true, if the item at index is a QgsLegendGroup*/
    bool isLegendGroup( const QModelIndex &index );

    /**Returns a string list of groups*/
    QStringList groups();

    //! Return the relationship between groups and layers in the legend
    QList< GroupLayerInfo > groupLayerRelationship();

    /**Returns the first item in the hierarchy*/
    QTreeWidgetItem* firstItem();

    /**Returns the next item (next sibling or next item on level above)*/
    QTreeWidgetItem* nextItem( QTreeWidgetItem* item );

    /**Returns the next sibling of an item or 0 if there is none*/
    QTreeWidgetItem* nextSibling( QTreeWidgetItem* item );

    /**Returns the previous sibling of an item or 0 if there is none*/
    QTreeWidgetItem* previousSibling( QTreeWidgetItem* item );

    /**Finds the next dom node. This function is used by QgsLegend, but probably its not a good place here*/
    static QDomNode nextDomNode( const QDomNode& theNode );

    /**Inserts an item into another one. Stores the item specific settings of the moved item (and its subitems)
     and applies it afterwards again*/
    void insertItem( QTreeWidgetItem* move, QTreeWidgetItem* into );

    /**Moves an item after another one. Stores the item specific settings of the moved item (and its subitems)
     and applies it afterwards again*/
    void moveItem( QTreeWidgetItem* move, QTreeWidgetItem* after );

    /**Removes an item from the legend. This is e.g. necessary before shifting it to another place*/
    void removeItem( QTreeWidgetItem* item );

    /**Returns the ids of the layers contained in this legend. The order is bottom->top*/
    QStringList layerIDs();

    /**Updates layer set of map canvas*/
    void updateMapCanvasLayerSet();

    /**Updates overview*/
    void updateOverview();

    /**Show/remove all layer in/from overview*/
    void enableOverviewModeAllLayers( bool isInOverview );

    /**Adds an entry to mPixmapWidthValues*/
    void addPixmapWidthValue( int width );

    /**Adds an entry to mPixmapHeightValues*/
    void addPixmapHeightValue( int height );

    /**Removes an entry from mPixmapWidthValues*/
    void removePixmapWidthValue( int width );

    /**Removes an entry from mPixmapHeightValues*/
    void removePixmapHeightValue( int height );

    /**Returns structure with legend pixmaps*/
    QgsLegendPixmaps& pixmaps() { return mPixmaps; }

    /**Returns a layers check state*/
    Qt::CheckState layerCheckState( QgsMapLayer * layer );

    /**Add group from other project file. Returns a pointer to the new group in case of success or 0 in case of error*/
    QgsLegendGroup* addEmbeddedGroup( const QString& groupName, const QString& projectFilePath, QgsLegendItem* parent = 0 );

    /** return canvas */
    QgsMapCanvas *canvas() { return mMapCanvas; }

  public slots:

    /*!Adds a new layer group with the maplayer to the canvas*/
    void addLayer( QgsMapLayer * layer );

    void setLayerVisible( QgsMapLayer * layer, bool visible );

    /**Updates symbology items for a layer*/
    void refreshLayerSymbology( QString key, bool expandItem = true );

    /*!
      * Slot called to clear the tree of all items
      * @note Usually connected to a QgsMapCanvas that will ask its legend to clear itself.
      * @return void
      */
    void removeAll();

    /*!
     * Called when the user wishes to toggle on or off all of the layers in
     * the legend, and in the map.
     * @return void
     */
    void selectAll( bool select );

    /*!
     * Slot called when user wishes to add a new empty layer group to the legend.
     * If a the legend has a currentItem() then the new group will be nested into it
     * The user will be prompted for the name of the newly added group.
     * @param name name of the new group
     * @param expand expand the group
     * @return index of inserted group
     */
    int addGroupToCurrentItem( QString name = QString(), bool expand = true );

    /*!
     * Slot called when user wishes to add a new empty layer group to the legend.
     * The user will be prompted for the name of the newly added group.
     * @param name name of the new group
     * @param expand expand the group
     * @return index of inserted group
     */
    int addGroup( QString name = QString(), bool expand = true, QTreeWidgetItem* parent = 0 );

    /*!
     * Slot called when user wishes to add a new empty layer group to the legend.
     * All parameter are mandatory to be used to programatically nest a new group
     * @param name name of the new group
     * @param expand expand the group
     * @return index of inserted group
     */
    int addGroup( QString name, bool expand, int parentIndex );

    /*!
     * Removes all groups with the given name.
     * @param name name of the groups to remove
     * @return void
     */
    void removeGroup( int groupIndex );

    void removeLayer( QString );

    /** called to read legend settings from project */
    void readProject( const QDomDocument & );

    /** called to write legend settings to project */
    void writeProject( QDomDocument & );

    /*!
     * Moves a layer to a group.
     * @param ml the maplayer to move
     * @param groupIndex index of group
     * @note keep in mind that the group's index changes, if the moved layer is above the group.
     */
    void moveLayer( QgsMapLayer* ml, int groupIndex );

    /**Toggle show in overview for current layer*/
    void legendLayerShowInOverview();

    /**Zooms to extent of the current legend layer (considers there may be several
    legend layer files*/
    void legendLayerZoom();

    /**Zooms so that the pixels of the raster layer occupies exactly one screen pixel.
        Only works on raster layers*/
    void legendLayerZoomNative();

    /**Stretches the raster layer, if stretching is active, based on the min and max of the current extent.
        Only workds on raster layers*/
    void legendLayerStretchUsingCurrentExtent();

    /**Updates check states when the map canvas layer set is changed */
    void refreshCheckStates();

    /** Remove selected layers */
    void removeSelectedLayers();

    /** Set CRS for selected layers */
    void setCRSForSelectedLayers( const QgsCoordinateReferenceSystem &crs );

    /** Update drawing order */
    bool updateDrawingOrder();

    /** Update drawing order */
    void setUpdateDrawingOrder( bool updateDrawingOrder );

    /** Update drawing order */
    void unsetUpdateDrawingOrder( bool dontUpdateDrawingOrder ) { setUpdateDrawingOrder( !dontUpdateDrawingOrder ); }

  protected:

    /*!Event handler for mouse movements.
     * Mainly intended so handle cases where user is dragging and dropping
     * items into or out of groups, or is reordering layers.
     * @note Overrides method of the same name in the QListView class.
     * @return void
     */
    void mouseMoveEvent( QMouseEvent * e );

    /*!
     * Event handler for buton mouse presses.
     * Mainly intended so handle cases where user is dragging and dropping
     * items into or out of groups, or is reordering layers.
     * @note Overrides method of the same name in the QListView class.
     * @return void
     */
    void mousePressEvent( QMouseEvent * e );

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
    void mouseReleaseEvent( QMouseEvent * e );
    void mouseDoubleClickEvent( QMouseEvent* e );

    /**Returns the legend layer to which a map layer belongs to*/
    QgsLegendLayer* findLegendLayer( const QString& layerKey );

    /**Returns the legend layer to which a map layer belongs to*/
    QgsLegendLayer* findLegendLayer( const QgsMapLayer *layer );

    /**Checks mPixmapWidthValues and mPixmapHeightValues and sets a new icon size if necessary*/
    void adjustIconSize();

    /**Initialize pixmaps - called when QgsLegend is constructed */
    void initPixmaps();

    /**This function compares the layer order before a drag with the current layer ordering and triggers a canvas repaint if it has changed*/
    bool checkLayerOrderUpdate();

    // mouse is pressed
    bool mMousePressedFlag;

    // position of mouse when it is pressed at the start of a drag event.
    QPoint mLastPressPos;

    // layer our prior to move
    QStringList mLayersPriorToMove;

    // keep track of the items being dragged
    QList< QTreeWidgetItem * > mItemsBeingMoved;

    // The target that the mouse is over when dragging
    QTreeWidgetItem *mDropTarget;

    // The action when the mouse is released
    enum { BEFORE, INSERT, AFTER } mDropAction;

    /** Groups defined in other project files.
    Key: group name, value: absolute path to project file*/
    QHash< QString, QString > mEmbeddedGroups;

    /** Hide the line that indicates insertion position */
    void hideLine();

    /** Show the line that indicates insertion position */
    void showLine( int y, int left );

    /** Update the widget with latest changes immediately */
    void updateLineWidget();

    /** Returns the last visible item in the tree widget */
    QTreeWidgetItem *lastVisibleItem();

    /** read layer settings from XML element and add item */
    QgsLegendLayer* readLayerFromXML( QDomElement& childelem, bool& isOpen );

  private slots:

    /**Calls 'handleRightClickEvent' on the item*/
    void handleRightClickEvent( QTreeWidgetItem* item, const QPoint& position );
    /**Removes the current legend group*/
    void legendGroupRemove();
    /**Set the CRS of the current legend group*/
    void legendGroupSetCRS();
    /**Removes a legend group and its layers*/
    void removeGroup( QgsLegendGroup * lg );
    /**Removes a legend group and its layers*/
    void setGroupCRS( QgsLegendGroup * lg, const QgsCoordinateReferenceSystem &crs );
    /**Sets all listview items to open*/
    void expandAll();
    /**Sets all listview items to closed*/
    void collapseAll();
    /** toogle update drawing order */
    void toggleDrawingOrderUpdate();
    void handleItemChange( QTreeWidgetItem* item, int row );
    /** delegates current layer to map canvas */
    void handleCurrentItemChanged( QTreeWidgetItem* current, QTreeWidgetItem* previous );
    /**Calls openPersistentEditor for the current item*/
    void openEditor();
    /**Removes the current item and inserts it as a toplevel item at the end of the legend*/
    void makeToTopLevelItem();

  private:
    bool readXML( QgsLegendGroup *parent, const QDomNode &node );
    bool writeXML( QList<QTreeWidgetItem *> items, QDomNode &node, QDomDocument &document );

    QList<QgsMapCanvasLayer> canvasLayers();

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
     * A function to determine how far down in the list an item is (starting with one for the first Item).
     *If the item is not in the legend, -1 is returned
     * @see mItemBeingMovedOrigPos
     */
    int getItemPos( QTreeWidgetItem* item );

    /**Returns true if the item is a group embedde from another project*/
    bool groupEmbedded( QTreeWidgetItem* item ) const;

    /**Returns true if the parent group is embedded from another project*/
    bool parentGroupEmbedded( QTreeWidgetItem* item ) const;

    /**Pointer to the main canvas. Used for requiring repaints in case of legend changes*/
    QgsMapCanvas* mMapCanvas;

    /**Stores the width values of the LegendSymbologyItem pixmaps. The purpose of this is that the legend may automatically change
     the global IconWidth when items are added or removed*/
    std::multiset<int> mPixmapWidthValues;

    /**Stores the width values of the LegendSymbologyItem pixmaps. The purpose of this is that the legend may automatically change
     the global IconWidth when items are added or removed*/
    std::multiset<int> mPixmapHeightValues;

    /**QgsLegend does not set the icon with/height to values lower than the minimum icon size*/
    QSize mMinimumIconSize;

    bool mChanging;

    /** moving the layers in the hierarchy also changes the drawing order */
    bool mUpdateDrawingOrder;

    /** structure which holds pixmap which are used in legend */
    class QgsLegendPixmaps
    {
      public:
        //! Pixmap which is shown by default
        QPixmap mOriginalPixmap;

        //! Pixmap to show a bogus vertex was encoutnered in this layer (applies to vector layers only)
        QPixmap mProjectionErrorPixmap;

        //! Pixmap to show if this layer is represented in overview or now
        QPixmap mInOverviewPixmap;

        //! Pixmap to show it this layer has currently editing turned on
        QPixmap mEditablePixmap;

    } mPixmaps;

    //! Widget that holds the indicator line //
    QWidget *mInsertionLine;

#ifdef QGISDEBUG
    void showItem( QString msg, QTreeWidgetItem *item );
#endif

    void updateGroupCheckStates( QTreeWidgetItem *item );

  signals:
    void itemMoved( QModelIndex oldIndex, QModelIndex newIndex );

    void zOrderChanged();
    void invisibleLayerRemoved();

    void updateDrawingOrderChecked( bool );
    void updateDrawingOrderUnchecked( bool );

    //! Emitted whenever current (selected) layer changes
    //  the pointer to layer can be null if no layer is selected
    void currentLayerChanged( QgsMapLayer * layer );
};
#endif
