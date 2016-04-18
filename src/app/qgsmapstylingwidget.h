#ifndef QGSMAPSTYLESDOCK_H
#define QGSMAPSTYLESDOCK_H

#include <QWidget>
#include <QTabWidget>
#include <QStackedWidget>
#include <QDialogButtonBox>
#include <QCheckBox>

class QgsLabelingWidget;
class QgsMapLayer;
class QgsMapCanvas;


class APP_EXPORT QgsMapStylingWidget : public QWidget
{
    Q_OBJECT
  public:
    explicit QgsMapStylingWidget( QgsMapCanvas *canvas, QWidget *parent = 0 );
    QgsMapLayer* layer() { return mCurrentLayer; }

  signals:
    void styleChanged( QgsMapLayer* layer );

  public slots:
    void setLayer( QgsMapLayer* layer );
    void apply();
    void autoApply();
    void resetSettings();

  private:
    int mNotSupportedPage;
    int mVectorPage;
    int mLabelTabIndex;
    QgsMapCanvas* mMapCanvas;
    bool mBlockAutoApply;
    QgsMapLayer* mCurrentLayer;
    QStackedWidget* mStackedWidget;
    QTabWidget *mMapStyleTabs;
    QgsLabelingWidget *mLabelingWidget;
    QDialogButtonBox* mButtonBox;
    QCheckBox* mLiveApplyCheck;

};

#endif // QGSMAPSTYLESDOCK_H
