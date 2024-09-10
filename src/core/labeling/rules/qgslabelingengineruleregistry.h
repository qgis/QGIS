/***************************************************************************
    qgslabelingengineruleregistry.h
    ---------------------
  Date                 : August 2024
  Copyright            : (C) 2024 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLABELINGENGINERULEREGISTRY_H
#define QGSLABELINGENGINERULEREGISTRY_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"

class QgsAbstractLabelingEngineRule;

/**
 * A registry for labeling engine rules.
 *
 * Labeling engine rules implement custom logic to modify the labeling solution for a map render,
 * e.g. by preventing labels being placed which violate custom constraints.
 *
 * This registry stores available rules and is responsible for creating rules.
 *
 * QgsLabelingEngineRuleRegistry is not usually directly created, but rather accessed through
 * QgsApplication::labelEngineRuleRegistry().
 *
 * \ingroup core
 * \since QGIS 3.40
 */
class CORE_EXPORT QgsLabelingEngineRuleRegistry
{
  public:

    /**
     * Constructor for QgsLabelingEngineRuleRegistry, containing a set of
     * default rules.
     */
    QgsLabelingEngineRuleRegistry();
    ~QgsLabelingEngineRuleRegistry();

    //! QgsLabelingEngineRuleRegistry cannot be copied
    QgsLabelingEngineRuleRegistry( const QgsLabelingEngineRuleRegistry &other ) = delete;
    //! QgsLabelingEngineRuleRegistry cannot be copied
    QgsLabelingEngineRuleRegistry &operator=( const QgsLabelingEngineRuleRegistry &other ) = delete;

    /**
     * Returns a list of the rule IDs for rules present in the registry.
     */
    QStringList ruleIds() const;

    /**
     * Returns a user-friendly, translated string representing the rule type with matching \a id.
     */
    QString displayType( const QString &id ) const;

    /**
     * Returns TRUE if the rule is with matching \a id is available for use within the current QGIS environment.
     *
     * Rules can return FALSE if required dependencies are not available, e.g. if a library version
     * is too old for the rule.
     */
    bool isAvailable( const QString &id ) const;

    /**
     * Creates a new rule from the type with matching \a id.
     *
     * Returns NULLPTR if no matching rule was found in the registry.
     *
     * The caller takes ownership of the returned object.
     */
    QgsAbstractLabelingEngineRule *create( const QString &id ) const SIP_TRANSFERBACK;

    /**
     * Adds a new \a rule type to the registry.
     *
     * The registry takes ownership of \a rule.
     *
     * \returns TRUE if the rule was successfully added.
     *
     * \see removeRule()
     */
    bool addRule( QgsAbstractLabelingEngineRule *rule SIP_TRANSFER );

    /**
     * Removes the rule with matching \a id from the registry.
     *
     * \see addRule()
     */
    void removeRule( const QString &id );

  private:

#ifdef SIP_RUN
    QgsLabelingEngineRuleRegistry( const QgsLabelingEngineRuleRegistry &other );
#endif

    std::map< QString, std::unique_ptr< QgsAbstractLabelingEngineRule > > mRules;

};

#endif // QGSLABELINGENGINERULEREGISTRY_H
