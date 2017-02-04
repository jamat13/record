#include <QtGui>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>
#include "version.h"
#include "record.h"
#include "helpers.h"
#include "report.h"
#include "night.h"
#include "regular.h"

#define JAMES		// Menu position

extern int debug;

/*-------------------------------------------------------------------------------------------------
 * constructor --
 *-----------------------------------------------------------------------------------------------*/
Record::Record (QWidget *parent)
        : QMainWindow(parent)
{
    if ((debug))
	qDebug ("[record] record");

#ifdef Q_OS_LINUX
    QFile file ("/usr/share/helpers/record.qss");
#endif
#ifdef Q_OS_MACX
    // this does not work here ?? ui file overides ?? menuBar ()->setNativeMenuBar (false);
    QFile file (QApplication::applicationDirPath () + "/../Resources/record.qss");
#endif
#ifdef Q_OS_WIN32
    QFile file (QCoreApplication::applicationDirPath() + "/record.qss");
#endif

    file.open (QFile::ReadOnly);
    QString styleSheet = QString::fromLatin1 (file.readAll());
    this->setStyleSheet (styleSheet);

    setupUi (this);
    readSettings ();

    /*------------------------------------------------------------------------------------------------------
     * Make sure the db repository exists
     *----------------------------------------------------------------------------------------------------*/
    QString path = QDir::homePath () + "/HELPERS";
    QDir dbdir (path);
    if (!(dbdir.exists ()))
        dbdir.mkpath (path);

    if (dbName.isEmpty ()) {
	dbName = QFileDialog::getOpenFileName (this, "Existing Helper Record", path, "databases (*.db)");
	if (dbName.isEmpty ()) {
	    QMessageBox::warning (this, "Helper Record",
				    "Choose an existing database\n"
				    "if you don't have one yet skip the warnings\n"
				    "create a new one file->new\n"
				    "add helpers edit->hrlpers\m"
				    "add night info edit->night");
	}
	//dbName.prepend (path + "/");
	//if (!(dbName.endsWith (".db")))
	    //dbName.append (".db");
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName (dbName);
    bool ok = db.open();
    if (!ok) {
	QMessageBox::warning (this, "Helper Record", tr ("open database %1 failed").arg (dbName));
        //exit (0);
    }

    actionAdd->setShortcut (Qt::Key_Return);

    connect (tableView,         SIGNAL (clicked(const QModelIndex)), this,
							SLOT (rowSelected (const QModelIndex)));
    connect (lineEditEnd,       SIGNAL (textChanged (const QString &)), this,
					    SLOT (on_lineEditStart_textChanged (const QString &)));
    connect (actionAboutQt, SIGNAL (triggered()), qApp, SLOT(aboutQt()));

    pushButtonCancel->setVisible (false);
    pushButtonSubmit->setVisible (false);
    lineEditRate->setVisible (false);
    lineEditHours->setVisible (false);
    lineEditPay->setVisible (false);
    labelRate->setVisible (false);
    labelHours->setVisible (false);
    labelPay->setVisible (false);
    actionDelete->setEnabled (false);

    model = new QStandardItemModel (0, 6, this); //0 Rows and 6 Columns

    //qDebug () << "readGeometry";
    QSettings settings("helpers", "record");
    restoreGeometry(settings.value("geometry").toByteArray());
    
    showRecord ();
}

/*----------------------------------------------------------------------------------------------------------
 * on_actionAbout_triggered --
 *--------------------------------------------------------------------------------------------------------*/
void Record::on_actionAbout_triggered ()
{
    //qWarning () << funcIndent << "about";

    QString ver = tr ("record helper times\n");
    ver.append(tr("GIT: "));
    ver.append(gitrev);

    QMessageBox::about (this, "asbuilt Editor", ver);
}

/*--------------------------------------------------------------------------------------------------
 * on_actionHelp_triggered --
 *------------------------------------------------------------------------------------------------*/
void Record::on_actionHelp_triggered ()
{
    QMessageBox::information (this, "Record",
			tr ("This is a app to track helper times\n\n"

			    "menu->file->reports\n"
			    "A summary of this period is given. You may qualify by helper and "
			    "print the results\n\n"

			    "menu->file->load\n"
			    "will load and use that database\n\n"

			    "menu->file->new\n"
			    "will create a new empty database and use that database\n"
			    "you need to setup helpers, maybe night and regular information\n\n"

			    "menu->file->clone\n"
			    "will create a new database inheriting all infomation "
			    "except for the actual times and use that database\n\n"

			    "menu->file->Add Regulars\n"
			    "will add the regular helpers for today\n\n"

			    "menu->edit->helpers\n"
			    "will allow helper names and pay rates to be set\n\n"

			    "menu->edit->Night\n"
			    "allows start and end of overnight sessions tobe set\n"
			    "note the convention that overnight stats at the end of today\n"
			    "so 24:00 is midnight and ends even later so 31:00 is 7am tomorrow\n\n"

			    "menu->edit->Regular Helpers\n"
			    "allows you to specify helpers who have a regular shift.\n"
			    ));
}

/*--------------------------------------------------------------------------------------------------
 * on_actionAppearence_triggered --
 *------------------------------------------------------------------------------------------------*/
void Record::on_actionAppearence_triggered ()
{
    QMessageBox::information (this, "Record",
			      tr ("You may customize the appearance of record by editing\n"
				  "linux:  /usr/share/rennisons/notes.qss\n"
				  "windows: Program Files/record/notes.qss\n"
				  "mac:/Applications/record.app/Contents/Resources/record.qss\n"
				  "remove the file to restore default black white grey theme\n"
				  "or for example\n\n"
				  " * {"
				  "background-color: #fbefcb;\n"
				  "}\n"
				  "QLineEdit {\n"
				  "background-color: #fcda90;\n"
				  "}\n"
				  "QPushButton {\n"
				  "background-color: #fcda90;\n"
				  "}\n"
				  "QMenuBar {\n"
				  "background-color: #fcda90;\n"
				  "}\n"
				  "QMenu::item {\n"
				  "background-color: #fcda90;\n"
				  "}\n"
				  "QMenu::item:selected {\n"
				  "background-color: #654321;\n"
				  "}\n"
				  "QHeaderView::section {\n"
				  "background-color: #fcda90;\n"
				  "}\n"
				  "QComboBox {\n"
				  "background-color: #e0c070;\n"
				  "}"));
}

/*----------------------------------------------------------------------------------------------------------
 * on_actionAddRegulars_triggered --
 *--------------------------------------------------------------------------------------------------------*/
void Record::on_actionAddRegulars_triggered ()
{
    QString today = QDateTime::currentDateTime ().toString ("ddd");
    qDebug () << "Add Regulars today" << today;
}

/*----------------------------------------------------------------------------------------------------------
 * on_lineEditStart_textChanged --
 *--------------------------------------------------------------------------------------------------------*/
void Record::on_lineEditStart_textChanged (const QString &str)
{
QSqlQuery query;

    if (lineEditEnd->text().isEmpty() || lineEditStart->text().isEmpty ()) {
	if ((debug & 2))
	    qDebug () << "[on_lineEditStartorEnd_textChanged/]";
	return;
    }

    if ((debug & 4))
	qDebug () << "[ on_lineEditStartorEnd_textChanged]";

    QDateTime shiftStart (QDateTime::fromString (lineEditStart->text (), "yyyy-MM-d h:mm"));
    QDateTime shiftEnd   (QDateTime::fromString (lineEditEnd->text (), "yyyy-MM-d h:mm"));
    if ((debug & 4))
	qDebug () << "lineEditStart" << lineEditStart->text ()
		  << "\nlineEditEnd  " << lineEditEnd->text ()
		  << "\nshiftStart   " << shiftStart
		  << "\nshiftEnd     " << shiftEnd
		  << "\nendSecs      " << shiftEnd.toTime_t()
		  << "\nstartSecs    " << shiftStart.toTime_t ();

    float hrs = (shiftEnd.toTime_t() - shiftStart.toTime_t ()) / 3600.0;
    float pay = hrs;

    float base = floor (hrs);
    if (pay < (base + 0.1))
	pay = base;
    else if (pay < (base + 0.6))
	pay = base + 0.5;
    else
	pay = base + 1.0;

    if ((debug & 4))
	qDebug () << "pay" << pay << "hrs" << hrs;

    lineEditPay->setText (QString::number (pay, 'f', 1));
    lineEditHours->setText (QString::number (hrs, 'f', 1));
    if ((debug & 4))
	qDebug () << "[ on_lineEditStartorEnd_textChanged]";
}

/*-------------------------------------------------------------------------------------------------
 * on_pushButtonCancel_wressed --
 *-----------------------------------------------------------------------------------------------*/
void Record::on_pushButtonCancel_pressed ()
{
    actionDelete->setEnabled (false);
    showRecord ();
}

/*-------------------------------------------------------------------------------------------------
 * on_pushButtonSubmit_pressed --
 *-----------------------------------------------------------------------------------------------*/
void Record::on_pushButtonSubmit_pressed ()
{
QString sql;
QSqlQuery query;

    if (lineEditStart->text().isEmpty()) {
        QMessageBox::warning(this, tr ("record"), tr ("You must keep at least a valid start time"));
    }
    else {
        //qDebug () << "[isEmpty] false" << lineEditStart->text();
        if (lineEditEnd->text().isEmpty())
            sql = "update time set start=\"" + lineEditStart->text() + ":00\", "
		  "rate=" + lineEditRate->text() + ", "
		  "pay=" + lineEditPay->text() + ", "
		  "name='" + comboBox->currentText () + "' " +
		  "where ID=" + QString::number(selectedId) + ";";
        else
            sql = "update time set start=\"" + lineEditStart->text() + ":00\", "
		  "rate=" + lineEditRate->text() + ", "
		  "pay=" + lineEditPay->text() + ", "
		  "name='" + comboBox->currentText () + "', " +
                  "end='" + lineEditEnd->text() + ":00' where ID=" +
                  QString::number(selectedId) + ";";

	//qDebug () << "[sql]" << sql;
        if (!query.exec (sql))
            sqlError (sql, query.lastError ());
	pushButtonCancel->setVisible (false);
	pushButtonSubmit->setVisible (false);
	lineEditRate->setVisible (false);
	lineEditHours->setVisible (false);
	lineEditPay->setVisible (false);
	labelRate->setVisible (false);
	labelHours->setVisible (false);
	labelPay->setVisible (false);
    }
    showRecord ();
}

/*-------------------------------------------------------------------------------------------------
 * rowSelected --
 *-----------------------------------------------------------------------------------------------*/
void Record::rowSelected (const QModelIndex index)
{
QModelIndex indx;
QVariant cell;

    pushButtonCancel->setVisible (true);
    pushButtonSubmit->setVisible (true);
    lineEditRate->setVisible (true);
    lineEditHours->setVisible (true);
    lineEditPay->setVisible (true);
    labelRate->setVisible (true);
    labelHours->setVisible (true);
    labelPay->setVisible (true);
    actionDelete->setEnabled (true);
    //pushButtonSubmit->setDefault (true); actionAdd->setShortcut (Qt::Key_Return);

    int row = index.row ();

    /*----------------------------------------------------------------------------------------------
     * Get the values from selected line and save (won't need so don't save hours)
     *--------------------------------------------------------------------------------------------*/
    indx = index.model()->index (row, 0);
    cell = indx.model()->data (indx, Qt::DisplayRole);
    selectedId = cell.toInt();

    indx = index.model()->index (row, 1);
    cell = indx.model()->data (indx, Qt::DisplayRole);
    selectedName = cell.toString();

    indx = index.model()->index (row, 2);
    cell = indx.model()->data (indx, Qt::DisplayRole);
    selectedRate = cell.toString();

    indx = index.model()->index (row, 3);
    cell = indx.model()->data (indx, Qt::DisplayRole);
    selectedHours = cell.toString ();

    indx = index.model()->index (row, 4);
    cell = indx.model()->data (indx, Qt::DisplayRole);
    selectedPay = cell.toString();

    indx = index.model()->index (row, 7);
    cell = indx.model()->data (indx, Qt::DisplayRole);
    selectedStart = cell.toString();
    selectedStart.chop (3);		// eat seconds

    indx = index.model()->index (row, 8);
    cell = indx.model()->data (indx, Qt::DisplayRole);
    selectedEnd = cell.toString();
    selectedEnd.chop (3);		// eat seconds

    /*----------------------------------------------------------------------------------------------
     * set submit box values
     *--------------------------------------------------------------------------------------------*/
    lineEditRate->setText  (selectedRate);
    lineEditHours->setText (selectedHours);
    lineEditPay->setText   (selectedPay);
    lineEditStart->setText (selectedStart);
    lineEditEnd->setText   (selectedEnd);
    comboBox->setCurrentIndex (comboBox->findText (selectedName));
}

/*-------------------------------------------------------------------------------------------------
 * showRecord --
 *-----------------------------------------------------------------------------------------------*/
void Record::showRecord ()
{
QString sql;
QSqlQuery query;
QStandardItem *itm;
QList <QStandardItem *>nextRow;
QDateTime now;

    lineEditRate->setVisible (false);
    lineEditHours->setVisible (false);
    lineEditPay->setVisible (false);
    pushButtonCancel->setVisible (false);
    pushButtonSubmit->setVisible (false);
    labelRate->setVisible (false);
    labelHours->setVisible (false);
    labelPay->setVisible (false);

    sql = "select ID, name, rate, pay, start, end from time order by start;";
    if (!query.exec (sql))
	sqlError (sql, query.lastError ());

    model->clear ();
    model->setHorizontalHeaderItem(0, new QStandardItem(QString("ID")));
    model->setHorizontalHeaderItem(1, new QStandardItem(QString("Name")));
    model->setHorizontalHeaderItem(2, new QStandardItem(QString("Rate")));
    model->setHorizontalHeaderItem(3, new QStandardItem(QString("Hours")));
    model->setHorizontalHeaderItem(4, new QStandardItem(QString("Pay")));
    model->setHorizontalHeaderItem(5, new QStandardItem(QString("Start")));
    model->setHorizontalHeaderItem(6, new QStandardItem(QString("End")));
    model->setHorizontalHeaderItem(7, new QStandardItem(QString("START")));
    model->setHorizontalHeaderItem(8, new QStandardItem(QString("END")));
 
    tableView->setModel(model);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    tableView->setColumnHidden (0, true);
    tableView->verticalHeader()->hide();

    tableView->setSelectionMode (QAbstractItemView::SingleSelection);
    tableView->setSelectionBehavior (QAbstractItemView::SelectRows);

    while (query.next()) {
	nextRow.clear ();

	QString id  = query.value(0).toString();
	itm = new QStandardItem (id);
	nextRow.append (itm);

	QString name = query.value(1).toString();
	itm = new QStandardItem (name);
	nextRow.append (itm);

	QString rate = query.value(2).toString();
	itm = new QStandardItem (rate);
	nextRow.append (itm);

	QString pay = query.value(3).toString();
	QString stringStart = query.value(4).toString ();
	QString stringEnd   = query.value(5).toString ();

	QDateTime shiftStart (QDateTime::fromString (stringStart, "yyyy-MM-dd hh:mm:ss"));
	QDateTime shiftEnd   (QDateTime::fromString (stringEnd,   "yyyy-MM-dd hh:mm:ss"));

	itm = new QStandardItem
		  (QString::number ((double)(shiftEnd.toTime_t () - shiftStart.toTime_t ()) / 3600, 'f', 2));
	nextRow.append (itm);

	itm = new QStandardItem (pay);
	nextRow.append (itm);

	itm = new QStandardItem (shiftStart.toString ("ddd h:mm a"));
	nextRow.append (itm);

	itm = new QStandardItem (shiftEnd.toString ("ddd h:mm a"));
	nextRow.append (itm);

	itm = new QStandardItem (stringStart);
	nextRow.append (itm);

	itm = new QStandardItem (stringEnd);
	nextRow.append (itm);

	model->appendRow (nextRow);
    }

    sql = "select name, ID from helpers order by name;";
    if (!query.exec (sql))
	sqlError (sql, query.lastError ());

    comboBox->clear();
    while (query.next()) {
	comboBox->addItem (query.value(0).toString(), query.value(1).toInt());
    }

    now = QDateTime::currentDateTime ();
    lineEditStart->setText (now.toString ("yyyy-MM-dd h:mm"));
    lineEditEnd->setText   (now.toString ("yyyy-MM-dd h:mm"));
}

/*-------------------------------------------------------------------------------------------------
 * on_actionNight_triggered --
 *-----------------------------------------------------------------------------------------------*/
void Record::on_actionNight_triggered ()
{
    NightDialog *dialog = new NightDialog (this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

/*-------------------------------------------------------------------------------------------------
 * on_actionRegular_triggered --
 *-----------------------------------------------------------------------------------------------*/
void Record::on_actionRegular_triggered ()
{
    RegularDialog *dialog = new RegularDialog (this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

/*-------------------------------------------------------------------------------------------------
 * on_actionClone_triggered --
 *-----------------------------------------------------------------------------------------------*/
void Record::on_actionClone_triggered ()
{
    QFile fil (dbName);

    QString offer = QDir::homePath () + "/HELPERS/" +
					(QDateTime::currentDateTime ()).toString ("dMMMyyyydddhhmm");
    QString fileName = QFileDialog::getSaveFileName (this, "Helper Record", offer, "(*.db)");

    if (fileName.isEmpty ())
	return;
    
    if (!(fileName.endsWith (".db")))
	fileName.append (".db");

    fil.copy (fileName);

    dbName = fileName;

    QSqlDatabase::removeDatabase ("defaultDatabase");
    QSqlDatabase db = QSqlDatabase::addDatabase ("QSQLITE");
    db.setDatabaseName (fileName);
    bool ok = db.open();
    if (!ok) {
	QMessageBox::warning (this, "Helper Record", tr ("open database %1 failed").arg (dbName));
        exit (0);
    }

    QSqlQuery query;
    QString sql = "delete from time;";
    if (!query.exec (sql))
	sqlError (sql, query.lastError ());
    showRecord ();
}

/*-------------------------------------------------------------------------------------------------
 * on_actionNew_triggered
 *-----------------------------------------------------------------------------------------------*/
void Record::on_actionNew_triggered ()
{
    int r = QMessageBox::warning(this,
	    tr ("record"),
	    tr ("You are about to create a new empty database. Perhaps you ment CLONE"
		"which will clear times but keep helper, regular and night information\n"
		"Do you really want to do this?"),
	    QMessageBox::Yes | QMessageBox::No);
    if (r == QMessageBox::No)
	return;

    QString offer = QDir::homePath () + "/HELPERS/" +
					(QDateTime::currentDateTime ()).toString ("dMMMyyyydddhhmm");
    
    QString dbName = QFileDialog::getSaveFileName (this, "Helper Record", offer, "(*.db)");
    if (dbName.isEmpty ()) {
	QMessageBox::warning (this, "Helper Record", "You must specify some name");
	exit (0);
    }

    if (!(dbName.endsWith (".db")))
	dbName.append (".db");

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName (dbName);
    bool ok = db.open();
    if (!ok) {
	QMessageBox::warning (this, "Helper Record", tr ("open database %1 failed").arg (dbName));
	exit (0);
    }

    QSqlQuery query;
    QString sql   = "CREATE TABLE helpers ( `ID` integer primary key AUTOINCREMENT, `name` tinytext, "
		    "`dayrate` float DEFAULT NULL, `werate` float DEFAULT NULL);";
    if (!query.exec (sql))
	sqlError (sql, query.lastError ());

    sql = "CREATE TABLE `nite` ( `start` tinytext, `end` tinytext, `rate` float DEFAULT NULL) ;";
    if (!query.exec (sql))
	sqlError (sql, query.lastError ());

    sql = "CREATE TABLE `regular` ( `CID` integer primary key AUTOINCREMENT, `name` tinytext, "
	  "`day` tinytext, `start` datetime DEFAULT NULL, `end` datetime DEFAULT NULL);";
    if (!query.exec (sql))
	sqlError (sql, query.lastError ());

    sql = "CREATE TABLE `time` ( `ID` integer primary key AUTOINCREMENT, `name` tinytext, "
	  "`rate` float DEFAULT NULL, `pay` float DEFAULT NULL, "
	  "`overnite` tinyint(4) DEFAULT NULL, `start` datetime DEFAULT NULL, "
	  "`end` datetime DEFAULT NULL);";
    if (!query.exec (sql))
	sqlError (sql, query.lastError ());

    showRecord ();
}

/*----------------------------------------------------------------------------------------------------------
 * on_actionLoad_triggered --
 *--------------------------------------------------------------------------------------------------------*/
void Record::on_actionLoad_triggered ()
{
    QString fileName = QFileDialog::getOpenFileName(this, "open a database",
		       QDir::homePath () + "/HELPERS", "db (*.db)");
    if (fileName.isNull())
        return;
    dbName = fileName;

    QSqlDatabase::removeDatabase ("defaultDatabase");
    QSqlDatabase db = QSqlDatabase::addDatabase ("QSQLITE");
    db.setDatabaseName (fileName);
    bool ok = db.open();
    if (!ok) {
	QMessageBox::warning (this, "Helper Record", tr ("open database %1 failed").arg (dbName));
        exit (0);
    }

    showRecord ();
}

/*-------------------------------------------------------------------------------------------------
 * on_actionReports_triggered --
			 "QLineEdit {background-color: #fcda90;} "
 *-----------------------------------------------------------------------------------------------*/
void Record::on_actionReports_triggered ()
{
    GenDialog *dialog = new GenDialog (this);
/*
#  ifdef Q_WS_MAC
    dialog->setStyleSheet ("* {background-color: #fbefcb;} "
			 "QPushButton {background-color: #fcda90;} "
			 "QHeaderView::section {background-color: #fcda90;}"
			 "QComboBox {background-color: #fcda90;}"
			  );
#  endif
*/
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

/*-------------------------------------------------------------------------------------------------
 * on_actionHelpers_triggered --
 *-----------------------------------------------------------------------------------------------*/
void Record::on_actionHelpers_triggered ()
{
    HelpersDialog *dialog = new HelpersDialog (this);
/*
#  ifdef Q_WS_MAC
    dialog->setStyleSheet ("* {background-color: #fbefcb;} "
			  "QLineEdit {background-color: #fac4cc; "
				     "border: 2px #fac4cc; "
				     "} "
			 "QPushButton {background-color: #fcda90;} "
			 "QHeaderView::section {background-color: #fcda90;}"
			  );
#  endif
*/
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

/*-------------------------------------------------------------------------------------------------
 * on_actionAdd_triggered
 *-----------------------------------------------------------------------------------------------*/
void Record::on_actionAdd_triggered ()
{
QSqlQuery query;
QString sql;

    QDateTime shiftStart (QDateTime::fromString (lineEditStart->text (), "yyyy-MM-d h:mm"));
    QDateTime shiftEnd   (QDateTime::fromString (lineEditEnd->text (), "yyyy-MM-d h:mm"));

    /*---------------------------------------------------------------------------------------------
     * Are the entries valid
     *-------------------------------------------------------------------------------------------*/
    if ((!(shiftStart.isValid())) || (!(shiftEnd.isValid ()))) {
        QMessageBox::warning(this, tr ("record"), tr ("You must have a valid start and end time"));
	return;
    }

    /*---------------------------------------------------------------------------------------------
     * Get the yearday of start and end times
     *-------------------------------------------------------------------------------------------*/
    sql = "select strftime ('%j', '" + lineEditStart->text () +
	  "'), strftime ('%j', '" + lineEditEnd->text ()+ "');";

    if (!query.exec (sql))
	sqlError (sql, query.lastError ());

    int   yeardayStart;
    int   yeardayEnd;
    while (query.next()) {
	yeardayStart = query.value (0).toInt ();
	yeardayEnd = query.value (1).toInt ();
    }

    if ((debug & 1))
	qDebug () << "    [sql]" << sql;

    if ((debug & 2))
	qDebug () << "    yeardayStart" << yeardayStart << "\n    yeardayEnd  " << yeardayEnd << "\n";

    /*-------------------------------------------------------------------------------------------------------
     * If start and end on same day the overnight does not apply and all hours are todays hours
     *-----------------------------------------------------------------------------------------------------*/
    if (yeardayStart == yeardayEnd) {
	/*---------------------------------------------------------------------------------------------
	 * Get the rate for this helper
	 *-------------------------------------------------------------------------------------------*/
	sql = "select strftime ('%w', '" + lineEditStart->text () +	// day of week`
	      "'), strftime ('%s', '"    + lineEditStart->text () +	// secs since epoc
	      "'), strftime ('%s', '"    + lineEditEnd->text () +	// secs since epoc
	      "'), dayrate, werate from helpers where name='" + comboBox->currentText () + "';";

	if (!query.exec (sql))
	    sqlError (sql, query.lastError ());

	float dayRate;
	float weekEndRate;
	int   today;
	int   theStart;
	int   theEnd;

	while (query.next()) {
	    today    = query.value(0).toInt ();
	    theStart = query.value(1).toInt ();
	    theEnd   = query.value(2).toInt ();
	    dayRate  = query.value(3).toFloat ();
	    weekEndRate   = query.value(4).toFloat ();

	    if ((debug & 1))
		qDebug () << "SameDay [sql]        " << sql
			  << "\n    [day]        " << today
			  << "\n    [unix start] " << theStart
			  << "\n    [unix end]   " << theEnd
			  << "\n    [dayRate]    " << dayRate
			  << "\n    [weekEndRate]" << weekEndRate
			  << "\n";
    }

	float theTime = ((float)(theEnd - theStart)) / 3600.0;
	if ((debug & 4))
	    qDebug () << "    [unround time]" << theTime;

	float base = floor (theTime);
	if (theTime < base + 0.1)
	    theTime = base;
	else if (theTime < base + 0.6)
	    theTime = base + 0.5;
	else
	    theTime = base +1;
	if ((debug & 4))
	    qDebug () << "      [round time]" << theTime;

	float theRate = ((today > 0) && (today < 6)) ? dayRate : weekEndRate;

	sql = QString ("insert into time values (NULL, '%1', %2, %3, NULL, '%4:00', '%5:00');")
			.arg (comboBox->currentText ())
			.arg (theRate)
			.arg (theTime)
			.arg (lineEditStart->text())
			.arg (lineEditEnd->text());
	if ((debug & 1))
	    qDebug () << "1   [sql]" << sql;
	if (!query.exec (sql))
	    sqlError (sql, query.lastError ());
    }
    else {
	/*---------------------------------------------------------------------------------------------------
	 * start and end not the same dday
	 * I assume niteStart and niteEnd are 0 .. 49 else you get wrong answers
	 *-------------------------------------------------------------------------------------------------*/
	float niteRate;
	QString niteEndString;
	QString niteStartString;

	sql = "select start, end, rate from nite;";

	if (!query.exec (sql))
	    sqlError (sql, query.lastError ());

	while (query.next()) {
	    niteStartString = query.value(0).toString ();
	    niteEndString = query.value(1).toString ();
	    niteRate = query.value(2).toFloat ();
	}

	if ((debug & 1))
	    qDebug () << "2   [sql]" << sql;

	if ((debug & 2))
	    qDebug () <<   "    niteStart   " << niteStartString
		      << "\n    niteEnd     " << niteEndString
		      << "\n    niteRate    " << niteRate
		      << "\n";

	int niteStartHrs = niteStartString.left (2).toInt ();
	int niteStartMin = niteStartString.right (2).toInt ();
	int niteEndHrs   = niteEndString.left (2).toInt ();
	int niteEndMin   = niteEndString.right (2).toInt ();
	bool bumpStartDays = false; 
	bool bumpEndDays = false; 

	if (niteStartHrs > 23) {
	    bumpStartDays = true;
	    niteStartHrs -= 24;
	}
	if (niteEndHrs > 23) {
	    bumpEndDays = true;
	    niteEndHrs -= 24;
	}

	QDateTime niteStart (shiftStart.date (), QTime (niteStartHrs, niteStartMin, 0));
	QDateTime niteEnd   (shiftStart.date (), QTime (niteEndHrs, niteEndMin, 0));
	if (bumpStartDays)
	    niteStart = niteStart.addDays (1);
	if (bumpEndDays)
	    niteEnd = niteEnd.addDays (1);

	if ((debug & 2)) 
	    qDebug () <<   "    niteStart" << niteStart
		  << "\n    niteEnd  " << niteEnd;

	sql = "select dayrate, werate from helpers where name='" + comboBox->currentText () + "';";

	if (!query.exec (sql))
	    sqlError (sql, query.lastError ());

	float dayRate;
	float weekEndRate;

	if ((debug & 1))
	    qDebug () << "[sql]" << sql;

	while (query.next()) {
	    dayRate  = query.value(0).toFloat ();
	    weekEndRate   = query.value(1).toFloat ();
	    if ((debug & 2))
		qDebug () << "    [dayRate]    " << dayRate
			  << "\n    [weekEndRate]" << weekEndRate
			  << "\n";
	}

	/*---------------------------------------------------------------------------------------------------
	 * loop adding the entries
	 *-------------------------------------------------------------------------------------------------*/
	for (;;) {

	    /*-----------------------------------------------------------------------------------------------
	     *                         +++ overnight +++
	     *                         ^ niteStart     ^ niteEnd       
	     * 12am                   12pm                        12am
	     * shift  ****************-------  
             *        ^ shiftStart      ^......^ shiftEnd
	     *---------------------------------------------------------------------------------------------*/

	    if (shiftEnd < niteEnd) {
		float theTime = (float)(shiftStart.secsTo (shiftEnd)) / 3600.0;
		if ((debug & 4))
		    qDebug () << "    [unround time]" << theTime;

		float base = floor (theTime);
		if (theTime < base + 0.1)
		    theTime = base;
		else if (theTime < base + 0.6)
		    theTime = base + 0.5;
		else
		    theTime = base +1;
		if ((debug & 4))
		    qDebug () << "      [round time]" << theTime;

		sql = "select strftime ('%w', '" + shiftStart.toString ("yyyy-MM-dd") + "');";

		if (!query.exec (sql))
		    sqlError (sql, query.lastError ());

		int   today;
		while (query.next()) {
		    today    = query.value(0).toInt ();
		}

		if ((debug & 1))
		    qDebug () << "ManyDay [sql]        " << sql;

		if ((debug & 2))
		     qDebug () << "    [day]        " << today;

		float theRate = ((today > 0) && (today < 6)) ? dayRate : weekEndRate;

		sql = QString ("insert into time values (NULL, '%1', %2, %3, NULL, '%4', '%5');")
			.arg (comboBox->currentText ())
			.arg (theRate)
			.arg (theTime)
			.arg (shiftStart.toString ("yyyy-MM-dd HH:mm:00"))
			.arg (shiftEnd.toString ("yyyy-MM-dd HH:mm:00"));
		if ((debug & 1))
		    qDebug () << "3   [sql]" << sql;
		if (!query.exec (sql))
		    sqlError (sql, query.lastError ());
		break;						//only 1 entry for this shift
	    }
	    /*-----------------------------------------------------------------------------------------------
	     * At least hours and overnight
	     *---------------------------------------------------------------------------------------------*/
	    else {
		float theTime = (float)(shiftStart.secsTo (niteStart)) / 3600.0;
		if ((debug & 4))
		    qDebug () << "    [unround time]" << theTime;

		float base = floor (theTime);
		if (theTime < base + 0.1)
		    theTime = base;
		else if (theTime < base + 0.6)
		    theTime = base + 0.5;
		else
		    theTime = base +1;
		if ((debug & 4))
		    qDebug () << "      [round time]" << theTime;

		sql = "select strftime ('%w', '" + shiftStart.toString ("yyyy-MM-dd") + "');";

		if (!query.exec (sql))
		    sqlError (sql, query.lastError ());

		int   today;
		while (query.next())
		    today    = query.value(0).toInt ();

		if ((debug & 1))
		    qDebug () << "ManyDay [sql]        " << sql;

		if ((debug & 2))
		     qDebug () << "    [day]        " << today;

		float theRate = ((today > 0) && (today < 6)) ? dayRate : weekEndRate;

		sql = QString ("insert into time values (NULL, '%1', %2, %3, NULL, '%4', '%5');")
			.arg (comboBox->currentText ())
			.arg (theRate)
			.arg (theTime)
			.arg (shiftStart.toString ("yyyy-MM-dd HH:mm:00"))
			.arg (niteStart.toString ("yyyy-MM-dd HH:mm:00"));
		if ((debug & 1))
		    qDebug () << "4   [sql]" << sql;
		if (!query.exec (sql))

		if ((debug & 1))
		    qDebug () << "5   [sql]" << sql;

		sql = QString ("insert into time values (NULL, '%1', %2, 0, 1, '%3', '%4');")
			.arg (comboBox->currentText ())
			.arg (niteRate)
			.arg (niteStart.toString ("yyyy-MM-dd HH:mm:00"))
			.arg (niteEnd.toString ("yyyy-MM-dd HH:mm:00"));

		if ((debug & 1))
		    qDebug () << "         " << sql;

		if (!query.exec (sql))
		    sqlError (sql, query.lastError ());

		shiftStart = niteEnd;
		niteStart = niteStart.addDays (1);
		niteEnd = niteEnd.addDays (1);

		if ((debug & 2))
		    qDebug () << "shuffle forward"
		                 "\n    shiftStart" << shiftStart
			      << "\n    niteStart " << niteStart               
			      << "\n    niteEnd   " << niteEnd;


		if (shiftStart >= shiftEnd)				// All Entries Done
		    break;
	    }
	}
    }
    showRecord ();
}

/*-------------------------------------------------------------------------------------------------
 * on_actionDelete_triggered
 *-----------------------------------------------------------------------------------------------*/
void Record::on_actionDelete_triggered ()
{
QString sql;
QSqlQuery query;

    QModelIndexList indexList = tableView->selectionModel()->selectedIndexes();
    int last = 0;
    foreach (QModelIndex index, indexList) {
	int row = index.row ();
	QModelIndex indx = index.model()->index (row, 0);
	QVariant cell = indx.model()->data (indx, Qt::DisplayRole);
	if (last != cell.toInt ()) {
	    last = cell.toInt ();
	    sql = "delete from time where ID=" + cell.toString() + ";";
	    if ((debug & 2))
		qDebug () << "[row]" << row << "    [sql]" << sql;;
	    if (!query.exec (sql))
		sqlError (sql, query.lastError ());
	}
    }
    actionDelete->setEnabled (false);
    showRecord ();
}

/*-------------------------------------------------------------------------------------------------
 * on_actionQuit_triggered --
 *	data is in DB so saving does not matter
 *-----------------------------------------------------------------------------------------------*/
void Record::on_actionQuit_triggered ()
{
    writeSettings();
    qApp->quit ();
}

/*-------------------------------------------------------------------------------------------------
 * sqlError --
 *-----------------------------------------------------------------------------------------------*/
void Record::sqlError (QString sql, QSqlError err)
{
    QMessageBox::warning(this, tr ("record"), tr ("Error:") + sql + "\n" + err.text());
}

/*-------------------------------------------------------------------------------------------------
 * readSettings --
 *-----------------------------------------------------------------------------------------------*/
void Record::readSettings()
{
    QSettings settings("helpers", "record");

    restoreGeometry(settings.value("geometry").toByteArray());
    dbName = settings.value("dbName").toString();
}

/*-------------------------------------------------------------------------------------------------
 * writeSettings --
 *-----------------------------------------------------------------------------------------------*/
void Record::writeSettings()
{
    QSettings settings("helpers", "record");
    settings.setValue("geometry", saveGeometry());
    //qDebug () << "save Geometry";
    settings.setValue("dbName", dbName);
}

/*--------------------------------------------------------------------------------------------------
 * setOverniteTime --
 * niteStart any time during night eg 18:00 to 30:00 (say 6 next morning)
 * niteEnd any time after that (say 31:00 eg 7 next morning)
 *------------------------------------------------------------------------------------------------*/
QDateTime Record::setOverniteTime (QString date, QString time)
{
    QDateTime period (QDateTime::fromString (date, "yyyy-MM-dd HH:mm"));
    if ((debug & 2)) {
	qDebug () << " setOverniteTime";
	qDebug () << "    date" << date << "time" << time;
    }
    int indx = time.indexOf (':');
    int hrs= time.mid (0, indx).toInt ();			// 0..1..2 length = 2 upto index 2
    if ((debug & 2))
	 qDebug () << "    hours" << hrs;
    if (hrs > 23) {
	period = period.addDays (1);
	if ((debug & 2))
	    qDebug () << "    add 1 day" << period;
	hrs -= 24;
	QString ourTime = QString::number (hrs);
	ourTime.append (time.mid (indx));
	period.setTime (QTime::fromString (ourTime, "HH:mm"));
	if ((debug & 2))
	    qDebug () << "    and setime" << period;
    }
    else
	period.setTime (QTime::fromString (time, "HH:mm"));

    if ((debug & 2)) {
	qDebug () << "    returning" << period.toString ("yyyy-MM-dd HH:mm");
	qDebug () << "/setOverniteTime";
    }
    return period;
}

/*--------------------------------------------------------------------------------------------------
    this->setStyleSheet ("* {background-color: #fbefcb;} "
			 "QLineEdit {background-color: #fcda90;} "
			 "QPushButton {background-color: #fcda90;} "
			 "QMenu::item:selected {background-color: #654321;} "
			 "QHeaderView::section {background-color: #fcda90;} "
			 "QComboBox {background-color: #e0c070;} "
			 //"QComboBox:!editable:on {color: yellow;}"
			 //"QComboBox:editable:on {background: white;}"
			 //"QComboBox:!editable {background: #654321;}"
			 );




    QListView * listView = new QListView(comboBox);
    listView->setStyleSheet("QListView::item {                              \
                             border-bottom: 5px solid white; margin:3px;    \
                             color: red;}				    \
                             QListView::item:selected {                     \
                             border-bottom: 5px solid black; margin:3px;    \
                             color: black;                                  \
                            }                                               \
                            ");
    comboBox->setView(listView);
 *------------------------------------------------------------------------------------------------*/
