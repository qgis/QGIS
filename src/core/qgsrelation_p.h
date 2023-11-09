/***************************************************************************
               qgsrelation_p.h
               --------------------------
    begin                : August 2018
    copyright            : (C) 2018 Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRELATION_P_H
#define QGSRELATION_P_H

#define SIP_NO_FILE

/// @cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include "qgsrelation.h"

#include <QSharedData>
#include <QPointer>

class QgsRelationPrivate : public QSharedData
{
  public:
    QgsRelationPrivate() = default;

    //! Unique Id
    QString mRelationId;
    //! Human redable name
    QString mRelationName;
    //! The child layer
    QString mReferencingLayerId;
    //! The child layer
    QPointer<QgsVectorLayer> mReferencingLayer;
    //! The parent layer id
    QString mReferencedLayerId;
    //! The parent layer
    QPointer<QgsVectorLayer> mReferencedLayer;
    //! The relation strength: Association, Composition
    Qgis::RelationshipStrength mRelationStrength = Qgis::RelationshipStrength::Association;
    //! The parent polymorphic relation id. If the relation is a normal relation, a null string is returned.
    QString mPolymorphicRelationId;

    /**
     * A list of fields which define the relation.
     * In most cases there will be only one value, but multiple values
     * are supported for composite foreign keys.
     * The first field is on the referencing layer, the second on the referenced
    */
    QList< QgsRelation::FieldPair > mFieldPairs;

    bool mValid = false;
    QString mValidationError;
};

/// @endcond

#endif // QGSRELATION_P_H
