/***************************************************************************
  qgsfields_p - %{Cpp:License:ClassName}

 ---------------------
 begin                : 22.9.2016
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
#ifndef QGSFIELDS_P_H
#define QGSFIELDS_P_H


/// @cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include "qgis_core.h"
#include <QSharedData>
#include "qgsfields.h"

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfields.cpp.
 * See details in QEP #17
 ****************************************************************************/

class QgsFieldsPrivate : public QSharedData
{
  public:

    QgsFieldsPrivate()
    {
    }

    QgsFieldsPrivate( const QgsFieldsPrivate &other )
      : QSharedData( other )
      , fields( other.fields )
      , nameToIndex( other.nameToIndex )
    {
    }

    ~QgsFieldsPrivate() {}

    //! internal storage of the container
    QVector<QgsFields::Field> fields;

    //! map for quick resolution of name to index
    QHash<QString, int> nameToIndex;

};

/// @endcond

#endif // QGSFIELDS_P_H
