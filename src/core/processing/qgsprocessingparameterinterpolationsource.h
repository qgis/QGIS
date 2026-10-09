/***************************************************************************
  qgsprocessingparameterinterpolationsource.h
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

#ifndef QGSPROCESSINGPARAMETERINTERPOLATIONSOURCE_H
#define QGSPROCESSINGPARAMETERINTERPOLATIONSOURCE_H

#include "qgsprocessingoutputs.h"
#include "qgsprocessingparameters.h"
#include "qgsprocessingparametertype.h"

#include <QString>

using namespace Qt::StringLiterals;


/**
 * \brief A parameter for specifying interpolation source layers and their properties.
 *
 * \ingroup core
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsProcessingParameterInterpolationSource : public QgsProcessingParameterDefinition
{
  public:
    //! Constructor for QgsProcessingParameterInterpolationSource.
    QgsProcessingParameterInterpolationSource( const QString &name, const QString &description = QString() );

    QgsProcessingParameterInterpolationSource *clone() const override;
    QString type() const override;
    //! Returns the type name for the parameter class.
    static QString typeName() { return u"interpolationdata"_s; } // cppcheck-suppress duplInheritedMember
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass ) const override;
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
};

#ifndef SIP_RUN
///@cond PRIVATE

/**
 * \brief Parameter type definition for QgsProcessingParameterInterpolationSource.
 *
 * \ingroup core
 * \note This class is not a part of public API.
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsProcessingParameterTypeInterpolationSource : public QgsProcessingParameterType
{
  public:
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY { return new QgsProcessingParameterInterpolationSource( name ); }

    QString description() const override { return QCoreApplication::translate( "Processing", "An input for specifying interpolation source layers and their properties." ); }

    QString name() const override { return QCoreApplication::translate( "Processing", "Interpolation Sources" ); }

    QString id() const override { return QgsProcessingParameterInterpolationSource::typeName(); }

    QString pythonImportString() const override { return u"from qgis.core import QgsProcessingParameterInterpolationSource"_s; }

    QString className() const override { return u"QgsProcessingParameterInterpolationSource"_s; }

    QStringList acceptedPythonTypes() const override { return QStringList() << u"str"_s; }

    QStringList acceptedStringValues() const override { return QStringList() << QObject::tr( "A string value representing encoded interpolation parameter format" ); }

    QStringList acceptedParameterTypes() const override { return QStringList() << QgsProcessingParameterInterpolationSource::typeName() << QgsProcessingParameterString::typeName(); }

    QStringList acceptedOutputTypes() const override { return QStringList() << QgsProcessingOutputVariant::typeName() << QgsProcessingOutputString::typeName(); }
};

///@endcond
#endif

#endif // QGSPROCESSINGPARAMETERINTERPOLATIONSOURCE_H
