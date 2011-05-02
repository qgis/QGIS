/***************************************************************************
                              qgsinterpolatordialog.h
                              ------------------------
  begin                : March 25, 2008
  copyright            : (C) 2008 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSINTERPOLATORDIALOG_H
#define QGSINTERPOLATORDIALOG_H

#include "qgsinterpolator.h"
#include <QDialog>
#include <QList>

class QgsVectorLayer;
class QgisInterface;

/**Abstract base class for dialogs that allow to enter the options for interpolators*/
class QgsInterpolatorDialog: public QDialog
{
  public:
    QgsInterpolatorDialog( QWidget* parent, QgisInterface* iface );
    virtual ~QgsInterpolatorDialog();

    /**Method that returns an interpolator object from the settings or 0 in case of error.
     The calling method takes ownership of the created interpolator and is responsible for its proper destruction*/
    virtual QgsInterpolator* createInterpolator() const = 0;

    void setInputData( const QList< QgsInterpolator::LayerData >& inputData );

  protected:
    /**Pointer to the running QGIS instance. This may be necessary to show interpolator properties on the map (e.g. triangulation)*/
    QgisInterface* mInterface;

    /**A list of input data layers, their interpolation attribute and their type (point, structure lines, breaklines)*/
    QList< QgsInterpolator::LayerData > mInputData;
};

#endif
