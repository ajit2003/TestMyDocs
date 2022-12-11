#include "showqtinfo.h"

#include <QApplication>

/*!
 * \fn main
 * \brief Entry point of the program
 * \details Initializes the mainwindow
 * \param argc
 * \param argv
 * \return
 */
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ShowQtInfo w;
    w.show();
    return a.exec();
}
