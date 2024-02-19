/***************************************************************************
  qgsprocessingparametervectortilewriterlayers.h
  ---------------------
  Date                 : April 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGPARAMETERVECTORTILEWRITERLAYERS_H
#define QGSPROCESSINGPARAMETERVECTORTILEWRITERLAYERS_H

#include "qgsprocessingparameters.h"
#include "qgsprocessingparametertype.h"
#include "qgsvectortilewriter.h"


/**
 * \brief A parameter for processing algorithms that need a list of input vector layers for writing
 * of vector tiles - this parameter provides processing framework's adapter for QList<QgsVectorTileWriter::Layer>.
 *
 * A valid value for this parameter is a list (QVariantList), where each item is a map (QVariantMap) in this form:
 * {
 *   'layer':  string or QgsMapLayer,
 *   // key-value pairs below are optional
 *   'layerName': string,
 *   'filterExpression': string,
 *   'minZoom': int,
 *   'maxZoom': int
 * }
 *
 * Static functions parametersAsLayers(), variantMapAsLayer(), layerAsVariantMap() provide conversion between
 * QgsVectorTileWriter::Layer representation and QVariant representation.
 *
 * \ingroup core
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsProcessingParameterVectorTileWriterLayers : public QgsProcessingParameterDefinition
{
  public:
    //! Constructor for QgsProcessingParameterVectorTileWriterLayers.
    QgsProcessingParameterVectorTileWriterLayers( const QString &name, const QString &description = QString() );

    QgsProcessingParameterDefinition *clone() const override;
    QString type() const override;
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass ) const override;

    //! Returns the type name for the parameter class.
    static QString typeName() { return QStringLiteral( "vectortilewriterlayers" ); }

    //! Converts a QVariant value (a QVariantList) to a list of input layers
    static QList<QgsVectorTileWriter::Layer> parameterAsLayers( const QVariant &layersVariant, QgsProcessingContext &context );
    //! Converts a QVariant value (a QVariantMap) to a single input layer
    static QgsVectorTileWriter::Layer variantMapAsLayer( const QVariantMap &layerVariantMap, QgsProcessingContext &context );
    //! Converts a single input layer to QVariant representation (a QVariantMap)
    static QVariantMap layerAsVariantMap( const QgsVectorTileWriter::Layer &layer );

};

#ifndef SIP_RUN
///@cond PRIVATE

/**
 * \brief Parameter type definition for QgsProcessingParameterVectorTileWriterLayers.
 *
 * \ingroup core
 * \note This class is not a part of public API.
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsProcessingParameterTypeVectorTileWriterLayers : public QgsProcessingParameterType
{
  public:
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterVectorTileWriterLayers( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "An input allowing selection of multiple layers for export in vector tiles." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Vector Tile Writer Layers" );
    }

    QString id() const override
    {
      return QgsProcessingParameterVectorTileWriterLayers::typeName();
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterVectorTileWriterLayers" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterVectorTileWriterLayers" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "list[dict]: list of input layers as dictionaries, see QgsProcessingParameterVectorTileWriterLayers docs" );
    }
};

///@endcond
#endif

#endif // QGSPROCESSINGPARAMETERVECTORTILEWRITERLAYERS_H
