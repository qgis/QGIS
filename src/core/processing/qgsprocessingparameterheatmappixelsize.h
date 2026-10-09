/***************************************************************************
  qgsprocessingparameterheatmappixelsize.h
  ---------------------
  Date                 : June 2026
  Copyright            : (C) 2026 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGPARAMETERHEATMAPPIXELSIZE_H
#define QGSPROCESSINGPARAMETERHEATMAPPIXELSIZE_H

#include "qgsprocessingoutputs.h"
#include "qgsprocessingparameters.h"
#include "qgsprocessingparametertype.h"

#include <QString>

using namespace Qt::StringLiterals;


/**
 * \brief A parameter for the heatmap algorithm pixel size parameter.
 *
 * \ingroup core
 * \since QGIS 4.2
 */
class CORE_EXPORT QgsProcessingParameterHeatmapPixelSize : public QgsProcessingParameterNumber
{
  public:
    //! Constructor for QgsProcessingParameterHeatmapPixelSize.
    QgsProcessingParameterHeatmapPixelSize(
      const QString &name,
      const QString &description = QString(),
      const QString &parentLayerParameter = QString(),
      const QString &radiusParameter = QString(),
      const QString &radiusFieldParameter = QString(),
      const QVariant &defaultValue = QVariant()
    );

    QgsProcessingParameterHeatmapPixelSize *clone() const override;
    QString type() const override;
    //! Returns the type name for the parameter class.
    static QString typeName() { return u"heatmappixelsize"_s; } // cppcheck-suppress duplInheritedMember
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass ) const override;

    /**
     * Returns the linked parent layer parameter name.
     */
    QString parentLayerParameter() const { return mParentLayer; }

    /**
     * Returns the linked radius parameter name.
     */
    QString radiusParameter() const { return mRadiusParam; }

    /**
     * Returns the radius field parameter name.
     */
    QString radiusFieldParameter() const { return mRadiusFieldParam; }

  private:
    QString mParentLayer;
    QString mRadiusParam;
    QString mRadiusFieldParam;
};

#ifndef SIP_RUN
///@cond PRIVATE

/**
 * \brief Parameter type definition for QgsProcessingParameterHeatmapPixelSize.
 *
 * \ingroup core
 * \note This class is not a part of public API.
 * \since QGIS 4.2
 */
class CORE_EXPORT QgsProcessingParameterTypeHeatmapPixelSize : public QgsProcessingParameterType
{
  public:
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY { return new QgsProcessingParameterHeatmapPixelSize( name ); }

    QString description() const override { return QCoreApplication::translate( "Processing", "An input allowing setting the pixel size for heatmap algorithms." ); }

    QString name() const override { return QCoreApplication::translate( "Processing", "Heatmap Pixel Size" ); }

    QString id() const override { return QgsProcessingParameterHeatmapPixelSize::typeName(); }

    QString pythonImportString() const override { return u"from qgis.core import QgsProcessingParameterHeatmapPixelSize"_s; }

    QString className() const override { return u"QgsProcessingParameterHeatmapPixelSize"_s; }

    QStringList acceptedPythonTypes() const override { return QStringList() << u"int"_s << u"float"_s; }

    QStringList acceptedStringValues() const override { return QStringList() << QObject::tr( "A numeric value" ); }

    QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterHeatmapPixelSize::typeName()
             << QgsProcessingParameterString::typeName()
             << QgsProcessingParameterNumber::typeName()
             << QgsProcessingParameterDistance::typeName()
             << QgsProcessingParameterVolume::typeName()
             << QgsProcessingParameterDuration::typeName()
             << QgsProcessingParameterScale::typeName();
    }

    QStringList acceptedOutputTypes() const override
    {
      return QStringList() << QgsProcessingOutputNumber::typeName() << QgsProcessingOutputVariant::typeName() << QgsProcessingOutputString::typeName();
    }

    QColor modelColor() const override { return QColor( 34, 157, 214 ); /* blue */ };
};

///@endcond
#endif

#endif // QGSPROCESSINGPARAMETERHEATMAPPIXELSIZE_H
