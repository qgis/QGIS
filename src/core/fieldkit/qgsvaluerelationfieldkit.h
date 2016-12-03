/***************************************************************************
  qgsvaluerelationfieldkit.h - QgsValueRelationFieldKit

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
#ifndef QGSVALUERELATIONFIELDKIT_H
#define QGSVALUERELATIONFIELDKIT_H

#include "qgsfieldkit.h"

class QgsValueRelationFieldKit : public QgsFieldKit
{
  public:
    QgsValueRelationFieldKit();

    QString representValue( QgsVectorLayer *layer, int fieldIdx, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const override;

    QVariant sortValue( QgsVectorLayer *vl, int fieldIdx, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const override;
};

#endif // QGSVALUERELATIONFIELDKIT_H
