#ifndef HELPERS_H
#define HELPERS_H

#include <QDialog>
#include <QStandardItemModel>

#include "ui_helpers.h"

class QSqlError;
class Record;

class HelpersDialog : public QDialog, public Ui::HelpersDialog
{
    Q_OBJECT

public:
    HelpersDialog(QWidget *parent = 0);

private slots:
    void doAdd ();
    void doDelete ();
    void doAccept ();
    void rowSelected (const QModelIndex indx);
    void on_buttonSubmit_pressed ();
    
private:
    QStandardItemModel *model;
    Record *record;
    QString selectedId;

    void sqlError (QString sql, QSqlError err);
    void showHelpers ();
    void formatWindow (int newWidth);
    void screenWidthChanged (int newWidth);
};

#endif

