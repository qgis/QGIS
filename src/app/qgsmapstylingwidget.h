#ifndef QGSMAPSTYLESDOCK_H
#define QGSMAPSTYLESDOCK_H

#include <QToolButton>
#include <QWidget>
#include <QLabel>
#include <QTabWidget>
#include <QStackedWidget>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QUndoCommand>
#include <QDomNode>
#include <QTimer>

#include "ui_qgsmapstylingwidgetbase.h"

class QgsLabelingWidget;
class QgsMapLayer;
class QgsMapCanvas;
class QgsRendererV2PropertiesDialog;
class QgsRendererRasterPropertiesWidget;
class QgsUndoWidget;
class QgsRasterHistogramWidget;

class APP_EXPORT QgsMapLayerStyleCommand : public QUndoCommand
{
  public:
    QgsMapLayerStyleCommand( QgsMapLayer* layer, const QDomNode& current, const QDomNode& last );

    virtual void undo() override;
    virtual void redo() override;

  private:
    QgsMapLayer* mLayer;
    QDomNode mXml;
    QDomNode mLastState;
};

class APP_EXPORT QgsMapStylingWidget : public QWidget, private Ui::QgsMapStylingWidget
{
    Q_OBJECT
  public:
    QgsMapStylingWidget( QgsMapCanvas *canvas, QWidget *parent = 0 );
    QgsMapLayer* layer() { return mCurrentLayer; }

  signals:
    void styleChanged( QgsMapLayer* layer );

  public slots:
    void setLayer( QgsMapLayer* layer );
    void apply();
    void autoApply();

  private slots:
    void updateCurrentWidgetLayer();
    void layerAboutToBeRemoved( QgsMapLayer* layer );

  private:
    void pushUndoItem( const QString& name );
    int mNotSupportedPage;
    int mLayerPage;
    int mVectorStyleTabIndex;
    int mVectorLabelTabIndex;
    int mRasterStyleTabIndex;
    int mRasterTransTabIndex;
    int mRasterHistogramTabIndex;
    QTimer* mAutoApplyTimer;
    QDomNode mLastStyleXml;
    QgsMapCanvas* mMapCanvas;
    bool mBlockAutoApply;
    QgsUndoWidget* mUndoWidget;
    QgsMapLayer* mCurrentLayer;
    QgsLabelingWidget *mLabelingWidget;
    QgsRendererV2PropertiesDialog* mVectorStyleWidget;
    QgsRendererRasterPropertiesWidget* mRasterStyleWidget;
};

#endif // QGSMAPSTYLESDOCK_H
