#ifndef QGSRULEBASEDLABELING_H
#define QGSRULEBASEDLABELING_H

#include <QStringList>

#include "qgsvectorlayerlabeling.h"

class QDomDocument;
class QDomElement;

class QgsExpression;
class QgsFeature;
class QgsPalLayerSettings;
class QgsRenderContext;


class CORE_EXPORT QgsRuleBasedLabeling : public QgsAbstractVectorLayerLabeling
{
  public:
    class Rule;
    typedef QList<Rule*> RuleList;
    typedef QMap<Rule*, QgsVectorLayerLabelProvider*> RuleToProviderMap;

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

        QgsPalLayerSettings* settings() const { return mSettings; }
        bool dependsOnScale() const { return mScaleMinDenom != 0 || mScaleMaxDenom != 0; }
        int scaleMinDenom() const { return mScaleMinDenom; }
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
        void setFilterExpression( QString filterExp ) { mFilterExp = filterExp; initFilter(); }
        /**
         * Set a human readable description for this rule
         *
         * @param description Description
         */
        void setDescription( QString description ) { mDescription = description; }
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
        void createSubProviders( QgsVectorLayer* layer, RuleToProviderMap& subProviders );

        //! call prepare() on sub-providers and populate attributeNames
        void prepare( const QgsRenderContext& context, QStringList& attributeNames, RuleToProviderMap& subProviders );

        //! register individual features
        RegisterResult registerFeature( QgsFeature& feature, const QgsRenderContext& context, RuleToProviderMap& subProviders );

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

        // temporary
        QgsExpression* mFilter;
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
    virtual QgsVectorLayerLabelProvider* provider( QgsVectorLayer* layer ) const override;

  protected:
    Rule* mRootRule;
};


#include "qgsvectorlayerlabelprovider.h"


class CORE_EXPORT QgsRuleBasedLabelProvider : public QgsVectorLayerLabelProvider
{
  public:
    QgsRuleBasedLabelProvider( const QgsRuleBasedLabeling& rules, QgsVectorLayer* layer, bool withFeatureLoop = true );
    ~QgsRuleBasedLabelProvider();

    // reimplemented

    virtual bool prepare( const QgsRenderContext& context, QStringList& attributeNames ) override;

    virtual void registerFeature( QgsFeature& feature, const QgsRenderContext& context ) override;

    // new methods

    virtual QList<QgsAbstractLabelProvider*> subProviders() override;

  protected:
    //! owned copy
    QgsRuleBasedLabeling mRules;
    //! label providers are owned by labeling engine
    QgsRuleBasedLabeling::RuleToProviderMap mSubProviders;
};


#endif // QGSRULEBASEDLABELING_H
