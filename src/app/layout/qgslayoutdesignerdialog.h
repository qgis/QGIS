/***************************************************************************
                             qgslayoutdesignerdialog.h
                             -------------------------
    begin                : July 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLAYOUTDESIGNERDIALOG_H
#define QGSLAYOUTDESIGNERDIALOG_H

#include "ui_qgslayoutdesignerbase.h"
#include "qgslayoutdesignerinterface.h"

class QgsLayoutDesignerDialog;
class QgsLayoutView;
class QgsLayoutViewToolAddItem;
class QgsLayoutViewToolPan;
class QgsLayoutViewToolZoom;
class QgsLayoutViewToolSelect;

class QgsAppLayoutDesignerInterface : public QgsLayoutDesignerInterface
{
    Q_OBJECT

  public:
    QgsAppLayoutDesignerInterface( QgsLayoutDesignerDialog *dialog );
    QgsLayout *layout() override;
    QgsLayoutView *view() override;

  public slots:

    void close() override;

  private:

    QgsLayoutDesignerDialog *mDesigner = nullptr;
};

/**
 * \ingroup app
 * \brief A window for designing layouts.
 */
class QgsLayoutDesignerDialog: public QMainWindow, private Ui::QgsLayoutDesignerBase
{
    Q_OBJECT

  public:

    QgsLayoutDesignerDialog( QWidget *parent = nullptr, Qt::WindowFlags flags = 0 );

    /**
     * Returns the designer interface for the dialog.
     */
    QgsAppLayoutDesignerInterface *iface();

    /**
     * Returns the current layout associated with the designer.
     * \see setCurrentLayout()
     */
    QgsLayout *currentLayout();

    /**
     * Returns the layout view utilized by the designer.
     */
    QgsLayoutView *view();

    /**
     * Sets the current \a layout to edit in the designer.
     * \see currentLayout()
     */
    void setCurrentLayout( QgsLayout *layout );

    /**
     * Sets the icon \a size for the dialog.
     */
    void setIconSizes( int size );


  public slots:

    /**
     * Opens the dialog, and sets default view.
     */
    void open();

    /**
     * Raise, unminimize and activate this window.
     */
    void activate();

    /**
     * Zooms to show full layout.
     */
    void zoomFull();

  signals:

    /**
     * Emitted when the dialog is about to close.
     */
    void aboutToClose();

  protected:

    virtual void closeEvent( QCloseEvent * ) override;

  private slots:

    void itemTypeAdded( int type );

  private:

    QgsAppLayoutDesignerInterface *mInterface = nullptr;

    QgsLayout *mLayout = nullptr;

    QActionGroup *mToolsActionGroup = nullptr;

    QgsLayoutView *mView = nullptr;

    QgsLayoutViewToolAddItem *mAddItemTool = nullptr;
    QgsLayoutViewToolPan *mPanTool = nullptr;
    QgsLayoutViewToolZoom *mZoomTool = nullptr;
    QgsLayoutViewToolSelect *mSelectTool = nullptr;

    //! Save window state
    void saveWindowState();

    //! Restore the window and toolbar state
    void restoreWindowState();

    //! Switch to new item creation tool, for a new item of the specified \a type.
    void activateNewItemCreationTool( int type );

};

#endif // QGSLAYOUTDESIGNERDIALOG_H

