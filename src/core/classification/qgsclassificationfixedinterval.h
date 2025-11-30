/***************************************************************************
    qgsclassificationfixedinterval.h
    ---------------------
    begin                : May 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCLASSIFICATIONFIXEDINTERVAL_H
#define QGSCLASSIFICATIONFIXEDINTERVAL_H


#include "qgsclassificationmethod.h"

/**
 * \ingroup core
 * \brief Implementation of a fixed interval classification.
 *
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsClassificationFixedInterval : public QgsClassificationMethod
{
  public:

    QgsClassificationFixedInterval();
    [[nodiscard]] std::unique_ptr< QgsClassificationMethod > clone() const override;
    [[nodiscard]] QString name() const override;
    [[nodiscard]] QString id() const override;
    [[nodiscard]] QIcon icon() const override;
    [[nodiscard]] bool valuesRequired() const override;

  private:
    QList<double> calculateBreaks( double &minimum, double &maximum, const QList<double> &values, int nclasses, QString &error ) override;


};

#endif // QGSCLASSIFICATIONFIXEDINTERVAL_H
