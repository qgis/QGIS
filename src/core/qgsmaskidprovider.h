/***************************************************************************
 qgsmaskidprovider.h
 ---------------------
 begin                : August 2019
 copyright            : (C) 2019 by Hugo Mercier / Oslandia
 email                : infos at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMASKIDPROVIDER_H
#define QGSMASKIDPROVIDER_H

#include <QList>
#include <QSet>
#include "qgssymbollayerreference.h"

/**
 * \ingroup core
 * \class QgsMaskIdProvider
 *
 * Some rendering operations may need multiple mask images. This is the case for label rendering in which we can
 * have different mask images: one different for each labeling rule for instance.
 * Some label layers may need to share their mask images, some other need to have distinct mask images.
 * Label layers share the same mask image if the set of symbol layers they mask is the same.
 *
 * A "mask id" is then associated to each label layer. They are contiguous integer numbers starting at 0.
 *
 * This class allows the creation of mask ids based on the different label layers and to give a mask id from a label layer.
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsMaskIdProvider
{
  public:

    /**
     * Inserts a label layer to the provider and returns its associated mask id.
     * \param layerId id of the vector layer that carries these labels
     * \param ruleId id of the labeling rule, if any
     * \param maskedSymbolLayers the symbol layers that are masked by this label layer
     * \return the associated mask id.
     */
    int insertLabelLayer( const QString &layerId, const QString &ruleId, const QSet<QgsSymbolLayerReference> &maskedSymbolLayers );

    /**
     * Returns the mask id associated with a label layer and its optional label rule.
     * Returns -1 if not found.
     */
    int maskId( const QString &labelLayerId = QString(), const QString &labelRuleId = QString() ) const;

    /**
     * Returns the number of identifiers allocated.
     */
    int size() const;

  private:

    /**
     * Storage of symbol layer references sets. The index in the list gives the associated mask id.
     */
    QList<QSet<QgsSymbolLayerReference>> mLabelLayers;

    /**
     * Mapping from a set of label layer identifiers (as string) to an integer (its index in the list)
     */
    QList<QSet<QString>> mMaskIds;
};

#endif
