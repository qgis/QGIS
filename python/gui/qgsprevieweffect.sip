/** \ingroup gui
 * A graphics effect which can be applied to a widget to simulate various printing and
 * color blindness modes.
 */

class QgsPreviewEffect : QGraphicsEffect
{
%TypeHeaderCode
#include "qgsprevieweffect.h"
%End

  public:
    enum PreviewMode
    {
      PreviewGrayscale,
      PreviewMono,
      PreviewProtanope,
      PreviewDeuteranope
    };

    QgsPreviewEffect( QObject* parent /TransferThis/ );
    ~QgsPreviewEffect();

    /** Sets the mode for the preview effect, which controls how the effect modifies a widgets appearance.
     * @param mode PreviewMode to use to draw the widget
     * @note added in 2.3
     * @see mode
    */
    void setMode( PreviewMode mode );
    /** Returns the mode used for the preview effect.
     * @returns PreviewMode currently used by the effect
     * @note added in 2.3
     * @see setMode
    */
    PreviewMode mode() const;

  protected:
    virtual void draw( QPainter *painter );
};
