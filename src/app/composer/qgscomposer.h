/***************************************************************************
                         qgscomposer.h
                             -------------------
    begin                : January 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : blazek@itc.it
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
#ifndef QGSCOMPOSER_H
#define QGSCOMPOSER_H
#include "ui_qgscomposerbase.h"
#include "qgscomposeritem.h"
#include "qgscontexthelp.h"
#include <QPrinter>

class QgisApp;
class QgsComposerArrow;
class QgsComposerLabel;
class QgsComposerLegend;
class QgsComposerMap;
class QgsComposerPicture;
class QgsComposerScaleBar;
class QgsComposerShape;
class QgsComposerAttributeTable;
class QgsComposerView;
class QgsComposition;
class QgsMapCanvas;

class QGridLayout;
class QDomNode;
class QDomDocument;
class QMoveEvent;
class QResizeEvent;
class QFile;
class QSizeGrip;
class QUndoView;

/** \ingroup MapComposer
 * \brief A gui for composing a printable map.
 */
class QgsComposer: public QMainWindow, private Ui::QgsComposerBase
{
    Q_OBJECT

  public:
    QgsComposer( QgisApp *qgis, const QString& title );
    ~QgsComposer();

    //! Set the pixmap / icons on the toolbar buttons
    void setupTheme();

    //! Open and show, set defaults if first time
    void open();

    //! Zoom to full extent of the paper
    void zoomFull();

    //! Return pointer to map canvas
    QgsMapCanvas *mapCanvas( void );

    //! Return pointer to composer view
    QgsComposerView *view( void );

    //! Return current composition
    //QgsComposition *composition(void);

    //! Show composition options in widget
    void showCompositionOptions( QWidget *w );

    //! Restore the window and toolbar state
    void restoreWindowState();

    QAction* windowAction() {return mWindowAction;}

    const QString& title() const {return mTitle;}
    void setTitle( const QString& title );

  protected:
    //! Move event
    virtual void moveEvent( QMoveEvent * );

    //! Resize event
    virtual void resizeEvent( QResizeEvent * );

#ifdef Q_WS_MAC
    //! Change event (update window menu on ActivationChange)
    virtual void changeEvent( QEvent * );

    //! Show event (add window to menu)
    virtual void showEvent( QShowEvent * );
#endif

  signals:
    //! Is emitted every time the view zoom has changed
    void zoomLevelChanged();


  public slots:
    //! Zoom to full extent of the paper
    void on_mActionZoomAll_triggered();

    //! Zoom in
    void on_mActionZoomIn_triggered();

    //! Zoom out
    void on_mActionZoomOut_triggered();

    //! Refresh view
    void on_mActionRefreshView_triggered();

    //! Print the composition
    void on_mActionPrint_triggered();

    //! Page Setup for composition
    void on_mActionPageSetup_triggered();

    //! Print as image
    void on_mActionExportAsImage_triggered();

    //! Print as SVG
    void on_mActionExportAsSVG_triggered();

    //! Print as PDF
    void on_mActionExportAsPDF_triggered();

    //! Select item
    void on_mActionSelectMoveItem_triggered();

    //! Add arrow
    void on_mActionAddArrow_triggered();

    //! Add new map
    void on_mActionAddNewMap_triggered();

    //! Add new legend
    void on_mActionAddNewLegend_triggered();

    //! Add new label
    void on_mActionAddNewLabel_triggered();

    //! Add new scalebar
    void on_mActionAddNewScalebar_triggered();

    //! Add new picture
    void on_mActionAddImage_triggered();

    //! Add ellipse shape item
    void on_mActionAddBasicShape_triggered();

    //! Add attribute table
    void on_mActionAddTable_triggered();

    //! Save composer as template
    void on_mActionSaveAsTemplate_triggered();

    void on_mActionLoadFromTemplate_triggered();

    //! Set tool to move item content
    void on_mActionMoveItemContent_triggered();

    //! Group selected items
    void on_mActionGroupItems_triggered();

    //! Ungroup selected item group
    void on_mActionUngroupItems_triggered();

    //! Move selected items one position up
    void on_mActionRaiseItems_triggered();

    //!Move selected items one position down
    void on_mActionLowerItems_triggered();

    //!Move selected items to top
    void on_mActionMoveItemsToTop_triggered();

    //!Move selected items to bottom
    void on_mActionMoveItemsToBottom_triggered();

    //!Align selected composer items left
    void on_mActionAlignLeft_triggered();

    //!Align selected composere items horizontally centered
    void on_mActionAlignHCenter_triggered();

    //!Align selected composer items right
    void on_mActionAlignRight_triggered();

    //!Align selected composer items to top
    void on_mActionAlignTop_triggered();

    //!Align selected composer items vertically centered
    void on_mActionAlignVCenter_triggered();

    //!Align selected composer items to bottom
    void on_mActionAlignBottom_triggered();

    //! Save window state
    void saveWindowState();

    /**Add a composer arrow to the item/widget map and creates a configuration widget for it*/
    void addComposerArrow( QgsComposerArrow* arrow );

    /**Add a composer map to the item/widget map and creates a configuration widget for it*/
    void addComposerMap( QgsComposerMap* map );

    /**Adds a composer label to the item/widget map and creates a configuration widget for it*/
    void addComposerLabel( QgsComposerLabel* label );

    /**Adds a composer scale bar to the item/widget map and creates a configuration widget for it*/
    void addComposerScaleBar( QgsComposerScaleBar* scalebar );

    /**Adds a composer legend to the item/widget map and creates a configuration widget for it*/
    void addComposerLegend( QgsComposerLegend* legend );

    /**Adds a composer picture to the item/widget map and creates a configuration widget*/
    void addComposerPicture( QgsComposerPicture* picture );

    /**Adds a composer shape to the item/widget map and creates a configuration widget*/
    void addComposerShape( QgsComposerShape* shape );

    /**Adds a composer table to the item/widget map and creates a configuration widget*/
    void addComposerTable( QgsComposerAttributeTable* table );

    /**Removes item from the item/widget map and deletes the configuration widget. Does not delete the item itself*/
    void deleteItem( QgsComposerItem* item );

    /**Shows the configuration widget for a composer item*/
    void showItemOptions( QgsComposerItem* i );

    //XML, usually connected with QgsProject::readProject and QgsProject::writeProject

    //! Stores state in Dom node
    void writeXML( QDomDocument& doc );

    //! Sets state from Dom document
    void readXML( const QDomDocument& doc );
    void readXML( const QDomElement& composerElem, const QDomDocument& doc, bool fromTemplate = false );

    void setSelectionTool();

    //! Raise, unminimize and activate this window
    void activate();

    void on_mButtonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }

  private:

    /**Establishes the signal slot connection for the class*/
    void connectSlots();

    //! True if a composer map contains a WMS layer
    bool containsWMSLayer() const;

    //! Displays a warning because of possible min/max size in WMS
    void showWMSPrintingWarning();

    //! Changes elements that are not suitable for this project
    void cleanupAfterTemplateRead();

    //! Print to a printer object
    void print( QPrinter &printer );

    //! Writes state under DOM element
    void writeXML( QDomNode& parentNode, QDomDocument& doc );

    //! Removes all the item from the graphics scene and deletes them
    void deleteItems();

    /**Composer title*/
    QString mTitle;

    //! Pointer to composer view
    QgsComposerView *mView;

    //! Current composition
    QgsComposition *mComposition;

    //! Pointer to QGIS application
    QgisApp *mQgis;

    //! The composer was opened first time (-> set defaults)
    bool mFirstTime;

    //! Layout
    QGridLayout *mCompositionOptionsLayout;

    //! Layout
    QGridLayout *mItemOptionsLayout;

    //! Size grip
    QSizeGrip *mSizeGrip;

    //! To know which item to show if selection changes
    QMap<QgsComposerItem*, QWidget*> mItemWidgetMap;

    //! Window menu action to select this window
    QAction *mWindowAction;

    //! Page & Printer Setup
    QPrinter mPrinter;

    QUndoView* mUndoView;
};

#endif
