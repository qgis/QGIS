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
#include "qgslayertreegroup.h"
#include "qgslayertree.h"


/**
 * \ingroup core
 * \class QgsProjectServerValidator
 * \brief Project server validator.
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsProjectServerValidator
{

  public:

    /**
     * Constructor for QgsProjectServerValidator.
     */
    QgsProjectServerValidator() = default;

    /**
     * Errors that might be raised by the validation process.
     */
    enum ValidationError
    {
      DuplicatedNames = 0, //!< Error related to a duplicated layer name in the layer tree.
      ShortNames = 1, //!< Layer short name is well formatted.
      Encoding = 2  //!< Encoding is not set correctly.
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
       * Identifier related to the error.
       */
      QVariant identifier;
    };

    /**
     * Validates a project to avoid some problems on QGIS Server, and returns TRUE if it's considered valid.
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
