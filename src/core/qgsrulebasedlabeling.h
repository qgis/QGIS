/***************************************************************************
    qgsrulebasedlabeling.h
    ---------------------
    begin                : September 2015
    copyright            : (C) 2015 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRULEBASEDLABELING_H
#define QGSRULEBASEDLABELING_H

#include <QStringList>
#include <QMap>

#include "qgsvectorlayerlabeling.h"
#include "qgsvectorlayerlabelprovider.h"

class QDomDocument;
class QDomElement;

class QgsExpression;
class QgsFeature;
class QgsPalLayerSettings;
class QgsRenderContext;
class QgsGeometry;
class QgsRuleBasedLabelProvider;

/** \ingroup core
 * @class QgsRuleBasedLabeling
 * @note not available in Python bindings
 * @note this class is not a part of public API yet. See notes in QgsLabelingEngineV2
 */

class CORE_EXPORT QgsRuleBasedLabeling : public QgsAbstractVectorLayerLabeling
{
  public:
    class Rule;
    typedef QList<Rule*> RuleList;
    typedef QMap<Rule*, QgsVectorLayerLabelProvider*> RuleToProviderMap;

    /**
     * \ingroup core
     * @class QgsRuleBasedLabeling::Rule
     * @note not available in Python bindings
     * @note this class is not a part of public API yet. See notes in QgsLabelingEngineV2
     */
    class CORE_EXPORT Rule
    {
      public:
        //! takes ownership of settings
        Rule( QgsPalLayerSettings* settings, int scaleMinDenom = 0, int scaleMaxDenom = 0, const QString& filterExp = QString(), const QString& description = QString(), bool elseRule = false );
        ~Rule();

        //! The result of registering a rule
        enum RegisterResult
        {
          Filtered = 0, //!< The rule does not apply
          Inactive,     //!< The rule is inactive
          Registered    //!< Something was registered
        };

        /**
         * Get the labeling settings. May return a null pointer.
         */
        QgsPalLayerSettings* settings() const { return mSettings; }

        /**
         * Determines if scale based labeling is active
         *
         * @return True if scale based labeling is active
         */
        bool dependsOnScale() const { return mScaleMinDenom != 0 || mScaleMaxDenom != 0; }

        /**
         * The minimum scale at which this label rule should be applied
         *
         * E.g. Denominator 1000 is a scale of 1:1000, where a rule with minimum denominator
         * of 900 will not be applied while a rule with 2000 will be applied.
         *
         * @return The minimum scale denominator
         */
        int scaleMinDenom() const { return mScaleMinDenom; }

        /**
         * The maximum scale denominator at which this label rule should be applied
         *
         * E.g. Denominator 1000 is a scale of 1:1000, where a rule with maximum denominator
         * of 900 will be applied while a rule with 2000 will not be applied.
         *
         * @return The maximum scale denominator
         */
        int scaleMaxDenom() const { return mScaleMaxDenom; }
        /**
         * A filter that will check if this rule applies
         * @return An expression
         */
        QString filterExpression() const { return mFilterExp; }
        /**
         * A human readable description for this rule
         *
         * @return Description
         */
        QString description() const { return mDescription; }
        /**
         * Returns if this rule is active
         *
         * @return True if the rule is active
         */
        bool active() const { return mIsActive; }
        /**
         * Check if this rule is an ELSE rule
         *
         * @return True if this rule is an else rule
         */
        bool isElse() const { return mElseRule; }

        //! Unique rule identifier (for identification of rule within labeling, used as provider ID)
        QString ruleKey() const { return mRuleKey; }

        //! set new settings (or NULL). Deletes old settings if any.
        void setSettings( QgsPalLayerSettings* settings );

        /**
         * Set the minimum denominator for which this rule shall apply.
         * E.g. 1000 if it shall be evaluated between 1:1000 and 1:100'000
         * Set to 0 to disable the minimum check
         * @param scaleMinDenom The minimum scale denominator for this rule
         */
        void setScaleMinDenom( int scaleMinDenom ) { mScaleMinDenom = scaleMinDenom; }
        /**
         * Set the maximum denominator for which this rule shall apply.
         * E.g. 100'000 if it shall be evaluated between 1:1000 and 1:100'000
         * Set to 0 to disable the maximum check
         * @param scaleMaxDenom maximum scale denominator for this rule
         */
        void setScaleMaxDenom( int scaleMaxDenom ) { mScaleMaxDenom = scaleMaxDenom; }
        /**
         * Set the expression used to check if a given feature shall be rendered with this rule
         *
         * @param filterExp An expression
         */
        void setFilterExpression( const QString& filterExp ) { mFilterExp = filterExp; initFilter(); }
        /**
         * Set a human readable description for this rule
         *
         * @param description Description
         */
        void setDescription( const QString& description ) { mDescription = description; }
        /**
         * Sets if this rule is active
         * @param state Determines if the rule should be activated or deactivated
         */
        void setActive( bool state ) { mIsActive = state; }
        /**
         * Sets if this rule is an ELSE rule
         *
         * @param iselse If true, this rule is an ELSE rule
         */
        void setIsElse( bool iselse ) { mElseRule = iselse; }

        //! Override the assigned rule key (should be used just internally by rule-based labeling)
        void setRuleKey( const QString& key ) { mRuleKey = key; }

        // parent / child operations

        /**
         * Return all children rules of this rule
         *
         * @return A list of rules
         */
        const RuleList& children() const { return mChildren; }
        /**
         * Return all children rules of this rule
         *
         * @return A list of rules
         */
        RuleList& children() { return mChildren; }

        /**
         * Returns all children, grand-children, grand-grand-children, grand-gra... you get it
         *
         * @return A list of descendant rules
         */
        RuleList descendants() const { RuleList l; Q_FOREACH ( Rule *c, mChildren ) { l += c; l += c->descendants(); } return l; }

        /**
         * The parent rule
         *
         * @return Parent rule
         */
        const Rule* parent() const { return mParent; }
        /**
         * The parent rule
         *
         * @return Parent rule
         */
        Rule* parent() { return mParent; }

        //! add child rule, take ownership, sets this as parent
        void appendChild( Rule* rule );

        //! add child rule, take ownership, sets this as parent
        void insertChild( int i, Rule* rule );

        //! delete child rule
        void removeChildAt( int i );

        //! Try to find a rule given its unique key
        const Rule* findRuleByKey( const QString& key ) const;

        //! clone this rule, return new instance
        Rule* clone() const;

        // load / save

        /**
         * Create a rule from an XML definition
         * @param ruleElem  The XML rule element
         * @return A new rule
         */
        static Rule* create( const QDomElement& ruleElem );

        //! store labeling info to XML element
        QDomElement save( QDomDocument& doc ) const;

        // evaluation

        //! add providers
        void createSubProviders( QgsVectorLayer* layer, RuleToProviderMap& subProviders, QgsRuleBasedLabelProvider *provider );

        //! append rule keys of descendants that contain valid settings (i.e. they will be sub-providers)
        void subProviderIds( QStringList& list ) const;

        //! call prepare() on sub-providers and populate attributeNames
        void prepare( const QgsRenderContext& context, QStringList& attributeNames, RuleToProviderMap& subProviders );

        //! register individual features
        RegisterResult registerFeature( QgsFeature& feature, QgsRenderContext& context, RuleToProviderMap& subProviders, QgsGeometry* obstacleGeometry = nullptr );

      protected:
        /**
         * Check if a given feature shall be labelled by this rule
         *
         * @param f         The feature to test
         * @param context   The context in which the rendering happens
         * @return          True if the feature shall be rendered
         */
        bool isFilterOK( QgsFeature& f, QgsRenderContext& context ) const;
        /**
         * Check if this rule applies for a given scale
         * @param scale The scale to check. If set to 0, it will always return true.
         *
         * @return If the rule will be evaluated at this scale
         */
        bool isScaleOK( double scale ) const;

        /**
         * Initialize filters. Automatically called by setFilterExpression.
         */
        void initFilter();

        /**
         * Check which child rules are else rules and update the internal list of else rules
         */
        void updateElseRules();

      protected:
        Rule* mParent; // parent rule (NULL only for root rule)
        QgsPalLayerSettings* mSettings;
        int mScaleMinDenom, mScaleMaxDenom;
        QString mFilterExp, mDescription;
        bool mElseRule;
        RuleList mChildren;
        RuleList mElseRules;
        bool mIsActive; // whether it is enabled or not

        QString mRuleKey; // string used for unique identification of rule within labeling

        // temporary
        QgsExpression* mFilter;

      private:

        Rule( const Rule& rh );
        Rule& operator=( const Rule& rh );
    };


    //! Constructs the labeling from given tree of rules (takes ownership)
    explicit QgsRuleBasedLabeling( QgsRuleBasedLabeling::Rule* root );
    //! Copy constructor
    QgsRuleBasedLabeling( const QgsRuleBasedLabeling& other );
    ~QgsRuleBasedLabeling();

    Rule* rootRule() { return mRootRule; }
    const Rule* rootRule() const { return mRootRule; }

    //! Create the instance from a DOM element with saved configuration
    static QgsRuleBasedLabeling* create( const QDomElement& element );

    // implementation of parent interface

    virtual QString type() const override;
    virtual QDomElement save( QDomDocument& doc ) const override;
    virtual QgsVectorLayerLabelProvider *provider( QgsVectorLayer* layer ) const override;
    virtual QStringList subProviders() const override;
    virtual QgsPalLayerSettings settings( QgsVectorLayer* layer, const QString& providerId = QString() ) const override;

  protected:
    Rule* mRootRule;
};


/** \ingroup core
 * @class QgsRuleBasedLabelProvider
 * @note not available in Python bindings
 * @note this class is not a part of public API yet. See notes in QgsLabelingEngineV2
 */
class CORE_EXPORT QgsRuleBasedLabelProvider : public QgsVectorLayerLabelProvider
{
  public:
    QgsRuleBasedLabelProvider( const QgsRuleBasedLabeling& rules, QgsVectorLayer* layer, bool withFeatureLoop = true );
    ~QgsRuleBasedLabelProvider();

    // reimplemented

    virtual bool prepare( const QgsRenderContext& context, QStringList& attributeNames ) override;

    virtual void registerFeature( QgsFeature& feature, QgsRenderContext& context, QgsGeometry* obstacleGeometry = nullptr ) override;

    //! create a label provider
    virtual QgsVectorLayerLabelProvider *createProvider( QgsVectorLayer *layer, const QString& providerId, bool withFeatureLoop, const QgsPalLayerSettings *settings );

    //! return subproviders
    virtual QList<QgsAbstractLabelProvider*> subProviders() override;

  protected:
    //! owned copy
    QgsRuleBasedLabeling mRules;
    //! label providers are owned by labeling engine
    QgsRuleBasedLabeling::RuleToProviderMap mSubProviders;
};


#endif // QGSRULEBASEDLABELING_H
