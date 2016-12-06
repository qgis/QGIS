/***************************************************************************
  qgslistfieldformatter.sip - QgsListFieldFormatter

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
class QgsListFieldFormatter : QgsFieldFormatter
{
%TypeHeaderCode
#include "qgslistfieldformatter.h"
%End

  public:
    QgsListFieldFormatter();

    QString id() const;

    QString representValue( QgsVectorLayer* layer, int fieldIndex, const QVariantMap& config, const QVariant& cache, const QVariant& value ) const;
};
