/***************************************************************************
                      qgsgeometryvalidationdock.h
                     --------------------------------------
Date                 : 7.9.2018
Copyright            : (C) 2018 by Matthias Kuhn
email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGEOMETRYVALIDATIONPANEL_H
#define QGSGEOMETRYVALIDATIONPANEL_H

#include "ui_qgsgeometryvalidationdockbase.h"
#include "qgsdockwidget.h"

class QgsGeometryValidationModel;

/**
 * @brief The QgsGeometryValidationDock class
 */
class QgsGeometryValidationDock : public QgsDockWidget, public Ui_QgsGeometryValidationDockBase
{
    Q_OBJECT

  public:
    QgsGeometryValidationDock( const QString &title, QWidget *parent = nullptr, Qt::WindowFlags flags = nullptr );

    QgsGeometryValidationModel *geometryValidationModel() const;
    void setGeometryValidationModel( QgsGeometryValidationModel *geometryValidationModel );

  private:
    QgsGeometryValidationModel *mGeometryValidationModel = nullptr;
};

#endif // QGSGEOMETRYVALIDATIONPANEL_H
