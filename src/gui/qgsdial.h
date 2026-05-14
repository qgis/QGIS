/***************************************************************************
                             qgsdial.h
                             -------------------
    begin                : July 2013
    copyright            : (C) 2013 by Daniel Vaz
    email                : danielvaz at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDIAL_H
#define QGSDIAL_H

#include "qgis_gui.h"
#include "qgis_sip.h"

#include <QDial>
#include <QVariant>

class QPaintEvent;

/**
 * \ingroup gui
 * \class QgsDial
 * \brief A QDial subclass with additional refinements.
 */
class GUI_EXPORT QgsDial : public QDial
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsDial
     * \param parent parent object
     */
    QgsDial( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets the dial range's minimum value.
     */
    void setMinimum( const QVariant &min );

    /**
     * Returns the dial range's minimum value.
     * \since QGIS 4.0
     */
    QVariant minimum() const { return mMin; };

    /**
     * Sets the dial range's maximum value.
     */
    void setMaximum( const QVariant &max );

    /**
     * Returns the dial range's maximum value.
     * \since QGIS 4.0
     */
    QVariant maximum() const { return mMax; };

    /**
     * Sets the dial's single step value.
     */
    void setSingleStep( const QVariant &step );

    /**
     * Returns the dial's single step value.
     */
    QVariant singleStep() const { return mStep; };

    void setValue( const QVariant &value );
    QVariant variantValue() const;

  signals:
    void valueChanged( const QVariant & );

  private slots:
    void onValueChanged( int );

  protected:
    void paintEvent( QPaintEvent *event ) override;

  private:
    void update();

    QVariant mMin, mMax, mStep, mValue;
};

#endif // QGSDIAL_H
