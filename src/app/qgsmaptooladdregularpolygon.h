#ifndef QGSMAPTOOLADDREGULARPOLYGON_H
#define QGSMAPTOOLADDREGULARPOLYGON_H

#include "qgsregularpolygon.h"
#include "qgsmaptoolcapture.h"
#include "qspinbox.h"

class QSpinBox;

class QgsMapToolAddRegularPolygon: public QgsMapToolCapture
{
    Q_OBJECT
    void clean();
  public:
    QgsMapToolAddRegularPolygon( QgsMapToolCapture *parentTool, QgsMapCanvas *canvas, CaptureMode mode = CaptureLine );
    ~QgsMapToolAddRegularPolygon();

    void keyPressEvent( QKeyEvent *e ) override;
    void keyReleaseEvent( QKeyEvent *e ) override;

    void deactivate() override;

    void activate() override;

  private slots:
    void setParentTool( QgsMapTool *newTool, QgsMapTool *oldTool );

  protected:
    explicit QgsMapToolAddRegularPolygon( QgsMapCanvas *canvas ); //forbidden

    QSpinBox *mNumberSidesSpinBox = nullptr;
    int mNumberSides;

    //! (re-)create the spin box to enter the number of sides
    void createNumberSidesSpinBox();
    //! delete the spin box to enter the number of sides, if it exists
    void deleteNumberSidesSpinBox();

    /** The parent map tool, e.g. the add feature tool.
     *  Completed regular polygon will be added to this tool by calling its addCurve() method.
     * */
    QgsMapToolCapture *mParentTool = nullptr;
    //! Regular Shape points (in map coordinates)
    QgsPointSequence mPoints;
    //! The rubberband to show the regular polygon currently working on
    QgsGeometryRubberBand *mTempRubberBand = nullptr;
    //! Regular shape as a regular polygon
    QgsRegularPolygon mRegularPolygon;

};

#endif // QGSMAPTOOLADDREGULARPOLYGON_H
