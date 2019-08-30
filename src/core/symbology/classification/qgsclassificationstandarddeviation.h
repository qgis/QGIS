/***************************************************************************
    qgsclassificationstandarddeviation.h
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

#ifndef QGSCLASSIFICATIONSTANDARDDEVIATION_H
#define QGSCLASSIFICATIONSTANDARDDEVIATION_H

#include "qgis_core.h"
#include "qgsclassificationmethod.h"

/**
 * \ingroup core
 * QgsClassificationCustom is an implementation of QgsClassification
 * based on standard deviation
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsClassificationStandardDeviation : public QgsClassificationMethod
{
  public:
    QgsClassificationStandardDeviation();

    QString name() const override;
    QString id() const override;
    QgsClassificationMethod *clone() const override;
    QString labelForRange( const double &lowerValue, const double &upperValue, ClassPosition position ) const override;
    void saveExtra( QDomElement &element, const QgsReadWriteContext &context ) const override;
    void readExtra( const QDomElement &element, const QgsReadWriteContext &context ) override;

    static const QString METHOD_ID;

  private:
    QList<double> calculateBreaks( double minimum, double maximum,
                                   const QList<double> &values, int nclasses ) override;

    QString valueToLabel( const double &value ) const override;

    double mStdDev = 1.0;
};

#endif // QGSCLASSIFICATIONSTANDARDDEVIATION_H
