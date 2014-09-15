
class QgsRelationReferenceWidget : QWidget
{
%TypeHeaderCode
#include <qgsrelationreferencewidget.h>
%End

  public:
    enum CanvasExtent
    {
      Fixed,
      Pan,
      Scale
    };

    explicit QgsRelationReferenceWidget( QWidget* parent /TransferThis/ );

    ~QgsRelationReferenceWidget();

    void setRelation( QgsRelation relation , bool allowNullValue );

    void setRelationEditable( bool editable );

    //! this sets the related feature using from the foreign key
    void setForeignKey( const QVariant &value );

    //! returns the related feature foreign key
    QVariant foreignKey();

    void setEditorContext( QgsAttributeEditorContext context, QgsMapCanvas* canvas, QgsMessageBar* messageBar );

    bool embedForm();
    void setEmbedForm( bool display );

    bool readOnlySelector();
    void setReadOnlySelector( bool readOnly );

    bool allowMapIdentification();
    void setAllowMapIdentification( bool allowMapIdentification );

    bool openFormButtonVisible();
    void setOpenFormButtonVisible( bool openFormButtonVisible );

  protected:
    virtual void showEvent( QShowEvent* e );

    void init();

   signals:
    void foreignKeyChanged( QVariant );
};
