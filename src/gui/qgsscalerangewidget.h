/***************************************************************************
   qgsscalerangewidget.h
    --------------------------------------
   Date                 : 25.04.2014
   Copyright            : (C) 2014 Denis Rouzaud
   Email                : denis.rouzaud@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef QGSSCALERANGEWIDGET_H
#define QGSSCALERANGEWIDGET_H

#include <QGridLayout>
#include "qgis_sip.h"
#include <QLabel>
#include "qgis_gui.h"

class QgsMapCanvas;
class QgsScaleWidget;

/**
 * \ingroup gui
 * \class QgsScaleRangeWidget
 * A widget allowing entry of a range of map scales, e.g. minimum scale and maximum scale.
 */
class GUI_EXPORT QgsScaleRangeWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( double minimumScale READ minimumScale WRITE setMinimumScale )
    Q_PROPERTY( double maximumScale READ maximumScale WRITE setMaximumScale )

  public:

    /**
     * Constructor for QgsScaleRangeWidget.
     */
    explicit QgsScaleRangeWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets the map \a canvas which will be used for the current scale buttons.
     * If not set, the buttons are hidden.
     */
    void setMapCanvas( QgsMapCanvas *canvas );

    /**
     * Returns the selected minimum scale (i.e. most "zoomed out" scale), or 0 if minimum scale is not set.
     * The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * \see maximumScale()
     * \see setMinimumScale()
     */
    double minimumScale() const;

    /**
     * Returns the selected maximum scale (i.e. most "zoomed in" scale), or 0 if maximum scale is not set.
     * The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * \see minimumScale()
     * \see setMaximumScale()
     */
    double maximumScale() const;

    /**
     * Call to reload the preset scales from the current project and apply them to the 2 scales combo boxes.
     */
    void reloadProjectScales();

  public slots:

    /**
     * Set the minimum \a scale (i.e. most "zoomed out" scale), or 0 to indicate the minimum is not set.
     * The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * \see minimumScale()
     * \see setMaximumScale()
     * \see setScaleRange()
     */
    void setMinimumScale( double scale );

    /**
     * Set the maximum \a scale (i.e. most "zoomed in" scale), or 0 to indicate the minimum is not set.
     * The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * \see maximumScale()
     * \see setMinimumScale()
     * \see setScaleRange()
     */
    void setMaximumScale( double scale );

    /**
     * Sets the scale range, from \a min scale (i.e. most "zoomed out" scale) to \a max scale (most "zoomed in" scale).
     * The scale values indicates the scale denominator, e.g. 1000.0 for a 1:1000 map,
     * or 0 to indicate not set.
     * \see setMinimumScale()
     * \see setMaximumScale()
     */
    void setScaleRange( double min, double max );

  signals:

    /**
     * Emitted when the scale range set in the widget is changed.
     * The scale values indicates the scale denominator, e.g. 1000.0 for a 1:1000 map,
     * or 0 to indicate not set.
     * \since QGIS 2.16
     */
    void rangeChanged( double min, double max );

  private slots:

    void emitRangeChanged();

  private:
    //! pointer to the map canvas used for current buttons.
    QgsMapCanvas *mCanvas = nullptr;

    // ui
    QGridLayout *mLayout = nullptr;
    QLabel *mMaximumScaleIconLabel = nullptr;
    QLabel *mMinimumScaleIconLabel = nullptr;
    QgsScaleWidget *mMaximumScaleWidget = nullptr;
    QgsScaleWidget *mMinimumScaleWidget = nullptr;
};

#endif // QGSSCALERANGEWIDGET_H
