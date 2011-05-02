/***************************************************************************
    qgsrulebasedrendererv2.h - Rule-based renderer (symbology-ng)
    ---------------------
    begin                : May 2010
    copyright            : (C) 2010 by Martin Dobias
    email                : wonder.sk at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRULEBASEDRENDERERV2_H
#define QGSRULEBASEDRENDERERV2_H

#include "qgsfield.h"
#include "qgssearchstring.h"

#include "qgsrendererv2.h"

class QgsCategorizedSymbolRendererV2;
class QgsGraduatedSymbolRendererV2;

/**
When drawing a vector layer with rule-based renderer, it goes through
the rules and draws features with symbols from rules that match.
 */
class CORE_EXPORT QgsRuleBasedRendererV2 : public QgsFeatureRendererV2
{
  public:

    /**
      This class keeps data about a rules for rule-based renderer.
      A rule consists of a symbol, filter expression and range of scales.
      If filter is empty, it matches all features.
      If scale range has both values zero, it matches all scales.
      If one of the min/max scale denominators is zero, there is no lower/upper bound for scales.
      A rule matches if both filter and scale range match.
     */
    class CORE_EXPORT Rule
    {
      public:
        //! Constructor takes ownership of the symbol
        Rule( QgsSymbolV2* symbol, int scaleMinDenom = 0, int scaleMaxDenom = 0, QString filterExp = QString(), QString label = QString(), QString description = QString() );
        Rule( const Rule& other );
        ~Rule();
        QString dump() const;
        QStringList needsFields() const;
        bool isFilterOK( const QgsFieldMap& fields, QgsFeature& f ) const;
        bool isScaleOK( double scale ) const;

        QgsSymbolV2* symbol() { return mSymbol; }
        bool dependsOnScale() const { return mScaleMinDenom != 0 || mScaleMaxDenom != 0; }
        int scaleMinDenom() const { return mScaleMinDenom; }
        int scaleMaxDenom() const { return mScaleMaxDenom; }
        QString filterExpression() const { return mFilterExp; }
        QString label() const { return mLabel; }
        QString description() const { return mDescription; }

        void setScaleMinDenom( int scaleMinDenom ) { mScaleMinDenom = scaleMinDenom; }
        void setScaleMaxDenom( int scaleMaxDenom ) { mScaleMaxDenom = scaleMaxDenom; }
        void setFilterExpression( QString filterExp ) { mFilterExp = filterExp; initFilter(); }
        void setLabel( QString label ) { mLabel = label; }
        void setDescription( QString description ) { mDescription = description; }

        Rule& operator=( const Rule& other );

      protected:

        void initFilter();

        QgsSymbolV2* mSymbol;
        int mScaleMinDenom, mScaleMaxDenom;
        QString mFilterExp, mLabel, mDescription;

        // temporary
        QgsSearchString mFilterParsed;
        QgsSearchTreeNode* mFilterTree;
    };

    /////

    static QgsFeatureRendererV2* create( QDomElement& element );

    //! Constructor. Takes ownership of the default symbol.
    QgsRuleBasedRendererV2( QgsSymbolV2* defaultSymbol );

    //! return symbol for current feature. Should not be used individually: there could be more symbols for a feature
    virtual QgsSymbolV2* symbolForFeature( QgsFeature& feature );

    virtual void renderFeature( QgsFeature& feature, QgsRenderContext& context, int layer = -1, bool selected = false, bool drawVertexMarker = false );

    virtual void startRender( QgsRenderContext& context, const QgsVectorLayer *vlayer );

    virtual void stopRender( QgsRenderContext& context );

    virtual QList<QString> usedAttributes();

    virtual QgsFeatureRendererV2* clone();

    virtual QgsSymbolV2List symbols();

    //! store renderer info to XML element
    virtual QDomElement save( QDomDocument& doc );

    //! return a list of symbology items for the legend
    virtual QgsLegendSymbologyList legendSymbologyItems( QSize iconSize );

    //! return a list of item text / symbol
    //! @note: this method was added in version 1.5
    virtual QgsLegendSymbolList legendSymbolItems();

    /////

    //! return the total number of rules
    int ruleCount();
    //! get reference to rule at index (valid indexes: 0...count-1)
    Rule& ruleAt( int index );
    //! add rule to the end of the list of rules
    void addRule( const Rule& rule );
    //! insert rule to a specific position of the list of rules
    void insertRule( int index, const Rule& rule );
    //! modify the rule at a specific position of the list of rules
    void updateRuleAt( int index, const Rule& rule );
    //! remove the rule at the specified index
    void removeRuleAt( int index );

    //////

    //! take a rule and create a list of new rules based on the categories from categorized symbol renderer
    static QList<Rule> refineRuleCategories( Rule& initialRule, QgsCategorizedSymbolRendererV2* r );
    //! take a rule and create a list of new rules based on the ranges from graduated symbol renderer
    static QList<Rule> refineRuleRanges( Rule& initialRule, QgsGraduatedSymbolRendererV2* r );
    //! take a rule and create a list of new rules with intervals of scales given by the passed scale denominators
    static QList<Rule> refineRuleScales( Rule& initialRule, QList<int> scales );

  protected:
    //! the list of rules
    QList<Rule> mRules;
    //! the default symbol, used for the first rule with no filter
    QgsSymbolV2* mDefaultSymbol;

    // temporary
    QList<Rule*> mCurrentRules;
    QgsFieldMap mCurrentFields;
    QgsSymbolV2* mCurrentSymbol;
};

#endif // QGSRULEBASEDRENDERERV2_H
