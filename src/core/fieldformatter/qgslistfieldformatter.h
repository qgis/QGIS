/***************************************************************************
  qgslistfieldformatter.h - QgsListFieldFormatter

 ---------------------
 begin                : 3.12.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLISTFIELDKIT_H
#define QGSLISTFIELDKIT_H

#include "qgis_core.h"
#include "qgsfieldformatter.h"

/**
 * \ingroup core
 * Field formatter for a list field.
 * This represents a list type value.
 * Values will be represented as a comma-separated list.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsListFieldFormatter : public QgsFieldFormatter
{
  public:

    /**
      * Default constructor of field formatter for a list field.
      */
    QgsListFieldFormatter() = default;
    QString id() const override;

    QString representValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const override;
};

#endif // QGSLISTFIELDKIT_H
