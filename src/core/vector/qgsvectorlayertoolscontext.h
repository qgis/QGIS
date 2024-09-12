/***************************************************************************
    qgsvectorlayertoolscontext.h
    ------------------------
    begin                : May 2024
    copyright            : (C) 2024 by Mathieu Pellerin
    email                : mathieu at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORLAYERTOOLSCONTEXT_H
#define QGSVECTORLAYERTOOLSCONTEXT_H

#include "qgsexpressioncontext.h"
#include "qgis_core.h"

#include <memory>

/**
 * \ingroup core
 * \class QgsVectorLayerToolsContext
 * \brief Contains settings which reflect the context in which vector layer tool operations should
 * consider.
 * \since QGIS 3.38
 */
class CORE_EXPORT QgsVectorLayerToolsContext
{
  public:

    QgsVectorLayerToolsContext() = default;

    QgsVectorLayerToolsContext( const QgsVectorLayerToolsContext &other );
    QgsVectorLayerToolsContext &operator=( const QgsVectorLayerToolsContext &other );

    /**
     * Sets the optional expression context used by the vector layer tools.
     * \param context expression context pointer. Ownership is not transferred.
     * \see expressionContext()
     * \see setAdditionalExpressionContextScope()
     */
    void setExpressionContext( const QgsExpressionContext *context );

    /**
     * Returns the optional expression context used by the vector layer tools.
     * \see setExpressionContext()
     * \see additionalExpressionContextScope()
     */
    QgsExpressionContext *expressionContext() const;

    /**
     * Sets an additional expression context scope to be made available when calculating expressions.
     * \param scope additional scope. Ownership is not transferred and a copy will be made.
     * \see additionalExpressionContextScope()
     */
    void setAdditionalExpressionContextScope( const QgsExpressionContextScope *scope );

    /**
     * Returns an additional expression context scope to be made available when calculating expressions.
     * \see setAdditionalExpressionContextScope()
     */
    const QgsExpressionContextScope *additionalExpressionContextScope() const;

    /**
     * Returns the widget which should be parented to tools dialogues.
     */
    QWidget *parentWidget() const { return mParentWidget; }

    /**
     * Sets the widget which should be parented to tools' dialogues.
     * \param parent the widget actign as parent
     */
    void setParentWidget( QWidget *parent ) { mParentWidget = parent; }

    /**
     * Returns whether tools' dialogues should be modal.
     */
    bool showModal() const { return mShowModal; }

    /**
     * Sets whether tools' dialogues should be modal.
     */
    void setShowModal( bool modal ) { mShowModal = modal; }

    /**
     * Returns whether the parent widget should be hidden when showing tools' dialogues.
     */
    bool hideParent() const { return mHideParent; }

    /**
     * Sets whether the parent widget should be hidden when showing tools' dialogues.
     */
    void setHideParent( bool hide ) { mHideParent = hide; }

  private:

    std::unique_ptr< QgsExpressionContext > mExpressionContext;
    std::unique_ptr< QgsExpressionContextScope > mAdditionalExpressionContextScope;

    QWidget *mParentWidget = nullptr;
    bool mShowModal = true;
    bool mHideParent = false;
};

#endif // QGSVECTORLAYERTOOLSCONTEXT_H
