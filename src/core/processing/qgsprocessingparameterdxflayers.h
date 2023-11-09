/***************************************************************************
  qgsprocessingparameterdxflayers.h
  ---------------------
  Date                 : September 2020
  Copyright            : (C) 2020 by Alexander Bruy
  Email                : alexander dot bruy at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGPARAMETERDXFLAYERS_H
#define QGSPROCESSINGPARAMETERDXFLAYERS_H

#include "qgsprocessingparameters.h"
#include "qgsprocessingparametertype.h"
#include "qgsdxfexport.h"

/**
 * \brief A parameter for Processing algorithms that need a list of input vector
 * layers to export as DXF file - this parameter provides Processing
 * framework's adapter for QList<QgsDxfExport::DxfLayer>.
 *
 * A valid value for this parameter is a list (QVariantList), where each
 * item is a map (QVariantMap) in this form:
 * {
 *   'layer':  string or QgsMapLayer,
 *   'attributeIndex': int
 * }
 *
 * Also it can accept lists (either string lists or QgsVectorLayer list)
 * as well as individual layer object or string representing layer source.
 *
 * Static functions parametersAsLayers(), variantMapAsLayer(),
 * layerAsVariantMap() provide conversion between QgsDxfExport::DxfLayer
 * representation and QVariant representation.
 *
 * \ingroup core
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsProcessingParameterDxfLayers : public QgsProcessingParameterDefinition
{
  public:
    //! Constructor for QgsProcessingParameterDxfLayers.
    QgsProcessingParameterDxfLayers( const QString &name, const QString &description = QString() );

    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override;
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonQgsProcessingAlgorithmSubclass ) const override;
    QString valueAsString( const QVariant &value, QgsProcessingContext &context, bool &ok SIP_OUT ) const override;
    QVariant valueAsJsonObject( const QVariant &value, QgsProcessingContext &context ) const override;

    //! Returns the type name for the parameter class.
    static QString typeName() { return QStringLiteral( "dxflayers" ); }
    //! Converts a QVariant value (a QVariantList) to a list of input layers
    static QList<QgsDxfExport::DxfLayer> parameterAsLayers( const QVariant &layersVariant, QgsProcessingContext &context );
    //! Converts a QVariant value (a QVariantMap) to a single input layer
    static QgsDxfExport::DxfLayer variantMapAsLayer( const QVariantMap &layerVariantMap, QgsProcessingContext &context );
    //! Converts a single input layer to QVariant representation (a QVariantMap)
    static QVariantMap layerAsVariantMap( const QgsDxfExport::DxfLayer &layer );
};

#ifndef SIP_RUN
///@cond PRIVATE

/**
 * \brief Parameter type definition for QgsProcessingParameterDxfLayers.
 *
 * \ingroup core
 * \note This class is not a part of public API.
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsProcessingParameterTypeDxfLayers : public QgsProcessingParameterType
{
  public:
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterDxfLayers( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "An input allowing selection of multiple layers for export to DXF file." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "DXF Layers" );
    }

    QString id() const override
    {
      return QgsProcessingParameterDxfLayers::typeName();
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterDxfLayers" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterDxfLayers" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "list[dict]: list of input layers as dictionaries, see QgsProcessingParameterDxfLayers docs" )
             << QObject::tr( "list[str]: list of layer IDs" )
             << QObject::tr( "list[str]: list of layer names" )
             << QObject::tr( "list[str]: list of layer sources" )
             << QObject::tr( "str: layer ID" )
             << QObject::tr( "str: layer name" )
             << QObject::tr( "str: layer source" )
             << QStringLiteral( "list[QgsMapLayer]" )
             << QStringLiteral( "QgsVectorLayer" );
    }
};

///@endcond
#endif

#endif // QGSPROCESSINGPARAMETERDXFLAYERS_H
