/***************************************************************************
                         qgsactivelayerfeatureslocatorfilters.h
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

#ifndef QGSACTIVELAYERFEATURESLOCATORFILTERS_H
#define QGSACTIVELAYERFEATURESLOCATORFILTERS_H

#include "qgis_app.h"
#include "qgslocatorfilter.h"
#include "qgsexpression.h"
#include "qgsfeatureiterator.h"


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

    QStringList prepare( const QString &string, const QgsLocatorContext &context ) override;
    void fetchResults( const QString &string, const QgsLocatorContext &context, QgsFeedback *feedback ) override;
    void triggerResult( const QgsLocatorResult &result ) override;
    void triggerResultFromAction( const QgsLocatorResult &result, const int actionId ) override;
    bool hasConfigWidget() const override { return true; }
    void openConfigWidget( QWidget *parent ) override;

    enum class ResultType
    {
      Feature,
      FieldRestriction,
    };
    Q_ENUM( ResultType )

  private:
    enum ContextMenuEntry
    {
      NoEntry,
      OpenForm
    };

    /**
     * Returns the field restriction if defined (starting with @)
     * The \a searchString is modified accordingly by removing the field restriction
     */
    static QString fieldRestriction( QString &searchString, bool *isRestricting = nullptr );

    QgsExpression mDispExpression;
    QgsExpressionContext mContext;
    QgsFeatureIterator mDisplayTitleIterator;
    QgsFeatureIterator mFieldIterator;
    QString mLayerId;
    bool mLayerIsSpatial = false;
    QIcon mLayerIcon;
    QStringList mAttributeAliases;
    QStringList mFieldsCompletion;
    int mMaxTotalResults = 30;

    friend class TestQgsAppLocatorFilters;
};


#endif // QGSACTIVELAYERFEATURESLOCATORFILTERS_H
