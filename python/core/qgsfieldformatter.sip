/***************************************************************************
  qgsfieldformatter.sip - QgsFieldFormatter

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
class QgsFieldFormatter
{
%TypeHeaderCode
#include "qgsfieldformatter.h"
%End
  public:
    QgsFieldFormatter();

    virtual ~QgsFieldFormatter();

    virtual QString id() const = 0;

    virtual QString representValue( QgsVectorLayer* layer, int fieldIdx, const QVariantMap& config, const QVariant& cache, const QVariant& value ) const;

    virtual QVariant sortValue( QgsVectorLayer* vl, int fieldIdx, const QVariantMap& config, const QVariant& cache, const QVariant& value ) const;

    virtual Qt::AlignmentFlag alignmentFlag( QgsVectorLayer* vl, int fieldIdx, const QVariantMap& config ) const;

    virtual QVariant createCache( QgsVectorLayer* vl, int fieldIdx, const QVariantMap& config ) const;
};
