#ifndef QGS3DMEASUREDIALOG_H
#define QGS3DMEASUREDIALOG_H

#include <QCloseEvent>

#include "ui_qgsmeasurebase.h"
#include "qgs3dmaptoolmeasureline.h"
#include "qgs3dmapcanvas.h"
#include "qgsunittypes.h"


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

    //! update UI
    void updateUi();

    //! Populating unit combo box
    void repopulateComboBoxUnits();

  public slots:
    void reject() override;

    void restart();

    //! Close event
    void closeEvent( QCloseEvent *e ) override;

  private slots:
    void unitsChanged( int index );

  private:
    Qgs3DMapToolMeasureLine *mTool;
//    Qgs3DMapCanvas *mCanvas;

    //! Total length
    double mTotal = 0.0;

    //! Number of decimal places we want.
    int mDecimalPlaces = 3;

    //! Indicates whether the user chose "Map units" instead of directly selecting a unit
    bool mUseMapUnits = true;

    //! Current unit for distance values
    QgsUnitTypes::DistanceUnit mDistanceUnits  = QgsUnitTypes::DistanceUnknownUnit;

    //! Current map unit for distance values
    QgsUnitTypes::DistanceUnit mMapDistanceUnits  = QgsUnitTypes::DistanceUnknownUnit;

};

#endif // QGS3DMEASUREDIALOG_H
