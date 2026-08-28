/***************************************************************************
  qgsprocessingparameterinterpolationpixelsize.h
  ---------------------
  Date                 : August 2026
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

#ifndef QGSPROCESSINGPARAMETERINTERPOLATIONPIXELSIZE_H
#define QGSPROCESSINGPARAMETERINTERPOLATIONPIXELSIZE_H

#include "qgsprocessingoutputs.h"
#include "qgsprocessingparameters.h"
#include "qgsprocessingparametertype.h"

#include <QString>

using namespace Qt::StringLiterals;


/**
 * \brief A parameter for the interpolation algorithm pixel size parameter.
 *
 * \ingroup core
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsProcessingParameterInterpolationPixelSize : public QgsProcessingParameterNumber
{
  public:
    //! Constructor for QgsProcessingParameterInterpolationPixelSize.
    QgsProcessingParameterInterpolationPixelSize(
      const QString &name,
      const QString &description = QString(),
      const QString &interpolationSourceParameter = QString(),
      const QString &extentParameter = QString(),
      const QVariant &defaultValue = QVariant()
    );

    QgsProcessingParameterInterpolationPixelSize *clone() const override;
    QString type() const override;
    //! Returns the type name for the parameter class.
    static QString typeName() { return u"interpolationpixelsize"_s; } // cppcheck-suppress duplInheritedMember
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass ) const override;

    /**
     * Returns the linked interpolation source parameter name.
     */
    QString interpolationSourceParameter() const { return mInterpolationSourceParam; }

    /**
     * Returns the linked extent parameter name.
     */
    QString extentParameter() const { return mExtentParam; }

  private:
    QString mInterpolationSourceParam;
    QString mExtentParam;
};

#ifndef SIP_RUN
///@cond PRIVATE

/**
 * \brief Parameter type definition for QgsProcessingParameterInterpolationPixelSize.
 *
 * \ingroup core
 * \note This class is not a part of public API.
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsProcessingParameterTypeInterpolationPixelSize : public QgsProcessingParameterType
{
  public:
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY { return new QgsProcessingParameterInterpolationPixelSize( name ); }

    QString description() const override { return QCoreApplication::translate( "Processing", "An input allowing setting the pixel size for interpolation algorithms." ); }

    QString name() const override { return QCoreApplication::translate( "Processing", "Interpolation Pixel Size" ); }

    QString id() const override { return QgsProcessingParameterInterpolationPixelSize::typeName(); }

    QString pythonImportString() const override { return u"from qgis.core import QgsProcessingParameterInterpolationPixelSize"_s; }

    QString className() const override { return u"QgsProcessingParameterInterpolationPixelSize"_s; }

    QStringList acceptedPythonTypes() const override { return QStringList() << u"int"_s << u"float"_s; }

    QStringList acceptedStringValues() const override { return QStringList() << QObject::tr( "A numeric value" ); }

    QStringList acceptedParameterTypes() const override
    {
      return QStringList()
             << QgsProcessingParameterInterpolationPixelSize::typeName()
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

#endif // QGSPROCESSINGPARAMETERINTERPOLATIONPIXELSIZE_H
