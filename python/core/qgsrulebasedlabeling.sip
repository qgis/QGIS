/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsrulebasedlabeling.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsRuleBasedLabeling : QgsAbstractVectorLayerLabeling
{
%Docstring
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsrulebasedlabeling.h"
%End
  public:
    typedef QList<QgsRuleBasedLabeling::Rule *> RuleList;
    typedef QMap<QgsRuleBasedLabeling::Rule *, QgsVectorLayerLabelProvider *> RuleToProviderMap;

    class Rule
{
%Docstring
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsrulebasedlabeling.h"
%End
      public:
        Rule( QgsPalLayerSettings *settings /Transfer/, int scaleMinDenom = 0, int scaleMaxDenom = 0, const QString &filterExp = QString(), const QString &description = QString(), bool elseRule = false );
%Docstring
takes ownership of settings
%End
        ~Rule();


        enum RegisterResult
        {
          Filtered,
          Inactive,
          Registered
        };

        QgsPalLayerSettings *settings() const;
%Docstring
 Get the labeling settings. May return a null pointer.
 :rtype: QgsPalLayerSettings
%End

        bool dependsOnScale() const;
%Docstring
 Determines if scale based labeling is active

 :return: True if scale based labeling is active
 :rtype: bool
%End

        int scaleMinDenom() const;
%Docstring
 The minimum scale at which this label rule should be applied

 E.g. Denominator 1000 is a scale of 1:1000, where a rule with minimum denominator
 of 900 will not be applied while a rule with 2000 will be applied.

 :return: The minimum scale denominator
 :rtype: int
%End

        int scaleMaxDenom() const;
%Docstring
 The maximum scale denominator at which this label rule should be applied

 E.g. Denominator 1000 is a scale of 1:1000, where a rule with maximum denominator
 of 900 will be applied while a rule with 2000 will not be applied.

 :return: The maximum scale denominator
 :rtype: int
%End

        QString filterExpression() const;
%Docstring
 A filter that will check if this rule applies
 :return: An expression
 :rtype: str
%End

        QString description() const;
%Docstring
 A human readable description for this rule

 :return: Description
 :rtype: str
%End

        bool active() const;
%Docstring
 Returns if this rule is active

 :return: True if the rule is active
 :rtype: bool
%End

        bool isElse() const;
%Docstring
 Check if this rule is an ELSE rule

 :return: True if this rule is an else rule
 :rtype: bool
%End

        QString ruleKey() const;
%Docstring
Unique rule identifier (for identification of rule within labeling, used as provider ID)
 :rtype: str
%End

        void setSettings( QgsPalLayerSettings *settings /Transfer/ );
%Docstring
set new settings (or NULL). Deletes old settings if any.
%End

        void setScaleMinDenom( int scaleMinDenom );
%Docstring
 Set the minimum denominator for which this rule shall apply.
 E.g. 1000 if it shall be evaluated between 1:1000 and 1:100'000
 Set to 0 to disable the minimum check
 \param scaleMinDenom The minimum scale denominator for this rule
%End

        void setScaleMaxDenom( int scaleMaxDenom );
%Docstring
 Set the maximum denominator for which this rule shall apply.
 E.g. 100'000 if it shall be evaluated between 1:1000 and 1:100'000
 Set to 0 to disable the maximum check
 \param scaleMaxDenom maximum scale denominator for this rule
%End

        void setFilterExpression( const QString &filterExp );
%Docstring
 Set the expression used to check if a given feature shall be rendered with this rule

 \param filterExp An expression
%End

        void setDescription( const QString &description );
%Docstring
 Set a human readable description for this rule

 \param description Description
%End

        void setActive( bool state );
%Docstring
 Sets if this rule is active
 \param state Determines if the rule should be activated or deactivated
%End

        void setIsElse( bool iselse );
%Docstring
 Sets if this rule is an ELSE rule

 \param iselse If true, this rule is an ELSE rule
%End

        void setRuleKey( const QString &key );
%Docstring
Override the assigned rule key (should be used just internally by rule-based labeling)
%End


        const QgsRuleBasedLabeling::RuleList &children() const;
%Docstring
 Return all children rules of this rule

 :return: A list of rules
 :rtype: QgsRuleBasedLabeling.RuleList
%End


        QgsRuleBasedLabeling::RuleList descendants() const;
%Docstring
 Returns all children, grand-children, grand-grand-children, grand-gra... you get it

 :return: A list of descendant rules
 :rtype: QgsRuleBasedLabeling.RuleList
%End


        QgsRuleBasedLabeling::Rule *parent();
%Docstring
 The parent rule

 :return: Parent rule
 :rtype: QgsRuleBasedLabeling.Rule
%End

        void appendChild( QgsRuleBasedLabeling::Rule *rule /Transfer/ );
%Docstring
add child rule, take ownership, sets this as parent
%End

        void insertChild( int i, QgsRuleBasedLabeling::Rule *rule /Transfer/ );
%Docstring
add child rule, take ownership, sets this as parent
%End

        void removeChildAt( int i );
%Docstring
delete child rule
%End

        const QgsRuleBasedLabeling::Rule *findRuleByKey( const QString &key ) const;
%Docstring
Try to find a rule given its unique key
 :rtype: QgsRuleBasedLabeling.Rule
%End

        QgsRuleBasedLabeling::Rule *clone() const /Factory/;
%Docstring
clone this rule, return new instance
 :rtype: QgsRuleBasedLabeling.Rule
%End


        static QgsRuleBasedLabeling::Rule *create( const QDomElement &ruleElem, const QgsReadWriteContext &context ) /Factory/;
%Docstring
 Create a rule from an XML definition
 \param ruleElem  The XML rule element
 \param context reading context
 :return: A new rule
 :rtype: QgsRuleBasedLabeling.Rule
%End

        QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) const;
%Docstring
store labeling info to XML element
 :rtype: QDomElement
%End






        bool requiresAdvancedEffects() const;
%Docstring
 Returns true if this rule or any of its children requires advanced composition effects
 to render.
 :rtype: bool
%End

      private:
        Rule( const QgsRuleBasedLabeling::Rule &rh );
    };


    explicit QgsRuleBasedLabeling( QgsRuleBasedLabeling::Rule *root /Transfer/ );
%Docstring
Constructs the labeling from given tree of rules (takes ownership)
%End
    ~QgsRuleBasedLabeling();

    QgsRuleBasedLabeling::Rule *rootRule();
%Docstring
 :rtype: QgsRuleBasedLabeling.Rule
%End

    static QgsRuleBasedLabeling *create( const QDomElement &element, const QgsReadWriteContext &context ) /Factory/;
%Docstring
Create the instance from a DOM element with saved configuration
 :rtype: QgsRuleBasedLabeling
%End


    virtual QString type() const;
    virtual QgsRuleBasedLabeling *clone() const /Factory/;
    virtual QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) const;
    virtual QStringList subProviders() const;
    virtual QgsPalLayerSettings settings( const QString &providerId = QString() ) const;
    virtual bool requiresAdvancedEffects() const;


  protected:
};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsrulebasedlabeling.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
