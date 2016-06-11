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
    void accepted( QgsRendererWidgetContainer* container );

    /**
     * Emiited when the internal widget changes state.
     * @param conatiner The container holding the widget that changed state.
     */
    void widgetChanged( QgsRendererWidgetContainer* conatiner );

  public slots:

    /**
     * Accept the container. Causes accepted to be emiited.
     */
    void accept();

    /**
     * Fire the widgetChanged event on the container. Connect your widgets dirty signal to
     * this slot to fire the and listen to widgetChanged to handle the event.
     */
    void emitWidgetChanged();


  protected:
    void keyPressEvent( QKeyEvent* event );
};
