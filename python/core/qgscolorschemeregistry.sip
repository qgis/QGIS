
/** \ingroup core
 * \class QgsColorSchemeRegistry
 * \brief Registry of color schemes
 *
 * A registry of QgsColorScheme color schemes. This class can be created directly, or
 * accessed via a global instance.
 */
class QgsColorSchemeRegistry
{
%TypeHeaderCode
#include <qgscolorschemeregistry.h>
%End
	public:

    /**Returns the global instance pointer, creating the object on the first call.
     * @note added in QGIS 2.5
     */
    static QgsColorSchemeRegistry * instance();

    /**Constructor for an empty color scheme registry
     * @note added in QGIS 2.5
     */
    QgsColorSchemeRegistry();

    virtual ~QgsColorSchemeRegistry();

    /**Adds all color schemes from the global instance to this color scheme.
     * @note added in QGIS 2.5
     * @see addDefaultSchemes
     * @see addColorScheme
     */
    void populateFromInstance();

    /**Adds all default color schemes to this color scheme.
     * @note added in QGIS 2.5
     * @see populateFromInstance
     * @see addColorScheme
     */
    void addDefaultSchemes();

    /**Adds a color scheme to the registry. Ownership of the scheme is transferred
     * to the registry.
     * @param scheme color scheme to add
     * @note added in QGIS 2.5
     * @see populateFromInstance
     * @see removeColorScheme
    */
    void addColorScheme( QgsColorScheme* scheme /Transfer/ );

    /**Removes all matching color schemes from the registry
     * @param scheme color scheme to remove
     * @returns true if scheme was found and removed
     * @note added in QGIS 2.5
     * @see addColorScheme
    */
    bool removeColorScheme( QgsColorScheme* scheme );

    /**Returns all color schemes in the registry
     * @returns list of color schemes
     * @note added in QGIS 2.5
    */
    QList<QgsColorScheme *> schemes() const;
	
}; // class QgsColorSchemeRegistry
