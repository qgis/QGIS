/***************************************************************************
    qgsclassificationlogarithmic.h
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

#ifndef QGSCLASSIFICATIONLOGARITHMIC_H
#define QGSCLASSIFICATIONLOGARITHMIC_H


#include "qgsclassificationmethod.h"

/**
 * \ingroup core
 * Implementation of a logarithmic scale method
 *
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsClassificationLogarithmic : public QgsClassificationMethod
{
  public:
    QgsClassificationLogarithmic();
    QgsClassificationMethod *clone() const override;
    QString name() const override;
    QString id() const override;
    QIcon icon() const override;
    QString labelForRange( double lowerValue, double upperValue, ClassPosition position ) const override;

  private:
    QList<double> calculateBreaks( double minimum, double maximum, const QList<double> &values, int nclasses ) override;
    QString valueToLabel( double value ) const override;
};

#endif // QGSCLASSIFICATIONLOGARITHMIC_H
