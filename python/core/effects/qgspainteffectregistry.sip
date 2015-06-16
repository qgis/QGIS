class QgsPaintEffectWidget /External/;

/** \ingroup core
 * \class QgsPaintEffectAbstractMetadata
 * \brief Stores metadata about a paint effect class.
 *
 * \note It's necessary to implement the createPaintEffect() function.
 *  In C++ you can use the QgsPaintEffectMetadata convenience class to
 * simplify creation of the metadata.
 *
 * \note Added in version 2.9
 */

class QgsPaintEffectAbstractMetadata
{
%TypeHeaderCode
#include <qgspainteffectregistry.h>
%End

  public:

    /** Construct a new QgsPaintEffectAbstractMetadata
     * @param name unique string representing paint effect class
     * @param visibleName user visible name representing paint effect class
     */
    QgsPaintEffectAbstractMetadata( const QString& name, const QString& visibleName );

    virtual ~QgsPaintEffectAbstractMetadata();

    /** Returns the unique string representing the paint effect class
     * @returns unique string
     * @see visibleName
     */
    QString name() const;

    /** Returns the user visible string representing the paint effect class
     * @returns friendly user visible string
     * @see name
     */
    QString visibleName() const;

    /** Create a paint effect of this class given an encoded map of properties.
     * @param map properties string map
     * @returns new paint effect
    */
    virtual QgsPaintEffect* createPaintEffect( const QgsStringMap& map ) = 0 /Factory/;

    /** Create configuration widget for paint effect of this class. Can return NULL
     * if there's no GUI for the paint effect class.
     * @returns configuration widget
    */
    virtual QgsPaintEffectWidget* createWidget() /Factory/;

};


/** \ingroup core
 * \class QgsPaintEffectRegistry
 * \brief Singleton registry of available paint effects
 *
 * \note Added in version 2.9
 */

class QgsPaintEffectRegistry
{
%TypeHeaderCode
#include <qgspainteffectregistry.h>
%End

  public:

    /** Returns the metadata for a specific effect.
     * @param name unique string name for paint effect class
     * @returns paint effect metadata if found, otherwise NULL
     */
    QgsPaintEffectAbstractMetadata* effectMetadata( const QString& name ) const;

    /** Registers a new effect type.
     * @param metadata effect metadata. Ownership is transferred to the registry.
     * @returns true if add was successful.
     */
    bool addEffectType( QgsPaintEffectAbstractMetadata* metadata /Transfer/ );

    /** Creates a new paint effect given the effect name and properties map.
     * @param name unique name representing paint effect class
     * @param properties encoded string map of effect properties
     * @returns new paint effect of specified class, or NULL if matching
     * paint effect could not be created
     */
    QgsPaintEffect* createEffect( const QString& name, const QgsStringMap& properties = QgsStringMap() ) const /Factory/;

    /** Creates a new paint effect given a DOM element storing paint effect
     * properties.
     * @param element encoded DOM element of effect properties
     * @returns new paint effect, or NULL if matching
     * paint effect could not be created
     */
    QgsPaintEffect* createEffect( const QDomElement& element ) const /Factory/;

    /** Returns a list of known paint effects.
     * @returns list of paint effect names
     */
    QStringList effects() const;

    /** Returns a new effect stack consisting of a sensible selection of default
     * effects. All effects except the standard draw source effect are disabled,
     * but are included so that they can be easily drawn just by enabling the effect.
     * @returns default effects stack
     */
    static QgsPaintEffect* defaultStack() /Factory/;

  protected:
    QgsPaintEffectRegistry();
    ~QgsPaintEffectRegistry();


};

