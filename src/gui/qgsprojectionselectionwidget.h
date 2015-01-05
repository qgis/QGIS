/***************************************************************************
    qgsprojectionselectionwidget.h
     --------------------------------------
    Date                 : 05.01.2015
    Copyright            : (C) 2015 Denis Rouzaud
    Email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSPROJECTIONSELECTIONWIDGET_H
#define QGSPROJECTIONSELECTIONWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QToolButton>

#include "qgscoordinatereferencesystem.h"

class GUI_EXPORT QgsProjectionSelectionWidget : public QWidget
{
    Q_OBJECT
  public:
    explicit QgsProjectionSelectionWidget( QWidget *parent = 0 );

  signals:
    void crsChanged( QgsCoordinateReferenceSystem );

  public slots:
    void setCrs( QgsCoordinateReferenceSystem crs );
    void selectCrs();

  private:
    QgsCoordinateReferenceSystem mCrs;
    QLineEdit* mCrsLineEdit;
    QToolButton* mButton;
};

#endif // QGSPROJECTIONSELECTIONWIDGET_H
