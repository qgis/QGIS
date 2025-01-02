/***************************************************************************
    qgsclassificationcustom.h
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

#ifndef QGSCLASSIFICATIONCUSTOM_H
#define QGSCLASSIFICATIONCUSTOM_H

#include "qgsclassificationmethod.h"


/**
 * \ingroup core
 * \brief QgsClassificationCustom is a dummy implementation of QgsClassification
 * which does not compute any break.
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsClassificationCustom : public QgsClassificationMethod
{
  public:
    QgsClassificationCustom();

    std::unique_ptr< QgsClassificationMethod > clone() const override;
    QString name() const override;
    QString id() const override;

    bool valuesRequired() const override {return false;}

    static const QString METHOD_ID;

  private:
    QList<double> calculateBreaks( double &minimum, double &maximum,
                                   const QList<double> &values, int nclasses, QString &error ) override;
};

#endif // QGSCLASSIFICATIONCUSTOM_H
