/***************************************************************************
                         qgsbookmarkalgorithms.h
                         ---------------------
    begin                : September 2019
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

#ifndef QGSBOOKMARKALGORITHMS_H
#define QGSBOOKMARKALGORITHMS_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"
#include "qgsbookmarkmanager.h"

///@cond PRIVATE

/**
 * Convert bookmarks to layer algorithm
 */
class QgsBookmarksToLayerAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsBookmarksToLayerAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QIcon icon() const override;
    QString svgIconPath() const override;
    QgsBookmarksToLayerAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    QList<QgsBookmark> mBookmarks;
};

/**
 * Convert layer to bookmarks algorithm
 */
class QgsLayerToBookmarksAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsLayerToBookmarksAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QIcon icon() const override;
    QString svgIconPath() const override;
    QgsLayerToBookmarksAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap postProcessAlgorithm( QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    QList<QgsBookmark> mBookmarks;
    int mDest = 0;
};

///@endcond PRIVATE

#endif // QGSBOOKMARKALGORITHMS_H
