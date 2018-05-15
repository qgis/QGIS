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
        Rule( QgsPalLayerSettings *settings /Transfer/, int maximumScale = 0, int minimumScale = 0, const QString &filterExp = QString(), const QString &description = QString(), bool elseRule = false );
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
%End

        bool dependsOnScale() const;
%Docstring
Determines if scale based labeling is active

:return: True if scale based labeling is active
%End

        double maximumScale() const;
%Docstring
Returns the maximum map scale (i.e. most "zoomed in" scale) at which the label rule will be active.
The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
A scale of 0 indicates no maximum scale visibility.

.. seealso:: :py:func:`minimumScale`

.. seealso:: :py:func:`setMaximumScale`

.. versionadded:: 3.0
%End

        double minimumScale() const;
%Docstring
Returns the minimum map scale (i.e. most "zoomed out" scale) at which the label rule will be active.
The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
A scale of 0 indicates no minimum scale visibility.

.. seealso:: :py:func:`maximumScale`

.. seealso:: :py:func:`setMinimumScale`

.. versionadded:: 3.0
%End

        QString filterExpression() const;
%Docstring
A filter that will check if this rule applies

:return: An expression
%End

        QString description() const;
%Docstring
A human readable description for this rule

:return: Description
%End

        bool active() const;
%Docstring
Returns if this rule is active

:return: True if the rule is active
%End

        bool isElse() const;
%Docstring
Check if this rule is an ELSE rule

:return: True if this rule is an else rule
%End

        QString ruleKey() const;
%Docstring
Unique rule identifier (for identification of rule within labeling, used as provider ID)
%End

        void setSettings( QgsPalLayerSettings *settings /Transfer/ );
%Docstring
set new settings (or NULL). Deletes old settings if any.
%End

        void setMinimumScale( double scale );
%Docstring
Sets the minimum map ``scale`` (i.e. most "zoomed out" scale) at which the label rule will be active.
The ``scale`` value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
A ``scale`` of 0 indicates no minimum scale visibility.

.. seealso:: :py:func:`minimumScale`

.. seealso:: :py:func:`setMaximumScale`
%End

        void setMaximumScale( double scale );
%Docstring
Sets the maximum map ``scale`` (i.e. most "zoomed in" scale) at which the rule will be active.
The ``scale`` value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
A ``scale`` of 0 indicates no maximum scale visibility.

.. seealso:: :py:func:`maximumScale`

.. seealso:: :py:func:`setMinimumScale`
%End

        void setFilterExpression( const QString &filterExp );
%Docstring
Set the expression used to check if a given feature shall be rendered with this rule

:param filterExp: An expression
%End

        void setDescription( const QString &description );
%Docstring
Set a human readable description for this rule

:param description: Description
%End

        void setActive( bool state );
%Docstring
Sets if this rule is active

:param state: Determines if the rule should be activated or deactivated
%End

        void setIsElse( bool iselse );
%Docstring
Sets if this rule is an ELSE rule

:param iselse: If true, this rule is an ELSE rule
%End

        void setRuleKey( const QString &key );
%Docstring
Override the assigned rule key (should be used just internally by rule-based labeling)
%End


        const QgsRuleBasedLabeling::RuleList &children() const;
%Docstring
Return all children rules of this rule

:return: A list of rules
%End


        QgsRuleBasedLabeling::RuleList descendants() const;
%Docstring
Returns all children, grand-children, grand-grand-children, grand-gra... you get it

:return: A list of descendant rules
%End


        QgsRuleBasedLabeling::Rule *parent();
%Docstring
The parent rule

:return: Parent rule
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
%End


        QgsRuleBasedLabeling::Rule *clone() const /Factory/;
%Docstring
clone this rule, return new instance
%End


        static QgsRuleBasedLabeling::Rule *create( const QDomElement &ruleElem, const QgsReadWriteContext &context ) /Factory/;
%Docstring
Create a rule from an XML definition

:param ruleElem:  The XML rule element
:param context: reading context

:return: A new rule
%End

        QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) const;
%Docstring
store labeling info to XML element
%End






        bool requiresAdvancedEffects() const;
%Docstring
Returns true if this rule or any of its children requires advanced composition effects
to render.
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

    static QgsRuleBasedLabeling *create( const QDomElement &element, const QgsReadWriteContext &context ) /Factory/;
%Docstring
Create the instance from a DOM element with saved configuration
%End


    virtual QString type() const;

    virtual QgsRuleBasedLabeling *clone() const /Factory/;

    virtual QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) const;

    virtual QStringList subProviders() const;

    virtual QgsPalLayerSettings settings( const QString &providerId = QString() ) const;


    virtual void setSettings( QgsPalLayerSettings *settings /Transfer/, const QString &providerId = QString() );

%Docstring
Set pal settings for a specific provider (takes ownership).

:param settings: Pal layer settings
:param providerId: The id of the provider

.. versionadded:: 3.0
%End
    virtual bool requiresAdvancedEffects() const;

    virtual void toSld( QDomNode &parent, const QgsStringMap &props ) const;


  protected:
};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsrulebasedlabeling.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
