#ifndef QGSMAP3DEXPORTWIDGET_H
#define QGSMAP3DEXPORTWIDGET_H

#include <QWidget>

namespace Ui {
class Map3DExportWidget;
}

class Qgs3DMapScene;

class QgsMap3DExportWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit QgsMap3DExportWidget(Qgs3DMapScene* scene, QWidget *parent = nullptr);
    ~QgsMap3DExportWidget();

    void exportScene();
  private:
    Ui::Map3DExportWidget *ui;
  private:
    Qgs3DMapScene* mScene = nullptr;
};

#endif // QGSMAP3DEXPORTWIDGET_H
