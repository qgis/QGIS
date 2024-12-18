/***************************************************************************
    qgsclassificationjenks.h
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

#ifndef QGSCLASSIFICATIONJENKS_H
#define QGSCLASSIFICATIONJENKS_H

#include "qgis_core.h"
#include "qgsclassificationmethod.h"

/**
 * \ingroup core
 * \brief QgsClassificationJenks is an implementation of QgsClassificationMethod
 * for natural breaks based on Jenks method
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsClassificationJenks : public QgsClassificationMethod
{
  public:
    QgsClassificationJenks();

    QString name() const override;
    QString id() const override;
    std::unique_ptr< QgsClassificationMethod > clone() const override;
    QIcon icon() const override;

  private:
    QList<double> calculateBreaks( double &minimum, double &maximum,
                                   const QList<double> &values, int nclasses, QString &error ) override;

    int mMaximumSize = 3000;
};

#endif // QGSCLASSIFICATIONJENKS_H
