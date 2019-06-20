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

#include "qgis_core.h"
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

/**
 * \ingroup core
 * \class QgsRuleBasedLabeling
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsRuleBasedLabeling : public QgsAbstractVectorLayerLabeling
{
  public:
    class Rule;
    typedef QList<QgsRuleBasedLabeling::Rule *> RuleList;
    typedef QMap<QgsRuleBasedLabeling::Rule *, QgsVectorLayerLabelProvider *> RuleToProviderMap;

    /**
     * \ingroup core
     * \class QgsRuleBasedLabeling::Rule
     * \since QGIS 3.0
     */
    class CORE_EXPORT Rule
    {
      public:
        //! takes ownership of settings, settings may be NULLPTR
        Rule( QgsPalLayerSettings *settings SIP_TRANSFER, double maximumScale = 0, double minimumScale = 0, const QString &filterExp = QString(), const QString &description = QString(), bool elseRule = false );
        ~Rule();

        //! Rules cannot be copied.
        Rule( const Rule &rh ) = delete;
        //! Rules cannot be copied.
        Rule &operator=( const Rule &rh ) = delete;

        //! The result of registering a rule
        enum RegisterResult
        {
          Filtered = 0, //!< The rule does not apply
          Inactive,     //!< The rule is inactive
          Registered    //!< Something was registered
        };

        /**
         * Returns the labeling settings. May return NULLPTR.
         */
        QgsPalLayerSettings *settings() const { return mSettings.get(); }

        /**
         * Determines if scale based labeling is active
         *
         * \returns TRUE if scale based labeling is active
         */
        bool dependsOnScale() const { return !qgsDoubleNear( mMinimumScale, 0.0 ) || !qgsDoubleNear( mMaximumScale, 0 ); }

        /**
         * Returns the maximum map scale (i.e. most "zoomed in" scale) at which the label rule will be active.
         * The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
         * A scale of 0 indicates no maximum scale visibility.
         * \see minimumScale()
         * \see setMaximumScale()
         * \since QGIS 3.0
         */
        double maximumScale() const { return mMaximumScale; }

        /**
         * Returns the minimum map scale (i.e. most "zoomed out" scale) at which the label rule will be active.
         * The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
         * A scale of 0 indicates no minimum scale visibility.
         * \see maximumScale()
         * \see setMinimumScale()
         * \since QGIS 3.0
         */
        double minimumScale() const { return mMinimumScale; }

        /**
         * A filter that will check if this rule applies
         * \returns An expression
         */
        QString filterExpression() const { return mFilterExp; }

        /**
         * A human readable description for this rule
         *
         * \returns Description
         */
        QString description() const { return mDescription; }

        /**
         * Returns if this rule is active
         *
         * \returns TRUE if the rule is active
         */
        bool active() const { return mIsActive; }

        /**
         * Check if this rule is an ELSE rule
         *
         * \returns TRUE if this rule is an else rule
         */
        bool isElse() const { return mElseRule; }

        //! Unique rule identifier (for identification of rule within labeling, used as provider ID)
        QString ruleKey() const { return mRuleKey; }

        //! Sets new settings (or NULLPTR). Deletes old settings if any.
        void setSettings( QgsPalLayerSettings *settings SIP_TRANSFER );

        /**
         * Sets the minimum map \a scale (i.e. most "zoomed out" scale) at which the label rule will be active.
         * The \a scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
         * A \a scale of 0 indicates no minimum scale visibility.
         * \see minimumScale()
         * \see setMaximumScale()
         */
        void setMinimumScale( double scale ) { mMinimumScale = scale; }

        /**
         * Sets the maximum map \a scale (i.e. most "zoomed in" scale) at which the rule will be active.
         * The \a scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
         * A \a scale of 0 indicates no maximum scale visibility.
         * \see maximumScale()
         * \see setMinimumScale()
         */
        void setMaximumScale( double scale ) { mMaximumScale = scale; }

        /**
         * Set the expression used to check if a given feature shall be rendered with this rule
         *
         * \param filterExp An expression
         */
        void setFilterExpression( const QString &filterExp ) { mFilterExp = filterExp; initFilter(); }

        /**
         * Set a human readable description for this rule
         *
         * \param description Description
         */
        void setDescription( const QString &description ) { mDescription = description; }

        /**
         * Sets if this rule is active
         * \param state Determines if the rule should be activated or deactivated
         */
        void setActive( bool state ) { mIsActive = state; }

        /**
         * Sets if this rule is an ELSE rule
         *
         * \param iselse If TRUE, this rule is an ELSE rule
         */
        void setIsElse( bool iselse ) { mElseRule = iselse; }

        //! Override the assigned rule key (should be used just internally by rule-based labeling)
        void setRuleKey( const QString &key ) { mRuleKey = key; }

        // parent / child operations

        /**
         * Returns all children rules of this rule
         *
         * \returns A list of rules
         */
        const QgsRuleBasedLabeling::RuleList &children() const { return mChildren; }

        /**
         * Returns all children rules of this rule
         *
         * \returns A list of rules
         */
        QgsRuleBasedLabeling::RuleList &children() SIP_SKIP { return mChildren; }

        /**
         * Returns all children, grand-children, grand-grand-children, grand-gra... you get it
         *
         * \returns A list of descendant rules
         */
        QgsRuleBasedLabeling::RuleList descendants() const;

        /**
         * The parent rule
         *
         * \returns Parent rule
         */
        const QgsRuleBasedLabeling::Rule *parent() const SIP_SKIP { return mParent; }

        /**
         * The parent rule
         *
         * \returns Parent rule
         */
        QgsRuleBasedLabeling::Rule *parent() { return mParent; }

        //! add child rule, take ownership, sets this as parent
        void appendChild( QgsRuleBasedLabeling::Rule *rule SIP_TRANSFER );

        //! add child rule, take ownership, sets this as parent
        void insertChild( int i, QgsRuleBasedLabeling::Rule *rule SIP_TRANSFER );

        //! delete child rule
        void removeChildAt( int i );

        //! Try to find a rule given its unique key
        const QgsRuleBasedLabeling::Rule *findRuleByKey( const QString &key ) const;

        /**
         * Find a labeling rule thanks to its key.
         *
         * \param key The key of the rule to find
         *
         * \returns The rule or NULLPTR if not found
         *
         * \since QGIS 3.0
         */
        QgsRuleBasedLabeling::Rule *findRuleByKey( const QString &key ) SIP_SKIP;

        //! clone this rule, return new instance
        QgsRuleBasedLabeling::Rule *clone() const SIP_FACTORY;

        // load / save

        /**
         * Create a rule from an XML definition
         * \param ruleElem  The XML rule element
         * \param context reading context
         * \returns A new rule
         */
        static QgsRuleBasedLabeling::Rule *create( const QDomElement &ruleElem, const QgsReadWriteContext &context ) SIP_FACTORY;

        //! store labeling info to XML element
        QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) const;

        // evaluation

        /**
         * add providers
         * \note not available in Python bindings
         */
        void createSubProviders( QgsVectorLayer *layer, RuleToProviderMap &subProviders, QgsRuleBasedLabelProvider *provider ) SIP_SKIP;

        /**
         * append rule keys of descendants that contain valid settings (i.e. they will be sub-providers)
         * \note not available in Python bindings
         */
        void subProviderIds( QStringList &list ) const SIP_SKIP;

        /**
         * call prepare() on sub-providers and populate attributeNames
         * \note not available in Python bindings
         */
        void prepare( const QgsRenderContext &context, QSet<QString> &attributeNames, RuleToProviderMap &subProviders ) SIP_SKIP;

        /**
         * register individual features
         * \note not available in Python bindings
         */
        RegisterResult registerFeature( const QgsFeature &feature, QgsRenderContext &context, RuleToProviderMap &subProviders, const QgsGeometry &obstacleGeometry = QgsGeometry() ) SIP_SKIP;

        /**
         * Returns TRUE if this rule or any of its children requires advanced composition effects
         * to render.
         */
        bool requiresAdvancedEffects() const;

      private:
#ifdef SIP_RUN
        Rule( const QgsRuleBasedLabeling::Rule &rh );
#endif

        /**
         * Check if a given feature shall be labelled by this rule
         *
         * \param f         The feature to test
         * \param context   The context in which the rendering happens
         * \returns          TRUE if the feature shall be rendered
         */
        bool isFilterOK( const QgsFeature &f, QgsRenderContext &context ) const;

        /**
         * Check if this rule applies for a given \a scale.
         * The \a scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
         * If set to 0, it will always return TRUE.
         *
         * \returns If the rule will be evaluated at this scale
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

      private:
        Rule *mParent = nullptr; // parent rule (nullptr only for root rule)
        std::unique_ptr<QgsPalLayerSettings> mSettings;
        double mMaximumScale = 0;
        double mMinimumScale = 0;
        QString mFilterExp;
        QString mDescription;
        bool mElseRule = false;
        RuleList mChildren;
        RuleList mElseRules;
        bool mIsActive = true; // whether it is enabled or not

        QString mRuleKey = QUuid::createUuid().toString(); // string used for unique identification of rule within labeling

        std::unique_ptr<QgsExpression> mFilter;

    };


    //! Constructs the labeling from given tree of rules (takes ownership)
    explicit QgsRuleBasedLabeling( QgsRuleBasedLabeling::Rule *root SIP_TRANSFER );
    ~QgsRuleBasedLabeling() override;

    QgsRuleBasedLabeling::Rule *rootRule();
    const Rule *rootRule() const SIP_SKIP;

    //! Create the instance from a DOM element with saved configuration
    static QgsRuleBasedLabeling *create( const QDomElement &element, const QgsReadWriteContext &context ) SIP_FACTORY;

    // implementation of parent interface

    QString type() const override;
    QgsRuleBasedLabeling *clone() const override SIP_FACTORY;
    QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) const override;
    //! \note not available in Python bindings
    QgsVectorLayerLabelProvider *provider( QgsVectorLayer *layer ) const override SIP_SKIP;
    QStringList subProviders() const override;
    QgsPalLayerSettings settings( const QString &providerId = QString() ) const override;

    /**
     * Set pal settings for a specific provider (takes ownership).
     *
     * \param settings Pal layer settings
     * \param providerId The id of the provider
     *
     * \since QGIS 3.0
     */
    void setSettings( QgsPalLayerSettings *settings SIP_TRANSFER, const QString &providerId = QString() ) override;
    bool requiresAdvancedEffects() const override;
    void toSld( QDomNode &parent, const QgsStringMap &props ) const override;

  protected:
    std::unique_ptr<Rule> mRootRule;
};

#ifndef SIP_RUN

/**
 * \ingroup core
 * \class QgsRuleBasedLabelProvider
 * \note not available in Python bindings
 * \note this class is not a part of public API yet. See notes in QgsLabelingEngine
 */
class CORE_EXPORT QgsRuleBasedLabelProvider : public QgsVectorLayerLabelProvider
{
  public:
    QgsRuleBasedLabelProvider( const QgsRuleBasedLabeling &rules, QgsVectorLayer *layer, bool withFeatureLoop = true );

    // reimplemented

    bool prepare( const QgsRenderContext &context, QSet<QString> &attributeNames ) override;

    void registerFeature( const QgsFeature &feature, QgsRenderContext &context, const QgsGeometry &obstacleGeometry = QgsGeometry() ) override;

    //! create a label provider
    virtual QgsVectorLayerLabelProvider *createProvider( QgsVectorLayer *layer, const QString &providerId, bool withFeatureLoop, const QgsPalLayerSettings *settings );

    //! Returns subproviders
    QList<QgsAbstractLabelProvider *> subProviders() override;

  protected:
    //! owned copy
    std::unique_ptr<QgsRuleBasedLabeling> mRules;
    //! label providers are owned by labeling engine
    QgsRuleBasedLabeling::RuleToProviderMap mSubProviders;
};

#endif

#endif // QGSRULEBASEDLABELING_H
