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

#include "qgsimagewarper.h"

#include "qgsgeorefwarpoptionsdialogbase.uic.h"


class QgsGeorefWarpOptionsDialog : public QgsGeorefWarpOptionsDialogBase 
{
Q_OBJECT
public:
  
  QgsGeorefWarpOptionsDialog(QWidget* parent);
  void getWarpOptions(QgsImageWarper::ResamplingMethod& resampling, 
		      bool& useZeroForTransparency);
  
public slots:

  void pbnOK_clicked();
  
private:
  
  QgsImageWarper::ResamplingMethod resampling;
  bool useZeroAsTransparency;
  
};

#endif
