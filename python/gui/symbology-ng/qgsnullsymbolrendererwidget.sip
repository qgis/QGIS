/** \ingroup gui
 * \class QgsNullSymbolRendererWidget
 * \brief Blank widget for customising QgsNullSymbolRenderer.
 * \note Added in version 2.16
 */

class QgsNullSymbolRendererWidget : QgsRendererV2Widget
{
%TypeHeaderCode
#include <qgsnullsymbolrendererwidget.h>
%End

  public:

    //! Creates a new QgsNullSymbolRendererWidget object
    static QgsRendererV2Widget* create( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer ) /Factory/;

    //! Constructor for QgsNullSymbolRendererWidget
    QgsNullSymbolRendererWidget( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer );
    ~QgsNullSymbolRendererWidget();

    //! Returns a pointer to the configured renderer
    virtual QgsFeatureRendererV2* renderer();
};
