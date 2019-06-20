/***************************************************************************
                          qgsmeshtimeformatdialog.h
                          -------------------------
    begin                : March 2019
    copyright            : (C) 2019 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESHTIMEFORMATDIALOG_H
#define QGSMESHTIMEFORMATDIALOG_H

#include "ui_qgsmeshtimeformatdialog.h"
#include "qgsmeshcalculator.h"
#include "qgshelp.h"
#include "qgis_app.h"

//! A dialog to enter a mesh calculation expression
class APP_EXPORT QgsMeshTimeFormatDialog: public QDialog, private Ui::QgsMeshTimeFormatDialog
{
    Q_OBJECT
  public:

    /**
     * Constructor for raster calculator dialog
     * \param meshLayer main mesh layer, will be used for default extent and projection
     * \param parent widget
     * \param f window flags
     */
    QgsMeshTimeFormatDialog( QgsMeshLayer *meshLayer = nullptr, QWidget *parent = nullptr, Qt::WindowFlags f = nullptr );
    ~QgsMeshTimeFormatDialog();

  private slots:

  private:
    void loadSettings();
    void saveSettings();
    void enableGroups( bool useAbsoluteTime );

    QgsMeshLayer *mLayer;
};

#endif // QGSMESHTIMEFORMATDIALOG_H
