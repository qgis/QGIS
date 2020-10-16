/***************************************************************************
                         qgsprocessingparameterfielddefinition.h
                         ----------------------
    begin                : September 2020
    copyright            : (C) 2020 by Ivan Ivanov
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

#ifndef QGSPROCESSINGPARAMETERFIELDDEFINITION_H
#define QGSPROCESSINGPARAMETERFIELDDEFINITION_H

#include "qgsprocessingparameters.h"
#include "qgsprocessingparametertype.h"

/**
 * \ingroup core
 * A parameter for creating or reusing existing layer fields.
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsProcessingParameterFieldDefinition : public QgsProcessingParameterDefinition
{
  public:
    //! Constructor for QgsProcessingParameterFieldDefinition.
    QgsProcessingParameterFieldDefinition( const QString &name, const QString &description = QString(), const QString &parentLayerParameterName = QString(), bool optional = false );

    //! Returns the type name for the parameter class.
    static QString typeName() { return QStringLiteral( "fields_form" ); }

    QgsProcessingParameterDefinition *clone() const override;
    QString type() const override;
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonQgsProcessingAlgorithmSubclass ) const override;
    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;
    QStringList dependsOnOtherParameters() const override;

    /**
     * Returns the name of the parent layer parameter, or an empty string if this is not set.
     * \see setParentLayerParameterName()
     */
    QString parentLayerParameterName() const;

    /**
     * Sets the \a name of the parent layer parameter. Use an empty string if this is not required.
     * \see parentLayerParameterName()
     */
    void setParentLayerParameterName( const QString &name );

  private:
    QString mParentLayerParameterName;

};

///@cond PRIVATE

/**
 * Parameter type definition for QgsProcessingParameterFieldDefinition.
 *
 * \ingroup core
 * \note This class is not a part of public API.
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsProcessingParameterTypeFieldDefinition : public QgsProcessingParameterType
{
  public:
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterFieldDefinition( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A mapping of field names to field type definitions and expressions. Used for the refactor fields algorithm." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Field Form" );
    }

    QString id() const override
    {
      return QgsProcessingParameterFieldDefinition::typeName();
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterFieldDefinition" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterFieldDefinition" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "dict: field definitions as dictionary" );
    }
};
///@endcond

#endif // QGSPROCESSINGPARAMETERFIELDDEFINITION_H
