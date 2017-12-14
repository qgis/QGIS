/***************************************************************************
  qgsfallbackfieldformatter.h - QgsFallbackFieldFormatter

 ---------------------
 begin                : 4.12.2016
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
#ifndef QGSFALLBACKFIELDKIT_H
#define QGSFALLBACKFIELDKIT_H

#include "qgis_core.h"
#include "qgsfieldformatter.h"

/**
 * \ingroup core
 * A default fallback field formatter in case no specialized field formatter is defined.
 * The values will be returned unmodified.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsFallbackFieldFormatter : public QgsFieldFormatter
{
  public:

    /**
      * Default constructor of field formatter as a fallback when no specialized formatter is defined.
      */
    QgsFallbackFieldFormatter() = default;
    QString id() const override;
};

#endif // QGSFALLBACKFIELDKIT_H
