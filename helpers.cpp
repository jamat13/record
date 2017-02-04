#include <QtGui>
#include <QModelIndex>
#include <QVariant>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include "helpers.h"
#include "record.h"

HelpersDialog::HelpersDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    model = new QStandardItemModel (0, 5, this); //0 Rows and 5 Columns
    record = qobject_cast<Record*>(parent);

    connect (buttonQuit,   SIGNAL (clicked()), this, SLOT(doAccept ()));
    connect (buttonAdd,    SIGNAL (clicked()), this, SLOT(doAdd ()));
    connect (buttonDelete, SIGNAL (clicked()), this, SLOT(doDelete ()));
    connect (tableView,     SIGNAL (clicked(const QModelIndex)), this,
							SLOT (rowSelected (const QModelIndex)));

    tableView->setSelectionMode (QAbstractItemView::SingleSelection);
    tableView->setSelectionBehavior (QAbstractItemView::SelectRows);

    buttonSubmit->setEnabled (false);
    buttonDelete->setEnabled (false);

    showHelpers ();
}

/*-------------------------------------------------------------------------------------------------
 * doAccept --
 *-----------------------------------------------------------------------------------------------*/
void HelpersDialog::doAccept ()
{
    record->showRecord ();
    accept ();
}

/*-------------------------------------------------------------------------------------------------
 * rowSelected --
 *-----------------------------------------------------------------------------------------------*/
void HelpersDialog::rowSelected (const QModelIndex index)
{
QModelIndex indx;
QVariant cell;

    buttonSubmit->setEnabled (true);
    buttonDelete->setEnabled (true);
    //pushButtonSubmit->setDefault (true); actionAdd->setShortcut (Qt::Key_Return);

    int row = index.row ();

    /*----------------------------------------------------------------------------------------------
     * Get the values from selected line and save (won't need so don't save hours)
     *--------------------------------------------------------------------------------------------*/
    indx = index.model()->index (row, 0);
    cell = indx.model()->data (indx, Qt::DisplayRole);
    lineEditName->setText (cell.toString());

    indx = index.model()->index (row, 1);
    cell = indx.model()->data (indx, Qt::DisplayRole);
    lineEditDayRate->setText (cell.toString());

    indx = index.model()->index (row, 2);
    cell = indx.model()->data (indx, Qt::DisplayRole);
    lineEditWeekend->setText (cell.toString ());

    indx = index.model()->index (row, 3);
    cell = indx.model()->data (indx, Qt::DisplayRole);
    selectedId = cell.toString();
}

/*-------------------------------------------------------------------------------------------------
 * on_buttonSubmit_pressed --
 *-----------------------------------------------------------------------------------------------*/
void HelpersDialog::on_buttonSubmit_pressed ()
{
QString sql;
QSqlQuery query;

    if (lineEditName->text().isEmpty() || lineEditDayRate->text().isEmpty() ||
					  lineEditWeekend->text().isEmpty()) {
        QMessageBox::warning(this, tr ("record"), tr ("You must fill all fields"));
	return;
    }

    sql = "update helpers set name=\"" + lineEditName->text () + "\", "
	  "dayrate=" + lineEditDayRate->text () + ", "
	  "werate=" + lineEditWeekend->text () +
	  " where ID=" + selectedId + ";";

    if (!query.exec (sql))
	sqlError (sql, query.lastError ());
    showHelpers ();

    buttonSubmit->setEnabled (false);
    buttonDelete->setEnabled (false);
    lineEditName->deselect ();
    lineEditDayRate->deselect ();
    lineEditWeekend->deselect ();
}

/*-------------------------------------------------------------------------------------------------
 * showHelpers--
 *	tableView from ui
 *-----------------------------------------------------------------------------------------------*/
void HelpersDialog::showHelpers ()
{
QString sql;
QSqlQuery query;
QStandardItem *itm;
QList <QStandardItem *>nextRow;

    sql= "select ID, name, dayrate, werate from helpers;";
    if (!query.exec (sql))
	sqlError (sql, query.lastError ());

    model->clear ();
    model->setHorizontalHeaderItem(0, new QStandardItem(QString("Name")));
    model->setHorizontalHeaderItem(1, new QStandardItem(QString("DayRate")));
    model->setHorizontalHeaderItem(2, new QStandardItem(QString("WeekEnd")));
    model->setHorizontalHeaderItem(3, new QStandardItem(QString("ID")));
 
    tableView->setModel(model);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableView->setColumnHidden (3, true);
    tableView->verticalHeader()->hide();

    while (query.next()) {
	nextRow.clear ();

	QString name  = query.value(1).toString();
	itm = new QStandardItem (name);
	nextRow.append (itm);

	QString dayrate = query.value(2).toString();
	itm = new QStandardItem (dayrate);
	nextRow.append (itm);

	QString weekend = query.value(3).toString();
	itm = new QStandardItem (weekend);
	nextRow.append (itm);

	QString ID = query.value(0).toString();
	itm = new QStandardItem (ID);
	nextRow.append (itm);

	model->appendRow (nextRow);
    }
}

/*-------------------------------------------------------------------------------------------------
 * doAdd
 *-----------------------------------------------------------------------------------------------*/
void HelpersDialog::doAdd ()
{
QString sql;
QSqlQuery query;

    if (lineEditName->text().isEmpty() || lineEditDayRate->text().isEmpty() ||
					  lineEditWeekend->text().isEmpty()) {
	QMessageBox::warning(this, tr ("record"),
                tr ("To add a helper both name and $ rates must be supplied"));
    }
    else {
	sql = "insert into helpers values (NULL, "
	      "\"" + lineEditName->text() + "\", "
	      "\"" + lineEditDayRate->text() + "\", "
	      "\"" + lineEditWeekend->text() + "\");";

	if (!query.exec (sql))
	    sqlError (sql, query.lastError ());
    }
    showHelpers ();
}

/*-------------------------------------------------------------------------------------------------
 * doDelete
 *-----------------------------------------------------------------------------------------------*/
void HelpersDialog::doDelete ()
{
    QString sql = "delete from helpers where ID=" + selectedId + ";";

    QSqlQuery query;
    if (!query.exec (sql))
	sqlError (sql, query.lastError ());

    buttonSubmit->setEnabled (false);
    buttonDelete->setEnabled (false);
    showHelpers ();
}

/*-------------------------------------------------------------------------------------------------
 * sqlError --
 *-----------------------------------------------------------------------------------------------*/
void HelpersDialog::sqlError (QString sql, QSqlError err)
{
    QMessageBox::warning(this, tr ("record"), tr ("Error:") + sql + "\n" + err.text());
}

/*-------------------------------------------------------------------------------------------------
 * formatWindow --
 *-----------------------------------------------------------------------------------------------*/
void HelpersDialog::formatWindow (int newWidth)
{
}

/*-------------------------------------------------------------------------------------------------
 * screenWidthChanged --
 *-----------------------------------------------------------------------------------------------*/
void HelpersDialog::screenWidthChanged (int newWidth)
{
    formatWindow (newWidth);
}
