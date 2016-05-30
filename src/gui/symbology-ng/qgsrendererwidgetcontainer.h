#ifndef QGSRENDERERWIDGETCONTAINER_H
#define QGSRENDERERWIDGETCONTAINER_H

#include <QWidget>
#include <QKeyEvent>

#include "ui_qgsrenderercontainerbase.h"


/**
 * @brief A container widget that can be used to show a renderer widget with a title and close button.
 * @note Mainly used for the style dock panels at the moment.
 */
class GUI_EXPORT QgsRendererWidgetContainer : public QWidget, private Ui::QgsRendererWidgetContainerBase
{
    Q_OBJECT
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

  public slots:

  protected:
    void keyPressEvent( QKeyEvent* event );

};

#endif // QGSRENDERERWIDGETCONTAINER_H
