/***************************************************************************
  qgsvaluerelationfieldkit.sip - QgsValueRelationFieldKit

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
class QgsValueRelationFieldKit : QgsFieldKit
{
%TypeHeaderCode
#include "qgsvaluerelationfieldkit"
%End

  public:
    struct ValueRelationItem
    {
      ValueRelationItem( const QVariant& key, const QString& value );

      ValueRelationItem();

      QVariant key;
      QString value;
    };

    typedef QVector < ValueRelationItem > ValueRelationCache;

    QgsValueRelationFieldKit();

    QString id() const;
    QString representValue( QgsVectorLayer *layer, int fieldIdx, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const;

    QVariant sortValue( QgsVectorLayer *vl, int fieldIdx, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const;

    QVariant createCache( QgsVectorLayer *vl, int fieldIdx, const QVariantMap &config ) const;

    static ValueRelationCache createCache( const QVariantMap& config );
};
