#ifndef QGSFIXATTRIBUTEDIALOG_H
#define QGSFIXATTRIBUTEDIALOG_H

#include "qgsattributeeditorcontext.h"
#include "qgis_sip.h"
#include "qgsattributeform.h"
#include "qgstrackedvectorlayertools.h"

#include <QDialog>
#include <QGridLayout>
#include <QProgressBar>
#include "qgis_gui.h"

/**
* \ingroup gui
* \class QgsFixAttributeDialog
* \since 3.12
*/
class GUI_EXPORT QgsFixAttributeDialog : public QDialog
{
    Q_OBJECT

  public:

    enum Feedback
    {
      VanishAll,
      CopyValid,
      CopyAll
    };

    QgsFixAttributeDialog( QgsVectorLayer *vl, QgsFeatureList &features, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /*
     * returns fixed features
     */
    QgsFeatureList fixedFeatures() { return mFixedFeatures; }

    /*
     * returns unfixed features
     */
    QgsFeatureList unfixedFeatures() { return mUnfixedFeatures; }

  public slots:
    void accept() override;
    void reject() override;

  private:
    void init( QgsVectorLayer *layer );
    QString descriptionText();

    QgsFeatureList mFeatures;
    QgsFeatureList::iterator currentFeature;

    //the fixed features
    QgsFeatureList mFixedFeatures;
    //the not yet fixed and the canceled features
    QgsFeatureList mUnfixedFeatures;

    QgsAttributeForm *mAttributeForm = nullptr;
    QProgressBar *mProgressBar = nullptr;
    QLabel *mDescription = nullptr;
};

#endif // QGSFIXATTRIBUTEDIALOG_H

