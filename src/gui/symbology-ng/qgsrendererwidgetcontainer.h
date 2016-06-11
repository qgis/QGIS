/***************************************************************************
    qgsrendererwidgetcontainer.h
    ---------------------
    begin                : June 2016
    copyright            : (C) 2016 by Nathan Woodrow
    email                :
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
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
    /**
     * @brief Overriden key press event to handle the esc event on the widget.
     * @param event The key event
     */
    void keyPressEvent( QKeyEvent* event );

  private:
    QWidget* mWidget;

};

#endif // QGSRENDERERWIDGETCONTAINER_H
