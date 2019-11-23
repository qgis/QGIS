/***************************************************************************
    qgsclassificationcustom.cpp
    ---------------------
    begin                : September 2019
    copyright            : (C) 2019 by Denis Rouzaud
    email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsclassificationcustom.h"

const QString QgsClassificationCustom::METHOD_ID = QStringLiteral( "Custom" );


QgsClassificationCustom::QgsClassificationCustom()
  : QgsClassificationMethod( ValuesNotRequired,
                             0 /*codeComplexity*/ )
{
}


QgsClassificationMethod *QgsClassificationCustom::clone() const
{
  QgsClassificationCustom *c = new QgsClassificationCustom();
  copyBase( c );
  return c;
}

QString QgsClassificationCustom::name() const
{
  return QObject::tr( "Custom" );
}

QString QgsClassificationCustom::id() const
{
  return METHOD_ID;
}

QList<double> QgsClassificationCustom::calculateBreaks( double minimum, double maximum,
    const QList<double> &values, int nclasses )
{
  Q_UNUSED( minimum )
  Q_UNUSED( maximum )
  Q_UNUSED( values )
  Q_UNUSED( nclasses )
  return QList<double>();
}
