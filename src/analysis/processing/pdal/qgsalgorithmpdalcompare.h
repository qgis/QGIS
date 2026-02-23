/***************************************************************************
                         qgsalgorithmpdalcompare.h
                         ---------------------
    begin                : January 2026
    copyright            : (C) 2026 by Jan Caha
    email                : jan.caha at outlook dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMPDALCOMPARE_H
#define QGSALGORITHMPDALCOMPARE_H


#include "qgis_sip.h"
#include "qgspdalalgorithmbase.h"

#include <QString>

#define SIP_NO_FILE

using namespace Qt::StringLiterals;

///@cond PRIVATE

/**
 * Native point cloud compare algorithm.
 */
class QgsPdalCompareAlgorithm : public QgsPdalAlgorithmBase
{
  public:
    QgsPdalCompareAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QString group() const override;
    QString groupId() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QgsPdalCompareAlgorithm *createInstance() const override SIP_FACTORY;
    bool checkParameterValues( const QVariantMap &parameters, QgsProcessingContext &context, QString *message ) const override;

    QStringList createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    QStringList mCylinderOrientationOptions = { u"up"_s, u"origin"_s, u"none"_s };

    friend class TestQgsProcessingPdalAlgs;
};

///@endcond PRIVATE

#endif // QGSALGORITHMPDALCOMPARE_H
