
class QgsAnimatedIcon : QObject
{
%TypeHeaderCode
#include <qgsanimatedicon.h>
%End
  public:

    QgsAnimatedIcon( const QString &iconPath = QString::null );

    QString iconPath() const;
    void setIconPath( const QString &iconPath );
    QIcon icon() const;
    bool connectFrameChanged( const QObject *receiver, const char *method );
    bool disconnectFrameChanged( const QObject *receiver, const char *method );

    int width() const;
    int height() const;
  signals:
    void frameChanged();
};
