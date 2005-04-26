void QgsAttributeTableBase::closeEvent(QCloseEvent* ev)
{
    ev->ignore();
    emit deleted();
    delete this;
}


void QgsAttributeTableBase::deleteAttributes()
{

}


void QgsAttributeTableBase::addAttribute()
{

}


void QgsAttributeTableBase::startEditing()
{

}


void QgsAttributeTableBase::stopEditing()
{

}


void QgsAttributeTableBase::invertSelection()
{

}


void QgsAttributeTableBase::selectedToTop()
{

}
