/***************************************************************************
    qgsclassificationprettybreaks.h
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

#ifndef QGSCLASSIFICATIONPRETTYBREAKS_H
#define QGSCLASSIFICATIONPRETTYBREAKS_H

#include "qgis_core.h"
#include "qgsclassificationmethod.h"

/**
 * \ingroup core
 * QgsClassificationPrettryBreaks is an implementation of QgsClassificationMethod
 * for pretty breaks
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsClassificationPrettyBreaks : public QgsClassificationMethod
{
  public:
    QgsClassificationPrettyBreaks();

    QString name() const override;
    QString id() const override;
    QgsClassificationMethod *clone() const override;
    QIcon icon() const override;

    bool valuesRequired() const override {return false;}

  private:
    QList<double> calculateBreaks( double &minimum, double &maximum,
                                   const QList<double> &values, int nclasses ) override;
};

#endif // QGSCLASSIFICATIONPRETTYBREAKS_H
