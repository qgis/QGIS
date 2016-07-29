/***************************************************************************
 *  qgsgeometrysnapperdialog.h                                             *
 *  -------------------                                                    *
 *  copyright            : (C) 2014 by Sandro Mani / Sourcepole AG         *
 *  email                : smani@sourcepole.ch                             *
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS_GEOMETRY_SNAPPER_DIALOG_H
#define QGS_GEOMETRY_SNAPPER_DIALOG_H

#include <QDialog>
#include <QMutex>

#include "qgsfeature.h"
#include "ui_qgsgeometrysnapperdialog.h"

class QgisInterface;
class QgsSpatialIndex;
class QgsVectorLayer;

class QgsGeometrySnapperDialog: public QDialog, private Ui::QgsGeometrySnapperDialog
{
    Q_OBJECT
  public:
    explicit QgsGeometrySnapperDialog( QgisInterface * iface );

  private:
    QgisInterface* mIface;
    QAbstractButton* mRunButton;
    QString mOutputDriverName;

    QgsVectorLayer* getInputLayer();
    QgsVectorLayer* getReferenceLayer();

  private slots:
    void run();
    void updateLayers();
    void validateInput();
    void selectOutputFile();
    void progressStep();
};

#endif // QGS_GEOMETRY_SNAPPER_DIALOG_H

