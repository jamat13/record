#include <QtGui>
#include <QModelIndex>
#include <QVariant>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include "regular.h"

RegularDialog::RegularDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    model = new QStandardItemModel (0, 5, this); //0 Rows and 5 Columns

    tableView->setSelectionMode (QAbstractItemView::SingleSelection);
    tableView->setSelectionBehavior (QAbstractItemView::SelectRows);

    pushButtonSubmit->setEnabled (false);
    pushButtonDel->setEnabled (false);

    connect (tableView, SIGNAL (clicked(const QModelIndex)), this, SLOT (rowSelected (const QModelIndex)));
    //connect (pushButtonDelete, SIGNAL (pressed ()), this, SLOT (on_pushButtonDelete_pressed ())); 
    showRegular ();
}

/*-----------------------------------------------------------------------------------------------------------
 * on_pushButtonAdd_pressed --
 *---------------------------------------------------------------------------------------------------------*/
void RegularDialog::on_pushButtonAdd_pressed ()
{

    if (lineEditName->text().isEmpty() || lineEditStart->text().isEmpty() ||
        lineEditEnd->text().isEmpty()  || lineEditDay->text ().isEmpty ()) {
	QMessageBox::warning(this, tr ("record"),
                tr ("To add a regular helper all fields must be supplied"));
    }
    else {
	QString sql = "insert into regular values (NULL, "
	      "'" + lineEditName->text() + "', "
	      "'" + lineEditDay->text() + "', "
	      "'" + lineEditStart->text() + "', "
	      "'" + lineEditEnd->text() + "');";
	QSqlQuery query;
	if (!query.exec (sql))
	    sqlError (sql, query.lastError ());
    }
    showRegular ();

}

/*-------------------------------------------------------------------------------------------------
 * on_pushButtonDel_pressed --
 *-----------------------------------------------------------------------------------------------*/
void RegularDialog::on_pushButtonDel_pressed ()
{
    QString sql = "delete from regular where CID=" + selectedId + ";";

    QSqlQuery query;
    if (!query.exec (sql))
	sqlError (sql, query.lastError ());

    pushButtonSubmit->setEnabled (false);
    pushButtonDel->setEnabled (false);
    showRegular ();
}

/*-------------------------------------------------------------------------------------------------
 * on_pushButtonQuit_pressed --
 *-----------------------------------------------------------------------------------------------*/
void RegularDialog::on_pushButtonQuit_pressed ()
{
    accept ();
}

/*-------------------------------------------------------------------------------------------------
 * rowSelected --
 *-----------------------------------------------------------------------------------------------*/
void RegularDialog::rowSelected (const QModelIndex index)
{
QModelIndex indx;
QVariant cell;

    pushButtonSubmit->setEnabled (true);
    pushButtonDel->setEnabled (true);

    int row = index.row ();

    /*----------------------------------------------------------------------------------------------
     * Get the values from selected line and save (won't need so don't save hours)
     *--------------------------------------------------------------------------------------------*/
    indx = index.model()->index (row, 0);
    cell = indx.model()->data (indx, Qt::DisplayRole);
    selectedId = cell.toString();

    indx = index.model()->index (row, 1);
    cell = indx.model()->data (indx, Qt::DisplayRole);
    lineEditName->setText (cell.toString());

    indx = index.model()->index (row, 2);
    cell = indx.model()->data (indx, Qt::DisplayRole);
    lineEditStart->setText (cell.toString());

    indx = index.model()->index (row, 3);
    cell = indx.model()->data (indx, Qt::DisplayRole);
    lineEditEnd->setText (cell.toString ());

    indx = index.model()->index (row, 4);
    cell = indx.model()->data (indx, Qt::DisplayRole);
    lineEditDay->setText (cell.toString ());
}

/*-------------------------------------------------------------------------------------------------
 * on_pushButtonSubmit_pressed --
 *-----------------------------------------------------------------------------------------------*/
void RegularDialog::on_pushButtonSubmit_pressed ()
{
QString sql;
QSqlQuery query;

    if (lineEditName->text().isEmpty() || lineEditStart->text().isEmpty() ||
	lineEditEnd->text().isEmpty()  || lineEditDay->text ().isEmpty ()) {
        QMessageBox::warning(this, tr ("regulars"), tr ("You must fill all fields"));
	return;
    }

    sql = "update regular set name='" + lineEditName->text () + "', "
	  "start='" + lineEditStart->text () + "', "
	  "end='" + lineEditEnd->text () + "', "
	  "day='" + lineEditDay->text () + "' "
	  "where CID=" + selectedId + ";";

    if (!query.exec (sql))
	sqlError (sql, query.lastError ());
    showRegular ();
    pushButtonSubmit->setEnabled (false);
    pushButtonDel->setEnabled (false);
    lineEditName->deselect ();
    lineEditDay->deselect ();
    lineEditStart->deselect ();
    lineEditEnd->deselect ();
}

/*-------------------------------------------------------------------------------------------------
 * showRegular --
 *	tableView from ui
 *-----------------------------------------------------------------------------------------------*/
void RegularDialog::showRegular ()
{
QString sql;
QSqlQuery query;
QStandardItem *itm;
QList <QStandardItem *>nextRow;

    sql= "select CID, name, start, end, day from regular;";
    if (!query.exec (sql))
	sqlError (sql, query.lastError ());

    model->clear ();
    model->setHorizontalHeaderItem(0, new QStandardItem(QString("ID")));
    model->setHorizontalHeaderItem(1, new QStandardItem(QString("Name")));
    model->setHorizontalHeaderItem(2, new QStandardItem(QString("Start")));
    model->setHorizontalHeaderItem(3, new QStandardItem(QString("End")));
    model->setHorizontalHeaderItem(4, new QStandardItem(QString("Days")));
 
    tableView->setModel(model);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableView->setColumnHidden (0, true);
    tableView->verticalHeader()->hide();

    while (query.next()) {
	nextRow.clear ();

	QString ID = query.value(0).toString();
	itm = new QStandardItem (ID);
	nextRow.append (itm);

	QString name  = query.value(1).toString();
	itm = new QStandardItem (name);
	nextRow.append (itm);

	QString start = query.value(2).toString();
	itm = new QStandardItem (start);
	nextRow.append (itm);

	QString end = query.value(3).toString();
	itm = new QStandardItem (end);
	nextRow.append (itm);

	QString day = query.value(4).toString();
	itm = new QStandardItem (day);
	nextRow.append (itm);

	model->appendRow (nextRow);
    }
}

/*-------------------------------------------------------------------------------------------------
 * sqlError --
 *-----------------------------------------------------------------------------------------------*/
void RegularDialog::sqlError (QString sql, QSqlError err)
{
    QMessageBox::warning(this, tr ("record"), tr ("Error:") + sql + "\n" + err.text());
}
