
/** \ingroup core
 * \class QgsColorSchemeRegistry
 * \brief Registry of color schemes
 *
 * A registry of QgsColorScheme color schemes. This class can be created directly, or
 * accessed via a global instance.
 * \note Added in version 2.5
 */
class QgsColorSchemeRegistry
{
%TypeHeaderCode
#include <qgscolorschemeregistry.h>
%End
  public:

    /** Returns the global instance pointer, creating the object on the first call.
     */
    static QgsColorSchemeRegistry * instance();

    /** Constructor for an empty color scheme registry
     */
    QgsColorSchemeRegistry();

    virtual ~QgsColorSchemeRegistry();

    /** Adds all color schemes from the global instance to this color scheme.
     * @see addDefaultSchemes
     * @see addColorScheme
     */
    void populateFromInstance();

    /** Adds all default color schemes to this color scheme.
     * @see populateFromInstance
     * @see addColorScheme
     * @see addUserSchemes
     */
    void addDefaultSchemes();

    /** Creates schemes for all gpl palettes in the user's palettes folder.
     * @see populateFromInstance
     * @see addDefaultSchemes
     * @see addColorScheme
     */
    void addUserSchemes();

    /** Adds a color scheme to the registry. Ownership of the scheme is transferred
     * to the registry.
     * @param scheme color scheme to add
     * @see populateFromInstance
     * @see removeColorScheme
    */
    void addColorScheme( QgsColorScheme* scheme /Transfer/ );

    /** Removes all matching color schemes from the registry
     * @param scheme color scheme to remove
     * @returns true if scheme was found and removed
     * @see addColorScheme
    */
    bool removeColorScheme( QgsColorScheme* scheme );

    /** Returns all color schemes in the registry
     * @returns list of color schemes
    */
    QList<QgsColorScheme *> schemes() const;

    /** Returns all color schemes in the registry which have a specified flag set
     * @param flag flag to match
     * @returns list of color schemes with flag set
    */
    QList<QgsColorScheme *> schemes( const QgsColorScheme::SchemeFlag flag ) const;

    /** Return color schemes of a specific type
     * @param schemeList destination list for matching schemes
     * @note not available in python bindings
    */
    // template<class T> void schemes( QList<T*>& schemeList );
};
