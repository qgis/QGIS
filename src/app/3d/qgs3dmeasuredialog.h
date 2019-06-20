#ifndef QGS3DMEASUREDIALOG_H
#define QGS3DMEASUREDIALOG_H

#include <QCloseEvent>

#include "ui_qgsmeasurebase.h"
#include "qgs3dmaptoolmeasureline.h"
#include "qgs3dmapcanvas.h"


class Qgs3DMeasureDialog : public QDialog, private Ui::QgsMeasureBase
{
    Q_OBJECT

  public:
    // Constructor
    Qgs3DMeasureDialog( Qgs3DMapToolMeasureLine *tool, Qt::WindowFlags f = nullptr );

    //! Save position
    void saveWindowLocation();

    //! Restore last window position/size
    void restorePosition();

    //! Add new point
    void addPoint();

    //! Get last distance
    double lastDistance();

  public slots:
    void reject() override;

    void restart();

    //! Close event
    void closeEvent( QCloseEvent *e ) override;

  private:
    Qgs3DMapToolMeasureLine *mTool;
//    Qgs3DMapCanvas *mCanvas;

    //! Total length
    double mTotal = 0.0;

    //! Number of decimal places we want.
    int mDecimalPlaces = 3;
};

#endif // QGS3DMEASUREDIALOG_H
