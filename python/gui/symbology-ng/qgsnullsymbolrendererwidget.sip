/** \ingroup gui
 * \class QgsNullSymbolRendererWidget
 * \brief Blank widget for customising QgsNullSymbolRenderer.
 * \note Added in version 2.16
 */

class QgsNullSymbolRendererWidget : QgsRendererWidget
{
%TypeHeaderCode
#include <qgsnullsymbolrendererwidget.h>
%End

  public:

    //! Creates a new QgsNullSymbolRendererWidget object
    static QgsRendererWidget* create( QgsVectorLayer* layer, QgsStyle* style, QgsFeatureRenderer* renderer ) /Factory/;

    //! Constructor for QgsNullSymbolRendererWidget
    QgsNullSymbolRendererWidget( QgsVectorLayer* layer, QgsStyle* style, QgsFeatureRenderer* renderer );
    ~QgsNullSymbolRendererWidget();

    //! Returns a pointer to the configured renderer
    virtual QgsFeatureRenderer* renderer();
};
