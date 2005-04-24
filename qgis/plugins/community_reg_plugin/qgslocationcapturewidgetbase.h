/****************************************************************************
** Form interface generated from reading ui file 'qgslocationcapturewidgetbase.ui'
**
** Created: Sun Apr 24 15:11:25 2005
**      by: The User Interface Compiler ($Id$)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef QGSLOCATIONCAPTUREWIDGETBASE_H
#define QGSLOCATIONCAPTUREWIDGETBASE_H

#include <qvariant.h>
#include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QgsMapCanvas;
class QPushButton;
class QLabel;

class QgsLocationCaptureWidgetBase : public QWidget
{
    Q_OBJECT

public:
    QgsLocationCaptureWidgetBase( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~QgsLocationCaptureWidgetBase();

    QgsMapCanvas* qgsMapCanvas;
    QPushButton* pbnZoomIn;
    QPushButton* pbnPan;
    QPushButton* pbnZoomOut;
    QPushButton* pbnCapturePos;
    QPushButton* pushButton5;
    QLabel* lblCapturePos;
    QLabel* lblCurrentPos;

public slots:
    virtual void qgsMapCanvas_xyClickCoordinates( QgsPoint & );
    virtual void qgsMapCanvas_xyCoordinates( QgsPoint & );

protected:
    QGridLayout* QgsLocationCaptureWidgetBaseLayout;

protected slots:
    virtual void languageChange();

};

#endif // QGSLOCATIONCAPTUREWIDGETBASE_H
