#ifndef NIGHT_H
#define NIGHT_H

#include <QDialog>

#include "ui_night.h"

class QSqlError;
class Record;

class NightDialog : public QDialog, public Ui::NightDialog
{
    Q_OBJECT

public:
    NightDialog(QWidget *parent = 0);

private slots:
    void on_pushButtonSubmit_pressed ();
    void on_pushButtonQuit_pressed ();
    
private:
    bool empty;
    void sqlError (QString sql, QSqlError err);
};

#endif

