void QgsAttributeTableBase::closeEvent(QCloseEvent* ev)
{
    ev->ignore();
    emit deleted();
    delete this;
}
