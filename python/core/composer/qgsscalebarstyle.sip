/** \ingroup MapComposer
 * Abstraction of composer scale bar style. Subclasses draw themselves, have the
possibility to implement custom labeling and calculate corresponding box size.
*/
class QgsScaleBarStyle
{
%TypeHeaderCode
#include "qgsscalebarstyle.h"
%End
  public:
    QgsScaleBarStyle( const QgsComposerScaleBar* bar );
    virtual ~QgsScaleBarStyle();

    /** Draws the style
     @param p painter object
     @param xOffset offset to account for centered labeling*/
    virtual void draw( QPainter* p, double xOffset = 0 ) const = 0; //to do by every subclass
    virtual void drawLabels( QPainter* p ) const; //default implementation provided
    virtual QRectF calculateBoxSize() const; //default implementation provided
    virtual QString name() const = 0; //return name of the style
};
