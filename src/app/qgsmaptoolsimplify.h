/***************************************************************************
    qgsmaptoolsimplify.h  - simplify vector layer features
    ---------------------
    begin                : April 2009
    copyright            : (C) 2009 by Richard Kostecky
    email                : csf dot kostej at mail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLSIMPLIFY_H
#define QGSMAPTOOLSIMPLIFY_H

#include "qgsmaptooledit.h"
#include "ui_qgssimplifytolerancedialog.h"

#include <QVector>
#include "qgstolerance.h"
#include "qgsgeometry.h"
#include "qgis_app.h"

class QgsRubberBand;
class QgsMapToolSimplify;
class QgsCoordinateTransform;
class QgsSimplifyUserInputWidget;


//! Map tool to simplify line/polygon features
class APP_EXPORT QgsMapToolSimplify: public QgsMapToolEdit
{
    Q_OBJECT
  public:

    enum Method
    {
      SimplifyDistance    = 0,
      SimplifySnapToGrid  = 1,
      SimplifyVisvalingam = 2,
      Smooth = 3
    };

    QgsMapToolSimplify( QgsMapCanvas *canvas );
    ~QgsMapToolSimplify() override;

    void canvasPressEvent( QgsMapMouseEvent *e ) override;
    void canvasMoveEvent( QgsMapMouseEvent *e ) override;
    void canvasReleaseEvent( QgsMapMouseEvent *e ) override;
    void keyReleaseEvent( QKeyEvent *e ) override;

    //! called when map tool is being deactivated
    void deactivate() override;

    double tolerance() const { return mTolerance; }

    QgsTolerance::UnitType toleranceUnits() const { return mToleranceUnits; }

    QString statusText() const;

    Method method() const;

    int smoothIterations() const;
    void setSmoothIterations( int smoothIterations );

    double smoothOffset() const;
    void setSmoothOffset( double smoothOffset );

  public slots:
    //! Slot to change display when slidebar is moved
    void setTolerance( double tolerance );

    void setToleranceUnits( QgsTolerance::UnitType units );

    //! Slot to store feature after simplification
    void storeSimplified();

    void clearSelection();

    void setMethod( Method method );

  private:

    void selectOneFeature( QPoint canvasPoint );
    void selectFeaturesInRect();

    void updateSimplificationPreview();

    void createUserInputWidget();

    /**
     * Simplifies a \a geometry to the specified \a tolerance, respecting the preset
     * simplification method.
     */
    QgsGeometry processGeometry( const QgsGeometry &geometry, double tolerance ) const;

    // data
    //! Dialog with slider to set correct tolerance value
    QgsSimplifyUserInputWidget *mSimplifyUserWidget = nullptr;

    //! Rubber bands to draw current state of simplification
    QList<QgsRubberBand *> mRubberBands;
    //! Features with which we are working
    QList<QgsFeature> mSelectedFeatures;

    //! Real value of tolerance
    double mTolerance = 1.0;

    QgsTolerance::UnitType mToleranceUnits = QgsTolerance::LayerUnits;

    //! stores actual selection rect
    QRect mSelectionRect;
    //! shows actual selection rect
    QgsRubberBand *mSelectionRubberBand = nullptr;
    //! Flag to indicate a map canvas drag operation is taking place
    bool mDragging = false;

    int mOriginalVertexCount = 0;
    int mReducedVertexCount = 0;
    bool mReducedHasErrors = false;

    Method mMethod = SimplifyDistance;

    int mSmoothIterations = 1;
    double mSmoothOffset = 0.25;
};


class APP_EXPORT QgsSimplifyUserInputWidget : public QWidget, private Ui::SimplifyUserInputWidgetBase
{
    Q_OBJECT

  public:

    QgsSimplifyUserInputWidget( QWidget *parent = nullptr );

    void updateStatusText( const QString &text );
    void enableOkButton( bool enabled );

    void setConfig( const QgsMapToolSimplify::Method &method, const double &tolerance,
                    const QgsTolerance::UnitType &units, const double &smoothOffset,
                    const int &smoothIterations );

  signals:
    void accepted();
    void rejected();
    void toleranceChanged( double tolerance );
    void toleranceUnitsChanged( QgsTolerance::UnitType units );
    void methodChanged( QgsMapToolSimplify::Method method );
    void smoothOffsetChanged( double offset );
    void smoothIterationsChanged( int iterations );

  protected:
    bool eventFilter( QObject *object, QEvent *ev ) override;
    void keyReleaseEvent( QKeyEvent *event ) override;
};

#endif
