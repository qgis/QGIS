/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/


void QgsPgQueryBuilderBase::insEqual()
{
txtSQL->insert(" = ");
}
void QgsPgQueryBuilderBase::insLt()
{
txtSQL->insert(" < ");
}
void QgsPgQueryBuilderBase::insGt()
{
txtSQL->insert(" > ");
}

void QgsPgQueryBuilderBase::insPct()
{
txtSQL->insert(" % ");
}
void QgsPgQueryBuilderBase::insIn()
{
txtSQL->insert(" IN ");
}
void QgsPgQueryBuilderBase::insNotIn()
{
txtSQL->insert(" NOT IN ");
}
void QgsPgQueryBuilderBase::insLike()
{
txtSQL->insert(" LIKE ");
}

QString QgsPgQueryBuilderBase::sql()
{
  return txtSQL->text();
}


void QgsPgQueryBuilderBase::setSql( QString sqlStatement)
{
    txtSQL->setText(sqlStatement);
}


void QgsPgQueryBuilderBase::testSql()
{

}

void QgsPgQueryBuilderBase::getAllValues()
{
}

void QgsPgQueryBuilderBase::getSampleValues()
{
}


void QgsPgQueryBuilderBase::fieldDoubleClick( QListBoxItem *item )
{
    txtSQL->insert(item->text());
}


void QgsPgQueryBuilderBase::valueDoubleClick( QListBoxItem *item )
{
     txtSQL->insert(item->text());
}


void QgsPgQueryBuilderBase::insLessThanEqual()
{
    txtSQL->insert(" <= ");
}


void QgsPgQueryBuilderBase::insGreaterThanEqual()
{
    txtSQL->insert(" >= ");
}


void QgsPgQueryBuilderBase::insNotEqual()
{
    txtSQL->insert(" != ");
}


void QgsPgQueryBuilderBase::insAnd()
{
    txtSQL->insert(" AND ");
}


void QgsPgQueryBuilderBase::insNot()
{
    txtSQL->insert(" NOT ");
}


void QgsPgQueryBuilderBase::insOr()
{
    txtSQL->insert(" OR ");
}


void QgsPgQueryBuilderBase::clearSQL()
{
    txtSQL->clear();
}


void QgsPgQueryBuilderBase::insIlike()
{
    txtSQL->insert(" ILIKE ");
}
void QgsPgQueryBuilderBase::setDatasourceDescription(QString uri)
{
    lblDataUri->setText(uri);
}
