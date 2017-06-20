/***************************************************************************
                          qgsmetadatawizard.h  -  description
                             -------------------
    begin                : 17/05/2017
    copyright            : (C) 2017 by Etienne Trimaille
    email                : etienne at kartoza.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMETADATAWIZARD_H
#define QGSMETADATAWIZARD_H

#include "qgis_app.h"
#include "qgsmaplayer.h"
#include "ui_qgsmetadatawizard.h"

class APP_EXPORT QgsMetadataWizard : public QDialog, private Ui::QgsMetadataWizard
{
    Q_OBJECT

  public:
    QgsMetadataWizard( QWidget *parent, QgsMapLayer *layer = nullptr );
    ~QgsMetadataWizard();

    void addLink();
    void removeLink();

  private:
    void cancelClicked();
    void backClicked();
    void nextClicked();
    void finishedClicked();
    void updatePanel();
    QgsMapLayer *mLayer = nullptr;
};

#endif
