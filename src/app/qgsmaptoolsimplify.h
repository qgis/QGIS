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


class QgsSimplifyDialog : public QDialog, private Ui::SimplifyLineDialog
{
  Q_OBJECT

public:
  QgsSimplifyDialog( QWidget* parent = NULL );

  void setRange(int minValue, int maxValue);

signals:
  void toleranceChanged( int tol );
  void storeSimplified();

private slots:
  void valueChanged( int value );
  void simplify();
};


/**Map tool to add vertices to line/polygon features*/
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

  int calculateDivider(double num);

  void calculateSliderBoudaries();

  QVector<QgsPoint> getPointList(QgsFeature& f);

  // data

  QgsSimplifyDialog* mSimplifyDialog;

  QgsRubberBand* mRubberBand;

  QgsFeature mSelectedFeature;

  int toleranceDivider;

  double mTolerance;

private slots:
  void toleranceChanged(int tolerance);
  void storeSimplified();

};

/**
  Implementation of Douglas-Peucker simplification algorithm.
 */
class QgsSimplifyFeature
{
   struct StackEntry {
     int anchor;
     int floater;
  };

public:
  /** simplify line feature with specified tolerance. Returns TRUE on success */
  static bool simplifyLine(QgsFeature &lineFeature, double tolerance);
  /** simplify a part of line feature specified by range of vertices with given tolerance. Returns TRUE on success */
  static bool simplifyPartOfLine(QgsFeature &lineFeature, int fromVertexNr, int toVertexNr, double tolerance);
  /** simplify a line given by a vector of points and tolerance. Returns simplified vector of points */
  static QVector<QgsPoint> simplifyPoints (const QVector<QgsPoint>& pts, double tolerance);


};

#endif
