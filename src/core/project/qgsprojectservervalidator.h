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
#include "qgis_sip.h"
#include "qgslayertreegroup.h"
#include "qgslayertree.h"


/**
 * \ingroup core
 * \class QgsProjectServerValidator
 * \brief Validates the server specific parts of the configuration of a QGIS project.
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsProjectServerValidator
{

  public:

    QgsProjectServerValidator() = default;

    /**
     * Errors that might be raised by the validation process.
     */
    enum ValidationError
    {
      DuplicatedNames = 0, //!< A duplicated layer/group name in the layer tree.
      LayerShortName = 1, //!< Layer/group short name is not valid.
      LayerEncoding = 2,  //!< Encoding is not correctly set on a vector layer.
      ProjectShortName = 3,  //!< The project short name is not valid.
      ProjectRootNameConflict = 4,  //!< The project root name is already used by a layer or a group.
    };

    /**
    * Returns a human readable string for a given error.
    * \param error the error.
    * \returns the human readable error.
    */
    static QString displayValidationError( QgsProjectServerValidator::ValidationError error );

    /**
     * Contains the parameters describing a project validation failure.
     */
    struct ValidationResult
    {

      /**
       * Constructor for ValidationResult.
       */
      ValidationResult( const QgsProjectServerValidator::ValidationError error, const QVariant &identifier )
        : error( error )
        , identifier( identifier )
      {}

      /**
       * Error which occurred during the validation process.
       */
      QgsProjectServerValidator::ValidationError error;

      /**
       * Identifier related to the error. It can be a layer/group name.
       */
      QVariant identifier;
    };

    /**
     * Validates a project to detect problems on QGIS Server, and returns TRUE if it's considered valid.
     * If validation fails, the \a results list will be filled with a list of
     * items describing why the validation failed and what needs to be rectified
     * \param project input project to check
     * \param results results of the validation
     * \returns bool
     */
    static bool validate( QgsProject *project, QList< QgsProjectServerValidator::ValidationResult > &results SIP_OUT );

  private:
    static void browseLayerTree( QgsLayerTreeGroup *treeGroup, QStringList &owsNames, QStringList &encodingMessages );

};

#endif // QGSPROJECTSERVERVALIDATOR_H
