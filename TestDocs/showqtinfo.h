#ifndef SHOWQTINFO_H
#define SHOWQTINFO_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class ShowQtInfo; }
QT_END_NAMESPACE

class ShowQtInfo : public QMainWindow
{
    Q_OBJECT

public:
    ShowQtInfo(QWidget *parent = nullptr);
    ~ShowQtInfo();

private:
    Ui::ShowQtInfo *ui;
    int returnStatus();
};
#endif // SHOWQTINFO_H
