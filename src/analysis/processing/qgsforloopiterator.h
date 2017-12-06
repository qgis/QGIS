/***************************************************************************
                         qgsforloopiterator.h
                         ---------------------
    begin                : December 2017
    copyright            : (C) 2017 by Arnaud Morvan
    email                : arnaud dot morvan at camptocamp dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFORLOOPITERATOR_H
#define QGSFORLOOPITERATOR_H

#include "qgis.h"
#include "qgsprocessingmodeliterator.h"

///@cond PRIVATE

/**
 * Native for loop iterator.
 */
class QgsForLoopIterator : public QgsProcessingModelIterator
{

  public:

    QgsForLoopIterator() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    virtual QStringList tags() const override;
    QString group() const override;
    QString shortHelpString() const override;
    QgsForLoopIterator *createInstance() const override SIP_FACTORY;
    bool next();

  protected:

    bool prepareAlgorithm( const QVariantMap &parameters,
                           QgsProcessingContext &context,
                           QgsProcessingFeedback *feedback ) override;
    virtual QVariantMap processAlgorithm( const QVariantMap &parameters,
                                          QgsProcessingContext &context,
                                          QgsProcessingFeedback *feedback ) override;

  private:

    long long mCurrentValue;
    long long mStep;
    long long mTo;
};

///@endcond PRIVATE


#endif // QGSFORLOOPITERATOR_H
