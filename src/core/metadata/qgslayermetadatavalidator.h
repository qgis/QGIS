/***************************************************************************
                             qgslayermetadatavalidator.h
                             ---------------------------
    begin                : April 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERMETADATAVALIDATOR_H
#define QGSLAYERMETADATAVALIDATOR_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include <QString>
#include <QVariant>

class QgsAbstractMetadataBase;
class QgsLayerMetadata;

/**
 * \ingroup core
 * \class QgsAbstractMetadataBaseValidator
 * \brief Abstract base class for metadata validators.
 * \since QGIS 3.0
 */

class CORE_EXPORT QgsAbstractMetadataBaseValidator
{

  public:

    /**
     * Contains the parameters describing a metadata validation
     * failure.
     */
    struct ValidationResult
    {

      /**
       * Constructor for ValidationResult.
       */
      ValidationResult( const QString &section, const QString &note, const QVariant &identifier = QVariant() )
        : section( section )
        , identifier( identifier )
        , note( note )
      {}

      //! Metadata section which failed the validation
      QString section;

      /**
       * Optional identifier for the failed metadata item.
       * For instance, in list type metadata elements this
       * will be set to the list index of the failed metadata
       * item.
       */
      QVariant identifier;

      //! The reason behind the validation failure.
      QString note;
    };

    virtual ~QgsAbstractMetadataBaseValidator() = default;

    /**
     * Validates a \a metadata object, and returns TRUE if the
     * metadata is considered valid.
     * If validation fails, the \a results list will be filled with a list of
     * items describing why the validation failed and what needs to be rectified
     * to fix the metadata.
     */
    virtual bool validate( const QgsAbstractMetadataBase *metadata, QList< QgsAbstractMetadataBaseValidator::ValidationResult > &results SIP_OUT ) const = 0;

};

/**
 * \ingroup core
 * \class QgsNativeMetadataBaseValidator
 * \brief A validator for the native base QGIS metadata schema definition.
 * \since QGIS 3.2
 */

class CORE_EXPORT QgsNativeMetadataBaseValidator : public QgsAbstractMetadataBaseValidator
{

  public:

    /**
     * Constructor for QgsNativeMetadataBaseValidator.
     */
    QgsNativeMetadataBaseValidator() = default;

    bool validate( const QgsAbstractMetadataBase *metadata, QList< QgsAbstractMetadataBaseValidator::ValidationResult > &results SIP_OUT ) const override;

};


/**
 * \ingroup core
 * \class QgsNativeMetadataValidator
 * \brief A validator for the native QGIS layer metadata schema definition.
 * \since QGIS 3.0
 */

class CORE_EXPORT QgsNativeMetadataValidator : public QgsNativeMetadataBaseValidator
{

  public:

    /**
     * Constructor for QgsNativeMetadataValidator.
     */
    QgsNativeMetadataValidator() = default;

    bool validate( const QgsAbstractMetadataBase *metadata, QList< QgsAbstractMetadataBaseValidator::ValidationResult > &results SIP_OUT ) const override;

};

/**
 * \ingroup core
 * \class QgsNativeProjectMetadataValidator
 * \brief A validator for the native QGIS project metadata schema definition.
 * \since QGIS 3.2
 */

class CORE_EXPORT QgsNativeProjectMetadataValidator : public QgsNativeMetadataBaseValidator
{

  public:

    /**
     * Constructor for QgsNativeProjectMetadataValidator.
     */
    QgsNativeProjectMetadataValidator() = default;

    bool validate( const QgsAbstractMetadataBase *metadata, QList< QgsAbstractMetadataBaseValidator::ValidationResult > &results SIP_OUT ) const override;

};

#endif // QGSLAYERMETADATAVALIDATOR_H
