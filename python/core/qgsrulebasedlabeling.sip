

class QgsRuleBasedLabeling : QgsAbstractVectorLayerLabeling
{
%TypeHeaderCode
#include "qgsrulebasedlabeling.h"
%End
  public:
    typedef QList<QgsRuleBasedLabeling::Rule *> RuleList;

    class Rule
    {
%TypeHeaderCode
#include "qgsrulebasedlabeling.h"
%End
      public:
        Rule( QgsPalLayerSettings *settings /Transfer/, int scaleMinDenom = 0, int scaleMaxDenom = 0, const QString &filterExp = QString(), const QString &description = QString(), bool elseRule = false );
        ~Rule();

        enum RegisterResult
        {
          Filtered,
          Inactive,
          Registered
        };

        QgsPalLayerSettings *settings() const;
        bool dependsOnScale() const;
        int scaleMinDenom() const;
        int scaleMaxDenom() const;
        QString filterExpression() const;
        QString description() const;
        bool active() const;
        bool isElse() const;
        QString ruleKey() const;
        void setSettings( QgsPalLayerSettings *settings /Transfer/ );
        void setScaleMinDenom( int scaleMinDenom );
        void setScaleMaxDenom( int scaleMaxDenom );
        void setFilterExpression( const QString &filterExp );
        void setDescription( const QString &description );
        void setActive( bool state );
        void setIsElse( bool iselse );
        void setRuleKey( const QString &key );
        const QgsRuleBasedLabeling::RuleList &children() const;
        QgsRuleBasedLabeling::RuleList descendants() const;
        QgsRuleBasedLabeling::Rule *parent();
        void appendChild( QgsRuleBasedLabeling::Rule *rule /Transfer/ );
        void insertChild( int i, QgsRuleBasedLabeling::Rule *rule /Transfer/ );
        void removeChildAt( int i );
        const QgsRuleBasedLabeling::Rule *findRuleByKey( const QString &key ) const;
        QgsRuleBasedLabeling::Rule *clone() const /Factory/;
        static QgsRuleBasedLabeling::Rule *create( const QDomElement &ruleElem, const QgsReadWriteContext &context ) /Factory/;
        QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) const;
        bool requiresAdvancedEffects() const;

      private:
        Rule( const QgsRuleBasedLabeling::Rule &rh );
    };

    explicit QgsRuleBasedLabeling( QgsRuleBasedLabeling::Rule *root /Transfer/ );
    ~QgsRuleBasedLabeling();

    QgsRuleBasedLabeling::Rule *rootRule();

    static QgsRuleBasedLabeling *create( const QDomElement &element, const QgsReadWriteContext &context ) /Factory/;

    virtual QString type() const;
    virtual QgsRuleBasedLabeling *clone() const /Factory/;
    virtual QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) const;
    virtual QStringList subProviders() const;
    virtual QgsPalLayerSettings settings( const QString &providerId = QString() ) const;
    virtual bool requiresAdvancedEffects() const;

};
