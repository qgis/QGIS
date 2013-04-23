class QgsAttributeDialog : QObject
{
%TypeHeaderCode
#include <qgsattributedialog.h>
%End

    public:
      QgsAttributeDialog( QgsVectorLayer *vl, QgsFeature *thepFeature, bool featureOwner, QgsDistanceArea myDa, QWidget* parent = 0, bool showDialogButtons = true );
      void saveGeometry();
      void restoreGeometry();
      QDialog *dialog();
      QgsFeature* feature();

    public slots:
      void accept();
      int exec();
      void show();
      void dialogDestroyed();
};
