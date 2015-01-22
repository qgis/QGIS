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
#include "qgsfeature.h"

class QgsRubberBand;
class QgsMapToolSimplify;
class QgsCoordinateTransform;

class APP_EXPORT QgsSimplifyDialog : public QDialog, private Ui::SimplifyLineDialog
{
    Q_OBJECT

  public:

    QgsSimplifyDialog( QgsMapToolSimplify* tool, QWidget* parent = NULL );

  private:
    QgsMapToolSimplify* mTool;

  private slots:

};


/** Map tool to simplify line/polygon features */
class APP_EXPORT QgsMapToolSimplify: public QgsMapToolEdit
{
    Q_OBJECT
  public:
    QgsMapToolSimplify( QgsMapCanvas* canvas );
    virtual ~QgsMapToolSimplify();

    void canvasPressEvent( QMouseEvent * e ) override;

    //! called when map tool is being deactivated
    void deactivate() override;

    double tolerance() const { return mTolerance; }

    enum ToleranceUnits { LayerUnits = 0, MapUnits = 1 };

    ToleranceUnits toleranceUnits() const { return mToleranceUnits; }

  public slots:
    /** slot to change display when slidebar is moved */
    void setTolerance( double tolerance );

    void setToleranceUnits( int units );

    /** slot to store feture after simplification */
    void storeSimplified();

    void removeRubberBand();

  private:

    void updateSimplificationPreview();

    /** Function to calculate tolerance boudaries for simplifying */
    //bool calculateSliderBoudaries();

    /** Function to get list of vertexes from feature */
    QVector<QgsPoint> getPointList( QgsFeature& f );

    // data
    /** Dialog with slider to set correct tolerance value */
    QgsSimplifyDialog* mSimplifyDialog;

    /** Rubber band to draw current state of simplification */
    QgsRubberBand* mRubberBand;

    /** Feature with which we are working */
    QgsFeature mSelectedFeature;

    /** real value of tolerance */
    double mTolerance;

    ToleranceUnits mToleranceUnits;

};

/**
  Implementation of Douglas-Peucker simplification algorithm.
 */
class APP_EXPORT QgsSimplifyFeature
{
    /** structure for one entry in stack for simplification algorithm */
    struct StackEntry
    {
      int anchor;
      int floater;
    };

  public:
    /** simplify line/polygon feature with specified tolerance. Returns true on success */
    static bool simplify( QgsFeature& feature, double tolerance, QgsMapToolSimplify::ToleranceUnits units, const QgsCoordinateTransform* ctLayerToMap );

  protected:
    /** simplify a line given by a vector of points and tolerance. Returns simplified vector of points */
    static QVector<QgsPoint> simplifyPoints( const QVector<QgsPoint>& pts, double tolerance, QgsMapToolSimplify::ToleranceUnits units, const QgsCoordinateTransform* ctLayerToMap );
    /** get indices of points that should be preserved after simplification */
    static QList<int> simplifyPointsIndices( const QVector<QgsPoint>& pts, double tolerance );

};

#endif
