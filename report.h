#ifndef REPORT_H
#define REPORT_H

#include <QDialog>
#include <QStandardItemModel>

#include "ui_report.h"

class QSqlError;
class QProcess;
class Record;

class GenDialog : public QDialog, public Ui::GenDialog
{
    Q_OBJECT

public:
    GenDialog(QWidget *parent = 0);

private slots:
    void doOnly ();
    void doPrint ();
    
private:
    QStandardItemModel *model;
    QStandardItemModel *model2;

    QString showOnly;
    QProcess *proc;
    Record *record;
    bool isSaved;

    void showReport ();
    void sqlError (QString sql, QSqlError err);
    void formatWindow (int newWidth);
    void screenWidthChanged (int newWidth);
};

#endif

