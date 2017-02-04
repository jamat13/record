#ifndef REGULAR_H
#define REGULAR_H

#include <QDialog>
#include <QStandardItemModel>

#include "ui_regular.h"

class QSqlError;
class Record;

class RegularDialog : public QDialog, public Ui::RegularDialog
{
    Q_OBJECT

public:
    RegularDialog(QWidget *parent = 0);

private slots:
    void on_pushButtonAdd_pressed ();
    void on_pushButtonDel_pressed ();
    void on_pushButtonQuit_pressed ();
    void on_pushButtonSubmit_pressed ();
    void rowSelected (const QModelIndex index);
    
private:
    QStandardItemModel *model;
    QString selectedId;

    void showRegular ();
    void sqlError (QString sql, QSqlError err);
};

#endif

