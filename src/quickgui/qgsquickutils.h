/***************************************************************************
  qgsquickutils.cpp
  --------------------------------------
  Date                 : 7.11.2022
  Copyright            : (C) 2022 by Tomas Mizera
  Email                : tomas.mizera (at) lutraconsulting.co.uk
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSQUICKUTILS_H
#define QGSQUICKUTILS_H

#include <QObject>
#include <qglobal.h>

#include "qgis_quick.h"

#include "qgspoint.h"

/**
 * \ingroup quick
 *
 * \brief The QgsQuickUtils class serves as a utility class for common operations
 * needed either from QML or cpp.
 *
 * \note use in qml as a singleton, registered as "Utils", e.g. QgsQuick.Utils.toQgsPoint
 *
 * \since QGIS 3.30
 */
class QUICK_EXPORT QgsQuickUtils : public QObject
{
    Q_OBJECT

  public:

    //! Creates new QgsQuickUtils - this class is meant to serve as a QML singleton
    explicit QgsQuickUtils( QObject *parent = nullptr );
    ~QgsQuickUtils() = default;

    /**
     * Helper function to convert QPointF to QgsPoint without any transformations.
     * Useful for converting these values in QML.
     */
    Q_INVOKABLE static QgsPoint toQgsPoint( const QPointF &point );
};

#endif // QGSQUICKUTILS_H
