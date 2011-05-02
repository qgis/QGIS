#ifndef QGSCOLORRAMPCOMBOBOX_H
#define QGSCOLORRAMPCOMBOBOX_H

#include <QComboBox>

class QgsStyleV2;
class QgsVectorColorRampV2;

class QgsColorRampComboBox : public QComboBox
{
    Q_OBJECT
  public:
    explicit QgsColorRampComboBox( QWidget *parent = 0 );

    ~QgsColorRampComboBox();

    //! initialize the combo box with color ramps from the style
    void populate( QgsStyleV2* style );

    //! add/select color ramp which was used previously by the renderer
    void setSourceColorRamp( QgsVectorColorRampV2* sourceRamp );

    //! return new instance of the current color ramp or NULL if there is no active color ramp
    QgsVectorColorRampV2* currentColorRamp();

    static QSize rampIconSize;

  signals:

  public slots:

    void colorRampChanged( int index );

  protected:
    QgsStyleV2* mStyle;
    QgsVectorColorRampV2* mSourceColorRamp; // owns the copy
};

#endif // QGSCOLORRAMPCOMBOBOX_H
