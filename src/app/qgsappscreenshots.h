#ifndef QGSAPPSCREENSHOTS_H
#define QGSAPPSCREENSHOTS_H

#include <QObject>

class QScreen;
class QgsVectorLayer;

class QgsAppScreenShots
{
    Q_GADGET
  public:
    enum GrabMode
    {
      GrabWidget,
      GrabWidgetAndFrame,
      GrabWholeWindow
    };
    Q_ENUM( GrabMode )

    enum Reference
    {
      Widget,
      QgisApp,
      Screen
    };

    enum Category
    {
      VectorLayerProperties = 1,
    };
    Q_ENUM( Category )
    Q_DECLARE_FLAGS( Categories, Category )
    Q_FLAG( Categories )

    QgsAppScreenShots( const QString &saveDirectory );

    //! if categories is null, then takes all categories
    void takeScreenshots( Categories categories = nullptr );

  private:
    QScreen *screen( QWidget *widget = nullptr );
    void moveWidgetTo( QWidget *widget, Qt::Corner corner, Reference reference = Screen );
    void saveScreenshot( const QString &name, QWidget *widget = nullptr, GrabMode mode = GrabWidgetAndFrame );

    void takeVectorLayerProperties();

    QString mSaveDirectory;
    QgsVectorLayer *mVectorLayer = nullptr;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsAppScreenShots::Categories )

#endif // QGSAPPSCREENSHOTS_H
