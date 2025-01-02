/***************************************************************************
    qgsclassificationprettybreaks.h
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

#ifndef QGSCLASSIFICATIONPRETTYBREAKS_H
#define QGSCLASSIFICATIONPRETTYBREAKS_H

#include "qgis_core.h"
#include "qgsclassificationmethod.h"

/**
 * \ingroup core
 * \brief QgsClassificationPrettyBreaks is an implementation of QgsClassificationMethod
 * for pretty breaks
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsClassificationPrettyBreaks : public QgsClassificationMethod
{
  public:
    QgsClassificationPrettyBreaks();

    QString name() const override;
    QString id() const override;
    std::unique_ptr< QgsClassificationMethod > clone() const override;
    QIcon icon() const override;

    bool valuesRequired() const override {return false;}

  private:
    QList<double> calculateBreaks( double &minimum, double &maximum,
                                   const QList<double> &values, int nclasses, QString &error ) override;
};

#endif // QGSCLASSIFICATIONPRETTYBREAKS_H
