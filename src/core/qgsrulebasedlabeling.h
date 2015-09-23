#ifndef QGSRULEBASEDLABELING_H
#define QGSRULEBASEDLABELING_H

#include <QStringList>

class QgsExpression;
class QgsFeature;
class QgsPalLayerSettings;
class QgsRenderContext;
class QgsVectorLayer;
class QgsVectorLayerLabelProvider;


class CORE_EXPORT QgsRuleBasedLabeling
{
  public:
    class Rule;
    typedef QList<Rule*> RuleList;
    typedef QMap<Rule*, QgsVectorLayerLabelProvider*> RuleToProviderMap;

    class Rule
    {
      public:
        //! takes ownership of settings
        Rule( QgsPalLayerSettings* settings, int scaleMinDenom = 0, int scaleMaxDenom = 0, const QString& filterExp = QString(), const QString& label = QString(), const QString& description = QString(), bool elseRule = false );
        ~Rule();

        QgsPalLayerSettings* settings() const { return mSettings; }
        QString label() const { return mLabel; }
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
         * Check if this rule is an ELSE rule
         *
         * @return True if this rule is an else rule
         */
        bool isElse() { return mElseRule; }

        void setLabel( QString label ) { mLabel = label; }
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
        RuleList& children() { return mChildren; }
        /**
         * The parent rule
         *
         * @return Parent rule
         */
        Rule* parent() { return mParent; }

        //! add child rule, take ownership, sets this as parent
        void appendChild( Rule* rule );

        //! clone this rule, return new instance
        Rule* clone() const;

        // TODO: load / save

        // evaluation

        //! add providers
        void createSubProviders( QgsVectorLayer* layer, RuleToProviderMap& subProviders );

        //! call prepare() on sub-providers and populate attributeNames
        void prepare( const QgsRenderContext& context, QStringList& attributeNames, RuleToProviderMap& subProviders );

        //! register individual features
        void registerFeature( QgsFeature& feature, const QgsRenderContext& context, RuleToProviderMap& subProviders );

      protected:
        /**
         * Check if a given feature shall be labelled by this rule
         *
         * @param f         The feature to test
         * @param context   The context in which the rendering happens
         * @return          True if the feature shall be rendered
         */
        bool isFilterOK( QgsFeature& f, QgsRenderContext& context ) const;

        void initFilter();

      protected:
        Rule* mParent; // parent rule (NULL only for root rule)
        QgsPalLayerSettings* mSettings;
        int mScaleMinDenom, mScaleMaxDenom;
        QString mFilterExp, mLabel, mDescription;
        bool mElseRule;
        RuleList mChildren;

        // temporary
        QgsExpression* mFilter;
    };


    //! Constructs the labeling from given tree of rules (takes ownership)
    explicit QgsRuleBasedLabeling( QgsRuleBasedLabeling::Rule* root );
    //! Copy constructor
    QgsRuleBasedLabeling( const QgsRuleBasedLabeling& other );
    ~QgsRuleBasedLabeling();

    Rule* rootRule() { return mRootRule; }

    // TODO: static create() from DOM



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
