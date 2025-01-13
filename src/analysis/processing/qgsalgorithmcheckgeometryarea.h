/***************************************************************************
                        qgsalgorithmcheckgeometryarea.h
                        ---------------------
   begin                : November 2023
   copyright            : (C) 2023 by Loïc Bartoletti
   email                : loic dot bartoletti at oslandia dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMCHECKGEOMETRYAREA_H
#define QGSALGORITHMCHECKGEOMETRYAREA_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

class QgsGeometryCheckAreaAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsGeometryCheckAreaAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    Qgis::ProcessingAlgorithmFlags flags() const override;
    QgsGeometryCheckAreaAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    int mTolerance { 8 };
    static QgsFields outputFields();
};

///@endcond PRIVATE

#endif // QGSALGORITHMCHECKGEOMETRYAREA_H
