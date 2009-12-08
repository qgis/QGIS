/***************************************************************************
 *   Copyright (C) 2003 by Tim Sutton                                      *
 *   tim@linfiniti.com                                                     *
 *                                                                         *
 *   This is a plugin generated from the QGIS plugin template              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef QGSGEOREFWARPOPTIONSDIALOG_H
#define QGSGEOREFWARPOPTIONSDIALOG_H

#include <cstdio>

#include "qgsimagewarper.h"

#include "ui_qgsgeorefwarpoptionsdialogbase.h"
#include <QDialog>

class QgsGeorefWarpOptionsDialog : public QDialog, private Ui::QgsGeorefWarpOptionsDialogBase
{
    Q_OBJECT
  public:

    QgsGeorefWarpOptionsDialog( QWidget* parent );
    void getWarpOptions( QgsImageWarper::ResamplingMethod& resampling,
                         bool& useZeroForTransparency, QString& compression );

  public slots:

    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

  private:

    QgsImageWarper::ResamplingMethod resampling;
    bool useZeroAsTransparency;

};

#endif
