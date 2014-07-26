
/** \ingroup core
 * \class QgsColorScheme
 * \brief Abstract base class for color schemes
 *
 * A color scheme for display in QgsColorButtonV2. Color schemes return lists
 * of colors with an optional associated color name. The colors returned
 * can be generated using an optional base color.
 */
class QgsColorScheme
{
%TypeHeaderCode
#include <qgscolorscheme.h>
%End

    public:

    QgsColorScheme();

    virtual ~QgsColorScheme();

    /**Gets the name for the color scheme
     * @returns color scheme name
     * @note added in QGIS 2.5
    */
    virtual QString schemeName() const = 0;

    /**Gets a list of colors from the scheme. The colors can optionally
     * be generated using the supplied context and base color.
     * @param context string specifiying an optional context for the returned
     * colors. For instance, a "recent colors" scheme may filter returned colors
     * by context so that colors used only in a "composer" context are returned.
     * @param baseColor base color for the scheme's colors. Some color schemes
     * may take advantage of this to filter or modify their returned colors
     * to colors related to the base color.
     * @returns a list of QPairs of color and color name
     * @note added in QGIS 2.5
    */
    virtual QList< QPair< QColor, QString > > fetchColors( const QString context = QString(),
        const QColor baseColor = QColor() ) = 0;

    virtual QgsColorScheme* clone() const = 0;
	
}; // class QgsColorScheme

/** \ingroup core
 * \class QgsRecentColorScheme
 * \brief A color scheme which contains the most recently used colors.
 */
class QgsRecentColorScheme : QgsColorScheme
{
%TypeHeaderCode
#include <qgscolorscheme.h>
%End

  public:

    QgsRecentColorScheme();

    virtual ~QgsRecentColorScheme();

    virtual QString schemeName() const;

    virtual QList< QPair< QColor, QString > > fetchColors( const QString context = QString(),
        const QColor baseColor = QColor() );

    QgsColorScheme* clone() const;
}; // class QgsRecentColorScheme
