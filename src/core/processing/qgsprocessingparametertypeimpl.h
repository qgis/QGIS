/***************************************************************************
                         qgsprocessingparametertypeimpl.h
                         ------------------------
    begin                : March 2018
    copyright            : (C) 2018 by Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGPARAMETERTYPEIMPL_H
#define QGSPROCESSINGPARAMETERTYPEIMPL_H

#include "qgis.h"
#include "qgis_sip.h"
#include "qgsprocessingparametertype.h"

#include <QCoreApplication>

#define SIP_NO_FILE

/**
 * \brief A raster layer parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('raster')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeRasterLayer : public QgsProcessingParameterType
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterRasterLayer( name );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A raster layer parameter." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Raster Layer" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterRasterLayer" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterRasterLayer" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "raster" );
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: layer ID" )
             << QObject::tr( "str: layer name" )
             << QObject::tr( "str: layer source" )
             << QStringLiteral( "QgsProcessingRasterLayerDefinition" )
             << QStringLiteral( "QgsProperty" )
             << QStringLiteral( "QgsRasterLayer" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Path to a raster layer" );
    }

    [[nodiscard]] QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterRasterLayer::typeName()
             << QgsProcessingParameterMapLayer::typeName()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterExpression::typeName();
    }

    [[nodiscard]] QStringList acceptedOutputTypes() const override
    {
      return QStringList()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputRasterLayer::typeName()
             << QgsProcessingOutputMapLayer::typeName()
             << QgsProcessingOutputFile::typeName()
             << QgsProcessingOutputFolder::typeName();
    }

    [[nodiscard]] QColor modelColor() const override { return QColor( 0, 180, 180 ); /* turquoise */ };
};

/**
 * \brief A mesh layer parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('mesh')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeMeshLayer : public QgsProcessingParameterType
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterMeshLayer( name );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A mesh layer parameter." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Mesh Layer" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterMeshLayer" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterMeshLayer" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "mesh" );
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: layer ID" )
             << QObject::tr( "str: layer name" )
             << QObject::tr( "str: layer source" )
             << QStringLiteral( "QgsMeshLayer" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Path to a mesh layer" );
    }

    [[nodiscard]] QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterMeshLayer::typeName()
             << QgsProcessingParameterMapLayer::typeName()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterExpression::typeName();
    }

    [[nodiscard]] QStringList acceptedOutputTypes() const override
    {
      return QStringList()
             << QgsProcessingOutputString::typeName()
             // TODO  << QgsProcessingOutputMeshLayer::typeName()
             << QgsProcessingOutputMapLayer::typeName()
             << QgsProcessingOutputFile::typeName()
             << QgsProcessingOutputFolder::typeName();
    }

    [[nodiscard]] QColor modelColor() const override { return QColor( 137, 150, 171 ); /* cold gray */ };
};

/**
 * A vector layer parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('vector')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeVectorLayer : public QgsProcessingParameterType
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterVectorLayer( name );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A vector layer parameter, e.g. for algorithms which change layer styles, edit layers in place, or other operations which affect an entire layer." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Vector Layer" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterVectorLayer" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterVectorLayer" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "vector" );
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: layer ID" )
             << QObject::tr( "str: layer name" )
             << QObject::tr( "str: layer source" )
             << QStringLiteral( "QgsProperty" )
             << QStringLiteral( "QgsVectorLayer" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Path to a vector layer" );
    }

    [[nodiscard]] QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterVectorLayer::typeName()
             << QgsProcessingParameterMapLayer::typeName()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterExpression::typeName();
    }

    [[nodiscard]] QStringList acceptedOutputTypes() const override
    {
      return QStringList()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputVectorLayer::typeName()
             << QgsProcessingOutputMapLayer::typeName()
             << QgsProcessingOutputFile::typeName()
             << QgsProcessingOutputFolder::typeName();
    }

    QList<int> acceptedDataTypes( const QgsProcessingParameterDefinition *parameter ) const override
    {
      if ( const QgsProcessingParameterVectorLayer *param = dynamic_cast<const QgsProcessingParameterVectorLayer *>( parameter ) )
        return param->dataTypes();
      else
        return QList<int>();
    }

    [[nodiscard]] QColor modelColor() const override { return QColor( 122, 0, 47 ); /* burgundy */ };
};

/**
 * \brief A generic map layer parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('maplayer')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeMapLayer : public QgsProcessingParameterType
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterMapLayer( name );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A generic map layer parameter, which accepts either vector or raster layers." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Map Layer" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "layer" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterMapLayer" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterMapLayer" );
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: layer ID" )
             << QObject::tr( "str: layer name" )
             << QObject::tr( "str: layer source" )
             << QStringLiteral( "QgsMapLayer" )
             << QStringLiteral( "QgsProperty" )
             << QStringLiteral( "QgsRasterLayer" )
             << QStringLiteral( "QgsVectorLayer" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Path to a vector, raster or mesh layer" );
    }

    [[nodiscard]] QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterRasterLayer::typeName()
             << QgsProcessingParameterMeshLayer::typeName()
             << QgsProcessingParameterVectorLayer::typeName()
             << QgsProcessingParameterMapLayer::typeName()
             << QgsProcessingParameterPointCloudLayer::typeName()
             << QgsProcessingParameterAnnotationLayer::typeName()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterExpression::typeName();
    }

    [[nodiscard]] QStringList acceptedOutputTypes() const override
    {
      return QStringList()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputRasterLayer::typeName()
             << QgsProcessingOutputVectorLayer::typeName()
             << QgsProcessingOutputMapLayer::typeName()
             << QgsProcessingOutputFile::typeName()
             << QgsProcessingOutputVariant::typeName();
    }

    [[nodiscard]] QColor modelColor() const override { return QColor( 137, 150, 171 ); /* cold gray */ };
};

/**
 * \brief A boolean parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('boolean')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeBoolean : public QgsProcessingParameterType
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterBoolean( name );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A boolean parameter, for true/false values." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Boolean" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "boolean" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterBoolean" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterBoolean" );
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "bool" )
             << QStringLiteral( "int" )
             << QStringLiteral( "str" )
             << QStringLiteral( "QgsProperty" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "1 for true/yes" )
             << QObject::tr( "0 for false/no" )
             << QObject::tr( "field:FIELD_NAME to use a data defined value taken from the FIELD_NAME field" )
             << QObject::tr( "expression:SOME EXPRESSION to use a data defined value calculated using a custom QGIS expression" );
    }

    [[nodiscard]] QStringList acceptedParameterTypes() const override
    {
      //pretty much everything is compatible here and can be converted to a bool!
      return QStringList() << QgsProcessingParameterBoolean::typeName()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterNumber::typeName()
             << QgsProcessingParameterDistance::typeName()
             << QgsProcessingParameterArea::typeName()
             << QgsProcessingParameterVolume::typeName()
             << QgsProcessingParameterDuration::typeName()
             << QgsProcessingParameterScale::typeName()
             << QgsProcessingParameterFile::typeName()
             << QgsProcessingParameterField::typeName()
             << QgsProcessingParameterFeatureSource::typeName()
             << QgsProcessingParameterMapLayer::typeName()
             << QgsProcessingParameterRasterLayer::typeName()
             << QgsProcessingParameterVectorLayer::typeName()
             << QgsProcessingParameterMeshLayer::typeName()
             << QgsProcessingParameterExpression::typeName()
             << QgsProcessingParameterProviderConnection::typeName()
             << QgsProcessingParameterPointCloudLayer::typeName()
             << QgsProcessingParameterAnnotationLayer::typeName();
    }
    [[nodiscard]] QStringList acceptedOutputTypes() const override
    {
      return QStringList() << QgsProcessingOutputNumber::typeName()
             << QgsProcessingOutputMapLayer::typeName()
             << QgsProcessingOutputFile::typeName()
             << QgsProcessingOutputRasterLayer::typeName()
             << QgsProcessingOutputVectorLayer::typeName()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputVariant::typeName()
             << QgsProcessingOutputBoolean::typeName();
    }

    [[nodiscard]] QColor modelColor() const override { return QColor( 51, 201, 28 ); /* green */ };
};

/**
 * \brief An expression parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('expression')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeExpression : public QgsProcessingParameterType
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterExpression( name );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "An expression parameter, to add custom expressions based on layer fields." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Expression" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "expression" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterExpression" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterExpression" );
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "str" )
             << QStringLiteral( "QgsProperty" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "A valid QGIS expression string, e.g \"road_name\" = 'MAIN RD'" );
    }

    [[nodiscard]] QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterExpression::typeName()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterNumber::typeName()
             << QgsProcessingParameterDistance::typeName()
             << QgsProcessingParameterArea::typeName()
             << QgsProcessingParameterVolume::typeName()
             << QgsProcessingParameterScale::typeName()
             << QgsProcessingParameterProviderConnection::typeName();
    }

    [[nodiscard]] QStringList acceptedOutputTypes() const override
    {
      return QStringList()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputNumber::typeName()
             << QgsProcessingOutputVariant::typeName();
    }

    [[nodiscard]] QColor modelColor() const override { return QColor( 255, 131, 23 ); /* orange */ };
};

/**
 * \brief A crs parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('crs')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeCrs : public QgsProcessingParameterType
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterCrs( name );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A coordinate reference system (CRS) input parameter." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "CRS" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "crs" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterCrs" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterCrs" );
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList()
             << QStringLiteral( "str: 'ProjectCrs'" )
             << QObject::tr( "str: CRS auth ID (e.g. 'EPSG:3111')" )
             << QObject::tr( "str: CRS PROJ4 (e.g. 'PROJ4:…')" )
             << QObject::tr( "str: CRS WKT (e.g. 'WKT:…')" )
             << QObject::tr( "str: layer ID. CRS of layer is used." )
             << QObject::tr( "str: layer name. CRS of layer is used." )
             << QObject::tr( "str: layer source. CRS of layer is used." )
             << QObject::tr( "QgsCoordinateReferenceSystem" )
             << QObject::tr( "QgsMapLayer: CRS of layer is used" )
             << QObject::tr( "QgsProcessingFeatureSourceDefinition: CRS of source is used" )
             << QStringLiteral( "QgsProperty" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "CRS as an auth ID (e.g. 'EPSG:3111')" )
             << QObject::tr( "CRS as a PROJ4 string (e.g. 'PROJ4:…')" )
             << QObject::tr( "CRS as a WKT string (e.g. 'WKT:…')" )
             << QObject::tr( "Path to a layer. The CRS of the layer is used." ) ;
    }

    [[nodiscard]] QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterCrs::typeName()
             << QgsProcessingParameterExpression::typeName()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterRasterLayer::typeName()
             << QgsProcessingParameterVectorLayer::typeName()
             << QgsProcessingParameterMeshLayer::typeName()
             << QgsProcessingParameterFeatureSource::typeName()
             << QgsProcessingParameterPointCloudLayer::typeName()
             << QgsProcessingParameterAnnotationLayer::typeName();
    }

    [[nodiscard]] QStringList acceptedOutputTypes() const override
    {
      return QStringList() << QgsProcessingOutputVectorLayer::typeName()
             << QgsProcessingOutputRasterLayer::typeName()
             << QgsProcessingOutputMapLayer::typeName()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputVariant::typeName();
    }

    [[nodiscard]] QColor modelColor() const override { return QColor( 255, 131, 23 ); /* orange */ };
};

/**
 * \brief A numeric range parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('range')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeRange : public QgsProcessingParameterType
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterRange( name );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A numeric range parameter for processing algorithms." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Range" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "range" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterRange" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterRange" );
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "list[float]: list of 2 float values" )
             << QObject::tr( "list[str]: list of strings representing floats" )
             << QObject::tr( "str: as two comma delimited floats, e.g. '1,10'" )
             << QStringLiteral( "QgsProperty" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Two comma separated numeric values, e.g. '1,10'" );
    }

    [[nodiscard]] QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterRange::typeName()
             << QgsProcessingParameterString::typeName();
    }

    [[nodiscard]] QStringList acceptedOutputTypes() const override
    {
      return QStringList() << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputVariant::typeName();
    }

    [[nodiscard]] QColor modelColor() const override { return QColor( 34, 157, 214 ); /* blue */ };
};

/**
 * \brief A point parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('point')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypePoint : public QgsProcessingParameterType
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterPoint( name );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A point geometry parameter." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Point" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "point" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterPoint" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterPoint" );
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: as an 'x,y' string, e.g. '1.5,10.1'" )
             << QStringLiteral( "QgsPointXY" )
             << QStringLiteral( "QgsProperty" )
             << QStringLiteral( "QgsReferencedPointXY" )
             << QStringLiteral( "QgsGeometry: centroid of geometry is used" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Point coordinate as an 'x,y' string, e.g. '1.5,10.1'" );
    }

    [[nodiscard]] QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterPoint::typeName()
             << QgsProcessingParameterString::typeName();
    }

    [[nodiscard]] QStringList acceptedOutputTypes() const override
    {
      return QStringList()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputVariant::typeName();
    }

    [[nodiscard]] QColor modelColor() const override { return QColor( 122, 0, 47 ); /* burgundy */ };
};

/**
 * \brief A geometry parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('geometry')
 * \since QGIS 3.16
 */
class CORE_EXPORT QgsProcessingParameterTypeGeometry : public QgsProcessingParameterType
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterGeometry( name );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A geometry parameter." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Geometry" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "geometry" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterGeometry" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterGeometry" );
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: as Well-Known Text string (WKT)" )
             << QStringLiteral( "QgsGeometry" )
             << QStringLiteral( "QgsProperty" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Well-Known Text string (WKT)" );
    }

    [[nodiscard]] QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterGeometry::typeName()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterPoint::typeName()
             << QgsProcessingParameterExtent::typeName();
    }

    [[nodiscard]] QStringList acceptedOutputTypes() const override
    {
      return QStringList()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputVariant::typeName();
    }

    [[nodiscard]] QColor modelColor() const override { return QColor( 122, 0, 47 ); /* burgundy */ };
};

/**
 * \brief An enum based parameter for processing algorithms, allowing for selection from predefined values.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('enum')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeEnum : public QgsProcessingParameterType
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterEnum( name );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "An enumerated type parameter." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Enum" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "enum" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterEnum" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterEnum" );
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "int" )
             << QObject::tr( "str: as string representation of int, e.g. '1'" )
             << QStringLiteral( "QgsProperty" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Number of selected option, e.g. '1'" )
             << QObject::tr( "Comma separated list of options, e.g. '1,3'" );
    }

    [[nodiscard]] QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterEnum::typeName()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterNumber::typeName();
    }

    [[nodiscard]] QStringList acceptedOutputTypes() const override
    {
      return QStringList()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputVariant::typeName()
             << QgsProcessingOutputNumber::typeName();
    }

    [[nodiscard]] QColor modelColor() const override { return QColor( 152, 68, 201 ); /* purple */ };
};

/**
 * \brief A rectangular map extent parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('extent')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeExtent : public QgsProcessingParameterType
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterExtent( name );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A map extent parameter." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Extent" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "extent" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterExtent" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterExtent" );
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: as comma delimited list of x min, x max, y min, y max. E.g. '4,10,101,105'" )
             << QObject::tr( "str: layer ID. Extent of layer is used." )
             << QObject::tr( "str: layer name. Extent of layer is used." )
             << QObject::tr( "str: layer source. Extent of layer is used." )
             << QObject::tr( "QgsMapLayer: Extent of layer is used" )
             << QObject::tr( "QgsProcessingFeatureSourceDefinition: Extent of source is used" )
             << QStringLiteral( "QgsProperty" )
             << QStringLiteral( "QgsRectangle" )
             << QStringLiteral( "QgsReferencedRectangle" )
             << QStringLiteral( "QgsGeometry: bounding box of geometry is used" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "A comma delimited string of x min, x max, y min, y max. E.g. '4,10,101,105'" )
             << QObject::tr( "Path to a layer. The extent of the layer is used." ) ;
    }

    [[nodiscard]] QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterExtent::typeName()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterMapLayer::typeName()
             << QgsProcessingParameterFeatureSource::typeName()
             << QgsProcessingParameterRasterLayer::typeName()
             << QgsProcessingParameterVectorLayer::typeName()
             << QgsProcessingParameterMeshLayer::typeName()
             << QgsProcessingParameterPointCloudLayer::typeName()
             << QgsProcessingParameterAnnotationLayer::typeName();
    }

    [[nodiscard]] QStringList acceptedOutputTypes() const override
    {
      return QStringList()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputRasterLayer::typeName()
             << QgsProcessingOutputVectorLayer::typeName()
             << QgsProcessingOutputMapLayer::typeName()
             << QgsProcessingOutputVariant::typeName();
    }

    [[nodiscard]] QColor modelColor() const override { return QColor( 34, 157, 214 ); /* blue */ };
};

/**
 * \brief A table (matrix) parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('matrix')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeMatrix : public QgsProcessingParameterType
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterMatrix( name );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A table (matrix) parameter for processing algorithms." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Matrix" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "matrix" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterMatrix" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterMatrix" );
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: as comma delimited list of values" )
             << QStringLiteral( "list" )
             << QStringLiteral( "QgsProperty" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "A comma delimited list of values" );
    }

    [[nodiscard]] QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterMatrix::typeName();
    }

    [[nodiscard]] QStringList acceptedOutputTypes() const override
    {
      return QStringList();
    }

    [[nodiscard]] QColor modelColor() const override { return QColor( 34, 157, 214 ); /* blue */ };
};

/**
 * \brief An input file or folder parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('file')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeFile : public QgsProcessingParameterType
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterFile( name );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A file or folder parameter, for use with non-map layer file sources or folders." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "File/Folder" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "file" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterFile" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterFile" );
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "str" )
             << QStringLiteral( "QgsProperty" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Path to a file" );
    }

    [[nodiscard]] QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterFile::typeName();
    }

    [[nodiscard]] QStringList acceptedOutputTypes() const override
    {
      return QStringList() << QgsProcessingOutputFile::typeName()
             << QgsProcessingOutputFolder::typeName()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputVariant::typeName()
             << QgsProcessingOutputRasterLayer::typeName()
             << QgsProcessingOutputVectorLayer::typeName()
             << QgsProcessingOutputMapLayer::typeName();
    }

    [[nodiscard]] QColor modelColor() const override { return QColor( 80, 80, 80 ); /* dark gray */ };
};

/**
 * \brief A vector layer or feature source field parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('field')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeField : public QgsProcessingParameterType
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterField( name );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A vector field parameter, for selecting an existing field from a vector source." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Vector Field" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "field" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterField" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterField" );
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "str" )
             << QStringLiteral( "QgsProperty" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "The name of an existing field" )
             << QObject::tr( "; delimited list of existing field names" );
    }

    [[nodiscard]] QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterField::typeName()
             << QgsProcessingParameterString::typeName();
    }

    [[nodiscard]] QStringList acceptedOutputTypes() const override
    {
      return QStringList()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputVariant::typeName();
    }
};


/**
 * \brief A common generic type for destination parameter, for specifying the destination path for a vector layer
 * created by the algorithm.
 *
 * \ingroup core
* \since QGIS 3.44
 */
class CORE_EXPORT QgsProcessingParameterTypeDestination  : public QgsProcessingParameterType
{
    [[nodiscard]] QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterRasterLayer::typeName()
             << QgsProcessingParameterMeshLayer::typeName()
             << QgsProcessingParameterVectorLayer::typeName()
             << QgsProcessingParameterMapLayer::typeName()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterExpression::typeName();
    }

    [[nodiscard]] QStringList acceptedOutputTypes() const override
    {
      return QStringList()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputFolder::typeName()
             << QgsProcessingOutputFile::typeName()
             << QgsProcessingOutputVariant::typeName();
    }
};

/**
 * \brief A vector layer destination parameter, for specifying the destination path for a vector layer
 * created by the algorithm.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('vectorDestination')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeVectorDestination : public QgsProcessingParameterTypeDestination
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterVectorDestination( name );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A vector layer destination parameter." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Vector Destination" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "vectorDestination" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterVectorDestination" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterVectorDestination" );
    }

    [[nodiscard]] Qgis::ProcessingParameterTypeFlags flags() const override
    {
      Qgis::ProcessingParameterTypeFlags flags = QgsProcessingParameterType::flags();
      flags.setFlag( Qgis::ProcessingParameterTypeFlag::ExposeToModeler, false );
      return flags;
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "str" )
             << QStringLiteral( "QgsProperty" )
             << QStringLiteral( "QgsProcessingOutputLayerDefinition" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Path for new vector layer" );
    }

    [[nodiscard]] QColor modelColor() const override { return QColor( 122, 0, 47 ); /* burgundy */ };
};

/**
 * \brief A generic file based destination parameter, for specifying the destination path for a file (non-map layer)
 * created by the algorithm.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('fileDestination')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeFileDestination : public QgsProcessingParameterTypeDestination
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterFileDestination( name );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A generic file based destination parameter." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "File Destination" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "fileDestination" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterFileDestination" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterFileDestination" );
    }

    [[nodiscard]] Qgis::ProcessingParameterTypeFlags flags() const override
    {
      Qgis::ProcessingParameterTypeFlags flags = QgsProcessingParameterType::flags();
      flags.setFlag( Qgis::ProcessingParameterTypeFlag::ExposeToModeler, false );
      return flags;
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "str" )
             << QStringLiteral( "QgsProperty" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Path for new file" );
    }

    [[nodiscard]] QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterFile::typeName();
    }

    [[nodiscard]] QStringList acceptedOutputTypes() const override
    {
      return QStringList() << QgsProcessingOutputFile::typeName()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputVariant::typeName()
             << QgsProcessingOutputRasterLayer::typeName()
             << QgsProcessingOutputVectorLayer::typeName()
             << QgsProcessingOutputMapLayer::typeName();
    }

    [[nodiscard]] QColor modelColor() const override { return QColor( 80, 80, 80 ); /* dark gray */ };
};

/**
 * \brief A folder destination parameter, for specifying the destination path for a folder created
 * by the algorithm or used for creating new files within the algorithm.
 * A folder output parameter.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('folderDestination')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeFolderDestination : public QgsProcessingParameterTypeDestination
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterFolderDestination( name );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A folder destination parameter." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Folder Destination" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "folderDestination" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterFolderDestination" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterFolderDestination" );
    }

    [[nodiscard]] Qgis::ProcessingParameterTypeFlags flags() const override
    {
      Qgis::ProcessingParameterTypeFlags flags = QgsProcessingParameterType::flags();
      flags.setFlag( Qgis::ProcessingParameterTypeFlag::ExposeToModeler, false );
      return flags;
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "str" )
             << QStringLiteral( "QgsProperty" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Path for an existing or new folder" );
    }

    [[nodiscard]] QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterFile::typeName();
    }

    [[nodiscard]] QStringList acceptedOutputTypes() const override
    {
      return QStringList() << QgsProcessingOutputFile::typeName()
             << QgsProcessingOutputFolder::typeName()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputVariant::typeName();
    }

    [[nodiscard]] QColor modelColor() const override { return QColor( 80, 80, 80 ); /* dark gray */ };
};

/**
 * \brief A raster layer destination parameter, for specifying the destination path for a raster layer
 * created by the algorithm.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('rasterDestination')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeRasterDestination : public QgsProcessingParameterTypeDestination
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterRasterDestination( name );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A raster layer destination parameter." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Raster Destination" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "rasterDestination" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterRasterDestination" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterRasterDestination" );
    }

    [[nodiscard]] Qgis::ProcessingParameterTypeFlags flags() const override
    {
      Qgis::ProcessingParameterTypeFlags flags = QgsProcessingParameterType::flags();
      flags.setFlag( Qgis::ProcessingParameterTypeFlag::ExposeToModeler, false );
      return flags;
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "str" )
             << QStringLiteral( "QgsProperty" )
             << QStringLiteral( "QgsProcessingOutputLayerDefinition" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Path for new raster layer" );
    }

    [[nodiscard]] QColor modelColor() const override { return QColor( 0, 180, 180 ); /* turquoise */ };
};

/**
 * \brief A string parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('string')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeString : public QgsProcessingParameterType
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterString( name );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A freeform string parameter." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "String" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "string" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterString" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterString" );
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "str" )
             << QStringLiteral( "QgsProperty" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "String value" )
             << QObject::tr( "field:FIELD_NAME to use a data defined value taken from the FIELD_NAME field" )
             << QObject::tr( "expression:SOME EXPRESSION to use a data defined value calculated using a custom QGIS expression" );
    }

    [[nodiscard]] QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterAuthConfig::typeName()
             << QgsProcessingParameterNumber::typeName()
             << QgsProcessingParameterDistance::typeName()
             << QgsProcessingParameterArea::typeName()
             << QgsProcessingParameterVolume::typeName()
             << QgsProcessingParameterDuration::typeName()
             << QgsProcessingParameterScale::typeName()
             << QgsProcessingParameterFile::typeName()
             << QgsProcessingParameterField::typeName()
             << QgsProcessingParameterExpression::typeName()
             << QgsProcessingParameterCoordinateOperation::typeName()
             << QgsProcessingParameterProviderConnection::typeName();
    }

    [[nodiscard]] QStringList acceptedOutputTypes() const override
    {
      return QStringList() << QgsProcessingOutputNumber::typeName()
             << QgsProcessingOutputVariant::typeName()
             << QgsProcessingOutputFile::typeName()
             << QgsProcessingOutputFolder::typeName()
             << QgsProcessingOutputString::typeName();
    }

    [[nodiscard]] QColor modelColor() const override { return QColor( 255, 131, 23 ); /* orange */ };
};

/**
 * \brief An authentication configuration parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('authcfg')
 * \since QGIS 3.6
 */
class CORE_EXPORT QgsProcessingParameterTypeAuthConfig : public QgsProcessingParameterType
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterAuthConfig( name );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "An authentication configuration parameter." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Authentication Configuration" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "authcfg" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterAuthConfig" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterAuthConfig" );
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "str" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "An existing QGIS authentication ID string" );
    }

    [[nodiscard]] QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterAuthConfig::typeName()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterExpression::typeName();
    }

    [[nodiscard]] QStringList acceptedOutputTypes() const override
    {
      return QStringList() << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputVariant::typeName();
    }
};

/**
 * \brief A parameter for processing algorithms which accepts multiple map layers.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('multilayer')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeMultipleLayers : public QgsProcessingParameterType
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterMultipleLayers( name );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "An input allowing selection of multiple sources, including multiple map layers or file sources." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Multiple Input" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "multilayer" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterMultipleLayers" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterMultipleLayers" );
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "list[str]: list of layer IDs" )
             << QObject::tr( "list[str]: list of layer names" )
             << QObject::tr( "list[str]: list of layer sources" )
             << QStringLiteral( "list[QgsMapLayer]" )
             << QStringLiteral( "QgsProperty" );
    }

    [[nodiscard]] QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterMultipleLayers::typeName()
             << QgsProcessingParameterMapLayer::typeName()
             << QgsProcessingParameterVectorLayer::typeName()
             << QgsProcessingParameterMeshLayer::typeName()
             << QgsProcessingParameterFeatureSource::typeName()
             << QgsProcessingParameterRasterLayer::typeName()
             << QgsProcessingParameterFile::typeName()
             << QgsProcessingParameterString::typeName();
    }
    [[nodiscard]] QStringList acceptedOutputTypes() const override
    {
      return QStringList()
             << QgsProcessingOutputMapLayer::typeName()
             << QgsProcessingOutputRasterLayer::typeName()
             << QgsProcessingOutputVectorLayer::typeName()
             << QgsProcessingOutputMultipleLayers::typeName()
             << QgsProcessingOutputFile::typeName()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputVariant::typeName();
    }

    [[nodiscard]] QColor modelColor() const override { return QColor( 137, 150, 171 ); /* cold gray */ };
};

/**
 * \brief An input feature source (such as vector layers) parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('source')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeFeatureSource : public QgsProcessingParameterType
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterFeatureSource( name );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A vector feature parameter, e.g. for algorithms which operate on the features within a layer." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Vector Features" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "source" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterFeatureSource" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterFeatureSource" );
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: layer ID" )
             << QObject::tr( "str: layer name" )
             << QObject::tr( "str: layer source" )
             << QStringLiteral( "QgsProcessingFeatureSourceDefinition" )
             << QStringLiteral( "QgsProperty" )
             << QStringLiteral( "QgsVectorLayer" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Path to a vector layer" );
    }

    [[nodiscard]] QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterFeatureSource::typeName()
             << QgsProcessingParameterVectorLayer::typeName()
             << QgsProcessingParameterMapLayer::typeName()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterExpression::typeName();
    }

    [[nodiscard]] QStringList acceptedOutputTypes() const override
    {
      return QStringList()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputVectorLayer::typeName()
             << QgsProcessingOutputMapLayer::typeName()
             << QgsProcessingOutputFile::typeName()
             << QgsProcessingOutputFolder::typeName();
    }

    QList<int> acceptedDataTypes( const QgsProcessingParameterDefinition *parameter ) const override
    {
      if ( const QgsProcessingParameterFeatureSource *param = dynamic_cast<const QgsProcessingParameterFeatureSource *>( parameter ) )
        return param->dataTypes();
      else
        return QList<int>();
    }

    [[nodiscard]] QColor modelColor() const override { return QColor( 122, 0, 47 ); /* burgundy */  };
};

/**
 * \brief A numeric parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('number')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeNumber : public QgsProcessingParameterType
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterNumber( name );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A numeric parameter, including float or integer values." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Number" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "number" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterNumber" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterNumber" );
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "int" )
             << QStringLiteral( "float" )
             << QStringLiteral( "QgsProperty" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "A numeric value" )
             << QObject::tr( "field:FIELD_NAME to use a data defined value taken from the FIELD_NAME field" )
             << QObject::tr( "expression:SOME EXPRESSION to use a data defined value calculated using a custom QGIS expression" );
    }

    [[nodiscard]] QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterNumber::typeName()
             << QgsProcessingParameterDistance::typeName()
             << QgsProcessingParameterVolume::typeName()
             << QgsProcessingParameterDuration::typeName()
             << QgsProcessingParameterScale::typeName();
    }

    [[nodiscard]] QStringList acceptedOutputTypes() const override
    {
      return QStringList() << QgsProcessingOutputNumber::typeName()
             << QgsProcessingOutputVariant::typeName()
             << QgsProcessingOutputString::typeName();
    }

    [[nodiscard]] QColor modelColor() const override { return QColor( 34, 157, 214 ); /* blue */ };
};

/**
 * \brief A distance parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('distance')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeDistance : public QgsProcessingParameterTypeNumber
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterDistance( name );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A numeric parameter representing a distance measure." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Distance" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "distance" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterDistance" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterDistance" );
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "int" )
             << QStringLiteral( "float" )
             << QStringLiteral( "QgsProperty" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "A numeric value" )
             << QObject::tr( "field:FIELD_NAME to use a data defined value taken from the FIELD_NAME field" )
             << QObject::tr( "expression:SOME EXPRESSION to use a data defined value calculated using a custom QGIS expression" );
    }

    [[nodiscard]] QColor modelColor() const override { return QColor( 34, 157, 214 ); /* blue */ };
};


/**
 * \brief An area parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('area')
 * \since QGIS 3.40
 */
class CORE_EXPORT QgsProcessingParameterTypeArea : public QgsProcessingParameterTypeNumber
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterArea( name );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A numeric parameter representing an area measure." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Area" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "area" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterArea" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterArea" );
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "int" )
             << QStringLiteral( "float" )
             << QStringLiteral( "QgsProperty" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "A numeric value" )
             << QObject::tr( "field:FIELD_NAME to use a data defined value taken from the FIELD_NAME field" )
             << QObject::tr( "expression:SOME EXPRESSION to use a data defined value calculated using a custom QGIS expression" );
    }
};


/**
 * \brief A volume parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('volume')
 * \since QGIS 3.40
 */
class CORE_EXPORT QgsProcessingParameterTypeVolume : public QgsProcessingParameterTypeNumber
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterVolume( name );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A numeric parameter representing a volume measure." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Volume" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "volume" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterVolume" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterVolume" );
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "int" )
             << QStringLiteral( "float" )
             << QStringLiteral( "QgsProperty" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "A numeric value" )
             << QObject::tr( "field:FIELD_NAME to use a data defined value taken from the FIELD_NAME field" )
             << QObject::tr( "expression:SOME EXPRESSION to use a data defined value calculated using a custom QGIS expression" );
    }
};



/**
 * \brief A duration parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('duration')
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsProcessingParameterTypeDuration : public QgsProcessingParameterTypeNumber
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterDuration( name );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A numeric parameter representing a duration measure." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Duration" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "duration" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterDuration" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterDuration" );
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "int" )
             << QStringLiteral( "float" )
             << QStringLiteral( "QgsProperty" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "A numeric value (unit type set by algorithms)" )
             << QObject::tr( "field:FIELD_NAME to use a data defined value taken from the FIELD_NAME field" )
             << QObject::tr( "expression:SOME EXPRESSION to use a data defined value calculated using a custom QGIS expression" );
    }

    [[nodiscard]] QColor modelColor() const override { return QColor( 34, 157, 214 ); /* blue */ };
};

/**
 * \brief A scale parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('scale')
 * \since QGIS 3.8
 */
class CORE_EXPORT QgsProcessingParameterTypeScale : public QgsProcessingParameterTypeNumber
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterScale( name );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A numeric parameter representing a map scale." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Scale" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "scale" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterScale" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterScale" );
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "int: scale denominator" )
             << QStringLiteral( "float: scale denominator" )
             << QStringLiteral( "QgsProperty" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "A numeric value representing the scale denominator" );
    }

    [[nodiscard]] QColor modelColor() const override { return QColor( 34, 157, 214 ); /* blue */ };
};

/**
 * \brief A raster band parameter for Processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('band')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeBand : public QgsProcessingParameterType
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterBand( name );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A raster band parameter, for selecting an existing band from a raster source." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Raster Band" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "band" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterBand" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterBand" );
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "int" )
             << QStringLiteral( "QgsProperty" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Integer value representing an existing raster band number" );
    }

    [[nodiscard]] QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterBand::typeName()
             << QgsProcessingParameterNumber::typeName();
    }

    [[nodiscard]] QStringList acceptedOutputTypes() const override
    {
      return QStringList()
             << QgsProcessingOutputNumber::typeName()
             << QgsProcessingOutputVariant::typeName();
    }
};

/**
 * \brief A feature sink parameter for Processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('band')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeFeatureSink : public QgsProcessingParameterTypeDestination
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterFeatureSink( name );
    }

    [[nodiscard]] Qgis::ProcessingParameterTypeFlags flags() const override
    {
      Qgis::ProcessingParameterTypeFlags flags = QgsProcessingParameterType::flags();
      flags.setFlag( Qgis::ProcessingParameterTypeFlag::ExposeToModeler, false );
      return flags;
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A feature sink destination parameter." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Feature Sink" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "sink" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterFeatureSink" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterFeatureSink" );
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: destination vector file, e.g. 'd:/test.shp'" )
             << QObject::tr( "str: 'memory:' to store result in temporary memory layer" )
             << QObject::tr( "str: using vector provider ID prefix and destination URI, e.g. 'postgres:…' to store result in PostGIS table" )
             << QStringLiteral( "QgsProcessingOutputLayerDefinition" )
             << QStringLiteral( "QgsProperty" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Path for new vector layer" );
    }

    [[nodiscard]] QColor modelColor() const override { return QColor( 122, 0, 47 ); /* burgundy */ };
};

/**
 * \brief A print layout parameter for Processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('layout')
 * \since QGIS 3.8
 */
class CORE_EXPORT QgsProcessingParameterTypeLayout : public QgsProcessingParameterType
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterLayout( name );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A print layout parameter." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Print Layout" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "layout" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterLayout" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterLayout" );
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: name of print layout in current project" )
             << QStringLiteral( "QgsProperty" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Name of print layout in current project" );
    }

    [[nodiscard]] QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterLayout::typeName()
             << QgsProcessingParameterString::typeName();
    }

    [[nodiscard]] QStringList acceptedOutputTypes() const override
    {
      return QStringList()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputVariant::typeName();
    }
};

/**
 * \brief A print layout item parameter for Processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('layoutitem')
 * \since QGIS 3.8
 */
class CORE_EXPORT QgsProcessingParameterTypeLayoutItem : public QgsProcessingParameterType
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterLayoutItem( name );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A print layout item parameter." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Print Layout Item" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "layoutitem" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterLayoutItem" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterLayoutItem" );
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: UUID of print layout item" )
             << QObject::tr( "str: id of print layout item" )
             << QStringLiteral( "QgsProperty" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "UUID or item id of layout item" );
    }

    [[nodiscard]] QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterLayoutItem::typeName()
             << QgsProcessingParameterString::typeName();
    }

    [[nodiscard]] QStringList acceptedOutputTypes() const override
    {
      return QStringList()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputVariant::typeName();
    }
};

/**
 * \brief A color parameter for Processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('color')
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsProcessingParameterTypeColor : public QgsProcessingParameterType
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterColor( name );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A color parameter." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Color" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "color" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterColor" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterColor" );
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: string representation of color, e.g #ff0000 or rgba(200,100,50,0.8)" )
             << QStringLiteral( "QColor" )
             << QStringLiteral( "QgsProperty" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "String representation of color, e.g #ff0000 or rgba(200,100,50,0.8)" )
             << QObject::tr( "field:FIELD_NAME to use a data defined value taken from the FIELD_NAME field" )
             << QObject::tr( "expression:SOME EXPRESSION to use a data defined value calculated using a custom QGIS expression" );
    }

    [[nodiscard]] QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterColor::typeName()
             << QgsProcessingParameterString::typeName();
    }

    [[nodiscard]] QStringList acceptedOutputTypes() const override
    {
      return QStringList()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputVariant::typeName();
    }

    [[nodiscard]] QColor modelColor() const override { return QColor( 34, 157, 214 ); /* blue */ };
};

/**
 * \brief A coordinate operation parameter for Processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('coordinateoperation')
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsProcessingParameterTypeCoordinateOperation : public QgsProcessingParameterType
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterCoordinateOperation( name );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A coordinate operation parameter." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Coordinate Operation" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "coordinateoperation" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterCoordinateOperation" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterCoordinateOperation" );
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: string representation of a Proj coordinate operation" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "String representation of Proj coordinate operation" );
    }

    [[nodiscard]] QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterCoordinateOperation::typeName()
             << QgsProcessingParameterString::typeName();
    }

    [[nodiscard]] QStringList acceptedOutputTypes() const override
    {
      return QStringList()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputVariant::typeName();
    }
};

/**
 * \brief A map theme parameter for Processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('maptheme')
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsProcessingParameterTypeMapTheme: public QgsProcessingParameterType
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterMapTheme( name );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A map theme parameter." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Map Theme" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "maptheme" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterMapTheme" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterMapTheme" );
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: name of an existing map theme" )
             << QStringLiteral( "QgsProperty" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Name of an existing map theme" );
    }

    [[nodiscard]] QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterExpression::typeName();
    }

    [[nodiscard]] QStringList acceptedOutputTypes() const override
    {
      return QStringList()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputVariant::typeName();
    }
};

/**
 * \brief A datetime parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('datetime')
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsProcessingParameterTypeDateTime : public QgsProcessingParameterType
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterDateTime( name );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A datetime parameter, including datetime, date or time values." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Datetime" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "datetime" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterDateTime" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterDateTime" );
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "str" )
             << QStringLiteral( "QDateTime" )
             << QStringLiteral( "QDate" )
             << QStringLiteral( "QTime" )
             << QStringLiteral( "QgsProperty" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "A datetime value in ISO format" )
             << QObject::tr( "field:FIELD_NAME to use a data defined value taken from the FIELD_NAME field" )
             << QObject::tr( "expression:SOME EXPRESSION to use a data defined value calculated using a custom QGIS expression" );
    }

    [[nodiscard]] QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterDateTime::typeName()
             << QgsProcessingParameterString::typeName();
    }

    [[nodiscard]] QStringList acceptedOutputTypes() const override
    {
      return QStringList()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputVariant::typeName();
    }

    [[nodiscard]] QColor modelColor() const override { return QColor( 255, 131, 23 ); /* orange */ };
};

/**
 * \brief A provider connection name parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('providerconnection')
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsProcessingParameterTypeProviderConnection : public QgsProcessingParameterType
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterProviderConnection( name, QString(), QString() );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A connection name parameter, for registered database connections." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Connection Name" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "providerconnection" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterProviderConnection" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterProviderConnection" );
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "str" )
             << QStringLiteral( "QgsProperty" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Name of registered database connection" );
    }

    [[nodiscard]] QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterProviderConnection::typeName()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterExpression::typeName();
    }

    [[nodiscard]] QStringList acceptedOutputTypes() const override
    {
      return QStringList()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputVariant::typeName();
    }
};

/**
 * \brief A database schema name parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('databaseschema')
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsProcessingParameterTypeDatabaseSchema : public QgsProcessingParameterType
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterDatabaseSchema( name, QString(), QString() );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A database schema parameter." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Database Schema" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "databaseschema" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterDatabaseSchema" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterDatabaseSchema" );
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "str" )
             << QStringLiteral( "QgsProperty" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Name of existing database schema" );
    }

    [[nodiscard]] QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterDatabaseSchema::typeName()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterExpression::typeName();
    }

    [[nodiscard]] QStringList acceptedOutputTypes() const override
    {
      return QStringList()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputVariant::typeName();
    }
};

/**
 * \brief A database table name parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('databasetable')
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsProcessingParameterTypeDatabaseTable: public QgsProcessingParameterType
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterDatabaseTable( name, QString(), QString(), QString() );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A database table parameter." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Database Table" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "databasetable" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterDatabaseTable" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterDatabaseTable" );
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "str" )
             << QStringLiteral( "QgsProperty" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Name of existing database table" );
    }

    [[nodiscard]] QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterDatabaseTable::typeName()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterExpression::typeName();
    }

    [[nodiscard]] QStringList acceptedOutputTypes() const override
    {
      return QStringList()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputVariant::typeName();
    }
};

/**
 * \brief A point cloud layer parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('pointcloud')
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsProcessingParameterTypePointCloudLayer : public QgsProcessingParameterType
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterPointCloudLayer( name );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A point cloud layer parameter." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Point Cloud Layer" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterPointCloudLayer" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterPointCloudLayer" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "pointcloud" );
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: layer ID" )
             << QObject::tr( "str: layer name" )
             << QObject::tr( "str: layer source" )
             << QStringLiteral( "QgsPointCloudLayer" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Path to a point cloud layer" );
    }

    [[nodiscard]] QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterPointCloudLayer::typeName()
             << QgsProcessingParameterMapLayer::typeName()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterExpression::typeName();
    }

    [[nodiscard]] QStringList acceptedOutputTypes() const override
    {
      return QStringList()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputPointCloudLayer::typeName()
             << QgsProcessingOutputMapLayer::typeName()
             << QgsProcessingOutputFile::typeName()
             << QgsProcessingOutputFolder::typeName();
    }

    [[nodiscard]] QColor modelColor() const override { return QColor( 137, 150, 171 ); /* cold gray */ };
};

/**
 * \brief An annotation layer parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('annotation')
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsProcessingParameterTypeAnnotationLayer : public QgsProcessingParameterType
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterAnnotationLayer( name );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "An annotation layer parameter." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Annotation Layer" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterAnnotationLayer" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterAnnotationLayer" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "annotation" );
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: layer ID" )
             << QObject::tr( "str: layer name" )
             << QObject::tr( "\"main\": main annotation layer for a project" )
             << QStringLiteral( "QgsAnnotationLayer" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Layer ID for an annotation layer, or \"main\" for the main annotation layer in a project." );
    }

    [[nodiscard]] QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterAnnotationLayer::typeName()
             << QgsProcessingParameterMapLayer::typeName()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterExpression::typeName();
    }

    [[nodiscard]] QStringList acceptedOutputTypes() const override
    {
      return QStringList()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputMapLayer::typeName()
             << QgsProcessingOutputVariant::typeName();
    }

    [[nodiscard]] QColor modelColor() const override { return QColor( 137, 150, 171 ); /* cold gray */ };
};

/**
 * \brief A pointcloud layer destination parameter, for specifying the destination path for a point cloud layer
 * created by the algorithm.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('pointCloudDestination')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypePointCloudDestination : public QgsProcessingParameterTypeDestination
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterPointCloudDestination( name );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A point cloud layer destination parameter." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Point Cloud Destination" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "pointCloudDestination" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterPointCloudDestination" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterPointCloudDestination" );
    }

    [[nodiscard]] Qgis::ProcessingParameterTypeFlags flags() const override
    {
      Qgis::ProcessingParameterTypeFlags flags = QgsProcessingParameterType::flags();
      flags.setFlag( Qgis::ProcessingParameterTypeFlag::ExposeToModeler, false );
      return flags;
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "str" )
             << QStringLiteral( "QgsProperty" )
             << QStringLiteral( "QgsProcessingOutputLayerDefinition" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Path for new point cloud layer" );
    }

    [[nodiscard]] QColor modelColor() const override { return QColor( 80, 80, 80 ); /* dark gray */ };
};

/**
 * \brief A point cloud layer attribute parameter for Processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('attribute')
 * \since QGIS 3.32
 */
class CORE_EXPORT QgsProcessingParameterTypePointCloudAttribute : public QgsProcessingParameterType
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterPointCloudAttribute( name );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A point cloud attribute parameter, for selecting an attribute from a point cloud source." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Point Cloud Attribute" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "attribute" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterPointCloudAttribute" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterPointCloudAttribute" );
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "str" )
             << QStringLiteral( "QgsProperty" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "The name of an attribute" )
             << QObject::tr( "; delimited list of attribute names" );
    }

    [[nodiscard]] QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterPointCloudAttribute::typeName()
             << QgsProcessingParameterString::typeName();
    }

    [[nodiscard]] QStringList acceptedOutputTypes() const override
    {
      return QStringList()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputVariant::typeName();
    }
};

/**
 * \brief A vector tile layer destination parameter, for specifying the destination path for a vector tile layer
 * created by the algorithm.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('vectorTileDestination')
 * \since QGIS 3.32
 */
class CORE_EXPORT QgsProcessingParameterTypeVectorTileDestination : public QgsProcessingParameterTypeDestination
{
    [[nodiscard]] QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterVectorTileDestination( name );
    }

    [[nodiscard]] QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A vector tiles layer destination parameter." );
    }

    [[nodiscard]] QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Vector Tile Destination" );
    }

    [[nodiscard]] QString id() const override
    {
      return QStringLiteral( "vectorTileDestination" );
    }

    [[nodiscard]] QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterVectorTileDestination" );
    }

    [[nodiscard]] QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterVectorTileDestination" );
    }

    [[nodiscard]] Qgis::ProcessingParameterTypeFlags flags() const override
    {
      Qgis::ProcessingParameterTypeFlags flags = QgsProcessingParameterType::flags();
      flags.setFlag( Qgis::ProcessingParameterTypeFlag::ExposeToModeler, false );
      return flags;
    }

    [[nodiscard]] QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "str" )
             << QStringLiteral( "QgsProperty" )
             << QStringLiteral( "QgsProcessingOutputLayerDefinition" );
    }

    [[nodiscard]] QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Path for new vector tile layer" );
    }

    [[nodiscard]] QColor modelColor() const override { return QColor( 80, 80, 80 ); /* dark gray */ };
};

#endif // QGSPROCESSINGPARAMETERTYPEIMPL_H
