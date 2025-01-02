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
 * \brief Implementation of a logarithmic scale method
 *
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsClassificationLogarithmic : public QgsClassificationMethod
{
  public:

    /**
       * Handling of negative and 0 values in the method
       * \since QGIS 3.12
       */
    enum NegativeValueHandling
    {
      NoHandling = 0, //!< No handling
      Discard,        //!< Negative values are discarded - this will require all values
      PrependBreak    //!< Prepend an extra break to include negative values - this will require all values
    };

    QgsClassificationLogarithmic();
    std::unique_ptr< QgsClassificationMethod > clone() const override;
    QString name() const override;
    QString id() const override;
    QIcon icon() const override;
    QString labelForRange( double lowerValue, double upperValue, ClassPosition position ) const override;
    bool valuesRequired() const override;

  private:
    QList<double> calculateBreaks( double &minimum, double &maximum, const QList<double> &values, int nclasses, QString &error ) override;
    QString valueToLabel( double value ) const override;


};

#endif // QGSCLASSIFICATIONLOGARITHMIC_H
