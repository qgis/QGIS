/***************************************************************************
  qgsvaluerelationfieldformatter.sip - QgsValueRelationFieldFormatter

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
class QgsValueRelationFieldFormatter : QgsFieldFormatter
{
%TypeHeaderCode
#include "qgsvaluerelationfieldformatter.h"
%End

  public:
    struct ValueRelationItem
    {
      ValueRelationItem( const QVariant& key, const QString& value );

      ValueRelationItem();

      QVariant key;
      QString value;
    };

    typedef QVector < QgsValueRelationFieldFormatter::ValueRelationItem > ValueRelationCache;

    QgsValueRelationFieldFormatter();

    QString id() const;
    QString representValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const;

    QVariant sortValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const;

    QVariant createCache( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config ) const;

    static QgsValueRelationFieldFormatter::ValueRelationCache createCache( const QVariantMap& config );
};
