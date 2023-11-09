/***************************************************************************
               qgspolymorphicrelation_p.h
               --------------------------
    begin                : December 2020
    copyright            : (C) 2020 Ivan Ivanov
    email                : ivan@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPOLYMORPHICRELATION_P_H
#define QGSPOLYMORPHICRELATION_P_H

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

#include "qgis.h"
#include "qgsrelation.h"

#include <QSharedData>
#include <QPointer>

class QgsVectorLayer;

class QgsPolymorphicRelationPrivate : public QSharedData
{
  public:
    QgsPolymorphicRelationPrivate() = default;

    //! Unique Id
    QString mRelationId;
    //! Human redable name
    QString mRelationName;
    //! The child layer
    QString mReferencingLayerId;
    //! The child layer
    QPointer<QgsVectorLayer> mReferencingLayer;
    //! The field in the child layer that stores the parent layer
    QString mReferencedLayerField;
    //! The expression to identify the referenced (parent) layer
    QString mReferencedLayerExpression;
    //! A list of layerids that are set as parents
    QStringList mReferencedLayerIds;

    /**
     * A list of fields which define the relation.
     * In most cases there will be only one value, but multiple values
     * are supported for composite foreign keys.
     * The first field is on the referencing layer, the second on the referenced
    */
    QList< QgsRelation::FieldPair > mFieldPairs;

    //! A map of the layerIds and the respective layers
    QMap<QString, QgsVectorLayer *> mReferencedLayersMap;

    //! The relation strength for all the generated normal relations
    Qgis::RelationshipStrength mRelationStrength = Qgis::RelationshipStrength::Association;

    //! Whether the polymorphic relation is valid
    bool mValid = false;
};

/// @endcond

#endif // QGSPOLYMORPHICRELATION_P_H
