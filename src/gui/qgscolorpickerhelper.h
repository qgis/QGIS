/***************************************************************************
     qgscolorpickerhelper.h
     ---------------
    Date                 : November 2026
    Copyright            : (C) 2026 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCOLORPICKERHELPER_H
#define QGSCOLORPICKERHELPER_H

#include "qgis_gui.h"
#include "qgis_sip.h"

#include <QColor>
#include <QObject>
#include <QPoint>
#include <QPointer>

#define SIP_NO_FILE

class QWidget;
class QScreen;
class QWindow;
class QTimer;


/**
 * \ingroup gui
 * \class QgsColorPickerHelper
 * \brief A utility class for sampling colors from the screen.
 *
 * \note Not available in Python bindings.
 *
 * \since QGIS 4.4
 */
class GUI_EXPORT QgsColorPickerHelper : public QObject
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsColorPickerHelper for the specified parent \a widget.
     */
    QgsColorPickerHelper( QObject *parent SIP_TRANSFERTHIS );
    ~QgsColorPickerHelper() override;

  public slots:

    /**
     * Activates the color picking operation.
     */
    void activate( int sampleRadius = 0 );

  signals:

    /**
     * Emitted when the user has finished picking a \a color from the screen.
     */
    void colorPicked( const QColor &color );

    /**
     * Emitted when the user hovers over a \a color from the screen.
     */
    void colorHovered( const QColor &color );

    /**
     * Emitted when the user cancels the picking operation.
     */
    void canceled();

  protected:
    bool eventFilter( QObject *watched, QEvent *event ) override;
  private slots:

    void hoverSampleTimeout();

  private:
    static QWidget *findEventWindowWidgetAndLocalPos( QObject *watched, const QPointF &eventPos, QPoint &outPos );

    void onColorPicked( const QColor &color );
    void cancelPicking();
    void stopPicking();

    QColor sampleColor( QWidget *widget, const QPoint &pos ) const;

    static QColor averageColor( const QImage &image );

    bool mPickingColor = false;
    QColor mCurrentPickingColor;
    QColor mFinalColor;

    QTimer *mSampleTimer = nullptr;
    QPointer<QWidget> mPendingTargetWidget;
    QPoint mPendingLocalPos;

    QList< QPointer< QWidget > > mWidgetsToCleanup;

    int mSampleRadius = 0;
};


#endif // QGSCOLORPICKERHELPER_H
