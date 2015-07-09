/***************************************************************************
                              qgstininterpolatordialog.h
                              --------------------------
  begin                : March 29, 2008
  copyright            : (C) 2008 by Marco Hugentobler
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

#ifndef QGSTININTERPOLATORDIALOG_H
#define QGSTININTERPOLATORDIALOG_H

#include "qgsinterpolatordialog.h"
#include "ui_qgstininterpolatordialogbase.h"

class QgsTINInterpolatorDialog: public QgsInterpolatorDialog, private Ui::QgsTINInterpolatorDialogBase
{
    Q_OBJECT
  public:
    QgsTINInterpolatorDialog( QWidget* parent, QgisInterface* iface );

    ~QgsTINInterpolatorDialog();

    /**Method that returns an interpolator object from the settings or 0 in case of error.
     The calling method takes ownership of the created interpolator and is responsible for its proper destruction*/
    QgsInterpolator* createInterpolator() const override;

  private slots:
    void on_mExportTriangulationCheckBox_stateChanged( int state );
    void on_mTriangulationFileButton_clicked();
};

#endif
