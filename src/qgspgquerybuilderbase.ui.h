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

void QgsPgQueryBuilderBase::insNot()
{
txtSQL->insert(" ! ");
}
void QgsPgQueryBuilderBase::insPct()
{
txtSQL->insert(" % ");
}
void QgsPgQueryBuilderBase::insIn()
{
txtSQL->insert(" in ");
}
void QgsPgQueryBuilderBase::insNotIn()
{
txtSQL->insert(" not in ");
}
void QgsPgQueryBuilderBase::insLike()
{
txtSQL->insert(" like ");
}

QString QgsPgQueryBuilderBase::sql()
{
  return txtSQL->text();
}


void QgsPgQueryBuilderBase::setSql( QString sqlStatement)
{
    txtSQL->setText(sqlStatement);
}


void QgsPgQueryBuilderBase::testSqll()
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
