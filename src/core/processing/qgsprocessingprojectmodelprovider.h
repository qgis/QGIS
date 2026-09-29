/***************************************************************************
                         qgsprocessingprojectmodelprovider.h
                         ------------------------
    begin                : August 2026
    copyright            : (C) 2026 by Nyall Dawson
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

#ifndef QGSPROCESSINGPROJECTMODELPROVIDER_H
#define QGSPROCESSINGPROJECTMODELPROVIDER_H

#include "qgis_core.h"
#include "qgsprocessingprovider.h"

#include <QDomDocument>
#include <QPointer>

class QgsProcessingModelAlgorithm;

/**
 * \brief A Processing provider based on models embedded in a QGIS project.
 * \ingroup core
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsProcessingProjectModelProvider : public QgsProcessingProvider
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsProcessingProjectModelProvider, storing models in the specified \a project.
     */
    QgsProcessingProjectModelProvider( QgsProject *project, QObject *parent SIP_TRANSFERTHIS = nullptr );

    QString name() const override;
    QString longName() const override;
    QString id() const override;
    QIcon icon() const override;
    QString svgIconPath() const override;
    bool supportsNonFileBasedOutput() const override;
    bool load() override;

    /**
     * Clears all models from the provider.
     */
    void clear();

    /**
     * Adds a \a model to the provider.
     */
    void addModel( const QgsProcessingModelAlgorithm &model );

    /**
     * Removes a \a model from the provider.
     */
    void removeModel( const QgsProcessingModelAlgorithm *model );

  public slots:

    /**
     * Reads project model definitions from a project DOM document \a document.
     */
    void readProject( const QDomDocument &document );

    /**
     * Writes project model definitions into a project DOM document \a document.
     */
    void writeProject( QDomDocument &document );

  protected:
    void loadAlgorithms() override;

  private slots:
    void onProviderAdded( const QString &providerId );

  private:
    QPointer< QgsProject > mProject;
    QMap< QString, QVariant > mModelDefinitions;
    bool mIsLoading = false;
};

#endif // QGSPROCESSINGPROJECTMODELPROVIDER_H
