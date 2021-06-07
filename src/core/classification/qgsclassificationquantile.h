/***************************************************************************
    qgsclassificationquantile.h
    ---------------------
    begin                : September 2019
    copyright            : (C) 2019 by Denis Rouzaud
    email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCLASSIFICATIONQUANTILE_H
#define QGSCLASSIFICATIONQUANTILE_H

#include "qgis_core.h"
#include "qgsclassificationmethod.h"


/**
 * \ingroup core
 * QgsClassificationQuantile is an implementation of QgsClassificationMethod
 * based on quantiles
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsClassificationQuantile : public QgsClassificationMethod
{
  public:
    QgsClassificationQuantile();

    QString name() const override;
    QString id() const override;
    QgsClassificationMethod *clone() const override;
    QIcon icon() const override;

  private:
    QList<double> calculateBreaks( double &minimum, double &maximum,
                                   const QList<double> &values, int nclasses ) override;
};

#endif // QGSCLASSIFICATIONQUANTILE_H
