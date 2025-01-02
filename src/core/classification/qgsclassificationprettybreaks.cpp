/***************************************************************************
    qgsclassificationprettybreaks.h
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

#include <QObject>

#include "qgsclassificationprettybreaks.h"
#include "qgssymbollayerutils.h"
#include "qgsapplication.h"


QgsClassificationPrettyBreaks::QgsClassificationPrettyBreaks()
  : QgsClassificationMethod( SymmetricModeAvailable )
{

}

QString QgsClassificationPrettyBreaks::name() const
{
  return QObject::tr( "Pretty Breaks" );
}

QString QgsClassificationPrettyBreaks::id() const
{
  return QStringLiteral( "Pretty" );
}

QList<double> QgsClassificationPrettyBreaks::calculateBreaks( double &minimum, double &maximum, const QList<double> &values, int nclasses, QString &error )
{
  Q_UNUSED( values );
  Q_UNUSED( error );
  QList<double> breaks = QgsSymbolLayerUtils::prettyBreaks( minimum, maximum, nclasses );

  if ( symmetricModeEnabled() )
    makeBreaksSymmetric( breaks, symmetryPoint(), symmetryAstride() );

  return breaks;
}

std::unique_ptr<QgsClassificationMethod> QgsClassificationPrettyBreaks::clone() const
{
  std::unique_ptr< QgsClassificationPrettyBreaks > c = std::make_unique< QgsClassificationPrettyBreaks >();
  copyBase( c.get() );
  return c;
}

QIcon QgsClassificationPrettyBreaks::icon() const
{
  return QgsApplication::getThemeIcon( "classification_methods/mClassificationPrettyBreak.svg" );
}
