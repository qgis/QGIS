/***************************************************************************
                             qgsprojectservervalidator.h
                             ---------------------------
    begin                : March 2020
    copyright            : (C) 2020 by Etienne Trimaille
    email                : etienne dot trimaille at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROJECTSERVERVALIDATOR_H
#define QGSPROJECTSERVERVALIDATOR_H

#include "qgis_core.h"
#include "qgslayermetadatavalidator.h"
#include "qgslayertreegroup.h"
#include "qgslayertree.h"


/**
 * \ingroup core
 * \class QgsProjectServerValidator
 * \brief Project server validator.
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsProjectServerValidator : public QgsAbstractBaseValidator
{

  public:

    /**
     * Constructor for QgsProjectServerValidator.
     */
    QgsProjectServerValidator() = default;

    /**
     * Validates a layer tree to avoid some problems on QGIS Server, and returns TRUE if it's considered valid.
     * If validation fails, the \a results list will be filled with a list of
     * items describing why the validation failed and what needs to be rectified
     * \param layerTree input layer tree
     * \param results results of the validation
     * \returns bool
     * \since QGIS 3.14
     */
    bool validate( QgsLayerTree *layerTree, QList< QgsAbstractBaseValidator::ValidationResult > &results SIP_OUT ) const;

  private:
    static void browseLayerTree( QgsLayerTreeGroup *treeGroup, QStringList &owsNames, QStringList &encodingMessages );

};

#endif // QGSPROJECTSERVERVALIDATOR_H
