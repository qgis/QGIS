/** \ingroup core
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
 * \note Added in version 2.9
 */

class QgsEffectStack : QgsPaintEffect
{
%TypeHeaderCode
#include <qgseffectstack.h>
%End

  public:

    /** Creates a new QgsEffectStack effect. This method ignores
     * the map parameter, and always returns an empty effect stack.
     * @param map unused encoded properties string map
     * @returns new QgsEffectStack
     */
    static QgsPaintEffect* create( const QgsStringMap& map ) /Factory/;

    QgsEffectStack();
    QgsEffectStack( const QgsEffectStack& other );

    /** Creates a new QgsEffectStack effect from a single initial effect.
     * @param effect initial effect to add to the stack. The effect will
     * be cloned, so ownership is not transferred to the stack.
     * @returns new QgsEffectStack containing initial effect
     */
    explicit QgsEffectStack( const QgsPaintEffect& effect );

    virtual ~QgsEffectStack();

    virtual QString type() const;
    virtual QgsPaintEffect* clone() const /Factory/;
    virtual bool saveProperties( QDomDocument& doc, QDomElement& element ) const;
    virtual bool readProperties( const QDomElement& element );

    /** Unused for QgsEffectStack, will always return an empty string map
     */
    virtual QgsStringMap properties() const;

    /** Unused for QgsEffectStack, props parameter will be ignored
     */
    virtual void readProperties( const QgsStringMap& props );

    /** Appends an effect to the end of the stack.
     * @param effect QgsPaintEffect to append. Ownership of the effect will be
     * transferred to the stack object.
     * @see insertEffect
     */
    void appendEffect( QgsPaintEffect* effect /Transfer/);

    /** Inserts an effect at a specified index within the stack.
     * @param index position to insert the effect
     * @param effect QgsPaintEffect to insert. Ownership of the effect will be
     * transferred to the stack object.
     * @see appendEffect
     */
    bool insertEffect( const int index, QgsPaintEffect* effect /Transfer/);

    /** Replaces the effect at a speficied position within the stack.
     * @param index position of effect to replace
     * @param effect QgsPaintEffect to replace with. Ownership of the effect will be
     * transferred to the stack object.
     */
    bool changeEffect( const int index, QgsPaintEffect *effect /Transfer/);

    /** Removes an effect from the stack and returns a pointer to it.
     * @param index position of effect to take
     */
    QgsPaintEffect* takeEffect( const int index );
    
    /** Returns a pointer to the list of effects currently contained by
     * the stack
     * @returns list of QgsPaintEffects within the stack
     */
    QList< QgsPaintEffect* >* effectList();
    
    /** Returns count of effects contained by the stack
     * @returns count of effects
     */
    int count() const;

    /** Returns a pointer to the effect at a specified index within the stack
     * @param index position of effect to return
     * @returns QgsPaintEffect at specified position
     */
    QgsPaintEffect* effect( int index ) const;

  protected:

    virtual void draw( QgsRenderContext& context );

};
