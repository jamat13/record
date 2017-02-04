#include <QtGui>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include "night.h"

NightDialog::NightDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    QString sql = "select start, end, rate from nite;";
    QSqlQuery query;

    if (!query.exec (sql))
	sqlError (sql, query.lastError ());

    while (query.next()) {
        lineEditStart->setText (query.value(0).toString());
        lineEditEnd->setText   (query.value(1).toString());
        lineEditRate->setText  (query.value(2).toString());
    }
    empty = lineEditStart->text ().isEmpty ();
}

/*-------------------------------------------------------------------------------------------------
 * on_pushButtonSubmit_pressed --
 *-----------------------------------------------------------------------------------------------*/
void NightDialog::on_pushButtonSubmit_pressed ()
{
    if (lineEditStart->text().isEmpty() || lineEditEnd->text().isEmpty() ||
					  lineEditRate->text().isEmpty()) {
        QMessageBox::warning(this, tr ("record"), tr ("You must fill all fields"));
	return;
    }

    QString sql;
    if (empty)
	sql = "insert into nite values ('" + lineEditStart->text () + "', "
	      "'" + lineEditEnd->text () + "', "
	      "" + lineEditRate->text () +
	      ");";
    else
	sql = "update nite set start='" + lineEditStart->text () + "', "
	      "end='" + lineEditEnd->text () + "', "
	      "rate=" + lineEditRate->text () +
	      ";";
    QSqlQuery query;

qDebug () << "[sql]" << sql;
    if (!query.exec (sql))
	sqlError (sql, query.lastError ());

    accept ();
}

/*-------------------------------------------------------------------------------------------------
 * on_pushButtonQuit_pressed --
 *-----------------------------------------------------------------------------------------------*/
void NightDialog::on_pushButtonQuit_pressed ()
{
    reject ();
}

/*-------------------------------------------------------------------------------------------------
 * sqlError --
 *-----------------------------------------------------------------------------------------------*/
void NightDialog::sqlError (QString sql, QSqlError err)
{
    QMessageBox::warning(this, tr ("record"), tr ("Error:") + sql + "\n" + err.text());
}
