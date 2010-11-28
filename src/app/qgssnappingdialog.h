/***************************************************************************
                              qgssnappingdialog.h
                              --------------------------
  begin                : June 11, 2007
  copyright            : (C) 2007 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSNAPPINGDIALOG_H
#define QGSSNAPPINGDIALOG_H

#include "qgsmaplayer.h"
#include "ui_qgssnappingdialogbase.h"

class QDockWidget;

class QgsMapCanvas;

/**A dialog to enter advanced editing properties, e.g. topological editing, snapping settings
for the individual layers*/
class QgsSnappingDialog: public QDialog, private Ui::QgsSnappingDialogBase
{
    Q_OBJECT

  public:

    //! Returns the instance pointer, creating the object on the first call
    //static QgsSnappingDialog * instance( QgsMapCanvas* canvas );
    QgsSnappingDialog( QWidget* parent, QgsMapCanvas* canvas );
    ~QgsSnappingDialog();

  public slots:
    //! apply the changes
    void apply();

    //! show dialog or dock
    void show();

    //! connect to the layers destroyed() and then update()
    void connectUpdate( QgsMapLayer* theMapLayer );

    void on_cbxEnableTopologicalEditingCheckBox_stateChanged( int );
    void on_mAvoidIntersectionsPushButton_clicked();

  protected:
    /**Constructor
    @param canvas pointer to the map canvas (for detecting which vector layers are loaded
    */
    //QgsSnappingDialog( QgsMapCanvas* canvas );

    /**
     * Handle closing of the window
     * @param event unused
     */
    void closeEvent( QCloseEvent* event );


  private:
    /**Default constructor forbidden*/
    QgsSnappingDialog();

    /**Stores ids of layers where intersections of new polygons is considered. Is passed to / read from QgsAvoidIntersectionsDialog*/
    QSet<QString> mAvoidIntersectionsSettings;

    /**Used to query the loaded layers*/
    QgsMapCanvas* mMapCanvas;

    QDockWidget *mDock;

  private slots:
    //! update the Dialog
    void update();
};

#endif
