/**
 * @brief A container widget that can be used to show a renderer widget with a title and close button.
 * @note Mainly used for the style dock panels at the moment.
 */
class QgsRendererWidgetContainer : QWidget
{
%TypeHeaderCode
#include "qgsrendererwidgetcontainer.h"
%End
  public:
    /**
     * @brief A container widget that can be used to show a renderer widget with a title and close button.
     * @param widget The internal widget to be shown to the user.
     * @param title The title to show on the widget.
     * @param parent The parent of the widget.
     */
    QgsRendererWidgetContainer( QWidget* widget, const QString &title, QWidget *parent = 0 );

    /**
     * @brief Returns the current internal widget.
     * @return The internal widget.
     */
    QWidget* widget();

  signals:
    /**
      * @brief Emitted when the container is accpeted and closed.
      * Listen to this to clean up the callers state.
      */
    void accepted();

  protected:
    void keyPressEvent( QKeyEvent* event );
};
