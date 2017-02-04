#ifndef RECORD_H
#define RECORD_H

#include <QMainWindow>
#include "build/ui_record.h"

class QStandardItemModel;
class QSqlError;
class QProcess;

class Record : public QMainWindow, public Ui::Record
{
    Q_OBJECT

//-----------------------------------------------------------------------------------------------
public:
    Record (QWidget *parent = 0);

    QString selectedName;
    QString selectedRate;
    QString selectedPay;
    QString selectedHours;
    QString selectedStart;
    QString selectedEnd;
    int selectedId;

public slots:
    void showRecord();					//used by helpers dialog

protected:
private slots:
    void on_actionAbout_triggered ();
    void on_actionAdd_triggered ();
    void on_actionAddRegulars_triggered ();
    void on_actionAppearence_triggered ();
    void on_actionClone_triggered ();
    void on_actionDelete_triggered ();
    void on_actionHelp_triggered ();
    void on_actionHelpers_triggered ();
    void on_actionLoad_triggered ();
    void on_actionNew_triggered ();
    void on_actionNight_triggered ();
    void on_actionQuit_triggered ();
    void on_actionRegular_triggered ();
    void on_actionReports_triggered ();
    void on_lineEditStart_textChanged (const QString &);
    void on_pushButtonCancel_pressed ();
    void on_pushButtonSubmit_pressed ();
    void rowSelected (const QModelIndex indx);

private:
    QString dbName;
    QString dbUser;
    QString dbPassword;
    QString dbHost;
    QString dbType;

    QProcess *proc;

    QStandardItemModel *model;

    void sqlError (QString sql, QSqlError err);
    void readSettings();
    void writeSettings();
    void dbSetup ();

    QDateTime setOverniteTime (QString date, QString time);
};

#endif
