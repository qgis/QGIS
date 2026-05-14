/***************************************************************************
                         qgsalgorithmselectbyattribute.h
                         ---------------------
    begin                : April 2026
    copyright            : (C) 2026 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMSELECTBYATTRIBUTE_H
#define QGSALGORITHMSELECTBYATTRIBUTE_H

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

#define SIP_NO_FILE

///@cond PRIVATE

/**
 * Native select by attribute algorithm.
 */
class QgsSelectByAttributeAlgorithm : public QgsProcessingAlgorithm
{
  public:
    enum Operation
    {
      Equals,
      NotEquals,
      GreaterThan,
      GreaterThanEqualTo,
      LessThan,
      LessThanEqualTo,
      BeginsWith,
      Contains,
      IsNull,
      IsNotNull,
      DoesNotContain,
    };

    QgsSelectByAttributeAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QgsSelectByAttributeAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * ) override;

  private:
    QString mLayerId;
};

///@endcond PRIVATE


#endif // QGSALGORITHMSELECTBYATTRIBUTE_H
