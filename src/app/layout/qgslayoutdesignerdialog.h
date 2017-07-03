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

class QgsAppLayoutDesignerInterface : public QgsLayoutDesignerInterface
{
    Q_OBJECT

  public:
    QgsAppLayoutDesignerInterface( QgsLayoutDesignerDialog *dialog );
    QgsLayout *layout() override;

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

  signals:

    /**
     * Emitted when the dialog is about to close.
     */
    void aboutToClose();

  protected:

    virtual void closeEvent( QCloseEvent * ) override;

  private:

    QgsAppLayoutDesignerInterface *mInterface = nullptr;

    QgsLayout *mLayout = nullptr;

    //! Save window state
    void saveWindowState();

    //! Restore the window and toolbar state
    void restoreWindowState();


};

#endif // QGSLAYOUTDESIGNERDIALOG_H

