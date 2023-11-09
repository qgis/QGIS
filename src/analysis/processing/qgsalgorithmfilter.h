/***************************************************************************
                         qgsalgorithmfilter.h
                         ---------------------
    begin                : April 2018
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

#ifndef QGSFILTERALGORITHM_H
#define QGSFILTERALGORITHM_H

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
class QgsFilterAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsFilterAlgorithm() = default;
    ~QgsFilterAlgorithm() override;

    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    Flags flags() const override;
    QString shortHelpString() const override;
    QgsFilterAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    QVariantMap processAlgorithm( const QVariantMap &parameters,
                                  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    struct Output
    {
      Output( const QString &name, const QString &expression )
        : name( name )
        , expression( expression )
      {}
      QString name;
      QgsExpression expression;
      std::unique_ptr< QgsFeatureSink > sink;
      QString destinationIdentifier;
    };

    QList<Output *> mOutputs;
};

///@endcond PRIVATE

#endif // QGSFILTERALGORITHM_H
