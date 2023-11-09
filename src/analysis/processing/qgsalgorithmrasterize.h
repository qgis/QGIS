/***************************************************************************
  qgsalgorithmrasterize.h - QgsRasterizeAlgorithm

 ---------------------

 Original implementation in Python:

 begin                : 2016-10-05
 copyright            : (C) 2016 by OPENGIS.ch
 email                : matthias@opengis.ch

 C++ port:

 begin                : 20.11.2019
 copyright            : (C) 2019 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRASTERIZEALGORITHM_H
#define QGSRASTERIZEALGORITHM_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

class QgsRasterizeAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsRasterizeAlgorithm() = default;

    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    Flags flags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QgsRasterizeAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    QVariantMap processAlgorithm( const QVariantMap &parameters,
                                  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:

    QMap<QString, QString> mMapThemeStyleOverrides;
    std::vector<std::unique_ptr<QgsMapLayer>> mMapLayers;

};

///@endcond PRIVATE

#endif // QGSRASTERIZEALGORITHM_H
