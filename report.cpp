#include <QtGui>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QPrinter>
#include <QPrintDialog>
#include <QFileDialog>
#include "record.h"
#include "report.h"

extern int debug;

GenDialog::GenDialog(QWidget *parent)
    : QDialog(parent)
{
    isSaved = false;
    proc = new QProcess;
    record = qobject_cast<Record*>(parent);

    setupUi(this);
    model  = new QStandardItemModel (0, 7, this);
    model2 = new QStandardItemModel (0, 4, this);

    this->setStyleSheet ("* {background-color: white;}");

    connect (buttonQuit,    SIGNAL (clicked()), this, SLOT(accept ()));
    connect (buttonOnly,    SIGNAL (clicked()), this, SLOT(doOnly ()));
    connect (buttonPrint,   SIGNAL (clicked()), this, SLOT(doPrint ()));
    showReport ();
}

/*-------------------------------------------------------------------------------------------------
 * showReport--
 *-----------------------------------------------------------------------------------------------*/
void GenDialog::showReport ()
{
QSqlQuery query;
QStandardItem *itm;
QList <QStandardItem *>nextRow;

    float niteHrs;
    QString sql = "select rate, (strftime ('%s', end) - strftime ('%s', start)) / 3600 from nite;";
    if (!query.exec (sql))
	sqlError (sql, query.lastError ());

    if ((debug & 2))
	qDebug () << "[sql]" << sql;

    while (query.next())			// 1st pass is void RFM
	niteHrs  = query.value(1).toFloat();

    if (niteHrs < 0.0)
	niteHrs += 24.0;

    /*---------------------------------------------------------------------------------------------
     * First the list of helpers for 'only show'
     *-------------------------------------------------------------------------------------------*/
    sql = "select name from helpers order by name;";
    if (!query.exec (sql))
	sqlError (sql, query.lastError ());

    if ((debug & 2))
	qDebug () << "[sql]" << sql;

    comboBox->clear();
    comboBox->addItem ("All");
    while (query.next()) {
	comboBox->addItem (query.value(0).toString());
    }

    /*-------------------------------------------------------------------------------------------------------
     * show the data
     *-----------------------------------------------------------------------------------------------------*/
    if (showOnly.isEmpty())
	sql = "select name, rate, pay, start, end from time order by start;";
    else
	sql = "select name, rate, pay, start, end from time where name='" + showOnly + "' order by start;";
    if (!query.exec (sql))
	sqlError (sql, query.lastError ());

    if ((debug & 2))
	qDebug () << "[sql]" << sql;

    model->clear ();
    model->setHorizontalHeaderItem(0, new QStandardItem(QString("Name")));
    model->setHorizontalHeaderItem(1, new QStandardItem(QString("Rate")));
    model->setHorizontalHeaderItem(2, new QStandardItem(QString("Date")));
    model->setHorizontalHeaderItem(3, new QStandardItem(QString("Start")));
    model->setHorizontalHeaderItem(4, new QStandardItem(QString("End")));
    model->setHorizontalHeaderItem(5, new QStandardItem(QString("Hours")));
 
    tableView->setModel(model);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tableView->verticalHeader()->hide();

    QString name;
    QString rate;
    QString hrs;
    QString start;
    QString end;
    QString nite;
    QString date;

    while (query.next()) {
	nextRow.clear ();

	name   = query.value(0).toString();
	rate   = query.value(1).toString();
	hrs    = query.value(2).toString();
	start  = query.value(3).toString();
	end    = query.value(4).toString();
	date   = query.value(5).toString();

	QDateTime dtStart (QDateTime::fromString (start, "yyyy-MM-dd hh:mm:ss"));
        QDateTime dtEnd   (QDateTime::fromString (end,   "yyyy-MM-dd hh:mm:ss"));

	itm = new QStandardItem (name);
	nextRow.append (itm);

	itm = new QStandardItem (rate);
	nextRow.append (itm);

	itm = new QStandardItem (dtStart.toString ("d MMM"));
	nextRow.append (itm);

	itm = new QStandardItem (dtStart.toString ("ddd h:mm a"));
	nextRow.append (itm);

	itm = new QStandardItem (dtEnd.toString ("ddd h:mm a"));
	nextRow.append (itm);

	itm = new QStandardItem (hrs);
	nextRow.append (itm);

	model->appendRow (nextRow);
    }

/*-------------------------------------------------------------------------------------------------
 * next the totals
 *-----------------------------------------------------------------------------------------------*/
    if (showOnly.isEmpty())
        sql = "select name, rate, pay, overnite from time order by name, rate;";
    else
        sql = "select name, rate, pay, overnite from time "
              "where name='" + showOnly + "' order by name, rate;";

    if (!query.exec (sql))
	sqlError (sql, query.lastError ());

    if ((debug & 2))
	qDebug () << "[sql]" << sql;

    model2->clear ();
    model2->setHorizontalHeaderItem(0, new QStandardItem(QString("Name")));
    model2->setHorizontalHeaderItem(1, new QStandardItem(QString("Hours")));
    model2->setHorizontalHeaderItem(2, new QStandardItem(QString("OverNight")));
    model2->setHorizontalHeaderItem(3, new QStandardItem(QString("Rate")));
    model2->setHorizontalHeaderItem(4, new QStandardItem(QString("$")));

    tableViewTotal->setModel(model2);
    tableViewTotal->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tableViewTotal->verticalHeader()->hide();

    QString lastName = "";
    int overNite;
    float totalHours;
    float rateValue;
    float totalDollars;

    while (query.next()) {
	name = query.value(0).toString();
	rate = query.value(1).toString();
	hrs  = query.value(2).toString();
	nite = query.value(3).toString();

	if ((debug & 2))
	    qDebug () << "[query]"
		      << "\n    name" << name
		      << "\n    rate" << rate
		      << "\n    hrs " << hrs
		      << "\n    nite" << nite;

	/*-----------------------------------------------------------------------------------------
	 * first ever loop
	 *---------------------------------------------------------------------------------------*/
        if (lastName.isEmpty ()) {
	    lastName = name;

	    rateValue = rate.toFloat ();		// be it day rates or nite rates
	    if (nite != "1") {
		totalHours = hrs.toFloat ();
		overNite = 0;
	    }
	    else {
		overNite = 1;
		totalHours = 0.0;
	    }
	    if ((debug & 4))
		qDebug () << "[1st] totalHours" << totalHours << "ntes" << overNite;
	}
	else {

	    /*-----------------------------------------------------------------------------------------
	     * every next loop Same name Same rate
	     *---------------------------------------------------------------------------------------*/
	    if ((name == lastName) && (rateValue == rate.toFloat ())) {

		rateValue = rate.toFloat ();
		if (nite != "1")
		    totalHours += hrs.toFloat ();
		else
		    overNite += 1;
		if ((debug & 4))
		    qDebug () << "[next] totalHours" << totalHours << "ntes" << overNite;
	    }
	
	    /*-----------------------------------------------------------------------------------------
	     * every next loop NEW line: different name and/or different rate
	     *---------------------------------------------------------------------------------------*/
	    else {
		totalDollars = overNite > 0 ? overNite * rateValue : totalHours * rateValue;
		if ((debug & 4))
		    qDebug () << "[new] totalHours" << totalHours << "ntes" << overNite;

		nextRow.clear ();
		/*-------------------------------------------------------------------------------------
		 * Add the data we have been collecting
		 *-----------------------------------------------------------------------------------*/
		itm = new QStandardItem (lastName);
		nextRow.append (itm);

		itm = new QStandardItem (QString::number (totalHours, 'f', 1));
		nextRow.append (itm);

		itm = new QStandardItem (QString::number (overNite));
		nextRow.append (itm);

		itm = new QStandardItem (QString::number (rateValue, 'f', 2));
		nextRow.append (itm);

		itm = new QStandardItem (QString::number (totalDollars, 'f', 2));
		nextRow.append (itm);

		model2->appendRow (nextRow);

		lastName = name;
		rateValue = rate.toFloat ();

		if (nite != "1") {
		    totalHours = hrs.toFloat ();
		    overNite = 0;
		}
		else {
		    overNite = 1;
		    totalHours = 0.0;
		}
	    }
	}
    }
    /*---------------------------------------------------------------------------------------------
     * And the last row
     *-------------------------------------------------------------------------------------------*/
    if ((debug & 2))
	qDebug () << "[last]";

    totalDollars = overNite > 0 ? overNite * rateValue : totalHours * rateValue;
    qDebug () << "[last] totalHours" << totalHours << "ntes" << overNite;
    nextRow.clear ();
    /*-------------------------------------------------------------------------------------
     * Add the data we have been collecting
     *-----------------------------------------------------------------------------------*/
    itm = new QStandardItem (lastName);
    nextRow.append (itm);

    itm = new QStandardItem (QString::number (totalHours, 'f', 1));
    nextRow.append (itm);

    itm = new QStandardItem (QString::number (overNite));
    nextRow.append (itm);

    itm = new QStandardItem (QString::number (rateValue, 'f', 2));
    nextRow.append (itm);

    itm = new QStandardItem (QString::number (totalDollars, 'f', 2));
    nextRow.append (itm);

    model2->appendRow (nextRow);
}

/*-------------------------------------------------------------------------------------------------
 * doOnly --
 *-----------------------------------------------------------------------------------------------*/
void GenDialog::doOnly ()
{
    //qDebug () << "[only]" << comboBox->currentText();
    if (comboBox->currentText() == "All")
	showOnly.clear();
    else
	showOnly = comboBox->currentText();
    showReport ();
}

/*-------------------------------------------------------------------------------------------------
 * sqlError --
 *-----------------------------------------------------------------------------------------------*/
void GenDialog::sqlError (QString sql, QSqlError err)
{
    QMessageBox::warning(this, tr ("record"), tr ("Error:") + sql + "\n" + err.text());
}

/*-------------------------------------------------------------------------------------------------
 * doPrint --
 *-----------------------------------------------------------------------------------------------*/
void GenDialog::doPrint ()
{

    //qDebug () << "[print]";
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog *dlg = new QPrintDialog(&printer, this);
    dlg->addEnabledOption(QAbstractPrintDialog::PrintSelection);
    dlg->setWindowTitle(tr("Print Document"));
    if (dlg->exec() == QDialog::Accepted) {
	QPainter painter;
	painter.begin (&printer);

	double xscale = printer.pageRect().width()/double(this->width());
	double yscale = printer.pageRect().height()/double(this->height());
	double scale = qMin(xscale, yscale);

	painter.translate(printer.paperRect().x() + printer.pageRect().width()/2,
			  printer.paperRect().y() + printer.pageRect().height()/2);
	painter.scale(scale, scale);
	painter.translate(-width()/2, -height()/2);

         this->render(&painter);
	}
    delete dlg;
}

/*-------------------------------------------------------------------------------------------------
 * formatWindow --
 *-----------------------------------------------------------------------------------------------*/
void GenDialog::formatWindow (int newWidth)
{
}

/*-------------------------------------------------------------------------------------------------
 * screenWidthChanged --
 *-----------------------------------------------------------------------------------------------*/
void GenDialog::screenWidthChanged (int newWidth)
{
    formatWindow (newWidth);
}
