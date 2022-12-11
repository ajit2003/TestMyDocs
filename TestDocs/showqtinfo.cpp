#include "showqtinfo.h"
#include "ui_showqtinfo.h"

/*!
 * \fn ShowQtInfo::ShowQtInfo
 * \brief Creates and setups the GUI of MainWindow
 * \details Uses setupUI() function of ui class
 * \param parent
 */
ShowQtInfo::ShowQtInfo(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ShowQtInfo)
{
    ui->setupUi(this);
}

/*!
 * \fn ShowQtInfo::~ShowQtInfo
 * \brief Destructor
 */
ShowQtInfo::~ShowQtInfo()
{
    delete ui;
}

/*!
 * \fn ShowQtInfo::returnStatus
 * \brief Returns the status
 * \return Status
 */
int ShowQtInfo::returnStatus()
{

}

