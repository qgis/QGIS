/***************************************************************************
  qgsdirectionallightwidget.h - QgsDirectionalLightWidget

 ---------------------
 begin                : 11.12.2022
 copyright            : (C) 2022 by Vincent Cloarec
 email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDIRECTIONALLIGHTWIDGET_H
#define QGSDIRECTIONALLIGHTWIDGET_H

#include <QWidget>
#include "ui_qgsdirectionallightwidget.h"
#include "qgis_gui.h"

/**
 * \ingroup gui
 * \class QgsDirectionalLightWidget
 *
 * \brief Widget for direction light settings
 *
 * The user can choose azimuth and altitude values.
 *
 * \since QGIS 3.30
 */
class GUI_EXPORT QgsDirectionalLightWidget : public QWidget, private Ui::QgsDirectionalLightWidget
{
    Q_OBJECT

  public:
    //! Constructor
    explicit QgsDirectionalLightWidget( QWidget *parent = nullptr );
    ~QgsDirectionalLightWidget();

    //! Sets the \a azimuth value (degree)
    void setAzimuth( double azimuth );

    //! Returns the \a azimuth value (degree)
    double azimuth() const;

    //! Sets the \a altitude value (degree)
    void setAltitude( double altitude );

    //! Returns the \a altitude value (degree)
    double altitude() const;

    //! Sets whether the azimut can be changed, for example, when using multidirectional light
    void setEnableAzimuth( bool enable );

  signals:
    //! Emitted when the direction is changed
    void directionChanged();

  private:
};

#endif // QGSDIRECTIONALLIGHTWIDGET_H
