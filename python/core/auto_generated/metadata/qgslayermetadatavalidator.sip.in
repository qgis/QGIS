/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/metadata/qgslayermetadatavalidator.h                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsAbstractMetadataBaseValidator
{
%Docstring
Abstract base class for metadata validators.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayermetadatavalidator.h"
%End
  public:

    struct ValidationResult
    {

      ValidationResult( const QString &section, const QString &note, const QVariant &identifier = QVariant() );
%Docstring
Constructor for ValidationResult.
%End

      QString section;

      QVariant identifier;

      QString note;
    };

    virtual ~QgsAbstractMetadataBaseValidator();

    virtual bool validate( const QgsAbstractMetadataBase *metadata, QList< QgsAbstractMetadataBaseValidator::ValidationResult > &results /Out/ ) const = 0;
%Docstring
Validates a ``metadata`` object, and returns ``True`` if the
metadata is considered valid.
If validation fails, the ``results`` list will be filled with a list of
items describing why the validation failed and what needs to be rectified
to fix the metadata.
%End

};


class QgsNativeMetadataBaseValidator : QgsAbstractMetadataBaseValidator
{
%Docstring
A validator for the native base QGIS metadata schema definition.

.. versionadded:: 3.2
%End

%TypeHeaderCode
#include "qgslayermetadatavalidator.h"
%End
  public:

    QgsNativeMetadataBaseValidator();
%Docstring
Constructor for QgsNativeMetadataBaseValidator.
%End

    virtual bool validate( const QgsAbstractMetadataBase *metadata, QList< QgsAbstractMetadataBaseValidator::ValidationResult > &results /Out/ ) const;


};



class QgsNativeMetadataValidator : QgsNativeMetadataBaseValidator
{
%Docstring
A validator for the native QGIS layer metadata schema definition.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayermetadatavalidator.h"
%End
  public:

    QgsNativeMetadataValidator();
%Docstring
Constructor for QgsNativeMetadataValidator.
%End

    virtual bool validate( const QgsAbstractMetadataBase *metadata, QList< QgsAbstractMetadataBaseValidator::ValidationResult > &results /Out/ ) const;


};


class QgsNativeProjectMetadataValidator : QgsNativeMetadataBaseValidator
{
%Docstring
A validator for the native QGIS project metadata schema definition.

.. versionadded:: 3.2
%End

%TypeHeaderCode
#include "qgslayermetadatavalidator.h"
%End
  public:

    QgsNativeProjectMetadataValidator();
%Docstring
Constructor for QgsNativeProjectMetadataValidator.
%End

    virtual bool validate( const QgsAbstractMetadataBase *metadata, QList< QgsAbstractMetadataBaseValidator::ValidationResult > &results /Out/ ) const;


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/metadata/qgslayermetadatavalidator.h                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
