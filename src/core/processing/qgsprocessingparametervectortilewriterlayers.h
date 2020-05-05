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

#include "qgsvectortilewriter.h"

class CORE_EXPORT QgsProcessingParameterVectorTileWriterLayers : public QgsProcessingParameterDefinition
{
  public:
    QgsProcessingParameterVectorTileWriterLayers( const QString &name, const QString &description = QString() );

    QgsProcessingParameterDefinition *clone() const override;
    QString type() const override;
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonQgsProcessingAlgorithmSubclass ) const override;

    static QString typeName() { return QStringLiteral( "vectortilewriterlayers" ); }

    static QList<QgsVectorTileWriter::Layer> parameterAsLayers( const QVariant &layersVariant, QgsProcessingContext &context );

    static QgsVectorTileWriter::Layer variantMapAsLayer( const QVariantMap &layerVariantMap, QgsProcessingContext &context );
    static QVariantMap layerAsVariantMap( const QgsVectorTileWriter::Layer &layer );

};

#include "qgsprocessingparametertype.h"

/**
 * A parameter for processing algorithms which accepts multiple map layers.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('vectortilewriterlayers')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeVectorTileWriterLayers : public QgsProcessingParameterType
{
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
    /*
        QString pythonImportString() const override
        {
          return QStringLiteral( "from qgis.core import QgsProcessingParameterMultipleLayers" );
        }

        QString className() const override
        {
          return QStringLiteral( "QgsProcessingParameterMultipleLayers" );
        }

        QStringList acceptedPythonTypes() const override
        {
          return QStringList() << QObject::tr( "list[str]: list of layer IDs" )
                 << QObject::tr( "list[str]: list of layer names" )
                 << QObject::tr( "list[str]: list of layer sources" )
                 << QStringLiteral( "list[QgsMapLayer]" )
                 << QStringLiteral( "QgsProperty" );
        }*/
};


#endif // QGSPROCESSINGPARAMETERVECTORTILEWRITERLAYERS_H
