/***************************************************************************
                         qgssisydialog.h  -  description
                             -------------------
    begin                : Oct 2003
    copyright            : (C) 2003 by Marco Hugentobler
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

#ifndef QGSSISYDIALOG_H
#define QGSSISYDIALOG_H

#include "qgssisydialogbase.h"

class QgsVectorLayer;

/**QgsSiSyDialog is a dialog to set symbology for the legend type 'single symbol'*/
class QgsSiSyDialog: public QgsSiSyDialogBase
{
    Q_OBJECT
 public:
    QgsSiSyDialog(QgsVectorLayer* layer);
    ~QgsSiSyDialog();
 protected:
    QgsVectorLayer* m_vectorlayer;
 public slots:
     /**applies the changes to the vector layer*/
    void apply();
 protected slots:
    void selectOutlineColor();
    void selectOutlineStyle();
    void selectFillColor();
    void selectFillPattern();
 private:
    /**Default constructor is privat to not use is*/
     QgsSiSyDialog();
};

#endif
