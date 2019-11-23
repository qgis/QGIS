/***************************************************************************
    qgsclassificationequalinterval.h
    ---------------------
    begin                : September 2019
    copyright            : (C) 2019 by Denis Rouzaud
    email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCLASSIFICATIONEQUALINTERVAL_H
#define QGSCLASSIFICATIONEQUALINTERVAL_H

#include "qgis_core.h"
#include "qgsclassificationmethod.h"

/**
 * \ingroup core
 * QgsClassificationEqualInterval is an implementation of QgsClassificationMethod
 * for equal intervals
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsClassificationEqualInterval : public QgsClassificationMethod
{
  public:
    QgsClassificationEqualInterval();

    QString name() const override;
    QString id() const override;
    QgsClassificationMethod *clone() const override;
    QIcon icon() const override;

    static const QString METHOD_ID;

  private:
    QList<double> calculateBreaks( double minimum, double maximum,
                                   const QList<double> &values, int nclasses ) override;


};

#endif // QGSCLASSIFICATIONEQUALINTERVAL_H
