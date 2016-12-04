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

#include <QVector>
#include <QVariant>

class CORE_EXPORT QgsValueRelationFieldKit : public QgsFieldKit
{
  public:
    struct ValueRelationItem
    {
      ValueRelationItem( const QVariant& key, const QString& value )
          : key( key )
          , value( value )
      {}

      ValueRelationItem()
      {}

      QVariant key;
      QString value;
    };

    typedef QVector < ValueRelationItem > ValueRelationCache;

    QgsValueRelationFieldKit();

    QString id() const override;
    QString representValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const override;

    QVariant sortValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const override;

    QVariant createCache( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config ) const override;

    static ValueRelationCache createCache( const QVariantMap& config );
};

Q_DECLARE_METATYPE( QgsValueRelationFieldKit::ValueRelationCache )

#endif // QGSVALUERELATIONFIELDKIT_H
