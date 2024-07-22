/***************************************************************************
                             qgseffectstack.h
                             ----------------
    begin                : December 2014
    copyright            : (C) 2014 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSEFFECTSTACK_H
#define QGSEFFECTSTACK_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgspainteffect.h"

/**
 * \ingroup core
 * \class QgsEffectStack
 * \brief A paint effect which consists of a stack of other chained paint effects
 *
 * Effect stacks can be used to apply multiple paint effects to a QPicture. For
 * instance, an effect stack may blur then apply a drop shadow.
 *
 * The way in which effects apply to a stack is controlled by the effect's drawMode.
 * Effects can either render their results onto the destination paint device,
 * or just modify the source picture which is drawn by subsequent effects in the
 * stack. For instance, a blur effect with a Modifier drawMode will blur the source
 * picture for the following drop shadow effect without actually drawing the blurred
 * picture to the paint device. If the blur effect had a Render drawMode then the
 * blurred picture will be drawn on the paint device, but the following drop shadow
 * effect will be drawn using the original picture, not the blurred version.
 *
 */

class CORE_EXPORT QgsEffectStack : public QgsPaintEffect SIP_NODEFAULTCTORS
{

  public:

    /**
     * Creates a new QgsEffectStack effect. This method ignores
     * the map parameter, and always returns an empty effect stack.
     * \param map unused encoded properties string map
     * \returns new QgsEffectStack
     */
    static QgsPaintEffect *create( const QVariantMap &map ) SIP_FACTORY;

    QgsEffectStack() = default;

    QgsEffectStack( const QgsEffectStack &other );

    /**
     * Move constructor.
     */
    QgsEffectStack( QgsEffectStack &&other ) SIP_SKIP;

    /**
     * Creates a new QgsEffectStack effect from a single initial effect.
     * \param effect initial effect to add to the stack. The effect will
     * be cloned, so ownership is not transferred to the stack.
     */
    explicit QgsEffectStack( const QgsPaintEffect &effect );

    ~QgsEffectStack() override;

    QString type() const override { return QStringLiteral( "effectStack" ); }
    QgsEffectStack *clone() const override SIP_FACTORY;
    bool saveProperties( QDomDocument &doc, QDomElement &element ) const override;
    bool readProperties( const QDomElement &element ) override;

    /**
     * Unused for QgsEffectStack, will always return an empty string map
     */
    QVariantMap properties() const override;

    /**
     * Unused for QgsEffectStack, props parameter will be ignored
     */
    void readProperties( const QVariantMap &props ) override;

    /**
     * Appends an effect to the end of the stack.
     * \param effect QgsPaintEffect to append. Ownership of the effect will be
     * transferred to the stack object.
     * \see insertEffect
     */
    void appendEffect( QgsPaintEffect *effect SIP_TRANSFER );

    /**
     * Inserts an effect at a specified index within the stack.
     * \param index position to insert the effect
     * \param effect QgsPaintEffect to insert. Ownership of the effect will be
     * transferred to the stack object.
     * \see appendEffect
     */
    bool insertEffect( int index, QgsPaintEffect *effect SIP_TRANSFER );

    /**
     * Replaces the effect at a specified position within the stack.
     * \param index position of effect to replace
     * \param effect QgsPaintEffect to replace with. Ownership of the effect will be
     * transferred to the stack object.
     */
    bool changeEffect( int index, QgsPaintEffect *effect SIP_TRANSFER );

    /**
     * Removes an effect from the stack and returns a pointer to it.
     * \param index position of effect to take
     */
    QgsPaintEffect *takeEffect( int index SIP_TRANSFERBACK );

    /**
     * Returns a pointer to the list of effects currently contained by
     * the stack
     * \returns list of QgsPaintEffects within the stack
     */
    QList< QgsPaintEffect * > *effectList();

    /**
     * Returns count of effects contained by the stack
     * \returns count of effects
     */
    int count() const { return mEffectList.count(); }

    /**
     * Returns a pointer to the effect at a specified index within the stack
     * \param index position of effect to return
     * \returns QgsPaintEffect at specified position
     */
    QgsPaintEffect *effect( int index ) const;

    QgsEffectStack &operator=( const QgsEffectStack &rhs );

    QgsEffectStack &operator=( QgsEffectStack &&other ) SIP_SKIP;


  protected:

    void draw( QgsRenderContext &context ) override;

  private:

    QList< QgsPaintEffect * > mEffectList;

    void clearStack();
};

#endif // QGSEFFECTSTACK_H

