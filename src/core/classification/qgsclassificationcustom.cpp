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
  : QgsClassificationMethod( NoFlag,
                             0 /*codeComplexity*/ )
{
}


std::unique_ptr<QgsClassificationMethod> QgsClassificationCustom::clone() const
{
  std::unique_ptr< QgsClassificationCustom > c = std::make_unique< QgsClassificationCustom >();
  copyBase( c.get() );
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

QList<double> QgsClassificationCustom::calculateBreaks( double &minimum, double &maximum,
    const QList<double> &values, int nclasses, QString &error )
{
  Q_UNUSED( minimum )
  Q_UNUSED( maximum )
  Q_UNUSED( values )
  Q_UNUSED( nclasses )
  Q_UNUSED( error )
  return QList<double>();
}
