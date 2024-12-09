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

#include "ui_qgsdisplayanglebase.h"
#include "qgis_app.h"

class QgsMapTool;

//! A class that displays results of angle measurements with the proper unit
class APP_EXPORT QgsDisplayAngle : public QDialog, private Ui::QgsDisplayAngleBase
{
    Q_OBJECT

  public:
    QgsDisplayAngle( QgsMapTool *tool = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );

    /**
     * Sets the measured angle value (in radians). The value is going to
     * be converted to degrees / gon automatically if necessary.
    */
    void setAngleInRadians( double value );

    /**
     * Sets the measured bearing (in radians).
    */
    void setBearingInRadians( double value );

    /**
     * Returns the current angular value (in radians)
     */
    double value() const { return mValue; }

    /**
     * Returns the current value, as a string.
     */
    QString text() const { return mAngleLineEdit->text(); }

  private:
    //! The value we're showing
    double mValue = 0.0;
};

#endif // QGSDISPLAYANGLE_H
