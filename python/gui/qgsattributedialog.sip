class QgsAttributeDialog : QObject
{
%TypeHeaderCode
#include <qgsattributedialog.h>
%End

    public:
      QgsAttributeDialog( QgsVectorLayer *vl, QgsFeature *thepFeature, bool featureOwner, QgsDistanceArea myDa, QWidget* parent = 0, bool showDialogButtons = true ) /Deprecated/;
      QgsAttributeDialog( QgsVectorLayer *vl, QgsFeature *thepFeature, bool featureOwner, QWidget* parent = 0, bool showDialogButtons = true );
      void saveGeometry();
      void restoreGeometry();
      QDialog *dialog();
      const QgsFeature* feature();

    public slots:
      void accept();
      int exec();
      void show();
      void dialogDestroyed();

private:
      bool eventFilter( QObject *obj, QEvent *event );
};
