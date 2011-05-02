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
/* $Id$ */

#ifndef QGSMAPTOOLSIMPLIFY_H
#define QGSMAPTOOLSIMPLIFY_H

#include "qgsmaptooledit.h"
#include "ui_qgssimplifytolerancedialog.h"

#include <QVector>
#include "qgsfeature.h"

class QgsRubberBand;


class QgsSimplifyDialog : public QDialog, private Ui::SimplifyLineDialog
{
    Q_OBJECT

  public:

    QgsSimplifyDialog( QWidget* parent = NULL );

    /** Setting range of slide bar */
    void setRange( int minValue, int maxValue );

  signals:
    /** Signal when slidebar is moved */
    void toleranceChanged( int tol );

    /** Signal to accept changes */
    void storeSimplified();

  private slots:
    /** Internal signal when value is changed */
    void valueChanged( int value );
    /** Internal signal to store simplified feature */
    void simplify();
};


/** Map tool to simplify line/polygon features */
class QgsMapToolSimplify: public QgsMapToolEdit
{
    Q_OBJECT

  public:
    QgsMapToolSimplify( QgsMapCanvas* canvas );
    virtual ~QgsMapToolSimplify();

    void canvasPressEvent( QMouseEvent * e );

    //! called when map tool is being deactivated
    void deactivate();

  public slots:
    void removeRubberBand();

  private:
    /** Divider calculation, because slider can go only by whole numbers */
    int calculateDivider( double minimum, double maximum );

    /** Function to calculate tolerance boudaries for simplifying */
    bool calculateSliderBoudaries();

    /** Function to get list of vertexes from feature */
    QVector<QgsPoint> getPointList( QgsFeature& f );

    // data
    /** Dialog with slider to set correct tolerance value */
    QgsSimplifyDialog* mSimplifyDialog;

    /** Rubber band to draw current state of simplification */
    QgsRubberBand* mRubberBand;

    /** Feature with which we are working */
    QgsFeature mSelectedFeature;

    /** tolerance divider is value which tells with which delete value from sidebar */
    long toleranceDivider;

    /** real value of tolerance */
    double mTolerance;

  private slots:
    /** slot to change display when slidebar is moved */
    void toleranceChanged( int tolerance );

    /** slot to store feture after simplification */
    void storeSimplified();

};

/**
  Implementation of Douglas-Peucker simplification algorithm.
 */
class QgsSimplifyFeature
{
    /** structure for one entry in stack for simplification algorithm */
    struct StackEntry
    {
      int anchor;
      int floater;
    };

  public:
    /** simplify line feature with specified tolerance. Returns true on success */
    static bool simplifyLine( QgsFeature &lineFeature, double tolerance );
    /** simplify polygon feature with specified tolerance. Returns true on success */
    static bool simplifyPolygon( QgsFeature &polygonFeature, double tolerance );
    /** simplify a line given by a vector of points and tolerance. Returns simplified vector of points */
    static QVector<QgsPoint> simplifyPoints( const QVector<QgsPoint>& pts, double tolerance );


};

#endif
