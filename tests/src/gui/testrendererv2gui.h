#ifndef TESTRENDERERV2GUI_H
#define TESTRENDERERV2GUI_H

#include <QMainWindow>

class QgsMapCanvas;

class TestRendererV2GUI : public QMainWindow
{
    Q_OBJECT
public:
    explicit TestRendererV2GUI(QWidget *parent = 0);
    void loadLayers();

signals:

public slots:
    void setRenderer();

protected:
  QgsMapCanvas* mMapCanvas;
};

#endif // TESTRENDERERV2GUI_H
