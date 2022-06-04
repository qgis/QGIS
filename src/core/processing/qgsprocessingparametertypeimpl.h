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
      return QStringLiteral( "from qgis.core import QgsProcessingParameterRasterLayer" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterRasterLayer" );
    }

    QString id() const override
    {
      return QStringLiteral( "raster" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: layer ID" )
             << QObject::tr( "str: layer name" )
             << QObject::tr( "str: layer source" )
             << QStringLiteral( "QgsProperty" )
             << QStringLiteral( "QgsRasterLayer" );
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Path to a raster layer" );
    }
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
      return QStringLiteral( "from qgis.core import QgsProcessingParameterMeshLayer" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterMeshLayer" );
    }

    QString id() const override
    {
      return QStringLiteral( "mesh" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: layer ID" )
             << QObject::tr( "str: layer name" )
             << QObject::tr( "str: layer source" )
             << QStringLiteral( "QgsMeshLayer" );
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Path to a mesh layer" );
    }
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
      return QStringLiteral( "from qgis.core import QgsProcessingParameterVectorLayer" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterVectorLayer" );
    }

    QString id() const override
    {
      return QStringLiteral( "vector" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: layer ID" )
             << QObject::tr( "str: layer name" )
             << QObject::tr( "str: layer source" )
             << QStringLiteral( "QgsProperty" )
             << QStringLiteral( "QgsVectorLayer" );
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Path to a vector layer" );
    }
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
      return QStringLiteral( "layer" );
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterMapLayer" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterMapLayer" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: layer ID" )
             << QObject::tr( "str: layer name" )
             << QObject::tr( "str: layer source" )
             << QStringLiteral( "QgsMapLayer" )
             << QStringLiteral( "QgsProperty" )
             << QStringLiteral( "QgsRasterLayer" )
             << QStringLiteral( "QgsVectorLayer" );
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Path to a vector, raster or mesh layer" );
    }
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
      return QStringLiteral( "boolean" );
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterBoolean" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterBoolean" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "bool" )
             << QStringLiteral( "int" )
             << QStringLiteral( "str" )
             << QStringLiteral( "QgsProperty" );
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "1 for true/yes" )
             << QObject::tr( "0 for false/no" );
    }
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
      return QStringLiteral( "expression" );
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterExpression" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterExpression" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "str" )
             << QStringLiteral( "QgsProperty" );
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "A valid QGIS expression string, e.g \"road_name\" = 'MAIN RD'" );
    }
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
      return QStringLiteral( "crs" );
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterCrs" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterCrs" );
    }

    QStringList acceptedPythonTypes() const override
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

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "CRS as an auth ID (e.g. 'EPSG:3111')" )
             << QObject::tr( "CRS as a PROJ4 string (e.g. 'PROJ4:…')" )
             << QObject::tr( "CRS as a WKT string (e.g. 'WKT:…')" )
             << QObject::tr( "Path to a layer. The CRS of the layer is used." ) ;
    }
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
      return QStringLiteral( "range" );
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterRange" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterRange" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "list[float]: list of 2 float values" )
             << QObject::tr( "list[str]: list of strings representing floats" )
             << QObject::tr( "str: as two comma delimited floats, e.g. '1,10'" )
             << QStringLiteral( "QgsProperty" );
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Two comma separated numeric values, e.g. '1,10'" );
    }
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
      return QStringLiteral( "point" );
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterPoint" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterPoint" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: as an 'x,y' string, e.g. '1.5,10.1'" )
             << QStringLiteral( "QgsPointXY" )
             << QStringLiteral( "QgsProperty" )
             << QStringLiteral( "QgsReferencedPointXY" )
             << QStringLiteral( "QgsGeometry: centroid of geometry is used" );
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Point coordinate as an 'x,y' string, e.g. '1.5,10.1'" );
    }
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
      return QStringLiteral( "geometry" );
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterGeometry" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterGeometry" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: as Well-Known Text string (WKT)" )
             << QStringLiteral( "QgsGeometry" )
             << QStringLiteral( "QgsProperty" );
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Well-Known Text string (WKT)" );
    }
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
      return QStringLiteral( "enum" );
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterEnum" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterEnum" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "int" )
             << QObject::tr( "str: as string representation of int, e.g. '1'" )
             << QStringLiteral( "QgsProperty" );
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Number of selected option, e.g. '1'" )
             << QObject::tr( "Comma separated list of options, e.g. '1,3'" );
    }
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
      return QStringLiteral( "extent" );
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterExtent" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterExtent" );
    }

    QStringList acceptedPythonTypes() const override
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

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "A comma delimited string of x min, x max, y min, y max. E.g. '4,10,101,105'" )
             << QObject::tr( "Path to a layer. The extent of the layer is used." ) ;
    }
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
      return QStringLiteral( "matrix" );
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterMatrix" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterMatrix" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: as comma delimited list of values" )
             << QStringLiteral( "list" )
             << QStringLiteral( "QgsProperty" );
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "A comma delimited list of values" );
    }
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
      return QStringLiteral( "file" );
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterFile" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterFile" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "str" )
             << QStringLiteral( "QgsProperty" );
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Path to a file" );
    }
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
      return QStringLiteral( "field" );
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterField" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterField" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "str" )
             << QStringLiteral( "QgsProperty" );
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "The name of an existing field" )
             << QObject::tr( "; delimited list of existing field names" );
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
class CORE_EXPORT QgsProcessingParameterTypeVectorDestination : public QgsProcessingParameterType
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
      return QStringLiteral( "vectorDestination" );
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterVectorDestination" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterVectorDestination" );
    }

    ParameterFlags flags() const override
    {
      ParameterFlags flags = QgsProcessingParameterType::flags();
      flags.setFlag( ParameterFlag::ExposeToModeler, false );
      return flags;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "str" )
             << QStringLiteral( "QgsProperty" )
             << QStringLiteral( "QgsProcessingOutputLayerDefinition" );
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Path for new vector layer" );
    }
};

/**
 * \brief A generic file based destination parameter, for specifying the destination path for a file (non-map layer)
 * created by the algorithm.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('fileDestination')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeFileDestination : public QgsProcessingParameterType
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
      return QStringLiteral( "fileDestination" );
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterFileDestination" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterFileDestination" );
    }

    ParameterFlags flags() const override
    {
      ParameterFlags flags = QgsProcessingParameterType::flags();
      flags.setFlag( ParameterFlag::ExposeToModeler, false );
      return flags;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "str" )
             << QStringLiteral( "QgsProperty" );
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Path for new file" );
    }
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
class CORE_EXPORT QgsProcessingParameterTypeFolderDestination : public QgsProcessingParameterType
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
      return QStringLiteral( "folderDestination" );
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterFolderDestination" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterFolderDestination" );
    }

    ParameterFlags flags() const override
    {
      ParameterFlags flags = QgsProcessingParameterType::flags();
      flags.setFlag( ParameterFlag::ExposeToModeler, false );
      return flags;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "str" )
             << QStringLiteral( "QgsProperty" );
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Path for an existing or new folder" );
    }
};

/**
 * \brief A raster layer destination parameter, for specifying the destination path for a raster layer
 * created by the algorithm.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('rasterDestination')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeRasterDestination : public QgsProcessingParameterType
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
      return QStringLiteral( "rasterDestination" );
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterRasterDestination" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterRasterDestination" );
    }

    ParameterFlags flags() const override
    {
      ParameterFlags flags = QgsProcessingParameterType::flags();
      flags.setFlag( ParameterFlag::ExposeToModeler, false );
      return flags;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "str" )
             << QStringLiteral( "QgsProperty" )
             << QStringLiteral( "QgsProcessingOutputLayerDefinition" );
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Path for new raster layer" );
    }
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
      return QStringLiteral( "string" );
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterString" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterString" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "str" )
             << QStringLiteral( "QgsProperty" );
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "String value" );
    }
};

/**
 * \brief A authentication configuration parameter for processing algorithms.
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
      return QCoreApplication::translate( "Processing", "A authentication configuration parameter." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Authentication Configuration" );
    }

    QString id() const override
    {
      return QStringLiteral( "authcfg" );
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterAuthConfig" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterAuthConfig" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "str" );
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "An existing QGIS authentication ID string" );
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
      return QStringLiteral( "multilayer" );
    }

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
    }
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
      return QStringLiteral( "source" );
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterFeatureSource" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterFeatureSource" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: layer ID" )
             << QObject::tr( "str: layer name" )
             << QObject::tr( "str: layer source" )
             << QStringLiteral( "QgsProcessingFeatureSourceDefinition" )
             << QStringLiteral( "QgsProperty" )
             << QStringLiteral( "QgsVectorLayer" );
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Path to a vector layer" );
    }
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
      return QStringLiteral( "number" );
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterNumber" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterNumber" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "int" )
             << QStringLiteral( "float" )
             << QStringLiteral( "QgsProperty" );
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "A numeric value" );
    }
};

/**
 * \brief A distance parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('distance')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeDistance : public QgsProcessingParameterType
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
      return QStringLiteral( "distance" );
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterDistance" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterDistance" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "int" )
             << QStringLiteral( "float" )
             << QStringLiteral( "QgsProperty" );
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "A numeric value" );
    }
};

/**
 * \brief A duration parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('duration')
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsProcessingParameterTypeDuration : public QgsProcessingParameterType
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
      return QStringLiteral( "duration" );
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterDuration" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterDuration" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "int" )
             << QStringLiteral( "float" )
             << QStringLiteral( "QgsProperty" );
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "A numeric value (unit type set by algorithms)" );
    }
};

/**
 * \brief A scale parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('scale')
 * \since QGIS 3.8
 */
class CORE_EXPORT QgsProcessingParameterTypeScale : public QgsProcessingParameterType
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
      return QStringLiteral( "scale" );
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterScale" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterScale" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "int: scale denominator" )
             << QStringLiteral( "float: scale denominator" )
             << QStringLiteral( "QgsProperty" );
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "A numeric value representing the scale denominator" );
    }
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
      return QStringLiteral( "band" );
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterBand" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterBand" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "int" )
             << QStringLiteral( "QgsProperty" );
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Integer value representing an existing raster band number" );
    }
};

/**
 * \brief A feature sink parameter for Processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('band')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeFeatureSink : public QgsProcessingParameterType
{
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterFeatureSink( name );
    }

    ParameterFlags flags() const override
    {
      ParameterFlags flags = QgsProcessingParameterType::flags();
      flags.setFlag( ParameterFlag::ExposeToModeler, false );
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
      return QStringLiteral( "sink" );
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterFeatureSink" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterFeatureSink" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: destination vector file, e.g. 'd:/test.shp'" )
             << QObject::tr( "str: 'memory:' to store result in temporary memory layer" )
             << QObject::tr( "str: using vector provider ID prefix and destination URI, e.g. 'postgres:…' to store result in PostGIS table" )
             << QStringLiteral( "QgsProcessingOutputLayerDefinition" )
             << QStringLiteral( "QgsProperty" );
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Path for new vector layer" );
    }
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
      return QStringLiteral( "layout" );
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterLayout" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterLayout" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: name of print layout in current project" )
             << QStringLiteral( "QgsProperty" );
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Name of print layout in current project" );
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
      return QStringLiteral( "layoutitem" );
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterLayoutItem" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterLayoutItem" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: UUID of print layout item" )
             << QObject::tr( "str: id of print layout item" )
             << QStringLiteral( "QgsProperty" );
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "UUID or item id of layout item" );
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
      return QStringLiteral( "color" );
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterColor" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterColor" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: string representation of color, e.g #ff0000 or rgba(200,100,50,0.8)" )
             << QStringLiteral( "QColor" )
             << QStringLiteral( "QgsProperty" );
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "String representation of color, e.g #ff0000 or rgba(200,100,50,0.8)" );
    }
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
      return QStringLiteral( "coordinateoperation" );
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterCoordinateOperation" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterCoordinateOperation" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: string representation of a Proj coordinate operation" );
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "String representation of Proj coordinate operation" );
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
      return QStringLiteral( "maptheme" );
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterMapTheme" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterMapTheme" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: name of an existing map theme" )
             << QStringLiteral( "QgsProperty" );
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Name of an existing map theme" );
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
      return QStringLiteral( "datetime" );
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterDateTime" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterDateTime" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "str" )
             << QStringLiteral( "QDateTime" )
             << QStringLiteral( "QDate" )
             << QStringLiteral( "QTime" )
             << QStringLiteral( "QgsProperty" );
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "A datetime value in ISO format" );
    }
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
      return QStringLiteral( "providerconnection" );
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterProviderConnection" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterProviderConnection" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "str" )
             << QStringLiteral( "QgsProperty" );
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Name of registered database connection" );
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
      return QStringLiteral( "databaseschema" );
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterDatabaseSchema" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterDatabaseSchema" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "str" )
             << QStringLiteral( "QgsProperty" );
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Name of existing database schema" );
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
      return QStringLiteral( "databasetable" );
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterDatabaseTable" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterDatabaseTable" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "str" )
             << QStringLiteral( "QgsProperty" );
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Name of existing database table" );
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
      return QStringLiteral( "from qgis.core import QgsProcessingParameterPointCloudLayer" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterPointCloudLayer" );
    }

    QString id() const override
    {
      return QStringLiteral( "pointcloud" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: layer ID" )
             << QObject::tr( "str: layer name" )
             << QObject::tr( "str: layer source" )
             << QStringLiteral( "QgsPointCloudLayer" );
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Path to a point cloud layer" );
    }
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
      return QStringLiteral( "from qgis.core import QgsProcessingParameterAnnotationLayer" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterAnnotationLayer" );
    }

    QString id() const override
    {
      return QStringLiteral( "annotation" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: layer ID" )
             << QObject::tr( "str: layer name" )
             << QObject::tr( "\"main\": main annotation layer for a project" )
             << QStringLiteral( "QgsAnnotationLayer" );
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Layer ID for an annotation layer, or \"main\" for the main annotation layer in a project." );
    }
};

/**
 * \brief A pointcloud layer destination parameter, for specifying the destination path for a point cloud layer
 * created by the algorithm.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('pointCloudDestination')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypePointCloudDestination : public QgsProcessingParameterType
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
      return QStringLiteral( "pointCloudDestination" );
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterPointCloudDestination" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterPointCloudDestination" );
    }

    ParameterFlags flags() const override
    {
      ParameterFlags flags = QgsProcessingParameterType::flags();
      flags.setFlag( ParameterFlag::ExposeToModeler, false );
      return flags;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "str" )
             << QStringLiteral( "QgsProperty" )
             << QStringLiteral( "QgsProcessingOutputLayerDefinition" );
    }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Path for new point cloud layer" );
    }
};

#endif // QGSPROCESSINGPARAMETERTYPEIMPL_H
