#ifndef QGSEXPRESSIONGUIHELPER_H
#define QGSEXPRESSIONGUIHELPER_H

#include <QObject>
#include <QAbstractButton>
#include <QComboBox>
#include <QStringList>

#include "qgsvectorlayer.h"

class GUI_EXPORT QgsExpressionGuiHelper : public QObject
{
    Q_OBJECT
  public:
    /**
     * @brief QgsExpressionGuiHelper is a helper class to populate a combo with the field of a layer and allows to add expressions by clicking on a button
     * @param layer the layer used for fields and expressions
     * @param fieldCombo the QCombobox to be populated
     * @param expressionButton the button which triggers the expression builder dialg
     * @param title the title to be shown in the dialog
     * @param parent
     */
    QgsExpressionGuiHelper( QgsVectorLayer* layer, QComboBox *fieldCombo, QAbstractButton *expressionButton, QString title, QWidget *parent = 0 );

    /**
     * @brief setExpression sets the str expression as current item in combobox. If str was not in current items, it is automatically added
     */
    void setExpression( const QString str );


  public slots:
    void showExpressionDialog();
    /**
     * @brief changeLayer changes the layer will repopulate the fields
     * @param layer if layer=0, this will empty the field combobox
     */
    void changeLayer( QgsVectorLayer *layer = 0 );

    void setGeomCalculator( const QgsDistanceArea &da );

  private:
    QgsVectorLayer* mLayer;
    QComboBox* mFieldcombo;
    QAbstractButton* mExpressionButton;
    QString mTitle;
    QWidget* mParent;
    QStringList mExpressions;
    QgsDistanceArea mDa;

    void populateFieldCombo();

  private slots:
    void attributeAdded( int idx );
    void attributeDeleted( int idx );
};

#endif // QGSEXPRESSIONGUIHELPER_H
