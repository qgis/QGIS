#include "qgsmap3dexportwidget.h"
#include "ui_map3dexportwidget.h"

#include <QPushButton>
#include <QFileDialog>

#include "qgs3dmapscene.h"
#include "qgssettings.h"

QgsMap3DExportWidget::QgsMap3DExportWidget(Qgs3DMapScene* scene, QWidget *parent) :
  QWidget(parent),
  ui(new Ui::Map3DExportWidget),
  mScene(scene)
{
  ui->setupUi(this);

  connect(ui->selectFolderBtn, &QPushButton::clicked, [=](bool ) {
    QgsSettings settings;  // where we keep last used filter in persistent state
    QString initialPath = settings.value( QStringLiteral( "UI/lastExportAsDir" ), QDir::homePath() ).toString();
    QString outputDir = QFileDialog::getExistingDirectory(this, QString("Export scane"), initialPath);
    ui->folderNameLineEdit->setText(outputDir);
  });
}

QgsMap3DExportWidget::~QgsMap3DExportWidget()
{
  delete ui;
}

void QgsMap3DExportWidget::exportScene() {
  QString sceneName = ui->sceneNameLineEdit->text();
  QString sceneFolder = ui->folderNameLineEdit->text();
  bool smoothEdges = ui->smoothEdgesCheckBox->isChecked();
  mScene->exportScene(sceneName, sceneFolder, smoothEdges);
}
