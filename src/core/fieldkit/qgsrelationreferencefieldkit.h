/***************************************************************************
  qgsrelationreferencefieldkit.h - QgsRelationReferenceFieldKit

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

#include "qgsfieldkit.h"

class CORE_EXPORT QgsRelationReferenceFieldKit : public QgsFieldKit
{
  public:
    QgsRelationReferenceFieldKit();

    QString id() const override;

    virtual QString representValue( QgsVectorLayer* vl, int fieldIdx, const QVariantMap& config, const QVariant& cache, const QVariant& value ) const override;

    virtual QVariant sortValue( QgsVectorLayer *vl, int fieldIdx, const QVariantMap&config, const QVariant& cache, const QVariant& value ) const override;
};

#endif // QGSRELATIONREFERENCEFIELDKIT_H
