/***************************************************************************
  qgsdatetimefieldformatter.sip - QgsDateTimeFieldFormatter

 ---------------------
 begin                : 2.12.2016
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
class QgsDateTimeFieldFormatter : QgsFieldFormatter
{
%TypeHeaderCode
#include "qgsdatetimefieldformatter.h"
%End
  public:
    QgsDateTimeFieldFormatter();

    QString id() const;

    virtual QString representValue( QgsVectorLayer* layer, int fieldIndex, const QVariantMap& config, const QVariant& cache, const QVariant& value ) const;

    static QString defaultFormat( const QVariant::Type type );
};
