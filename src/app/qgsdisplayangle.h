/***************************************************************************
    qgsdisplayangle.h
    ------------------------
    begin                : January 2010
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco at hugis dot net
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDISPLAYANGLE_H
#define QGSDISPLAYANGLE_H

#include "qgsmaptoolmeasureangle.h"
#include "ui_qgsdisplayanglebase.h"

/** A class that displays results of angle measurements with the proper unit*/
class APP_EXPORT QgsDisplayAngle: public QDialog, private Ui::QgsDisplayAngleBase
{
    Q_OBJECT

  public:
    QgsDisplayAngle( QgsMapToolMeasureAngle * tool = nullptr, Qt::WindowFlags f = nullptr );
    ~QgsDisplayAngle();
    /** Sets the measured angle value (in radians). The value is going to
      be converted to degrees / gon automatically if necessary*/
    void setValueInRadians( double value );

  private:
    //! pointer to tool which owns this dialog
    QgsMapToolMeasureAngle * mTool;

    //! The value we're showing
    double mValue;

    //! Updates UI according to user settings.
    void updateUi();
};

#endif // QGSDISPLAYANGLE_H
