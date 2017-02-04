#include <QApplication>
#include <QtDebug>
#include <QDir>
#include "record.h"
     
int debug = 0;

#ifdef Q_OS_MAC
extern void qt_set_sequence_auto_mnemonic(bool b);
#endif

/*-------------------------------------------------------------------------------------------------
 * Main --
 *-----------------------------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    for (int argpos = 1; argpos < argc; ++argpos) {
        QString arg(argv[argpos]);
        if ((arg == "-h") || (arg == "--help")) {
            qDebug ("Usage: [-v(sql) -vv (deneral) -vvv (detail) [-h] [--help]");
            exit (0);
        }
        if (arg == "-v") {
	    debug |= 1;
	}
        if (arg == "-vv") {
	    debug |= 2;
	}
        if (arg == "-vvv") {
	    debug |= 4;
	}
    }

    QApplication app(argc, argv);
    Record window;
    window.show();
    return app.exec();
}
