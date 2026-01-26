/***************************************************************************
                         qgsalgorithmconditionalbranch.h
                         ---------------------
    begin                : March 2020
    copyright            : (C) 2020 by Nyall Dawson
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

#ifndef QGSALGORITHMCONDITIONALBRANCH_H
#define QGSALGORITHMCONDITIONALBRANCH_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

class QgsProcessingModelAlgorithm;
class QTableWidget;

///@cond PRIVATE

/**
 * Feature filter algorithm for modeler.
 * Accepts a list of expressions and names and creates outputs where
 * matching features are sent to.
 *
 * \since QGIS 3.2
 */
class QgsConditionalBranchAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsConditionalBranchAlgorithm() = default;
    ~QgsConditionalBranchAlgorithm() override;

    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    Qgis::ProcessingAlgorithmFlags flags() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QgsConditionalBranchAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    struct Output
    {
        Output( const QString &name, const QString &expression )
          : name( name )
          , expression( expression )
        {}
        QString name;
        QgsExpression expression;
    };

    QList<Output *> mOutputs;
};

///@endcond PRIVATE

#endif // QGSALGORITHMCONDITIONALBRANCH_H
