/***************************************************************************
  qgsrulebased3drenderer.h
  --------------------------------------
  Date                 : January 2019
  Copyright            : (C) 2019 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRULEBASED3DRENDERER_H
#define QGSRULEBASED3DRENDERER_H

#include "qgis_3d.h"

#include "qgs3drendererregistry.h"
#include "qgsabstractvectorlayer3drenderer.h"
#include "qgsabstract3dsymbol.h"
#include "qgsmaplayerref.h"
#include <QUuid>

class Qgs3DRenderContext;
class QgsFeature3DHandler;


/**
 * \ingroup 3d
 * \brief Metadata for rule-based 3D renderer to allow creation of its instances from XML
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 *
 * \since QGIS 3.6
 */
class _3D_EXPORT QgsRuleBased3DRendererMetadata : public Qgs3DRendererAbstractMetadata
{
  public:
    QgsRuleBased3DRendererMetadata();

    //! Creates an instance of a 3D renderer based on a DOM element with renderer configuration
    QgsAbstract3DRenderer *createRenderer( QDomElement &elem, const QgsReadWriteContext &context ) override SIP_FACTORY;
};


/**
 * \ingroup 3d
 * \brief Rule-based 3D renderer.
 *
 * Similar to rule-based 2D renderer and rule-based labeling, it allows specification of rules for 3D symbols.
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 *
 * \since QGIS 3.6
 */
class _3D_EXPORT QgsRuleBased3DRenderer : public QgsAbstractVectorLayer3DRenderer
{
  public:
    class Rule;
    typedef QList<QgsRuleBased3DRenderer::Rule *> RuleList;
    typedef QHash<const QgsRuleBased3DRenderer::Rule *, QgsFeature3DHandler *> RuleToHandlerMap;

    /**
     * \ingroup 3d
     * \brief A child rule for a QgsRuleBased3DRenderer
     * \since QGIS 3.6
     */
    class _3D_EXPORT Rule
    {
      public:
        //! takes ownership of symbol, symbol may be NULLPTR
        Rule( QgsAbstract3DSymbol *symbol SIP_TRANSFER, const QString &filterExp = QString(), const QString &description = QString(), bool elseRule = false );
        ~Rule();

        Rule( const Rule &rh ) = delete;
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
        QgsAbstract3DSymbol *symbol() const { return mSymbol.get(); }

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

        //! Sets new symbol (or NULLPTR). Deletes old symbol if any.
        void setSymbol( QgsAbstract3DSymbol *symbol SIP_TRANSFER );

        /**
         * Set the expression used to check if a given feature shall be rendered with this rule
         *
         * \param filterExp An expression
         */
        void setFilterExpression( const QString &filterExp )
        {
          mFilterExp = filterExp;
          initFilter();
        }

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

        // TODO: needed?
        //! Override the assigned rule key (should be used just internally by rule-based renderer)
        void setRuleKey( const QString &key ) { mRuleKey = key; }

        // parent / child operations

        /**
         * Returns all children rules of this rule
         *
         * \returns A list of rules
         */
        const QgsRuleBased3DRenderer::RuleList &children() const { return mChildren; }

        /**
         * Returns all children rules of this rule
         *
         * \returns A list of rules
         */
        QgsRuleBased3DRenderer::RuleList &children() SIP_SKIP { return mChildren; }

        /**
         * Returns all children, grand-children, grand-grand-children, grand-gra... you get it
         *
         * \returns A list of descendant rules
         */
        QgsRuleBased3DRenderer::RuleList descendants() const;

        /**
         * The parent rule
         *
         * \returns Parent rule
         */
        const QgsRuleBased3DRenderer::Rule *parent() const SIP_SKIP { return mParent; }

        /**
         * The parent rule
         *
         * \returns Parent rule
         */
        QgsRuleBased3DRenderer::Rule *parent() { return mParent; }

        //! add child rule, take ownership, sets this as parent
        void appendChild( QgsRuleBased3DRenderer::Rule *rule SIP_TRANSFER );

        //! add child rule, take ownership, sets this as parent
        void insertChild( int i, QgsRuleBased3DRenderer::Rule *rule SIP_TRANSFER );

        //! delete child rule
        void removeChildAt( int i );

        //! Try to find a rule given its unique key
        const QgsRuleBased3DRenderer::Rule *findRuleByKey( const QString &key ) const;

        /**
         * Find a rule thanks to its key.
         * \param key The key of the rule to find
         * \returns The rule or NULLPTR if not found
         */
        QgsRuleBased3DRenderer::Rule *findRuleByKey( const QString &key ) SIP_SKIP;

        //! clone this rule, return new instance
        QgsRuleBased3DRenderer::Rule *clone() const SIP_FACTORY;

        // load / save

        /**
         * Create a rule from an XML definition
         * \param ruleElem  The XML rule element
         * \param context reading context
         * \returns A new rule
         */
        static QgsRuleBased3DRenderer::Rule *create( const QDomElement &ruleElem, const QgsReadWriteContext &context ) SIP_FACTORY;

        //! store labeling info to XML element
        QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) const;

        // evaluation

        /**
         * add handlers
         * \note not available in Python bindings
         */
        void createHandlers( QgsVectorLayer *layer, RuleToHandlerMap &handlers ) const SIP_SKIP;

        /**
         * call prepare() on handlers and populate attributeNames
         * \note not available in Python bindings
         */
        void prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames, const QgsVector3D &chunkOrigin, RuleToHandlerMap &handlers ) const SIP_SKIP;

        /**
         * register individual features
         * \note not available in Python bindings
         */
        RegisterResult registerFeature( QgsFeature &feature, Qgs3DRenderContext &context, RuleToHandlerMap &handlers ) const SIP_SKIP;

      private:
#ifdef SIP_RUN
        Rule( const QgsRuleBased3DRenderer::Rule &rh );
#endif

        /**
         * Check if a given feature shall be labelled by this rule
         *
         * \param f         The feature to test
         * \param context   The context in which the rendering happens
         * \returns          TRUE if the feature shall be rendered
         */
        bool isFilterOK( QgsFeature &f, Qgs3DRenderContext &context ) const;

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
        std::unique_ptr<QgsAbstract3DSymbol> mSymbol;
        QString mFilterExp;
        QString mDescription;
        bool mElseRule = false;
        RuleList mChildren;
        RuleList mElseRules;
        bool mIsActive = true; // whether it is enabled or not

        QString mRuleKey = QUuid::createUuid().toString(); // string used for unique identification of rule within labeling

        std::unique_ptr<QgsExpression> mFilter;
    };


    //! Construct renderer with the given root rule (takes ownership)
    QgsRuleBased3DRenderer( QgsRuleBased3DRenderer::Rule *root SIP_TRANSFER );
    ~QgsRuleBased3DRenderer() override;

    //! Returns pointer to the root rule
    QgsRuleBased3DRenderer::Rule *rootRule() { return mRootRule; }
    //! Returns pointer to the root rule
    const Rule *rootRule() const SIP_SKIP { return mRootRule; }

    QString type() const override { return "rulebased"; }
    QgsRuleBased3DRenderer *clone() const override SIP_FACTORY;
    Qt3DCore::QEntity *createEntity( Qgs3DMapSettings *map ) const override SIP_SKIP;

    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;

  private:
    Rule *mRootRule = nullptr;
};

#endif // QGSRULEBASED3DRENDERER_H
