/***************************************************************************
                         qgsprocessing.h
                         ---------------
    begin                : July 2017
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

#ifndef QGSPROCESSING_H
#define QGSPROCESSING_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingstree.h"
#include <QString>

//
// Output definitions
//

/**
 * \class QgsProcessing
 * \ingroup core
 *
 * \brief Contains enumerations and other constants for use in processing algorithms
 * and parameters.
 *
 */

class CORE_EXPORT QgsProcessing
{
    Q_GADGET

  public:

    //! Available Python output types
    enum class PythonOutputType SIP_MONKEYPATCH_SCOPEENUM
    {
      PythonQgsProcessingAlgorithmSubclass, //!< Full Python QgsProcessingAlgorithm subclass
    };
    Q_ENUM( PythonOutputType )

    /**
     * Layer options flags
     *
     * \since QGIS 3.32
     */
    enum class LayerOptionsFlag : int SIP_ENUM_BASETYPE( IntFlag )
    {
      SkipIndexGeneration = 1 << 0, //!< Do not generate index when creating a layer. Makes sense only for point cloud layers
    };
    Q_ENUM( LayerOptionsFlag )
    Q_DECLARE_FLAGS( LayerOptionsFlags, LayerOptionsFlag )
    Q_FLAG( LayerOptionsFlags )

    /**
     * Converts a source \a type to a string representation.
     *
     * \since QGIS 3.6
     */
    static QString sourceTypeToString( Qgis::ProcessingSourceType type )
    {
      switch ( type )
      {
        case Qgis::ProcessingSourceType::MapLayer:
          return QStringLiteral( "TypeMapLayer" );
        case Qgis::ProcessingSourceType::VectorAnyGeometry:
          return QStringLiteral( "TypeVectorAnyGeometry" );
        case Qgis::ProcessingSourceType::VectorPoint:
          return QStringLiteral( "TypeVectorPoint" );
        case Qgis::ProcessingSourceType::VectorLine:
          return QStringLiteral( "TypeVectorLine" );
        case Qgis::ProcessingSourceType::VectorPolygon:
          return QStringLiteral( "TypeVectorPolygon" );
        case Qgis::ProcessingSourceType::Raster:
          return QStringLiteral( "TypeRaster" );
        case Qgis::ProcessingSourceType::File:
          return QStringLiteral( "TypeFile" );
        case Qgis::ProcessingSourceType::Vector:
          return QStringLiteral( "TypeVector" );
        case Qgis::ProcessingSourceType::Mesh:
          return QStringLiteral( "TypeMesh" );
        case Qgis::ProcessingSourceType::Plugin:
          return QStringLiteral( "TypePlugin" );
        case Qgis::ProcessingSourceType::PointCloud:
          return QStringLiteral( "TypePointCloud" );
        case Qgis::ProcessingSourceType::Annotation:
          return QStringLiteral( "TypeAnnotation" );
        case Qgis::ProcessingSourceType::VectorTile:
          return QStringLiteral( "TypeVectorTile" );
      }
      return QString();
    }

    /**
     * Converts a documentation \a flag to a translated string.
     *
     * \since QGIS 3.40
     */
    static QString documentationFlagToString( Qgis::ProcessingAlgorithmDocumentationFlag flag );

    /**
     * Constant used to indicate that a Processing algorithm output should be a temporary layer/file.
     *
     * \since QGIS 3.6
     */
    static const QString TEMPORARY_OUTPUT;

#ifndef SIP_RUN
    static inline QgsSettingsTreeNode *sTreeConfiguration = QgsSettingsTree::sTreeQgis->createChildNode( QStringLiteral( "configuration" ) );

    //! Settings entry prefer filename as layer name
    static const QgsSettingsEntryBool *settingsPreferFilenameAsLayerName;
    //! Settings entry temp path
    static const QgsSettingsEntryString *settingsTempPath;
    //! Settings entry default output vector layer ext
    static const QgsSettingsEntryString *settingsDefaultOutputVectorLayerExt;
    //! Settings entry default output raster layer ext
    static const QgsSettingsEntryString *settingsDefaultOutputRasterLayerExt;
#endif
};

#endif // QGSPROCESSING_H
