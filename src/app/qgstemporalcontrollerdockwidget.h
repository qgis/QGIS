/***************************************************************************
                         qgstemporalcontrollerdockwidget.h
                         ---------------
    begin                : February 2020
    copyright            : (C) 2020 by Samweli Mwakisambwe
    email                : samweli at kartoza dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTEMPORALCONTROLLERDOCKWIDGET_H
#define QGSTEMPORALCONTROLLERDOCKWIDGET_H

#include "qgsdockwidget.h"
#include "qgis_app.h"

class QgsTemporalControllerWidget;
class QgsTemporalController;
class QgsMapCanvas;

/**
 * \ingroup app
 * \brief The QgsTemporalControllerDockWidget class
 *
 * \since QGIS 3.14
 */
class APP_EXPORT QgsTemporalControllerDockWidget : public QgsDockWidget
{
    Q_OBJECT
  public:
    /**
      * Constructor for QgsTemporalControllerDockWidget, with the specified \a parent widget.
      */
    QgsTemporalControllerDockWidget( const QString &name, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the temporal controller object used by this object in navigation.
     *
     * The dock widget retains ownership of the returned object.
     */
    QgsTemporalController *temporalController();

    void setMapCanvas( QgsMapCanvas *canvas );

  protected:
    bool eventFilter( QObject *object, QEvent *event ) override;

  private slots:

    void exportAnimation();

  private:
    QgsTemporalControllerWidget *mControllerWidget = nullptr;
};

#endif // QGSTEMPORALCONTROLLERDOCKWIDGET_H
