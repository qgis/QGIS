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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterRasterLayer( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A raster layer parameter." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Raster Layer" );
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterRasterLayer"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterRasterLayer"_s;
    }

    QString id() const override
    {
      return u"raster"_s;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: layer ID" )
             << QObject::tr( "str: layer name" )
             << QObject::tr( "str: layer source" )
             << u"QgsProcessingRasterLayerDefinition"_s
             << u"QgsProperty"_s
             << u"QgsRasterLayer"_s;
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Path to a raster layer" );
    }

    QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterRasterLayer::typeName()
             << QgsProcessingParameterMapLayer::typeName()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterExpression::typeName();
    }

    QStringList acceptedOutputTypes() const override
    {
      return QStringList()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputRasterLayer::typeName()
             << QgsProcessingOutputMapLayer::typeName()
             << QgsProcessingOutputFile::typeName()
             << QgsProcessingOutputFolder::typeName();
    }

    QColor modelColor() const override { return QColor( 0, 180, 180 ); /* turquoise */ };
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterMeshLayer( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A mesh layer parameter." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Mesh Layer" );
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterMeshLayer"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterMeshLayer"_s;
    }

    QString id() const override
    {
      return u"mesh"_s;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: layer ID" )
             << QObject::tr( "str: layer name" )
             << QObject::tr( "str: layer source" )
             << u"QgsMeshLayer"_s;
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Path to a mesh layer" );
    }

    QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterMeshLayer::typeName()
             << QgsProcessingParameterMapLayer::typeName()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterExpression::typeName();
    }

    QStringList acceptedOutputTypes() const override
    {
      return QStringList()
             << QgsProcessingOutputString::typeName()
             // TODO  << QgsProcessingOutputMeshLayer::typeName()
             << QgsProcessingOutputMapLayer::typeName()
             << QgsProcessingOutputFile::typeName()
             << QgsProcessingOutputFolder::typeName();
    }

    QColor modelColor() const override { return QColor( 137, 150, 171 ); /* cold gray */ };
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterVectorLayer( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A vector layer parameter, e.g. for algorithms which change layer styles, edit layers in place, or other operations which affect an entire layer." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Vector Layer" );
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterVectorLayer"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterVectorLayer"_s;
    }

    QString id() const override
    {
      return u"vector"_s;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: layer ID" )
             << QObject::tr( "str: layer name" )
             << QObject::tr( "str: layer source" )
             << u"QgsProperty"_s
             << u"QgsVectorLayer"_s;
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Path to a vector layer" );
    }

    QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterVectorLayer::typeName()
             << QgsProcessingParameterMapLayer::typeName()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterExpression::typeName();
    }

    QStringList acceptedOutputTypes() const override
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

    QColor modelColor() const override { return QColor( 122, 0, 47 ); /* burgundy */ };
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterMapLayer( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A generic map layer parameter, which accepts either vector or raster layers." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Map Layer" );
    }

    QString id() const override
    {
      return u"layer"_s;
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterMapLayer"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterMapLayer"_s;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: layer ID" )
             << QObject::tr( "str: layer name" )
             << QObject::tr( "str: layer source" )
             << u"QgsMapLayer"_s
             << u"QgsProperty"_s
             << u"QgsRasterLayer"_s
             << u"QgsVectorLayer"_s;
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Path to a vector, raster or mesh layer" );
    }

    QStringList acceptedParameterTypes() const override
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

    QStringList acceptedOutputTypes() const override
    {
      return QStringList()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputRasterLayer::typeName()
             << QgsProcessingOutputVectorLayer::typeName()
             << QgsProcessingOutputMapLayer::typeName()
             << QgsProcessingOutputFile::typeName()
             << QgsProcessingOutputVariant::typeName();
    }

    QColor modelColor() const override { return QColor( 137, 150, 171 ); /* cold gray */ };
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterBoolean( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A boolean parameter, for true/false values." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Boolean" );
    }

    QString id() const override
    {
      return u"boolean"_s;
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterBoolean"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterBoolean"_s;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << u"bool"_s
             << u"int"_s
             << u"str"_s
             << u"QgsProperty"_s;
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "1 for true/yes" )
             << QObject::tr( "0 for false/no" )
             << QObject::tr( "field:FIELD_NAME to use a data defined value taken from the FIELD_NAME field" )
             << QObject::tr( "expression:SOME EXPRESSION to use a data defined value calculated using a custom QGIS expression" );
    }

    QStringList acceptedParameterTypes() const override
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
    QStringList acceptedOutputTypes() const override
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

    QColor modelColor() const override { return QColor( 51, 201, 28 ); /* green */ };
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterExpression( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "An expression parameter, to add custom expressions based on layer fields." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Expression" );
    }

    QString id() const override
    {
      return u"expression"_s;
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterExpression"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterExpression"_s;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << u"str"_s
             << u"QgsProperty"_s;
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "A valid QGIS expression string, e.g \"road_name\" = 'MAIN RD'" );
    }

    QStringList acceptedParameterTypes() const override
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

    QStringList acceptedOutputTypes() const override
    {
      return QStringList()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputNumber::typeName()
             << QgsProcessingOutputVariant::typeName();
    }

    QColor modelColor() const override { return QColor( 255, 131, 23 ); /* orange */ };
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterCrs( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A coordinate reference system (CRS) input parameter." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "CRS" );
    }

    QString id() const override
    {
      return u"crs"_s;
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterCrs"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterCrs"_s;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList()
             << u"str: 'ProjectCrs'"_s
             << QObject::tr( "str: CRS auth ID (e.g. 'EPSG:3111')" )
             << QObject::tr( "str: CRS PROJ4 (e.g. 'PROJ4:…')" )
             << QObject::tr( "str: CRS WKT (e.g. 'WKT:…')" )
             << QObject::tr( "str: layer ID. CRS of layer is used." )
             << QObject::tr( "str: layer name. CRS of layer is used." )
             << QObject::tr( "str: layer source. CRS of layer is used." )
             << QObject::tr( "QgsCoordinateReferenceSystem" )
             << QObject::tr( "QgsMapLayer: CRS of layer is used" )
             << QObject::tr( "QgsProcessingFeatureSourceDefinition: CRS of source is used" )
             << u"QgsProperty"_s;
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "CRS as an auth ID (e.g. 'EPSG:3111')" )
             << QObject::tr( "CRS as a PROJ4 string (e.g. 'PROJ4:…')" )
             << QObject::tr( "CRS as a WKT string (e.g. 'WKT:…')" )
             << QObject::tr( "Path to a layer. The CRS of the layer is used." ) ;
    }

    QStringList acceptedParameterTypes() const override
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

    QStringList acceptedOutputTypes() const override
    {
      return QStringList() << QgsProcessingOutputVectorLayer::typeName()
             << QgsProcessingOutputRasterLayer::typeName()
             << QgsProcessingOutputMapLayer::typeName()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputVariant::typeName();
    }

    QColor modelColor() const override { return QColor( 255, 131, 23 ); /* orange */ };
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterRange( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A numeric range parameter for processing algorithms." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Range" );
    }

    QString id() const override
    {
      return u"range"_s;
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterRange"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterRange"_s;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "list[float]: list of 2 float values" )
             << QObject::tr( "list[str]: list of strings representing floats" )
             << QObject::tr( "str: as two comma delimited floats, e.g. '1,10'" )
             << u"QgsProperty"_s;
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Two comma separated numeric values, e.g. '1,10'" );
    }

    QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterRange::typeName()
             << QgsProcessingParameterString::typeName();
    }

    QStringList acceptedOutputTypes() const override
    {
      return QStringList() << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputVariant::typeName();
    }

    QColor modelColor() const override { return QColor( 34, 157, 214 ); /* blue */ };
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterPoint( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A point geometry parameter." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Point" );
    }

    QString id() const override
    {
      return u"point"_s;
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterPoint"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterPoint"_s;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: as an 'x,y' string, e.g. '1.5,10.1'" )
             << u"QgsPointXY"_s
             << u"QgsProperty"_s
             << u"QgsReferencedPointXY"_s
             << u"QgsGeometry: centroid of geometry is used"_s;
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Point coordinate as an 'x,y' string, e.g. '1.5,10.1'" );
    }

    QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterPoint::typeName()
             << QgsProcessingParameterString::typeName();
    }

    QStringList acceptedOutputTypes() const override
    {
      return QStringList()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputVariant::typeName();
    }

    QColor modelColor() const override { return QColor( 122, 0, 47 ); /* burgundy */ };
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterGeometry( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A geometry parameter." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Geometry" );
    }

    QString id() const override
    {
      return u"geometry"_s;
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterGeometry"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterGeometry"_s;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: as Well-Known Text string (WKT)" )
             << u"QgsGeometry"_s
             << u"QgsProperty"_s;
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Well-Known Text string (WKT)" );
    }

    QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterGeometry::typeName()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterPoint::typeName()
             << QgsProcessingParameterExtent::typeName();
    }

    QStringList acceptedOutputTypes() const override
    {
      return QStringList()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputVariant::typeName();
    }

    QColor modelColor() const override { return QColor( 122, 0, 47 ); /* burgundy */ };
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterEnum( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "An enumerated type parameter." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Enum" );
    }

    QString id() const override
    {
      return u"enum"_s;
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterEnum"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterEnum"_s;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << u"int"_s
             << QObject::tr( "str: as string representation of int, e.g. '1'" )
             << u"QgsProperty"_s;
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Number of selected option, e.g. '1'" )
             << QObject::tr( "Comma separated list of options, e.g. '1,3'" );
    }

    QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterEnum::typeName()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterNumber::typeName();
    }

    QStringList acceptedOutputTypes() const override
    {
      return QStringList()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputVariant::typeName()
             << QgsProcessingOutputNumber::typeName();
    }

    QColor modelColor() const override { return QColor( 152, 68, 201 ); /* purple */ };
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterExtent( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A map extent parameter." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Extent" );
    }

    QString id() const override
    {
      return u"extent"_s;
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterExtent"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterExtent"_s;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: as comma delimited list of x min, x max, y min, y max. E.g. '4,10,101,105'" )
             << QObject::tr( "str: layer ID. Extent of layer is used." )
             << QObject::tr( "str: layer name. Extent of layer is used." )
             << QObject::tr( "str: layer source. Extent of layer is used." )
             << QObject::tr( "QgsMapLayer: Extent of layer is used" )
             << QObject::tr( "QgsProcessingFeatureSourceDefinition: Extent of source is used" )
             << u"QgsProperty"_s
             << u"QgsRectangle"_s
             << u"QgsReferencedRectangle"_s
             << u"QgsGeometry: bounding box of geometry is used"_s;
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "A comma delimited string of x min, x max, y min, y max. E.g. '4,10,101,105'" )
             << QObject::tr( "Path to a layer. The extent of the layer is used." ) ;
    }

    QStringList acceptedParameterTypes() const override
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

    QStringList acceptedOutputTypes() const override
    {
      return QStringList()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputRasterLayer::typeName()
             << QgsProcessingOutputVectorLayer::typeName()
             << QgsProcessingOutputMapLayer::typeName()
             << QgsProcessingOutputVariant::typeName();
    }

    QColor modelColor() const override { return QColor( 34, 157, 214 ); /* blue */ };
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterMatrix( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A table (matrix) parameter for processing algorithms." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Matrix" );
    }

    QString id() const override
    {
      return u"matrix"_s;
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterMatrix"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterMatrix"_s;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: as comma delimited list of values" )
             << u"list"_s
             << u"QgsProperty"_s;
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "A comma delimited list of values" );
    }

    QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterMatrix::typeName();
    }

    QStringList acceptedOutputTypes() const override
    {
      return QStringList();
    }

    QColor modelColor() const override { return QColor( 34, 157, 214 ); /* blue */ };
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterFile( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A file or folder parameter, for use with non-map layer file sources or folders." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "File/Folder" );
    }

    QString id() const override
    {
      return u"file"_s;
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterFile"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterFile"_s;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << u"str"_s
             << u"QgsProperty"_s;
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Path to a file" );
    }

    QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterFile::typeName();
    }

    QStringList acceptedOutputTypes() const override
    {
      return QStringList() << QgsProcessingOutputFile::typeName()
             << QgsProcessingOutputFolder::typeName()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputVariant::typeName()
             << QgsProcessingOutputRasterLayer::typeName()
             << QgsProcessingOutputVectorLayer::typeName()
             << QgsProcessingOutputMapLayer::typeName();
    }

    QColor modelColor() const override { return QColor( 80, 80, 80 ); /* dark gray */ };
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterField( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A vector field parameter, for selecting an existing field from a vector source." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Vector Field" );
    }

    QString id() const override
    {
      return u"field"_s;
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterField"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterField"_s;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << u"str"_s
             << u"QgsProperty"_s;
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "The name of an existing field" )
             << QObject::tr( "; delimited list of existing field names" );
    }

    QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterField::typeName()
             << QgsProcessingParameterString::typeName();
    }

    QStringList acceptedOutputTypes() const override
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
    QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterRasterLayer::typeName()
             << QgsProcessingParameterMeshLayer::typeName()
             << QgsProcessingParameterVectorLayer::typeName()
             << QgsProcessingParameterMapLayer::typeName()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterExpression::typeName();
    }

    QStringList acceptedOutputTypes() const override
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterVectorDestination( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A vector layer destination parameter." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Vector Destination" );
    }

    QString id() const override
    {
      return u"vectorDestination"_s;
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterVectorDestination"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterVectorDestination"_s;
    }

    Qgis::ProcessingParameterTypeFlags flags() const override
    {
      Qgis::ProcessingParameterTypeFlags flags = QgsProcessingParameterType::flags();
      flags.setFlag( Qgis::ProcessingParameterTypeFlag::ExposeToModeler, false );
      return flags;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << u"str"_s
             << u"QgsProperty"_s
             << u"QgsProcessingOutputLayerDefinition"_s;
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Path for new vector layer" );
    }

    QColor modelColor() const override { return QColor( 122, 0, 47 ); /* burgundy */ };
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterFileDestination( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A generic file based destination parameter." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "File Destination" );
    }

    QString id() const override
    {
      return u"fileDestination"_s;
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterFileDestination"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterFileDestination"_s;
    }

    Qgis::ProcessingParameterTypeFlags flags() const override
    {
      Qgis::ProcessingParameterTypeFlags flags = QgsProcessingParameterType::flags();
      flags.setFlag( Qgis::ProcessingParameterTypeFlag::ExposeToModeler, false );
      return flags;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << u"str"_s
             << u"QgsProperty"_s;
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Path for new file" );
    }

    QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterFile::typeName();
    }

    QStringList acceptedOutputTypes() const override
    {
      return QStringList() << QgsProcessingOutputFile::typeName()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputVariant::typeName()
             << QgsProcessingOutputRasterLayer::typeName()
             << QgsProcessingOutputVectorLayer::typeName()
             << QgsProcessingOutputMapLayer::typeName();
    }

    QColor modelColor() const override { return QColor( 80, 80, 80 ); /* dark gray */ };
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterFolderDestination( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A folder destination parameter." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Folder Destination" );
    }

    QString id() const override
    {
      return u"folderDestination"_s;
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterFolderDestination"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterFolderDestination"_s;
    }

    Qgis::ProcessingParameterTypeFlags flags() const override
    {
      Qgis::ProcessingParameterTypeFlags flags = QgsProcessingParameterType::flags();
      flags.setFlag( Qgis::ProcessingParameterTypeFlag::ExposeToModeler, false );
      return flags;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << u"str"_s
             << u"QgsProperty"_s;
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Path for an existing or new folder" );
    }

    QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterFile::typeName();
    }

    QStringList acceptedOutputTypes() const override
    {
      return QStringList() << QgsProcessingOutputFile::typeName()
             << QgsProcessingOutputFolder::typeName()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputVariant::typeName();
    }

    QColor modelColor() const override { return QColor( 80, 80, 80 ); /* dark gray */ };
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterRasterDestination( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A raster layer destination parameter." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Raster Destination" );
    }

    QString id() const override
    {
      return u"rasterDestination"_s;
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterRasterDestination"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterRasterDestination"_s;
    }

    Qgis::ProcessingParameterTypeFlags flags() const override
    {
      Qgis::ProcessingParameterTypeFlags flags = QgsProcessingParameterType::flags();
      flags.setFlag( Qgis::ProcessingParameterTypeFlag::ExposeToModeler, false );
      return flags;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << u"str"_s
             << u"QgsProperty"_s
             << u"QgsProcessingOutputLayerDefinition"_s;
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Path for new raster layer" );
    }

    QColor modelColor() const override { return QColor( 0, 180, 180 ); /* turquoise */ };
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterString( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A freeform string parameter." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "String" );
    }

    QString id() const override
    {
      return u"string"_s;
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterString"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterString"_s;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << u"str"_s
             << u"QgsProperty"_s;
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "String value" )
             << QObject::tr( "field:FIELD_NAME to use a data defined value taken from the FIELD_NAME field" )
             << QObject::tr( "expression:SOME EXPRESSION to use a data defined value calculated using a custom QGIS expression" );
    }

    QStringList acceptedParameterTypes() const override
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

    QStringList acceptedOutputTypes() const override
    {
      return QStringList() << QgsProcessingOutputNumber::typeName()
             << QgsProcessingOutputVariant::typeName()
             << QgsProcessingOutputFile::typeName()
             << QgsProcessingOutputFolder::typeName()
             << QgsProcessingOutputString::typeName();
    }

    QColor modelColor() const override { return QColor( 255, 131, 23 ); /* orange */ };
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterAuthConfig( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "An authentication configuration parameter." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Authentication Configuration" );
    }

    QString id() const override
    {
      return u"authcfg"_s;
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterAuthConfig"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterAuthConfig"_s;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << u"str"_s;
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "An existing QGIS authentication ID string" );
    }

    QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterAuthConfig::typeName()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterExpression::typeName();
    }

    QStringList acceptedOutputTypes() const override
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterMultipleLayers( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "An input allowing selection of multiple sources, including multiple map layers or file sources." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Multiple Input" );
    }

    QString id() const override
    {
      return u"multilayer"_s;
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterMultipleLayers"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterMultipleLayers"_s;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "list[str]: list of layer IDs" )
             << QObject::tr( "list[str]: list of layer names" )
             << QObject::tr( "list[str]: list of layer sources" )
             << u"list[QgsMapLayer]"_s
             << u"QgsProperty"_s;
    }

    QStringList acceptedParameterTypes() const override
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
    QStringList acceptedOutputTypes() const override
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

    QColor modelColor() const override { return QColor( 137, 150, 171 ); /* cold gray */ };
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterFeatureSource( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A vector feature parameter, e.g. for algorithms which operate on the features within a layer." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Vector Features" );
    }

    QString id() const override
    {
      return u"source"_s;
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterFeatureSource"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterFeatureSource"_s;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: layer ID" )
             << QObject::tr( "str: layer name" )
             << QObject::tr( "str: layer source" )
             << u"QgsProcessingFeatureSourceDefinition"_s
             << u"QgsProperty"_s
             << u"QgsVectorLayer"_s;
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Path to a vector layer" );
    }

    QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterFeatureSource::typeName()
             << QgsProcessingParameterVectorLayer::typeName()
             << QgsProcessingParameterMapLayer::typeName()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterExpression::typeName();
    }

    QStringList acceptedOutputTypes() const override
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

    QColor modelColor() const override { return QColor( 122, 0, 47 ); /* burgundy */  };
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterNumber( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A numeric parameter, including float or integer values." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Number" );
    }

    QString id() const override
    {
      return u"number"_s;
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterNumber"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterNumber"_s;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << u"int"_s
             << u"float"_s
             << u"QgsProperty"_s;
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "A numeric value" )
             << QObject::tr( "field:FIELD_NAME to use a data defined value taken from the FIELD_NAME field" )
             << QObject::tr( "expression:SOME EXPRESSION to use a data defined value calculated using a custom QGIS expression" );
    }

    QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterNumber::typeName()
             << QgsProcessingParameterDistance::typeName()
             << QgsProcessingParameterVolume::typeName()
             << QgsProcessingParameterDuration::typeName()
             << QgsProcessingParameterScale::typeName();
    }

    QStringList acceptedOutputTypes() const override
    {
      return QStringList() << QgsProcessingOutputNumber::typeName()
             << QgsProcessingOutputVariant::typeName()
             << QgsProcessingOutputString::typeName();
    }

    QColor modelColor() const override { return QColor( 34, 157, 214 ); /* blue */ };
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterDistance( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A numeric parameter representing a distance measure." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Distance" );
    }

    QString id() const override
    {
      return u"distance"_s;
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterDistance"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterDistance"_s;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << u"int"_s
             << u"float"_s
             << u"QgsProperty"_s;
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "A numeric value" )
             << QObject::tr( "field:FIELD_NAME to use a data defined value taken from the FIELD_NAME field" )
             << QObject::tr( "expression:SOME EXPRESSION to use a data defined value calculated using a custom QGIS expression" );
    }

    QColor modelColor() const override { return QColor( 34, 157, 214 ); /* blue */ };
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterArea( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A numeric parameter representing an area measure." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Area" );
    }

    QString id() const override
    {
      return u"area"_s;
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterArea"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterArea"_s;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << u"int"_s
             << u"float"_s
             << u"QgsProperty"_s;
    }

    QStringList acceptedStringValues() const override
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterVolume( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A numeric parameter representing a volume measure." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Volume" );
    }

    QString id() const override
    {
      return u"volume"_s;
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterVolume"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterVolume"_s;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << u"int"_s
             << u"float"_s
             << u"QgsProperty"_s;
    }

    QStringList acceptedStringValues() const override
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterDuration( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A numeric parameter representing a duration measure." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Duration" );
    }

    QString id() const override
    {
      return u"duration"_s;
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterDuration"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterDuration"_s;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << u"int"_s
             << u"float"_s
             << u"QgsProperty"_s;
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "A numeric value (unit type set by algorithms)" )
             << QObject::tr( "field:FIELD_NAME to use a data defined value taken from the FIELD_NAME field" )
             << QObject::tr( "expression:SOME EXPRESSION to use a data defined value calculated using a custom QGIS expression" );
    }

    QColor modelColor() const override { return QColor( 34, 157, 214 ); /* blue */ };
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterScale( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A numeric parameter representing a map scale." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Scale" );
    }

    QString id() const override
    {
      return u"scale"_s;
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterScale"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterScale"_s;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << u"int: scale denominator"_s
             << u"float: scale denominator"_s
             << u"QgsProperty"_s;
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "A numeric value representing the scale denominator" );
    }

    QColor modelColor() const override { return QColor( 34, 157, 214 ); /* blue */ };
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterBand( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A raster band parameter, for selecting an existing band from a raster source." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Raster Band" );
    }

    QString id() const override
    {
      return u"band"_s;
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterBand"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterBand"_s;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << u"int"_s
             << u"QgsProperty"_s;
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Integer value representing an existing raster band number" );
    }

    QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterBand::typeName()
             << QgsProcessingParameterNumber::typeName();
    }

    QStringList acceptedOutputTypes() const override
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterFeatureSink( name );
    }

    Qgis::ProcessingParameterTypeFlags flags() const override
    {
      Qgis::ProcessingParameterTypeFlags flags = QgsProcessingParameterType::flags();
      flags.setFlag( Qgis::ProcessingParameterTypeFlag::ExposeToModeler, false );
      return flags;
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A feature sink destination parameter." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Feature Sink" );
    }

    QString id() const override
    {
      return u"sink"_s;
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterFeatureSink"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterFeatureSink"_s;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: destination vector file, e.g. 'd:/test.shp'" )
             << QObject::tr( "str: 'memory:' to store result in temporary memory layer" )
             << QObject::tr( "str: using vector provider ID prefix and destination URI, e.g. 'postgres:…' to store result in PostGIS table" )
             << u"QgsProcessingOutputLayerDefinition"_s
             << u"QgsProperty"_s;
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Path for new vector layer" );
    }

    QColor modelColor() const override { return QColor( 122, 0, 47 ); /* burgundy */ };
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterLayout( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A print layout parameter." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Print Layout" );
    }

    QString id() const override
    {
      return u"layout"_s;
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterLayout"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterLayout"_s;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: name of print layout in current project" )
             << u"QgsProperty"_s;
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Name of print layout in current project" );
    }

    QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterLayout::typeName()
             << QgsProcessingParameterString::typeName();
    }

    QStringList acceptedOutputTypes() const override
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterLayoutItem( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A print layout item parameter." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Print Layout Item" );
    }

    QString id() const override
    {
      return u"layoutitem"_s;
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterLayoutItem"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterLayoutItem"_s;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: UUID of print layout item" )
             << QObject::tr( "str: id of print layout item" )
             << u"QgsProperty"_s;
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "UUID or item id of layout item" );
    }

    QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterLayoutItem::typeName()
             << QgsProcessingParameterString::typeName();
    }

    QStringList acceptedOutputTypes() const override
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterColor( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A color parameter." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Color" );
    }

    QString id() const override
    {
      return u"color"_s;
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterColor"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterColor"_s;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: string representation of color, e.g #ff0000 or rgba(200,100,50,0.8)" )
             << u"QColor"_s
             << u"QgsProperty"_s;
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "String representation of color, e.g #ff0000 or rgba(200,100,50,0.8)" )
             << QObject::tr( "field:FIELD_NAME to use a data defined value taken from the FIELD_NAME field" )
             << QObject::tr( "expression:SOME EXPRESSION to use a data defined value calculated using a custom QGIS expression" );
    }

    QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterColor::typeName()
             << QgsProcessingParameterString::typeName();
    }

    QStringList acceptedOutputTypes() const override
    {
      return QStringList()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputVariant::typeName();
    }

    QColor modelColor() const override { return QColor( 34, 157, 214 ); /* blue */ };
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterCoordinateOperation( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A coordinate operation parameter." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Coordinate Operation" );
    }

    QString id() const override
    {
      return u"coordinateoperation"_s;
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterCoordinateOperation"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterCoordinateOperation"_s;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: string representation of a Proj coordinate operation" );
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "String representation of Proj coordinate operation" );
    }

    QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterCoordinateOperation::typeName()
             << QgsProcessingParameterString::typeName();
    }

    QStringList acceptedOutputTypes() const override
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterMapTheme( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A map theme parameter." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Map Theme" );
    }

    QString id() const override
    {
      return u"maptheme"_s;
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterMapTheme"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterMapTheme"_s;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: name of an existing map theme" )
             << u"QgsProperty"_s;
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Name of an existing map theme" );
    }

    QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterExpression::typeName();
    }

    QStringList acceptedOutputTypes() const override
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterDateTime( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A datetime parameter, including datetime, date or time values." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Datetime" );
    }

    QString id() const override
    {
      return u"datetime"_s;
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterDateTime"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterDateTime"_s;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << u"str"_s
             << u"QDateTime"_s
             << u"QDate"_s
             << u"QTime"_s
             << u"QgsProperty"_s;
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "A datetime value in ISO format" )
             << QObject::tr( "field:FIELD_NAME to use a data defined value taken from the FIELD_NAME field" )
             << QObject::tr( "expression:SOME EXPRESSION to use a data defined value calculated using a custom QGIS expression" );
    }

    QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterDateTime::typeName()
             << QgsProcessingParameterString::typeName();
    }

    QStringList acceptedOutputTypes() const override
    {
      return QStringList()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputVariant::typeName();
    }

    QColor modelColor() const override { return QColor( 255, 131, 23 ); /* orange */ };
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterProviderConnection( name, QString(), QString() );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A connection name parameter, for registered database connections." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Connection Name" );
    }

    QString id() const override
    {
      return u"providerconnection"_s;
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterProviderConnection"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterProviderConnection"_s;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << u"str"_s
             << u"QgsProperty"_s;
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Name of registered database connection" );
    }

    QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterProviderConnection::typeName()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterExpression::typeName();
    }

    QStringList acceptedOutputTypes() const override
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterDatabaseSchema( name, QString(), QString() );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A database schema parameter." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Database Schema" );
    }

    QString id() const override
    {
      return u"databaseschema"_s;
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterDatabaseSchema"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterDatabaseSchema"_s;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << u"str"_s
             << u"QgsProperty"_s;
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Name of existing database schema" );
    }

    QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterDatabaseSchema::typeName()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterExpression::typeName();
    }

    QStringList acceptedOutputTypes() const override
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterDatabaseTable( name, QString(), QString(), QString() );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A database table parameter." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Database Table" );
    }

    QString id() const override
    {
      return u"databasetable"_s;
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterDatabaseTable"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterDatabaseTable"_s;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << u"str"_s
             << u"QgsProperty"_s;
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Name of existing database table" );
    }

    QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterDatabaseTable::typeName()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterExpression::typeName();
    }

    QStringList acceptedOutputTypes() const override
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterPointCloudLayer( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A point cloud layer parameter." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Point Cloud Layer" );
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterPointCloudLayer"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterPointCloudLayer"_s;
    }

    QString id() const override
    {
      return u"pointcloud"_s;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: layer ID" )
             << QObject::tr( "str: layer name" )
             << QObject::tr( "str: layer source" )
             << u"QgsPointCloudLayer"_s;
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Path to a point cloud layer" );
    }

    QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterPointCloudLayer::typeName()
             << QgsProcessingParameterMapLayer::typeName()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterExpression::typeName();
    }

    QStringList acceptedOutputTypes() const override
    {
      return QStringList()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputPointCloudLayer::typeName()
             << QgsProcessingOutputMapLayer::typeName()
             << QgsProcessingOutputFile::typeName()
             << QgsProcessingOutputFolder::typeName();
    }

    QColor modelColor() const override { return QColor( 137, 150, 171 ); /* cold gray */ };
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterAnnotationLayer( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "An annotation layer parameter." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Annotation Layer" );
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterAnnotationLayer"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterAnnotationLayer"_s;
    }

    QString id() const override
    {
      return u"annotation"_s;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: layer ID" )
             << QObject::tr( "str: layer name" )
             << QObject::tr( "\"main\": main annotation layer for a project" )
             << u"QgsAnnotationLayer"_s;
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Layer ID for an annotation layer, or \"main\" for the main annotation layer in a project." );
    }

    QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterAnnotationLayer::typeName()
             << QgsProcessingParameterMapLayer::typeName()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterExpression::typeName();
    }

    QStringList acceptedOutputTypes() const override
    {
      return QStringList()
             << QgsProcessingOutputString::typeName()
             << QgsProcessingOutputMapLayer::typeName()
             << QgsProcessingOutputVariant::typeName();
    }

    QColor modelColor() const override { return QColor( 137, 150, 171 ); /* cold gray */ };
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterPointCloudDestination( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A point cloud layer destination parameter." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Point Cloud Destination" );
    }

    QString id() const override
    {
      return u"pointCloudDestination"_s;
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterPointCloudDestination"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterPointCloudDestination"_s;
    }

    Qgis::ProcessingParameterTypeFlags flags() const override
    {
      Qgis::ProcessingParameterTypeFlags flags = QgsProcessingParameterType::flags();
      flags.setFlag( Qgis::ProcessingParameterTypeFlag::ExposeToModeler, false );
      return flags;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << u"str"_s
             << u"QgsProperty"_s
             << u"QgsProcessingOutputLayerDefinition"_s;
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Path for new point cloud layer" );
    }

    QColor modelColor() const override { return QColor( 80, 80, 80 ); /* dark gray */ };
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterPointCloudAttribute( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A point cloud attribute parameter, for selecting an attribute from a point cloud source." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Point Cloud Attribute" );
    }

    QString id() const override
    {
      return u"attribute"_s;
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterPointCloudAttribute"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterPointCloudAttribute"_s;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << u"str"_s
             << u"QgsProperty"_s;
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "The name of an attribute" )
             << QObject::tr( "; delimited list of attribute names" );
    }

    QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterPointCloudAttribute::typeName()
             << QgsProcessingParameterString::typeName();
    }

    QStringList acceptedOutputTypes() const override
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
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterVectorTileDestination( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A vector tiles layer destination parameter." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Vector Tile Destination" );
    }

    QString id() const override
    {
      return u"vectorTileDestination"_s;
    }

    QString pythonImportString() const override
    {
      return u"from qgis.core import QgsProcessingParameterVectorTileDestination"_s;
    }

    QString className() const override
    {
      return u"QgsProcessingParameterVectorTileDestination"_s;
    }

    Qgis::ProcessingParameterTypeFlags flags() const override
    {
      Qgis::ProcessingParameterTypeFlags flags = QgsProcessingParameterType::flags();
      flags.setFlag( Qgis::ProcessingParameterTypeFlag::ExposeToModeler, false );
      return flags;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << u"str"_s
             << u"QgsProperty"_s
             << u"QgsProcessingOutputLayerDefinition"_s;
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Path for new vector tile layer" );
    }

    QColor modelColor() const override { return QColor( 80, 80, 80 ); /* dark gray */ };
};

#endif // QGSPROCESSINGPARAMETERTYPEIMPL_H
