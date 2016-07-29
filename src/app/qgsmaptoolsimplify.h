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
#include "qgstolerance.h"

class QgsRubberBand;
class QgsMapToolSimplify;
class QgsCoordinateTransform;

class APP_EXPORT QgsSimplifyDialog : public QDialog, private Ui::SimplifyLineDialog
{
    Q_OBJECT

  public:

    QgsSimplifyDialog( QgsMapToolSimplify* tool, QWidget* parent = nullptr );

    void updateStatusText();
    void enableOkButton( bool enabled );

  protected:

    //! Also cancels pending simplification
    virtual void closeEvent( QCloseEvent* e ) override;

  private:
    QgsMapToolSimplify* mTool;

};


/** Map tool to simplify line/polygon features */
class APP_EXPORT QgsMapToolSimplify: public QgsMapToolEdit
{
    Q_OBJECT
  public:
    QgsMapToolSimplify( QgsMapCanvas* canvas );
    virtual ~QgsMapToolSimplify();

    void canvasPressEvent( QgsMapMouseEvent* e ) override;
    void canvasMoveEvent( QgsMapMouseEvent* e ) override;
    void canvasReleaseEvent( QgsMapMouseEvent* e ) override;

    //! called when map tool is being deactivated
    void deactivate() override;

    double tolerance() const { return mTolerance; }

    QgsTolerance::UnitType toleranceUnits() const { return mToleranceUnits; }

    QString statusText() const;

  public slots:
    /** Slot to change display when slidebar is moved */
    void setTolerance( double tolerance );

    void setToleranceUnits( int units );

    /** Slot to store feture after simplification */
    void storeSimplified();

    void clearSelection();

  private:

    void selectOneFeature( QPoint canvasPoint );
    void selectFeaturesInRect();

    void updateSimplificationPreview();

    int vertexCount( const QgsGeometry *g ) const;

    // data
    /** Dialog with slider to set correct tolerance value */
    QgsSimplifyDialog* mSimplifyDialog;

    /** Rubber bands to draw current state of simplification */
    QList<QgsRubberBand*> mRubberBands;
    /** Features with which we are working */
    QList<QgsFeature> mSelectedFeatures;

    /** Real value of tolerance */
    double mTolerance;

    QgsTolerance::UnitType mToleranceUnits;

    //! stores actual selection rect
    QRect mSelectionRect;
    //! shows actual selection rect
    QgsRubberBand* mSelectionRubberBand;
    //! Flag to indicate a map canvas drag operation is taking place
    bool mDragging;

    int mOriginalVertexCount;
    int mReducedVertexCount;
    bool mReducedHasErrors;
};

#endif
