/***************************************************************************
                         qgsprocessingparameterfieldmap.h
                         -------------------------
    begin                : June 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGPARAMETERFIELDMAP_H
#define QGSPROCESSINGPARAMETERFIELDMAP_H

#include "qgsprocessingparameters.h"
#include "qgsprocessingparametertype.h"

/**
 * \ingroup core
 * \brief A parameter for "field mapping" configurations, which consist of a definition
 * of desired output fields, types, and expressions used to populate then.
 *
 * Designed for use with the "Refactor fields" algorithm.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsProcessingParameterFieldMapping : public QgsProcessingParameterDefinition
{
  public:
    //! Constructor for QgsProcessingParameterFieldMapping.
    QgsProcessingParameterFieldMapping( const QString &name, const QString &description = QString(), const QString &parentLayerParameterName = QString(), bool optional = false );

    QgsProcessingParameterDefinition *clone() const override;
    QString type() const override;
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass ) const override;
    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;
    QStringList dependsOnOtherParameters() const override;

    //! Returns the type name for the parameter class.
    static QString typeName() { return QStringLiteral( "fields_mapping" ); }

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

#ifndef SIP_RUN
///@cond PRIVATE

/**
 * \brief Parameter type definition for QgsProcessingParameterFieldMapping.
 *
 * \ingroup core
 * \note This class is not a part of public API.
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsProcessingParameterTypeFieldMapping : public QgsProcessingParameterType
{
  public:
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterFieldMapping( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A mapping of field names to field type definitions and expressions. Used for the refactor fields algorithm." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Fields Mapper" );
    }

    QString id() const override
    {
      return QgsProcessingParameterFieldMapping::typeName();
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterFieldMapping" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterFieldMapping" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "list[dict]: list of field definitions as dictionaries" );
    }
};

///@endcond
#endif

#endif // QGSPROCESSINGPARAMETERFIELDMAP_H
