/***************************************************************************
                          qgssimadialog.h 
 Single marker renderer dialog
                             -------------------
    begin                : March 2004
    copyright            : (C) 2004 by Marco Hugentobler
    email                : mhugent@geo.unizh.ch
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

#include "qgssimadialogbase.uic.h"

class QgsVectorLayer;

class QgsSiMaDialog: public QgsSiMaDialogBase
{
    Q_OBJECT
 public:
    QgsSiMaDialog(QgsVectorLayer* vectorlayer);
    ~QgsSiMaDialog();
 public slots:
     void apply();
 protected:
    QgsVectorLayer* mVectorLayer;
    bool mMarkerSizeDirty;
 protected slots:
     void selectMarker();
     void updateMarkerSize();
     void setMarkerSizeDirty();
 private:
    /**Default constructor is privat to not use is*/
     QgsSiMaDialog();
};
