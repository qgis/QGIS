/***************************************************************************
  qgscheckboxfieldformatter.h - QgsCheckBoxFieldFormatter

 ---------------------
 begin                : 23.09.2019
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
#ifndef QGSCHECKBOXFIELDFORMATTER_H
#define QGSCHECKBOXFIELDFORMATTER_H

#include "qgis_core.h"
#include "qgsfieldformatter.h"


/**
 * \ingroup core
 * Field formatter for a checkbox field.
 *
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsCheckBoxFieldFormatter : public QgsFieldFormatter
{
  public:

    /**
     * Constructor for QgsCheckBoxFieldFormatter.
     */
    QgsCheckBoxFieldFormatter() = default;

    QString id() const override;

    QString representValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const override;
};


#endif // QGSCHECKBOXFIELDFORMATTER_H
