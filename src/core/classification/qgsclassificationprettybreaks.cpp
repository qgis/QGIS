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

#include "qgsclassificationprettybreaks.h"

#include "qgsapplication.h"
#include "qgssymbollayerutils.h"

#include <QObject>

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
  return u"Pretty"_s;
}

QList<double> QgsClassificationPrettyBreaks::calculateBreaks( double &minimum, double &maximum, const QList<double> &values, int nclasses, QString &error )
{
  Q_UNUSED( values );
  Q_UNUSED( error );
  QList<double> breaks = QgsSymbolLayerUtils::prettyBreaks( minimum, maximum, nclasses );

  if ( symmetricModeEnabled() )
    makeBreaksSymmetric( breaks, symmetryPoint(), symmetryAstride() );

  // Special case for single class
  if ( minimum == maximum && breaks.isEmpty() )
  {
    // 1 is totally arbitrary but we need something
    breaks << maximum + 1.0;
  }

  return breaks;
}

std::unique_ptr<QgsClassificationMethod> QgsClassificationPrettyBreaks::clone() const
{
  auto c = std::make_unique< QgsClassificationPrettyBreaks >();
  copyBase( c.get() );
  return c;
}

QIcon QgsClassificationPrettyBreaks::icon() const
{
  return QgsApplication::getThemeIcon( "classification_methods/mClassificationPrettyBreak.svg" );
}
