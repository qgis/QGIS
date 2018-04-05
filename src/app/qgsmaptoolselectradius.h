/***************************************************************************
qgsmaptoolselectradius.h  -  map tool for selecting features by radius
---------------------
begin                : May 2010
copyright            : (C) 2010 by Jeremy Palmer
email                : jpalmer at linz dot govt dot nz
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef QGSMAPTOOLSELECTRADIUS_H
#define QGSMAPTOOLSELECTRADIUS_H


#include "qgsmaptool.h"
#include "qgspointxy.h"
#include "qgis_app.h"

class QHBoxLayout;

class QgsDoubleSpinBox;
class QgsMapCanvas;
class QgsRubberBand;
class QgsSnapIndicator;
class QgsMapToolSelectionHandler;

class APP_EXPORT QgsDistanceWidget : public QWidget
{
    Q_OBJECT

  public:

    explicit QgsDistanceWidget( const QString &label = QString(), QWidget *parent = nullptr );

    void setDistance( double distance );

    double distance();

    QgsDoubleSpinBox *editor() {return mDistanceSpinBox;}

  signals:
    void distanceChanged( double distance );
    void distanceEditingFinished( double distance, const Qt::KeyboardModifiers &modifiers );
    void distanceEditingCanceled();

  protected:
    bool eventFilter( QObject *obj, QEvent *ev ) override;

  private slots:
    void distanceSpinBoxValueChanged( double distance );

  private:
    QHBoxLayout *mLayout = nullptr;
    QgsDoubleSpinBox *mDistanceSpinBox = nullptr;
};


class APP_EXPORT QgsMapToolSelectRadius : public QgsMapTool
{
    Q_OBJECT
  public:
    QgsMapToolSelectRadius( QgsMapCanvas *canvas );

    ~QgsMapToolSelectRadius() override;

    //! Overridden mouse move event
    void canvasMoveEvent( QgsMapMouseEvent *e ) override;

    //! Overridden mouse release event
    void canvasReleaseEvent( QgsMapMouseEvent *e ) override;

    //! called when map tool is being deactivated
    void deactivate() override;

    //! catch escape when active to cancel selection
    void keyReleaseEvent( QKeyEvent *e ) override;

  private slots:
    //! update the rubber band from the input widget
    void updateRubberband( const double &radius );

    /**
     * triggered when the user input widget has a new value
     * either programmatically (from the mouse event) or entered by the user
     */
    void radiusValueEntered( const double &radius, const Qt::KeyboardModifiers &modifiers );

    //! cancel selecting (between two click events)
    void cancel();

  private:
    //! perform selection using radius from rubberband
    void selectFromRubberband( const Qt::KeyboardModifiers &modifiers );

    //! on mouse events update the rubber band using the mouse position
    void updateRadiusFromEdge( QgsPointXY &radiusEdge );

    void deleteRubberband();
    void deleteRotationWidget();
    void createRotationWidget();

    void createRubberBand();

    //! used for storing all of the maps point for the polygon
    std::unique_ptr< QgsRubberBand > mRubberBand;
    std::unique_ptr<QgsSnapIndicator> mSnapIndicator;

    //! Center point for the radius
    QgsPointXY mRadiusCenter;

    bool mActive = false;

    QColor mFillColor = QColor( 254, 178, 76, 63 );
    QColor mStrokeColor = QColor( 254, 58, 29, 100 );

    //! Shows current angle value and allows numerical editing
    QgsDistanceWidget *mDistanceWidget = nullptr;

    QgsMapToolSelectionHandler *mSelectionHandler;
};

#endif
