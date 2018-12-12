/***************************************************************************
                         qgsinbuiltlocatorfilters.h
                         --------------------------
    begin                : May 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#ifndef QGSINBUILTLOCATORFILTERS_H
#define QGSINBUILTLOCATORFILTERS_H

#include "qgis_app.h"
#include "qgslocatorfilter.h"
#include "qgsexpressioncontext.h"
#include "qgsfeatureiterator.h"
#include "qgsvectorlayerfeatureiterator.h"


class QAction;

class APP_EXPORT QgsLayerTreeLocatorFilter : public QgsLocatorFilter
{
    Q_OBJECT

  public:

    QgsLayerTreeLocatorFilter( QObject *parent = nullptr );
    QgsLayerTreeLocatorFilter *clone() const override;
    QString name() const override { return QStringLiteral( "layertree" ); }
    QString displayName() const override { return tr( "Project Layers" ); }
    Priority priority() const override { return Highest; }
    QString prefix() const override { return QStringLiteral( "l" ); }
    QgsLocatorFilter::Flags flags() const override { return QgsLocatorFilter::FlagFast; }

    void fetchResults( const QString &string, const QgsLocatorContext &context, QgsFeedback *feedback ) override;
    void triggerResult( const QgsLocatorResult &result ) override;

};

class APP_EXPORT QgsLayoutLocatorFilter : public QgsLocatorFilter
{
    Q_OBJECT

  public:

    QgsLayoutLocatorFilter( QObject *parent = nullptr );
    QgsLayoutLocatorFilter *clone() const override;
    QString name() const override { return QStringLiteral( "layouts" ); }
    QString displayName() const override { return tr( "Project Layouts" ); }
    Priority priority() const override { return Highest; }
    QString prefix() const override { return QStringLiteral( "pl" ); }
    QgsLocatorFilter::Flags flags() const override { return QgsLocatorFilter::FlagFast; }

    void fetchResults( const QString &string, const QgsLocatorContext &context, QgsFeedback *feedback ) override;
    void triggerResult( const QgsLocatorResult &result ) override;

};

class APP_EXPORT QgsActionLocatorFilter : public QgsLocatorFilter
{
    Q_OBJECT

  public:

    QgsActionLocatorFilter( const QList<QWidget *> &parentObjectsForActions, QObject *parent = nullptr );
    QgsActionLocatorFilter *clone() const override;
    QString name() const override { return QStringLiteral( "actions" ); }
    QString displayName() const override { return tr( "Actions" ); }
    Priority priority() const override { return Lowest; }
    QString prefix() const override { return QStringLiteral( "." ); }
    QgsLocatorFilter::Flags flags() const override { return QgsLocatorFilter::FlagFast; }

    void fetchResults( const QString &string, const QgsLocatorContext &context, QgsFeedback *feedback ) override;
    void triggerResult( const QgsLocatorResult &result ) override;
  private:

    QList< QWidget * > mActionParents;

    void searchActions( const QString &string, QWidget *parent, QList< QAction *> &found );

};

class APP_EXPORT QgsActiveLayerFeaturesLocatorFilter : public QgsLocatorFilter
{
    Q_OBJECT

  public:

    QgsActiveLayerFeaturesLocatorFilter( QObject *parent = nullptr );
    QgsActiveLayerFeaturesLocatorFilter *clone() const override;
    QString name() const override { return QStringLiteral( "features" ); }
    QString displayName() const override { return tr( "Active Layer Features" ); }
    Priority priority() const override { return Medium; }
    QString prefix() const override { return QStringLiteral( "f" ); }

    void prepare( const QString &string, const QgsLocatorContext &context ) override;
    void fetchResults( const QString &string, const QgsLocatorContext &context, QgsFeedback *feedback ) override;
    void triggerResult( const QgsLocatorResult &result ) override;

  private:

    QgsExpression mDispExpression;
    QgsExpressionContext mContext;
    QgsFeatureIterator mIterator;
    QString mLayerId;
    QIcon mLayerIcon;
    QStringList mAttributeAliases;
};

class APP_EXPORT QgsAllLayersFeaturesLocatorFilter : public QgsLocatorFilter
{
    Q_OBJECT

  public:
    enum ContextMenuEntry
    {
      NoEntry,
      OpenForm
    };

    struct PreparedLayer
    {
      public:
        QgsExpression expression;
        QgsExpressionContext context;
        std::unique_ptr<QgsVectorLayerFeatureSource> featureSource;
        QgsFeatureRequest request;
        QString layerName;
        QString layerId;
        QIcon layerIcon;
    };

    QgsAllLayersFeaturesLocatorFilter( QObject *parent = nullptr );
    QgsAllLayersFeaturesLocatorFilter *clone() const override;
    QString name() const override { return QStringLiteral( "allfeatures" ); }
    QString displayName() const override { return tr( "Features In All Layers" ); }
    Priority priority() const override { return Medium; }
    QString prefix() const override { return QStringLiteral( "af" ); }

    void prepare( const QString &string, const QgsLocatorContext &context ) override;
    void fetchResults( const QString &string, const QgsLocatorContext &context, QgsFeedback *feedback ) override;
    void triggerResult( const QgsLocatorResult &result ) override;
    void triggerResultFromAction( const QgsLocatorResult &result, const int actionId ) override;

  private:
    int mMaxResultsPerLayer = 6;
    int mMaxTotalResults = 12;
    QList<std::shared_ptr<PreparedLayer>> mPreparedLayers;


};

class APP_EXPORT QgsExpressionCalculatorLocatorFilter : public QgsLocatorFilter
{
    Q_OBJECT

  public:

    QgsExpressionCalculatorLocatorFilter( QObject *parent = nullptr );
    QgsExpressionCalculatorLocatorFilter *clone() const override;
    QString name() const override { return QStringLiteral( "calculator" ); }
    QString displayName() const override { return tr( "Calculator" ); }
    Priority priority() const override { return Highest; }
    QString prefix() const override { return QStringLiteral( "=" ); }
    QgsLocatorFilter::Flags flags() const override { return QgsLocatorFilter::FlagFast; }

    void fetchResults( const QString &string, const QgsLocatorContext &context, QgsFeedback *feedback ) override;
    void triggerResult( const QgsLocatorResult &result ) override;
};


class APP_EXPORT QgsBookmarkLocatorFilter : public QgsLocatorFilter
{
    Q_OBJECT

  public:

    QgsBookmarkLocatorFilter( QObject *parent = nullptr );
    QgsBookmarkLocatorFilter *clone() const override;
    QString name() const override { return QStringLiteral( "bookmarks" ); }
    QString displayName() const override { return tr( "Spatial Bookmarks" ); }
    Priority priority() const override { return Highest; }
    QString prefix() const override { return QStringLiteral( "b" ); }
    QgsLocatorFilter::Flags flags() const override { return QgsLocatorFilter::FlagFast; }

    void fetchResults( const QString &string, const QgsLocatorContext &context, QgsFeedback *feedback ) override;
    void triggerResult( const QgsLocatorResult &result ) override;
};

class APP_EXPORT QgsSettingsLocatorFilter : public QgsLocatorFilter
{
    Q_OBJECT

  public:

    QgsSettingsLocatorFilter( QObject *parent = nullptr );
    QgsSettingsLocatorFilter *clone() const override;
    QString name() const override { return QStringLiteral( "optionpages" ); }
    QString displayName() const override { return tr( "Settings" ); }
    Priority priority() const override { return Highest; }
    QString prefix() const override { return QStringLiteral( "set" ); }
    QgsLocatorFilter::Flags flags() const override { return QgsLocatorFilter::FlagFast; }

    void fetchResults( const QString &string, const QgsLocatorContext &context, QgsFeedback *feedback ) override;
    void triggerResult( const QgsLocatorResult &result ) override;

  private:

    QMap<QString, QString> settingsPage( const QString &type,  const QString &page );
};

#endif // QGSINBUILTLOCATORFILTERS_H


