/***************************************************************************
                         qgsprojectstylealgorithms.h
                         ---------------------
    begin                : July 2019
    copyright            : (C) 2019 by Nyall Dawson
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

#ifndef QGSPROJECTSTYLESALGORITHMS_H
#define QGSPROJECTSTYLESALGORITHMS_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgis_analysis.h"
#include "qgsprocessingalgorithm.h"
#include "qgsstyle.h"
#include "qgsstyleentityvisitor.h"

///@cond PRIVATE

/**
 * \class QgsSaveToStyleVisitor
 * \ingroup analysis
 * Style entity visitor which saves encountered style entities to a QgsStyle database.
 * \since QGIS 3.10
 */
class ANALYSIS_EXPORT QgsSaveToStyleVisitor : public QgsStyleEntityVisitorInterface
{
  public:

    /**
     * Constructor for QgsSaveToStyleVisitor, saving encountered objects to the specified
     * \a style database (which must already be initialized). The \a objects list
     * dictates which entity types should be saved to the style.
     *
     * An empty \a objects list indicates all entity types should be saved.
     */
    QgsSaveToStyleVisitor( QgsStyle *style, const QList< QgsStyle::StyleEntity > &objects = QList< QgsStyle::StyleEntity >() );

    bool visit( const QgsStyleEntityVisitorInterface::StyleLeaf &entity ) override;
    bool visitEnter( const QgsStyleEntityVisitorInterface::Node &node ) override;
    bool visitExit( const QgsStyleEntityVisitorInterface::Node &node ) override;

  private:

    QgsStyle *mStyle = nullptr;
    QList< QgsStyle::StyleEntity > mObjects;
    QStringList mParentNames;

};

/**
 * Native create style from project algorithm
 */
class QgsStyleFromProjectAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsStyleFromProjectAlgorithm();
    ~QgsStyleFromProjectAlgorithm() override;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QgsStyleFromProjectAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters,
                                  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:

    QString mProjectPath;
    QList< QgsStyle::StyleEntity > mObjects;
    std::unique_ptr< QgsStyle > mStyle;
};

///@endcond PRIVATE

#endif // QGSPROJECTSTYLESALGORITHMS_H


