/***************************************************************************
  qgsrelationreferencefieldformatter.h - QgsRelationReferenceFieldFormatter

 ---------------------
 begin                : 3.12.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRELATIONREFERENCEFIELDKIT_H
#define QGSRELATIONREFERENCEFIELDKIT_H

#include "qgis_core.h"
#include "qgsfieldformatter.h"

/**
 * \ingroup core
 * \brief Field formatter for a relation reference field.
 *
 * A value relation field formatter looks up the values from
 * features on another layer.
 *
 */
class CORE_EXPORT QgsRelationReferenceFieldFormatter : public QgsFieldFormatter
{
  public:

    /**
      * Default constructor of field formatter for a relation reference field.
      */
    QgsRelationReferenceFieldFormatter();

    QString id() const override;

    QString representValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const override;

    QVariant sortValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const override;

    QVariant createCache( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config ) const override;

    QList<QgsVectorLayerRef> layerDependencies( const QVariantMap &config ) const override SIP_SKIP;

    QVariantList availableValues( const QVariantMap &config, int countLimit, const QgsFieldFormatterContext &context ) const override;

    //friend class TestQgsRelationReferenceFieldFormatter;

};

#endif // QGSRELATIONREFERENCEFIELDKIT_H
