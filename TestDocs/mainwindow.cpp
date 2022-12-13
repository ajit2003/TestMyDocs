/*!
  \class mainwWindow.cpp
 */
#include "mainwindow.h"
#include "dashboard.h"
#include "qobjectdefs.h"
#include "ui_dashboard.h"
#include "ui_globalreplacedialog.h"
#include "ui_mainwindow.h"
#include "averageaccuracies.h"
#include "eddis.h"
#include "slpNPatternDict.h" //as included through lcsqt.h
#include "trieEditdis.h"
#include "meanStdPage.h"
#include <math.h>
#include "QProgressBar"
#include <QPrinter>
#include <QPrintPreviewDialog>
#include "DiffView.h"
#include <QtConcurrent/QtConcurrent>
#include "diff_match_patch.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include "interndiffview.h"
#include "commentsview.h"
#include "Symbols.h"
#include "loadingspinner.h"
#include "textfinder.h"
#include "resizeimageview.h"
#include <string>
#include <fstream>
#include <vector>
#include <utility>
#include <regex>
#include "crashlog.h"
#include "ProjectHierarchyWindow.h"
#include <QDomDocument>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QTreeView>
#include <QFont>
#include <git2.h>
#include "shortcutguidedialog.h"
#include <QFileSystemWatcher>
#include <set>
#include <algorithm>
#include <QSet>
#include <QAction>
#include "ProjectWizard.h"
#include <QDebug>
#include <QtCore>
#include <QtXml>
#include <QJsonObject>
#include <QTextDocumentFragment>
#include <sstream>
#include <QVector>
#include <QThread>
#include <vector>
#include <QJsonValue>
#include <QGraphicsRectItem>
#include <QToolTip>
#include <QSyntaxHighlighter>
#ifdef __unix__
#include <unistd.h>
#endif
#include <editdistance.h>
#include <QRegularExpressionMatch>
#include "undoglobalreplace.h"
#include "globalreplacepreview.h"
#include "qtextdocumentfragment.h"
#include <QColorDialog>
#include "worker.h"
#include <QThread>
#include "verifyset.h"
#include "loaddataworker.h"
#include "globalreplaceworker.h"
#include "pdfhandling.h"
#include "customtextbrowser.h"
#include "pdfrangedialog.h"
#include <QtNetworkAuth>
#include <QOAuth2AuthorizationCodeFlow>
#include <dashboard.h>
#include "printworker.h"
#include <QRadioButton>
#include <equationeditor.h>
#include "threadingpush.h">
#include <QThread>

//gs -dNOPAUSE -dBATCH -sDEVICE=jpeg -r300 -sOutputFile='page-%00d.jpeg' Book.pdf
map<string, string> LSTM;
map<string, int> Dict, GBook, IBook, PWords, PWordsP,ConfPmap,ConfPmapFont,CPairRight;
trie TDict,TGBook,TGBookP, newtrie,TPWords,TPWordsP;
vector<string> vGBook,vIBook;
QImage imageOrig;
QString gDirOneLevelUp,gDirTwoLevelUp,gCurrentPageName, gCurrentDirName;
map<QString, QString> gInitialTextHtml;
QString gTimeLogLocation;
map<QString, int> timeLog;
QMap<QString, QJsonArray> newTimeLog; // Introducing newTimeLog for storing more than one jsonvalue.
vector<QString> vs; vector<int> vx, vy, vw, vh, vright;
map<string, vector<string>> SRules;
map<string, string> TopConfusions;
map<string, int> TopConfusionsMask;
map<string, int> TimeLog;
string TimeLogLocation = "../Logs/log.txt";
string alignment = "left";
bool prevTRig = 0;
map<string, vector<int>> synonym;
vector<vector<string>> synrows;
//map<string, int> GPage; trie TGPage;
//map<string, int> PWords;//Common/Possitive OCR Words // already defined before
map<string, string> CPair;//Correction Pairs
std::map<string, set<string> > CPairs;
bool highlightchecked = false;
map<int, QString> commentdict;
map<int, vector<int>> commentederrors;
int openedFileChars;
int openedFileWords;
bool gSaveTriggered = 0;
bool LoadDataFlag = 1; //To load data only once
bool uploadReplaceFlag = 0; //To highlight in Upload and Replace
map<QString, QString> filestructure_fw;
QMap <QString, QString> mapOfReplacements;

map<QString, QString> filestructure_bw = { {"VerifierOutput","CorrectorOutput"},
                                           {"CorrectorOutput","Inds"},
                                           {"Inds" , "Inds"}
                                         };

QString gSanskrit, gHindi;

bool drawRectangleFlag=false;      //This flag is to prevent inserting the IMAGEHOLDER twice

bool loadimage=false;           //Check image is loaded on not

bool shouldIDraw=false;         //button functioning over marking a region for figure/table/equations

int pressedFlag;            //Resposible for dynamic rectangular drawing
QString ProjFile;
QString branchName;
int grdFlag = 0;

QMap<QString, QString> globallyReplacedWords;

QList<QString> filesChangedUsingGlobalReplace;


/*!
 * \fn MainWindow::MainWindow
 * \brief This is the constructor which creates the GUI and does all the prerequisites which are needed to be done
 * \param parent
 */
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    CustomTextBrowser *customtextbrowser = new CustomTextBrowser();
    customtextbrowser->setStyleSheet("background-color:white; color:black;");
    ui->splitter->replaceWidget(1,customtextbrowser);
    customtextbrowser->show();

    ui->splitter->setStyleSheet("background-color:white;color:black;");
    ui->splitter_2->setStyleSheet("background-color:rgb(32, 33, 72);color:black;");

    qInstallMessageHandler(crashlog::myMessageHandler);
    toolDirAbsolutePath = QDir::currentPath(); // Setting toolPath

    int largeWidth = QGuiApplication::primaryScreen ()->size ().width ();
    ui->splitter->setSizes(QList<int>({largeWidth/2 , largeWidth, largeWidth}));
    ui->lineEditSearch->setPlaceholderText("Search");
    QIcon search_1("./Resources/search.jpeg");
    ui->lineEditSearch->addAction(search_1, QLineEdit::LeadingPosition);
    ui->lineEdit_4->setPlaceholderText("Search");
    ui->lineEdit_4->addAction(search_1, QLineEdit::LeadingPosition);
    ui->lineEdit->setReadOnly(true);
    ui->lineEdit_2->setReadOnly(true);
    ui->lineEdit_3->setReadOnly(true);

    googleAuth();

    QString password  = "";
    QString passwordFilePath = QDir::currentPath() + "/pass.txt";
    QFile passwordFile(passwordFilePath);
    if(passwordFile.open(QFile::ReadOnly | QFile::Text))
        password = passwordFile.readAll().replace("\n","").replace("\r","");
    passwordFile.close();


    map<QString, QString> passwordRoleMap = { { "x3JzWx5KY}Gd&,]A" ,"Verifier"},
                                              { "3`t,FxjytJ[uU,HW" ,"Corrector"},
                                              { "$5Y9hkc+`{<7N%{L:KuR", "Admin"},
                                              { "sfbkasg81!248-Bks","Project Manager"}
                                            };
    if(!setRole(passwordRoleMap[password])){
        mExitStatus = true;
    }

    this->show();
    this->setWindowState(Qt::WindowState::WindowActive);
    //ask for login
    QSettings settings("IIT-B", "OpenOCRCorrect");
    settings.beginGroup("loginConsent");
    QString value = settings.value("consent").toString();
    if(value != "dna" && value != "loggedIn"){
    QMessageBox login;
    login.setWindowTitle("Login using Google");
    login.setWindowFlags(Qt::CustomizeWindowHint|Qt::WindowTitleHint|Qt::WindowCloseButtonHint);
    login.setIcon(QMessageBox::Information);
    login.setInformativeText("You can save your edits on the cloud. To enable this feature please login using your google account now or later in settings > login");
    QPushButton *confirmButton = login.addButton(tr("Login"),QMessageBox::AcceptRole);
    QPushButton *cancelButton = login.addButton(tr("Cancel"),QMessageBox::ActionRole);
    QCheckBox *cb = new QCheckBox("Do not ask again");
    login.setCheckBox(cb);
    cb->setStyleSheet("QCheckBox{color:rgb(227, 228, 228);border:0px;}");

    login.exec();
    if(cb->checkState() == Qt::Checked){
        settings.setValue("consent","dna");
    }
    if(login.clickedButton() == confirmButton)
    {
      authenticate();
      this->close();
    }
    if(login.clickedButton() == cancelButton){
        login.close();
    }
    }
    //show login/logout options
    QString consent = settings.value("consent").toString();
    settings.endGroup();
    if(consent != "loggedIn"){
        this->ui->actionLogin->setVisible(true);
        this->ui->actionLogout->setVisible(false);
    }
    else{
        this->ui->actionLogin->setVisible(false);
        this->ui->actionLogout->setVisible(true);
    }

    QString common = "डॉ - xZ,, अ  - a,, आ/ ा  - A,, इ/ ि  - i,, ई/ ी  - I,, उ/ ु  - u,, ऊ/ ू  - U,, ऋ/ ृ  - f,, ए/ े  - e,, ऐ/ ै  - E,, ओ/ ो  - o,, औ/ ौ  - O,, ं  - M,, ः  - H,,  ँ   - ~,, ज्ञ  - jYa,, त्र  - tra,, श्र  - Sra,, क्ष्/क्ष  - kz/kza,, द्य्/द्य  - dy/dya,, क्/क  - k/ka,, ख्/ख  - K/Ka,, ग्/ग  - g/ga,, घ्/घ  - G/Ga,, ङ्/ङ  - N/Na,, च्/च  - c/ca,, छ्/छ  - C/Ca,, ज्/ज  - j/ja,, झ्/झ  - J/Ja,, ञ्/ञ  - Y/Ya,, ट्/ट  - w/wa,, ठ्/ठ  - W/Wa,, ड्/ड  - q/qa,, ढ्/ढ  - Q/Qa,, ण्/ण  - R/Ra,, त्/त  - t/ta,, थ्/थ  - T/Ta,, द्/द  - d/da,, ध्/ध  - D/Da,, न्/न  - n/na,, प्/प  - p/pa,, फ्/फ  - P/Pa,, ब्/ब  - b/ba,, भ्/भ  - B/Ba,, म्/म  - m/ma,, य्/य  - y/ya,, र्/र  - r/ra,, ल्/ल  - l/la,, व्/व  - v/va,, श्/श  - S/Sa,, ष्/ष  - z/za,, स्/स  - s/sa,, ह्/ह  - h/ha,, ळ्/ळ  - L/La,, १  - 1,, २  - 2,, ३  - 3,, ४  - 4,, ५  - 5,, ६  - 6,, ७  - 7,, ८  - 8,, ९  - 9,, ०  - 0,, ।  - |,, ॥  - ||";
    gSanskrit = "SLP1 Sanskrit Guide:";
    gSanskrit += "\n";
    gSanskrit+= "ऽ - $,, ॐ - %,, ᳲ  - Z,,  ᳳ  - V,, ॠ/ ॄ  - F,, ऌ/ ॢ  - x,, ॡ/ \"ॣ\”  - X,, ,, ,, ";
    gSanskrit += common;
    gSanskrit.replace(",, ", "\n");

    gHindi = "SLP1 Hindi Guide:";
    gHindi += "\n";
    gHindi+= "ग़् - $,, ऩ् - %,, ऑ - Z,, ऱ् - V,, ज़ - F,, ड़्/ड़ -x/xa,, ढ़्/ढ़  - X/Xa,, य़्  - &,, क़ - @,, ख़ - #,, फ़् - ^,, ॅ - *,, ,, ,, ";
    gHindi += common;
    gHindi.replace(",, ", "\n");
    QFont font("Chandas");
    font.setWeight(14);
    font.setPointSize(12);
    ui->textEdit->setFont(font);
    ui->sanButton->setChecked(true);

    connect(ui->treeView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(CustomContextMenuTriggered(const QPoint&)));
    connect(ui->treeView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(file_click(const QModelIndex&)));

    ui->horizontalSlider->setMinimum(0);
    ui->horizontalSlider->setMaximum(800);
    ui->horizontalSlider->setValue(100);
    ui->horizontalSlider->setEnabled(false);

    connect(ui->horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(zoom_slider_valueChanged(int)));
    connect(ui->horizontalSlider, SIGNAL(sliderMoved(int)), this, SLOT(zoom_slider_moved(int)));

    qApp->installEventFilter(this);
    AddRecentProjects();

    // Add custom fonts
    QFontDatabase::addApplicationFont(":/Fonts/fonts/Meera/Meera-Regular.ttf");
    QFontDatabase::addApplicationFont(":/Fonts/fonts/Shobhika/Shobhika-Regular.otf");
    QFontDatabase::addApplicationFont(":/Fonts/fonts/Shobhika/Shobhika-Bold.otf");
    QFontDatabase::addApplicationFont(":/Fonts/fonts/Mandali/Mandali Regular.otf");
    QFontDatabase::addApplicationFont(":/Fonts/fonts/Latha/latha.ttf");
    QFontDatabase::addApplicationFont(":/Fonts/fonts/Nirmala/Nirmala Regular.ttf");
    QFontDatabase::addApplicationFont(":/Fonts/fonts/Chandas/chandas.ttf");

    if (!isVerifier)
    {
        ui->actionHighlight->setEnabled(false);
    }

    // Hiding options

    ui->actionLoadDict->setVisible(false);
    ui->actionLoadOCRWords->setVisible(false);
    ui->actionLoadDomain->setVisible(false);
    ui->actionLoadSubPS->setVisible(false);
    ui->actionLoadConfusions->setVisible(false);
    ui->actionLoadGDocPage->setVisible(false);
    ui->menuSelectLanguage->setTitle("");
    ui->menuCreateReports->setTitle("");

    // Disabling some buttons while opening the tool

    // File Menu
    ui->actionSave->setEnabled(false);
    ui->actionSave_As->setEnabled(false);
    ui->actionSpell_Check->setEnabled(false);
    ui->actionLoad_Prev_Page->setEnabled(false);
    ui->actionLoad_Next_Page->setEnabled(false);
    ui->actionToDevanagari->setEnabled(false);
    ui->actionToSlp1->setEnabled(false);
    ui->actionLoadGDocPage->setEnabled(false);
    ui->actionLoadData->setEnabled(false);
    ui->actionLoadDict->setEnabled(false);
    ui->actionLoadOCRWords->setEnabled(false);
    ui->actionLoadDomain->setEnabled(false);
    ui->actionLoadSubPS->setEnabled(false);
    ui->actionLoadConfusions->setEnabled(false);
    ui->actionSugg->setVisible(false);

    // Edit Menu
    ui->actionUndo->setEnabled(false);
    ui->actionRedo->setEnabled(false);
    ui->actionFind_and_Replace->setEnabled(false);
    ui->actionUndo_Global_Replace->setEnabled(false);
    ui->actionUpload->setEnabled(false);

    // Language Menu
    ui->actionSanskrit_2->setEnabled(false);
    ui->actionEnglish->setEnabled(false);
    ui->actionHindi->setEnabled(false);

    // Reports Menu
    ui->actionAccuracyLog->setEnabled(false);
    ui->actionViewAverageAccuracies->setEnabled(false);

    // View Menu
    ui->actionAllFontProperties->setEnabled(false);
    ui->actionBold->setEnabled(false);
    ui->actionItalic->setEnabled(false);
    ui->actionHighlight->setEnabled(false); // Already disabled in Corrector mode
    ui->actionLeftAlign->setEnabled(false);
    ui->actionRightAlign->setEnabled(false);
    ui->actionCentreAlign->setEnabled(false);
    ui->actionJusitfiedAlign->setEnabled(false);
    ui->actionSuperscript->setEnabled(false);
    ui->actionSubscript->setEnabled(false);
    ui->actionInsert_Horizontal_Line->setEnabled(false);
    ui->actionFontBlack->setEnabled(false);
    ui->actionInsert_Tab_Space->setEnabled(false);
    ui->actionPDF_Preview->setEnabled(false);

    // Table Menu inside View Menu
    ui->actionInsert_Table_2->setEnabled(false);
    ui->actionInsert_Columnleft->setEnabled(false);
    ui->actionInsert_Columnright->setEnabled(false);
    ui->actionInsert_Rowabove->setEnabled(false);
    ui->actionInsert_Rowbelow->setEnabled(false);
    ui->actionRemove_Column->setEnabled(false);
    ui->actionRemove_Row->setEnabled(false);

    // Versions Menu
    ui->actionFetch_2->setEnabled(false);
    ui->actionTurn_In->setEnabled(false);
    ui->actionVerifier_Turn_In->setEnabled(false);

    // Download Menu
    ui->actionas_PDF->setEnabled(false);

    ui->actionSymbols->setEnabled(false);
    ui->actionZoom_In->setEnabled(false);
    ui->actionZoom_Out->setEnabled(false);
}

/*!
 * \fn    MainWindow::setRole
 * \brief This function is used to show a box to user to select his role as verifier, project manager or
 *        corrector. After user chooses a role then this function sets other variables accordingly for limiting
 *        the access.
 * \param role
 * \return true/false
 */
bool MainWindow::setRole(QString role)
{
    this->mRole = role;

    //! Checking role
    if(mRole == "Admin")
    {
        QSettings settings("IIT-B", "OpenOCRCorrect");
        settings.beginGroup("SetRole");
        QString role;
        role = settings.value("role").toString();
        settings.endGroup();
        if(!role.isEmpty()){
            mRole = role;
            qDebug()<<"mRole Admin if: "<<mRole<<endl;
        }
        else
        {
            QMessageBox RoleBox;
            RoleBox.setWindowTitle("Select Role");
            RoleBox.setIcon(QMessageBox::Question);
            RoleBox.setInformativeText("Which Role do you want to Load?");

            #ifdef Q_OS_WIN
            QPushButton *correctorButton = RoleBox.addButton(("Corrector"),QMessageBox::AcceptRole);
            QPushButton *verifierButton = RoleBox.addButton(("Verifier"),QMessageBox::AcceptRole);
            QPushButton *managerButton = RoleBox.addButton(("Project Manager"),QMessageBox::AcceptRole);
            QAbstractButton *cancel = RoleBox.addButton(tr("Cancel"), QMessageBox::RejectRole);

            #else
            QPushButton *managerButton = RoleBox.addButton(("Project Manager"),QMessageBox::AcceptRole);
            QPushButton *verifierButton = RoleBox.addButton(("Verifier"),QMessageBox::AcceptRole);
            QPushButton *correctorButton = RoleBox.addButton(("Corrector"),QMessageBox::AcceptRole);
            QAbstractButton *cancel = RoleBox.addButton(tr("Cancel"), QMessageBox::RejectRole);

            #endif
            cancel->hide();

            QCheckBox *cb = new QCheckBox("Set Selected Role As Default");
            RoleBox.setCheckBox(cb);
            cb->setStyleSheet("QCheckBox{color:rgb(227, 228, 228);border:0px;}");

            RoleBox.exec();
            if(RoleBox.clickedButton() == verifierButton)
                mRole = "Verifier";
            else if(RoleBox.clickedButton() == correctorButton)
                mRole = "Corrector";
            else if(RoleBox.clickedButton() == managerButton)
                mRole = "Project Manager";
            else
                exit(0);

            if(cb->checkState() == Qt::Checked)
            {
                saveRole = mRole;
                qDebug()<<"save Role else:"<<saveRole;
                QSettings settings("IIT-B", "OpenOCRCorrect");
                settings.beginGroup("SetRole");
                settings.setValue("role",saveRole);
                settings.endGroup();
            }
        }

    }

    if(mRole == "Project Manager")
    {
        ui->actionNew_Project->setEnabled(true);    //enable the option
        ui->actionNew_Project->setVisible(true);    //make it visible
        mRole = "Verifier";
    }

    if(mRole == "Verifier")
    {
        //! setting its each filesructure with verifieroutput
        filestructure_fw = { {"Inds","VerifierOutput"},
                             {"CorrectorOutput","VerifierOutput",},
                             {"VerifierOutput","VerifierOutput" }
                           };
        isVerifier = 1;
        ui->actionTurn_In->setVisible(false);      //set false to its visibility; now shown
        ui->actionTurn_In->setEnabled(false);      //disable the option

        this->setWindowTitle("Udaan Editing Tool-Verifier");

    }
    else if(mRole == "Corrector")
    {
        //! setting its each filesructure with correctoroutput
        filestructure_fw = { {"Inds","CorrectorOutput"},
                             {"CorrectorOutput","CorrectorOutput",},
                             {"VerifierOutput","CorrectorOutput" }
                           };

        ui->compareVerifierOutput->setVisible(false);
        ui->compareVerifierOutput->setEnabled(false);

        ui->actionVerifier_Turn_In->setVisible(false);
        ui->actionVerifier_Turn_In->setEnabled(false);

        isVerifier = 0;
        this->setWindowTitle("Udaan Editing Tool-Corrector");
    }
    else
    {
        int result = QMessageBox::information(this,"Login","Login Failed");
        return false;
    }

    return true;
}

/*!
 * \fn MainWindow::~MainWindow
 * \brief Destructor of mainwindow
 */
MainWindow::~MainWindow()
{
    delete ui;
}

/*!
 * \fn    readJsonFile
 * \brief This function is used to load the json files into the software in read only mode or in text mode.
 * \param filepath
 * \return mainObj
 * \note   Uses QJsonObject library for json files.
 */
QJsonObject readJsonFile(QString filepath)
{
    QFile jsonFile(filepath);
    jsonFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QByteArray data = jsonFile.readAll();

    QJsonParseError errorPtr;
    QJsonDocument document = QJsonDocument::fromJson(data, &errorPtr);
    QJsonObject mainObj = document.object();
    jsonFile.close();
    return mainObj;
}

/*!
 * \fn    readJsonFile
 * \brief This function is used to write into the json files in the software. It uses write only mode of fle.
 * \param filepath
 * \param mainObj
 * \note  Uses QJsonObject library for json files.
 */
void writeJsonFile(QString filepath, QJsonObject mainObj)
{
    QJsonDocument document1(mainObj);

    QFile jsonFile(filepath);
    jsonFile.open(QIODevice::WriteOnly);
    jsonFile.write(document1.toJson());
    jsonFile.close();
}

/*!
 * \fn MainWindow::authenticate
 * \brief It starts the authentication process of the user.
 */
void MainWindow::authenticate() {
    this->google->grant();
}

/*!
 * \fn MainWindow::googleAuth
 * \brief Opens a browser where user has to sign in through his/her google account to authenticate
 */
void MainWindow::googleAuth()
{
     google = new QOAuth2AuthorizationCodeFlow;
     google->setScope("email");
     connect(google, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
             &QDesktopServices::openUrl);

     QProcess process;
     process.execute("curl -d -X -k -POST --header "
                     "\"Content-type:application/x-www-form-urlencoded\" https://udaaniitb.aicte-india.org/udaan/email/ -o client.json");

     QFile jsonFile("client.json");
     jsonFile.open(QIODevice::ReadOnly | QIODevice::Text);
     QByteArray data = jsonFile.readAll();

     QJsonParseError errorPtr;
     QJsonDocument document = QJsonDocument::fromJson(data, &errorPtr);
     QJsonObject mainObj = document.object();
     jsonFile.close();
     QString id = mainObj.value("client_id").toString();
     QString secret = mainObj.value("client_secret").toString();
     QFile::remove("client.json");
     QByteArray array = id.toLocal8Bit();
     const auto clientId = array.data();
     const QUrl authUri("https://accounts.google.com/o/oauth2/auth");
     const QUrl tokenUri("https://oauth2.googleapis.com/token");
     array = secret.toLocal8Bit();
     const auto clientSecret = array.data();
     const auto port = 8080;

     google->setAuthorizationUrl(authUri);
     google->setClientIdentifier(clientId);
     google->setAccessTokenUrl(tokenUri);
     google->setClientIdentifierSharedKey(clientSecret);

     google->setModifyParametersFunction([](QAbstractOAuth::Stage stage, QVariantMap* parameters) {
         // Percent-decode the "code" parameter so Google can match it
         if (stage == QAbstractOAuth::Stage::RequestingAccessToken) {
             QByteArray code = parameters->value("code").toByteArray();
             (*parameters)["code"] = QUrl::fromPercentEncoding(code);
         }
     });

     QOAuthHttpServerReplyHandler* replyHandler = new QOAuthHttpServerReplyHandler(port, this);
     google->setReplyHandler(replyHandler);
     connect(this->google, &QOAuth2AuthorizationCodeFlow::granted, [=](){
            const QString token = this->google->token();
//            qDebug()<<"Token "<<token;
            emit gotToken(token);

            auto reply = this->google->get(QUrl("https://www.googleapis.com/oauth2/v2/userinfo?access_token="+token));
            connect(reply, &QNetworkReply::finished, [reply, token, this](){
                const auto objectDetails = (QString)reply->readAll();
                QJsonDocument jsonResponse = QJsonDocument::fromJson(objectDetails.toUtf8());
                QJsonObject jsonObject = jsonResponse.object();
                QString email = jsonObject["email"].toString();
                QString id = jsonObject["id"].toString();
//                const QByteArray data = email.toUtf8();
//                qDebug()<<"hash"<<QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex();
             // qDebug()<<"email"<<email;
            //save details in QSettings
                QSettings settings("IIT-B", "OpenOCRCorrect");
                settings.beginGroup("login");
                settings.setValue("token",token);
                settings.setValue("email",email);
                settings.setValue("id",id);
                settings.endGroup();

                //saving details in database via Post request to api
//                QProcess process;

//               //qDebug()<<"curl -d -X -POST --header \"Content-type:application/x-www-form-urlencoded\" https://oauth2.googleapis.com/revoke?token="+token;
//                process.execute("curl -d -X -k -POST --header "
//                                "\"Content-type:application/x-www-form-urlencoded\" https://udaaniitb.aicte-india.org/udaan/user_detail/{email:"+email+",user_id:"+id+",token:"+token+"}/");

                //now login dialog should not appear
                settings.beginGroup("loginConsent");
                settings.setValue("consent","loggedIn");
                settings.endGroup();
                this->ui->actionLogin->setVisible(false);
                this->ui->actionLogout->setVisible(true);
            });
        });

}
QString file = "";
bool fileFlag = 0;
QElapsedTimer myTimer;
int secs;
int gSeconds;

/*!
 * \fn  MainWindow::SaveTimeLog()
 * \brief This function saves the time of changes commit into the log json file.
 *
 * \sa writeJsonFile()
 */
void MainWindow::SaveTimeLog()
{
    QJsonObject mainObj;
    QJsonObject page;

    int nMilliseconds = myTimer.elapsed();
    int sec = nMilliseconds / 1000;
    //! Iterating over newTimeLog and and assinging time details into the page and finally inserting page into json object.
    for (auto i = newTimeLog.begin(); i != newTimeLog.end(); i++)
    {
        page["directory"] = i.key();
        page["seconds"] = i.value().at(0).toInt()+sec;
        page["Date/Time"] = i.value().at(1).toString();
        mainObj.insert(i.key(), page);
    }
    writeJsonFile(gTimeLogLocation, mainObj);
}

/*!
 * \fn    MainWindow::DisplayTimeLog()
 * \brief This function displays the time in statusbar and gets update on every right click.
 */
void MainWindow::DisplayTimeLog()
{
    QString currentVersion = mProject.get_version();     //getting project version
    if(mRole == "Verifier" && mRole != currentVersion)
        currentVersion = QString::number(currentVersion.toInt() - 1);

//    gSeconds = timeLog[mRole +":"+ gCurrentPageName +":V-"+ currentVersion];
    gSeconds = newTimeLog.value(mRole +":"+ gCurrentPageName +":V-"+ currentVersion).at(0).toInt();
    int nMilliseconds = myTimer.elapsed();
    int gSeconds_ = gSeconds + nMilliseconds / 1000;
    int mins = gSeconds_ / 60;
    int seconds = gSeconds_ - mins * 60;
    ui->lineEdit->setText(QString::number(mins) + "mins " + QString::number(seconds) +
                          " secs elapsed on this page(Right Click to update)");        //updating time in UI
}

/*!
 * \fn    MainWindow::UpdateFileBrekadown
 * \brief This function is called whenever there is changes in folder structure or files.
 *        This function updates the location of files and folder into the global variables.
 */
void MainWindow::UpdateFileBrekadown()
{
    QFileInfo finfo(mFilename);
    QString qstr = finfo.fileName();

    string str = qstr.toStdString();
    str.erase(remove(str.begin(), str.end(), ' '), str.end());

    gCurrentPageName = QString::fromStdString(str);
    gDirTwoLevelUp = mProject.GetDir().absolutePath();
    gCurrentDirName = finfo.dir().dirName();
    gDirOneLevelUp = gDirTwoLevelUp + "/" + gCurrentDirName;

}

/*!
 * \fn    DisplayError
 * \brief This function is called whenever there is a expected error occured  and this function
 *        displays that error message using a message box.
 * \param error
 */
void DisplayError(QString error)
{
    QMessageBox msgBox;
    msgBox.setText(error);
    msgBox.exec();
}

vector<string> vGPage, vIPage, vCPage; // for calculating WER
vector<string> vBest;

bool RightclickFlag = 0;
string selectedStr ="";

/*!
 * \fn MainWindow::mousePressEvent
 * \param ev
 * \brief Checks if the right click is pressed on the mouse and loads suggestion changes
 * \details Custom mouse event is created which loads a suggestion and translation menu for the string on which the current string highlight is present. These are loaded with the help of dictionary files which are loaded with the help of loaddata function. Works with the help of a flag.
 * \sa print5NearestEntries(), print2OCRSugg(), loadWConfusionsNindex1(), editDist(), make_pair()
 */
void MainWindow::mousePressEvent(QMouseEvent *ev)
{
//!GIVE EVENT TO TEXT BROWSER INSTEAD OF MAINWINDOW
    if(!curr_browser)
        return;
    slpNPatternDict slnp;
    trieEditDis trie;

    // to make sure the right menu click is not taking place outside of the tabWidget_2
    QRect tabRect = curr_browser->frameGeometry();

    QPoint topLeftPoint = tabRect.topLeft();
    QPoint botRightPoint = tabRect.bottomRight();

    int topLeftx = topLeftPoint.x();
    int topLefty = topLeftPoint.y();
    int h =tabRect.height();
    int botRightx = botRightPoint.x();
    int botRighty = botRightPoint.y();
    QPoint point = ev->pos();
    int px = point.x();
    int py = point.y();


    if(!(px>=topLeftx && px<=botRightx &&  py>=150 /*&& py<(botRighty)*/)) return;

    if (curr_browser)
    {
        curr_browser->cursorForPosition(ev->pos());

        DisplayTimeLog();
        if((ev->button() == Qt::RightButton) && (LoadDataFlag)){
            QTextCursor cursor1 = curr_browser->cursorForPosition(ev->pos());
            QTextCursor cursor = curr_browser->textCursor();
            cursor.select(QTextCursor::WordUnderCursor);
            curr_browser->setContextMenuPolicy(Qt::CustomContextMenu);//IMP TO AVOID UNDO ETC AFTER SELECTING A SUGGESTION
            QMenu* popup_menu = curr_browser->createStandardContextMenu();
            QMenu* clipboard_menu;
            clipboard_menu = new QMenu("clipboard", this);
            clipboard_menu->setStyleSheet("height: 6em; width: 10em; overflow: hidden; white-space: nowrap; color: black; background-color: white;");
            QString menuStyle(
                        "QMenu::item{"
                        "background-color: rgb(255,255,255);"
                        "color: rgb(0,0,0);"
                        "}"

                        "QMenu::item:selected{"
                        "background-color: rgb(0, 85, 127);"
                        "color: rgb(255, 255, 255);"
                        "}"
                        "QMenu::item:disabled{"
                        "background-color: rgb(255, 255, 255);"
                        "color: rgb(128, 128, 128);"
                        "}"

                        );
            popup_menu->setStyleSheet(menuStyle);
            clipboard_menu->setStyleSheet(menuStyle);

            //QFont font("Shobhika-Regular");
            //font.setWeight(16);
            //font.setPointSize(16);
            //clipboard_menu->setFont(font);
            QAction* act;
            QSettings settings("IIT-B", "OpenOCRCorrect");
            settings.beginGroup("Clipboard");
            QString s1 = settings.value("copy1").toString();
            QString s2 = settings.value("copy2").toString();
            QString s3 = settings.value("copy3").toString();
            settings.endGroup();
            act = new QAction(s1,clipboard_menu);
            clipboard_menu->addAction(act);
            clipboard_menu->addSeparator();
            act = new QAction(s2,clipboard_menu);
            clipboard_menu->addAction(act);
            clipboard_menu->addSeparator();
            act = new QAction(s3,clipboard_menu);
            clipboard_menu->addAction(act);
            QAction* gsearch;
            gsearch = new QAction("Search over google",popup_menu);
            QAction* gtrans;
            gtrans = new QAction("Google translate",popup_menu);
            QAction* insertImage;
            insertImage = new QAction("Insert image",popup_menu);
            popup_menu->insertSeparator(popup_menu->actions()[0]);
            popup_menu->insertMenu(popup_menu->actions()[0], clipboard_menu);
            popup_menu->addAction(gsearch);
            popup_menu->addAction(gtrans);
            popup_menu->addAction(insertImage);

            connect(clipboard_menu, SIGNAL(triggered(QAction*)), this, SLOT(clipboard_paste(QAction*)));
            connect(gsearch, SIGNAL(triggered()), this, SLOT(SearchOnGoogle()));
            connect(gtrans, SIGNAL(triggered()), this, SLOT(GoogleTranslation()));
            connect(insertImage, SIGNAL(triggered()), this, SLOT(insertImageAction()));
            popup_menu->exec(ev->globalPos());
            popup_menu->close(); popup_menu->clear();
        }
        //! if right click
        //!

        if (((ev->button() == Qt::RightButton) && (!LoadDataFlag)) || (RightclickFlag))
        {
            QMenu* spell_menu, *translate_menu, *clipboard_menu;
            QAction *act;
            QTextCursor cursor1 = curr_browser->cursorForPosition(ev->pos());
            QTextCursor cursor = curr_browser->textCursor();
            cursor.select(QTextCursor::WordUnderCursor);
            //BlockUnderCursor
            // code to copy selected string:-
            QString str1 = cursor.selectedText();
            selectedStr = str1.toUtf8().constData();

            curr_browser->setContextMenuPolicy(Qt::CustomContextMenu);//IMP TO AVOID UNDO ETC AFTER SELECTING A SUGGESTION
            QMenu* popup_menu = curr_browser->createStandardContextMenu();

            translate_menu = new QMenu("translate", this);
            clipboard_menu = new QMenu("clipboard", this);
            clipboard_menu->setStyleSheet("height: 6em; width: 10em; overflow: hidden; white-space: nowrap; color: black; background-color: white;");
            QFont font("Shobhika-Regular");
            font.setWeight(16);
            font.setPointSize(16);

            translate_menu->setFont(font);
            clipboard_menu->setFont(font);

            QSettings settings("IIT-B", "OpenOCRCorrect");
            settings.beginGroup("Clipboard");
            QString s1 = settings.value("copy1").toString();
            QString s2 = settings.value("copy2").toString();
            QString s3 = settings.value("copy3").toString();
            settings.endGroup();
            act = new QAction(s1,clipboard_menu);
            clipboard_menu->addAction(act);
            clipboard_menu->addSeparator();
            act = new QAction(s2,clipboard_menu);
            clipboard_menu->addAction(act);
            clipboard_menu->addSeparator();
            act = new QAction(s3,clipboard_menu);
            clipboard_menu->addAction(act);


            popup_menu->insertSeparator(popup_menu->actions()[1]);
            popup_menu->insertMenu(popup_menu->actions()[1], translate_menu);

            popup_menu->insertSeparator(popup_menu->actions()[2]);
            popup_menu->insertMenu(popup_menu->actions()[2], clipboard_menu);


            //connect(spell_menu, SIGNAL(triggered(QAction*)), this, SLOT(menuSelection(QAction*)));
            connect(translate_menu, SIGNAL(triggered(QAction*)), this, SLOT(translate_replace(QAction*)));
            connect(clipboard_menu, SIGNAL(triggered(QAction*)), this, SLOT(clipboard_paste(QAction*)));
            QAction* gsearch;
            gsearch = new QAction("Search over google",popup_menu);
            QAction* gtrans;
            gtrans = new QAction("Google translate",popup_menu);
            QAction* insertImage;
            insertImage = new QAction("Insert image",popup_menu);
            popup_menu->insertSeparator(popup_menu->actions()[0]);
            //popup_menu->insertMenu(popup_menu->actions()[0], clipboard_menu);
            popup_menu->addAction(gsearch);
            popup_menu->addAction(gtrans);
            popup_menu->addAction(insertImage);

            //connect(clipboard_menu, SIGNAL(triggered(QAction*)), this, SLOT(clipboard_paste(QAction*)));
            connect(gsearch, SIGNAL(triggered()), this, SLOT(SearchOnGoogle()));
            connect(gtrans, SIGNAL(triggered()), this, SLOT(GoogleTranslation()));
            connect(insertImage, SIGNAL(triggered()), this, SLOT(insertImageAction()));
            QString str = QString::fromStdString(selectedStr);
              vector<string> Alligned = trie.print5NearestEntries(TGBookP, selectedStr);
            if (!selectedStr.empty() && !Alligned.empty()) {


                spell_menu = new QMenu("suggestions", this);

                spell_menu->setFont(font);


                QAction* act;
                vector<string>  Words1 = trie.print5NearestEntries(TGBook, selectedStr);
               // if (Words1.empty()) return;


                //if (Alligned.empty()) return;

                vector<string> PWords1 = trie.print5NearestEntries(TPWords, selectedStr);
               // if (PWords1.empty()) return;

                string PairSugg = slnp.print2OCRSugg(selectedStr, Alligned[0], ConfPmap, Dict); // map<string,int>&
              //  if (PairSugg.empty())return;

                vector<string>  Words = trie.print1OCRNearestEntries(slnp.toslp1(selectedStr), vIBook);
              //  if (Words.empty())return;


                //! find nearest confirming to OCR Sugg from Book
                string nearestCOnfconfirmingSuggvec;
                vector<string> vec = Words1;
                int min = 100;
                for (size_t t = 0; t < vec.size(); t++)
                {
                    vector<string> wordConfusions; vector<int> wCindex;
                    int minFactor = slnp.loadWConfusionsNindex1(selectedStr, vec[t], ConfPmap, wordConfusions, wCindex);
                    wordConfusions.clear(); wCindex.clear();
                    if (minFactor < min) { min = minFactor; nearestCOnfconfirmingSuggvec = vec[t]; }
                }

                //! find nearest confirming to OCR Sugg from PWords
                string nearestCOnfconfirmingSuggvec1;
                vector<string> vec1 = PWords1;
                min = 100;
                for (size_t t = 0; t < vec1.size(); t++) {
                    vector<string> wordConfusions; vector<int> wCindex;
                    int minFactor = slnp.loadWConfusionsNindex1(selectedStr, vec1[t], ConfPmap, wordConfusions, wCindex);
                    wordConfusions.clear(); wCindex.clear();
                    if (minFactor < min) { min = minFactor; nearestCOnfconfirmingSuggvec1 = vec1[t]; }
                }

                vector<pair<int, string>> vecSugg, vecSugg1;
                map<string, int> mapSugg;

                vector<string> out;
                map<string, set<string>>::iterator itr;
                set<string>::iterator set_it;

                for (itr = CPairs.begin(); itr != CPairs.end(); ++itr)
                {
                    if(slnp.toslp1(itr->first) == slnp.toslp1(selectedStr))
                    {
                        for (set_it = itr->second.begin(); set_it != itr->second.end(); ++set_it)
                        {
                           out.push_back(slnp.toslp1(*set_it));
                        }
                    }
                }

                cout<<"From CPairs: ";
                for(auto& it : out){
                    cout << slnp.toslp1(it) << endl;
                }
                for (size_t ksugg1 = 0; ksugg1 < 6; ksugg1++)
                {
                    if (out.size() > ksugg1)  mapSugg[slnp.toslp1(out[ksugg1])]++;
                }

                if(mProject.get_configuration()=="True")
                {
                    if (Words.size() > 0)  mapSugg[slnp.toslp1(Words[0])]++;
                    if (Words1.size() > 0) mapSugg[slnp.toslp1(nearestCOnfconfirmingSuggvec)]++;
                    if (PWords1.size() > 0) mapSugg[slnp.toslp1(nearestCOnfconfirmingSuggvec1)]++;
                    if (PairSugg.size() > 0) mapSugg[slnp.toslp1(PairSugg)]++;
                    mapSugg[trie.SamasBreakLRCorrect(slnp.toslp1(selectedStr), Dict, PWords, TPWords, TPWordsP)]++;
                    string s1 = slnp.toslp1(selectedStr);
                    string nearestCOnfconfirmingSuggvecFont = "";
                    min = 100;
                    for (size_t t = 0; t < vec.size(); t++)
                    {
                        vector<string> wordConfusions; vector<int> wCindex;
                        int minFactor = slnp.loadWConfusionsNindex1(s1, vec[t], ConfPmapFont, wordConfusions, wCindex);
                        wordConfusions.clear(); wCindex.clear();
                        if (minFactor < min) { min = minFactor; nearestCOnfconfirmingSuggvecFont = vec[t]; }
                    }
                    //if (nearestCOnfconfirmingSuggvecFont.size() > 0) mapSugg[nearestCOnfconfirmingSuggvecFont]++;

                    string PairSuggFont = "";
                    if (Alligned.size() > 0) PairSuggFont = slnp.print2OCRSugg(s1, Alligned[0], ConfPmap, Dict);
                    //if (PairSuggFont.size() > 0) mapSugg[PairSuggFont]++;

                    string sugg9 = "";
                    sugg9 = slnp.generatePossibilitesNsuggest(s1, TopConfusions, TopConfusionsMask, Dict, SRules);
                    //if (sugg9.size() > 0) mapSugg[sugg9]++;

                    cout<<"selected string: "<<slnp.toslp1(selectedStr)<<endl;
                    cout<<"Mapped Suggestion 0: "<<endl; //(string,int) Words, last no. of occuring, create single entry for single same word

                    cout<<"From Primary OCR: ";
                    for(auto& it : Words){
                        for(uint i = 0;i<it.size();i++){
                             cout << it[i];
                        }
                        cout<<"\n";
                    }
                    cout<<"Nearest confirming from Secondary OCR "<<nearestCOnfconfirmingSuggvec<<endl;
                    cout<<"Nearest confirming from PWords "<<nearestCOnfconfirmingSuggvec1<<endl;
                    cout<<"One suggestion from ConfusionPair and secondary OCR Trie Pattern Data "<<slnp.toslp1(PairSugg)<<endl;
                    cout<<"One suggestion from Pwords which is present in Dict "<<trie.SamasBreakLRCorrect(slnp.toslp1(selectedStr), Dict, PWords, TPWords, TPWordsP)<<endl;
            //                cout<<"Nearest confirming from Secondary OCR by converting the string in English "<<nearestCOnfconfirmingSuggvecFont<<endl;
            //                cout<<"One suggestion from ConfusionPair and secondary OCR Trie Pattern Data by converting the string in English "<<toslp1(PairSuggFont)<<endl;
            //                cout<<"One suggestion from TopConfusion and SandhiRules by converting the string in English "<<sugg9<<endl;
                }
                eddis e;
                for (map<string, int>::const_iterator eptr = mapSugg.begin(); eptr != mapSugg.end(); eptr++)
                {
                    vecSugg.push_back(make_pair(e.editDist(slnp.toslp1(eptr->first), slnp.toslp1(selectedStr)), eptr->first));
                }

                sort(vecSugg.begin(), vecSugg.end());

                cout << "\nVector Suggestions 0\n";
                cout << "MappingNum\t vector_int\t Word\n";
                for (uint i = 0; i < vecSugg.size(); i++){
                    cout<<mapSugg[vecSugg[i].second]<<"\t"<<vecSugg[i].first<<"\t"<<slnp.toDev(vecSugg[i].second)<<endl;
                }

                for (uint bitarrayi = 0; bitarrayi < vecSugg.size(); bitarrayi++)
                {
                    act = new QAction(QString::fromStdString(slnp.toDev(vecSugg[bitarrayi].second)), spell_menu);
                    spell_menu->addAction(act);
                }

                selectedStr.erase(remove(selectedStr.begin(), selectedStr.end(), ' '), selectedStr.end());
                vector<string> translate;
                vector<int>& syn = synonym[selectedStr];
                for(int i=0; i < syn.size(); i++)
                {
                    vector<string>& rowit = synrows[syn[i]];
                    for(int j=0; j < rowit.size(); j++)
                    {
                        if(rowit[j] != selectedStr)
                        {
                            translate.push_back(rowit[j]);
                            cout << rowit[j] << endl;
                        }
                    }
                }

                for (uint bitarrayi = 0; bitarrayi < translate.size(); bitarrayi++) {

                    act = new QAction(QString::fromStdString(translate[bitarrayi]), translate_menu);
                    translate_menu->addAction(act);
                }
                //For clipboard

                popup_menu->insertSeparator(popup_menu->actions()[0]);
                popup_menu->insertMenu(popup_menu->actions()[0], spell_menu);



                connect(spell_menu, SIGNAL(triggered(QAction*)), this, SLOT(menuSelection(QAction*)));



            }

                DisplayTimeLog();

                //QMenu* popup_menu = curr_browser->createStandardContextMenu();
                popup_menu->exec(ev->globalPos());
                popup_menu->close(); popup_menu->clear();

        } // if right click
    }
}// if mouse event

/*!
 * \fn    MainWindow::menuSelection
 * \brief This function provides the functionality to select the menu options like slp Dict
 *        words or other to  insert into the page.
 * \param action
 */
void MainWindow::menuSelection(QAction* action)
{
    slpNPatternDict slnp;
    trieEditDis trie;
    if (curr_browser)  //checks for current browser that is page.
    {
        QTextCursor cursor = curr_browser->textCursor();
        cursor.select(QTextCursor::WordUnderCursor);
        cursor.beginEditBlock();
        cursor.removeSelectedText();

        string target = (action->text().toUtf8().constData());
        CPair[slnp.toslp1(selectedStr)] = slnp.toslp1(target);
        PWords[slnp.toslp1(target)]++;
        cursor.insertText(action->text());     //inserting into the page

        cursor.endEditBlock();
    }
}

/*!
 * \fn    MainWindow::translate_replace
 * \brief This function replaces the words translated in the current page by accessing the selected
 *        text and removing old text and inserts new text on that selected part.
 * \param action
 */
void MainWindow::translate_replace(QAction* action)
{
    if (curr_browser)
    {
        QTextCursor cursor = curr_browser->textCursor();
        cursor.select(QTextCursor::WordUnderCursor);
        cursor.beginEditBlock();
        cursor.removeSelectedText();       //removes selected text

        string target = (action->text().toUtf8().constData());
        cursor.insertText(action->text());    //inserts new text
        cursor.endEditBlock();
    }
}

/*!
 * \fn MainWindow::clipboard_paste
 * \brief Inserts the text according to the action chosen
 * \param action
 */
void MainWindow::clipboard_paste(QAction* action)
{
    QTextCursor cursor = curr_browser->textCursor();
    cursor.insertText(action->text());
}
/*!
 * \fn MainWindow::on_actionSanskrit_triggered()
 * \brief Sets the language of the current broweser to Sanskrit by by passing the SanFlag as true
 * \sa setText()
*/
void MainWindow::on_actionSanskrit_triggered()
{
    int HinFlag = 0, SanFlag = 1;
    ui->textEdit->setText(gSanskrit);//whenever language change is required it will be converted to Sanskrit using the slpNPatternDict.h
    ui->hinButton->setChecked(HinFlag);
}


/*!
 * \fn MainWindow::on_actionHindi_triggered
 * \brief Sets the language of the current broweser to Hindi by by passing the HinFlag as true
 * \sa setText()
*/
void MainWindow::on_actionHindi_triggered()
{
    int HinFlag = 1, SanFlag = 0;
    ui->textEdit->setText(gHindi);  //whenever language change is required it will be converted to Hindi using the slpNPatternDict.h
    ui->sanButton->setChecked(SanFlag);
}


/*!
 * \fn MainWindow::on_actionEnglish_triggered()
 * \brief Sets the language of the current broweser to Hindi by passing the HinFlag and the SanFlag as false
 * \sa setText()
*/
void MainWindow::on_actionEnglish_triggered()
{
    int HinFlag = 0, SanFlag = 0;
    ui->hinButton->setChecked(HinFlag);//whenever language change is required it will be left as it is
    ui->sanButton->setChecked(SanFlag);
}

/*!
 * \fn MainWindow::on_actionOpen_Project_triggered
 * \brief Opens a new OCR project
 * \note Every project contains six folders - Images, Inds, CorrectorOutput, VerifierOutput, Dicts and Comments.
 *
 * \sa process_xml(), open_git_repo(), get_stage(), get_version(), getModel(), AddTemp(), getFilter(), insert(), UpdateFileBrekadown(), readJsonFile()
 */
void MainWindow::on_actionOpen_Project_triggered() { //Version Based

    /* Description
     * 1. Check if file named "project.xml" exists else terminates the function.
     * 2. Create a new directory if CorrectorOutput, VerifierOutput or Comments folders does not exist.
     * 3. Loading the requisites.
     *    a) Processing the project.xml file.
     *    b) Load git repository.
     * 4. Set the model for ProjectHierarchyWindow(TreeView). TreeView is composed of Documents and Images.
     * 5. Reset the current file name and directory levels.
     * 6. Get the value for time elapsed from Timelog.json.
     */

    //QString ProjFile;
    if(mProject.isProjectOpen()){ //checking if some project is opened, then closing it before opening new project
        on_actionClose_project_triggered();
    }
    int totalFileCountInDir = 0;
    QMap<QString, int> fileCountInDir;
//to choose between recent three files
    if(isRecentProjclick == true && proj_flag == '0')
    {
      ProjFile = RecentProjFile;
    }
    else if(isRecentProjclick == true && proj_flag == '1')
    {
      ProjFile = RecentProjFile2;
    }
    else if(isRecentProjclick == true && proj_flag == '2')
    {
      ProjFile = RecentProjFile3;
    }
    else{
      ProjFile = QFileDialog::getOpenFileName(this, "Open Project", "./", tr("Project(*.xml)"));   //Opens only if the file name is Project.xml
    }

    if (ProjFile == "")
        return;

    isRecentProjclick = false;

    // Testing of project.xml
    VerifySet verifySetObj(ProjFile, toolDirAbsolutePath + "/projectXMLFormat.xml");
    int result = verifySetObj.testProjectXML();

    if (result != 0) {
        mProject.setProjectOpen(false);
        QMessageBox::warning(0, "Project XML file Error", "Project XML File is corrupted \n\nError "+ QString::fromStdString(std::to_string(verifySetObj.getErrorCode()))+": " + verifySetObj.getErrorString()+"\n\nPlease Report this to your administrator");
        return;
    }

    currentZoomLevel = 100;

    QFile xml(ProjFile);
    QFileInfo finfo(xml);
    QString basedir = finfo.absoluteDir().absolutePath();

    //!Initializes the string with directory name
    QString s1 = basedir + "/Images/";
    QString s2 = basedir + "/Inds/";
    QString s3 = basedir+"/CorrectorOutput/";
    QString s4 = basedir + "/VerifierOutput/";
    QString s5 = basedir + "/Comments/";

    //! Terminates function if Project.xml doesn't exist
    if (finfo.fileName() == "")
        return;

    //! Creates a new directory if the CorrectorOutput, VerifierOutput or Comments folders does not exist.
    if (!QDir(s3).exists())
    {
        QDir().mkdir(s3);
    }
    if (!QDir(s4).exists())
    {
        QDir().mkdir(s4);
    }
    if (!QDir(s5).exists())
    {
        QDir().mkdir(s5);
    }


    bool exists = QDir(s1).exists() && QDir(s2).exists();
    if (xml.exists()&& exists)
    {
        ui->treeView->reset();    //reinitialize the ProjectHierarchyWindow
        mProject.process_xml(xml);
        mProject.open_git_repo();   //Open git repo
        if(!mProject.isProjectOpen())
        {
            QMessageBox::warning(0, "Project Error", "Couldn't open project. Please check your project.");
            return;
        }
        ui->treeView->setModel(mProject.getModel());
        ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);

        QString stage = mProject.get_stage();                          //fetches the stage from project.xml file
        mProject.set_stage(mRole);
        QString version = mProject.get_version();                      //Fetches version from project.xml file
        ui->lineEdit_2->setText("Version: " + version);                //Updates version in ui

        //!Get the path of all necessary directories
        QDir dir = mProject.GetDir();

        QString dirName=dir.dirName();
        QString str1 = mProject.GetDir().absolutePath()+"/CorrectorOutput/";
        QString str2 = mProject.GetDir().absolutePath() + "/VerifierOutput/";
        QString str3 = mProject.GetDir().absolutePath() + "/Inds/";
        QString str4 = mProject.GetDir().absolutePath() + "/Images/";

        //!To lookout for changes in CorrectorOutput and VerifierOutput directory
        watcher.addPath(str1);
        watcher.addPath(str2);

        //!To Display tree view for Document
        QDir cdir(str1);

        Filter * filter = mProject.getFilter("CorrectorOutput");
        //Filter * filter2 = mProject.getFilter("CorrectorOutput");
        //Filter * filter1 = mProject.getFilter("VerifierOutput");
        //!Adds each file present in CorrectorOutput directory to treeView
        auto list = cdir.entryList(QDir::Filter::Files);

        for (auto f : list)
        {   QStringList x = f.split(QRegExp("[.]"));

            QString t = str1 + "/" + f;
            QFile f2(t);
            if(x[1]=="html") {
                totalFileCountInDir++;
                mProject.AddTemp(filter,f2,"");
            }
            corrector_set.insert(f);
        }
        fileCountInDir["Corrector"] = totalFileCountInDir;
        totalFileCountInDir = 0;
        //!Adds each file present in VerifierOutput directory to treeView
        cdir.setPath(str2);

        filter = mProject.getFilter("VerifierOutput");
        list = cdir.entryList(QDir::Filter::Files);
        for (auto f : list)
        {
            QStringList x = f.split(QRegExp("[.]"));

            QString t = str2 + "/" + f;
            QFile f2(t);
            if(x[1]=="html") {
                totalFileCountInDir++;
                mProject.AddTemp(filter, f2, "");
            }
            verifier_set.insert(f);
        }
        fileCountInDir["Verifier"] = totalFileCountInDir;
        totalFileCountInDir = 0;

        filter = mProject.getFilter("Document");
        //!Adds the files from inds folder to treeView
        cdir.setPath(str3);

        list = cdir.entryList(QDir::Filter::Files);
        for (auto f : list)
        {
            QString t = str3 + "/" + f;
            QFile f2(t);
            mProject.AddTemp(filter, f2, "");
            totalFileCountInDir++;
        }
        fileCountInDir["Inds"] = totalFileCountInDir;
        totalFileCountInDir = 0;

        //!To Display treeView for Image
        filter = mProject.getFilter("Image");

        //!Adds the files from Image folder to treeView
        cdir.setPath(str4);

        list = cdir.entryList(QDir::Filter::Files);
        for (auto f : list) {
            QString t = str4 + "/" + f;
            QFile f2(t);
            mProject.AddTemp(filter, f2, "");
            totalFileCountInDir++;
        }
        fileCountInDir["Image"] = totalFileCountInDir;
        totalFileCountInDir = 0; // Resetting variable to 0

        // Resizing scroll bar for project window

        int maxFilesInDir = 0;
        for (auto fileCount : fileCountInDir.values())
        {
            if (fileCount > maxFilesInDir)
                maxFilesInDir = fileCount;
        }

        QString heightValue;
        if (maxFilesInDir < 50) {
            heightValue = "200px";
        }
        else if (maxFilesInDir < 200) {
            heightValue = "60px";
        }
        else {
            heightValue = "50px";
        }

        QString projectWindowStylesheet = ui->treeView->styleSheet();
        int indexOfScrollBarProp = projectWindowStylesheet.indexOf("QScrollBar::handle:vertical");
        int heightProp = projectWindowStylesheet.indexOf("height:", indexOfScrollBarProp);

        if (heightProp != -1) {
            int startIndex = heightProp + 7; // here, 7 is the length of "height:" string
            int endIndex;
            for (int i = startIndex; projectWindowStylesheet[i] != ';'; i++)
                endIndex = i;

            int replaceSize = endIndex - startIndex + 1;
            projectWindowStylesheet.replace(startIndex, replaceSize, heightValue);
        }
        else {
            int insertHeightProp = projectWindowStylesheet.indexOf("{", indexOfScrollBarProp) + 1;
            projectWindowStylesheet.insert(insertHeightProp, "height:" + heightValue);
        }
        ui->treeView->setStyleSheet(projectWindowStylesheet);



//        //!Disable Corrector Turn In once the Corrector has Turned in until the next version is fetched.
//        if(!isVerifier)
//        {
//            if (stage != "Corrector")
//            {
//                ui->actionTurn_In->setEnabled(false);
//            }
//        }
        UpdateFileBrekadown();    //Reset the current file and dir levels

        //!Get the elapsed time in Timelog.json file under Comments folder
        gTimeLogLocation = gDirTwoLevelUp + "/Comments/Timelog.json";     //Navigate to Timelog.json uder Comments folder
        QJsonObject mainObj =  readJsonFile(gTimeLogLocation);

        //!Get the seconds elapsed for their file name in json file
        foreach(const QJsonValue &val, mainObj)
        {
            QString directory = val.toObject().value("directory").toString();
            int seconds    = val.toObject().value("seconds").toInt();
            QString dateTime = val.toObject().value("Date/Time").toString();

            newTimeLog[directory] = {seconds, dateTime};
//            timeLog[directory] = seconds;
        }

        //bool isSet = QDir::setCurrent(mProject.GetDir().absolutePath() + "/CorrectorOutput") ; //Change application Directory to any subfolder of mProject folder for Image Insertion feature.
        //if(!QDir(mProject.GetDir().absolutePath() + "/Images/Inserted").exists())
        //    QDir().mkdir(mProject.GetDir().absolutePath() + "/Images/Inserted");

        QMessageBox::information(0, "Success", "Project opened successfully.");
//        ui->tabWidget_2->removeTab(0);
        //!Genearte image.xml for figure/table/equation entries and initialize these values by 1.

        markRegion objectMarkRegion;

        objectMarkRegion.createImageInfoXMLFile();

        //!save project paths for showing it on recent projects
        QSettings settings("IIT-B", "OpenOCRCorrect");
        QString new_project = finfo.path()+"/project.xml";
        if(new_project != RecentProjFile && new_project != RecentProjFile2 && new_project != RecentProjFile3 ){
        settings.beginGroup("RecentProjects");
        settings.setValue("Project3",RecentProjFile2 );
        settings.setValue("Project2",RecentProjFile );
        settings.setValue("Project",finfo.path()+"/project.xml" );
        settings.endGroup();}
        //changing priorities of recent opened projects
        if(new_project == RecentProjFile2){
            QSettings settings("IIT-B", "OpenOCRCorrect");
            settings.beginGroup("RecentProjects");
            settings.setValue("Project",RecentProjFile2 );
            settings.setValue("Project2",RecentProjFile );
            settings.endGroup();
        }
        else if(new_project == RecentProjFile3){
            QSettings settings("IIT-B", "OpenOCRCorrect");
            settings.beginGroup("RecentProjects");
            settings.setValue("Project",RecentProjFile3 );
            settings.setValue("Project2",RecentProjFile );
            settings.setValue("Project3",RecentProjFile2 );
            settings.endGroup();
        }
        isRecentProjclick = false;

        // Setting Project window size and dict window size = 50% - 50%
//        QList<int> list1 = ui->splitter_2->sizes();
//        int totalHeight;
//        totalHeight = list1.at(0) + list1.at(1);
//        ui->splitter_2->setSizes(QList<int>() << totalHeight/2 << totalHeight/2);
    }
    else
    {
        QMessageBox::warning(0, "Project Error", "Couldn't open project. Please check your project.");
        return;
    }
     AddRecentProjects();//to load recent project without restarting app
     //--for last opened page--//
     QSettings settings("IIT-B", "OpenOCRCorrect");
     settings.beginGroup("RecentPageLoaded");
     QString stored_project = settings.value("projectName1").toString();
     QString stored_project2 = settings.value("projectName2").toString();
     QString stored_project3 = settings.value("projectName3").toString();
     settings.endGroup();
     if(ProjFile == stored_project || ProjFile == stored_project2 || ProjFile == stored_project3){
         RecentPageInfo();
     }

     // Enabling the buttons again after a project is opened

     // File Menu
     ui->actionSave->setEnabled(true);
     ui->actionSave_As->setEnabled(true);
     ui->actionSpell_Check->setEnabled(true);
     ui->actionLoad_Prev_Page->setEnabled(true);
     ui->actionLoad_Next_Page->setEnabled(true);
     ui->actionToDevanagari->setEnabled(true);
     ui->actionToSlp1->setEnabled(true);
     ui->actionLoadGDocPage->setEnabled(true);
     ui->actionLoadData->setEnabled(true);
     ui->actionLoadDict->setEnabled(true);
     ui->actionLoadOCRWords->setEnabled(true);
     ui->actionLoadDomain->setEnabled(true);
     ui->actionLoadSubPS->setEnabled(true);
     ui->actionLoadConfusions->setEnabled(true);
     ui->actionSugg->setEnabled(true);

     // Edit Menu
     ui->actionUndo->setEnabled(true);
     ui->actionRedo->setEnabled(true);
     ui->actionFind_and_Replace->setEnabled(true);
     ui->actionUndo_Global_Replace->setEnabled(true);
     ui->actionUpload->setEnabled(true);

     // Language Menu
     ui->actionSanskrit_2->setEnabled(true);
     ui->actionEnglish->setEnabled(true);
     ui->actionHindi->setEnabled(true);

     // Reports Menu
     ui->actionAccuracyLog->setEnabled(true);
     ui->actionViewAverageAccuracies->setEnabled(true);

     // View Menu
     ui->actionAllFontProperties->setEnabled(true);
     ui->actionBold->setEnabled(true);
     ui->actionItalic->setEnabled(true);
     ui->actionLeftAlign->setEnabled(true);
     ui->actionRightAlign->setEnabled(true);
     ui->actionCentreAlign->setEnabled(true);
     ui->actionJusitfiedAlign->setEnabled(true);
     ui->actionSuperscript->setEnabled(true);
     ui->actionSubscript->setEnabled(true);
     ui->actionInsert_Horizontal_Line->setEnabled(true);
     ui->actionFontBlack->setEnabled(true);
     ui->actionInsert_Tab_Space->setEnabled(true);
     ui->actionPDF_Preview->setEnabled(true);
     if (isVerifier)
         ui->actionHighlight->setEnabled(true);

     // Table Menu inside View Menu
     ui->actionInsert_Table_2->setEnabled(true);
     ui->actionInsert_Columnleft->setEnabled(true);
     ui->actionInsert_Columnright->setEnabled(true);
     ui->actionInsert_Rowabove->setEnabled(true);
     ui->actionInsert_Rowbelow->setEnabled(true);
     ui->actionRemove_Column->setEnabled(true);
     ui->actionRemove_Row->setEnabled(true);

     // Versions Menu
     ui->actionFetch_2->setEnabled(true);
     ui->actionTurn_In->setEnabled(true);
     ui->actionVerifier_Turn_In->setEnabled(true);

     // Download Menu
     ui->actionas_PDF->setEnabled(true);

     ui->actionSymbols->setEnabled(true);
     ui->actionZoom_In->setEnabled(true);
     ui->actionZoom_Out->setEnabled(true);
     //Reset loadData flag
     LoadDataFlag = 1;
     //reset data
     mFilename.clear();
     //mFilename1.clear();
     LSTM.clear();
     CPairs.clear();
     Dict.clear();
     GBook.clear();
     IBook.clear();
     PWords.clear();
     ConfPmap.clear();
     vGBook.clear();
     vIBook.clear();
     TDict.clear();
     TGBook.clear();
     TGBookP.clear();
     TPWords.clear();
     TPWordsP.clear();
     synonym.clear();
     synrows.clear();

     ui->pushButton->setDisabled(false);
      ui->pushButton_2->setDisabled(false);
     ui->viewComments->setDisabled(false);
     ui->compareCorrectorOutput->setDisabled(false);
    ui->groupBox->setDisabled(false);

}

/*!
 * \fn MainWindow::AddRecentProjects
 * \brief This function will allow user to open the last opened project.
 *
 * We use QSettings which allows for persistent storage and we store the project and we load the project
 * if user selects the name of the project in recent project section.
 *
 */
void MainWindow::AddRecentProjects()
{
    QSettings settings("IIT-B", "OpenOCRCorrect");
    settings.beginGroup("RecentProjects");
    RecentProjFile = settings.value("Project").toString();
    RecentProjFile2 = settings.value("Project2").toString();
    RecentProjFile3 = settings.value("Project3").toString();
    settings.endGroup();
    ui->menuRecent_Project->clear();

    if(RecentProjFile!="")
     {
      QAction *FileAction = new QAction(this);
      FileAction->setIconText("~"+ RecentProjFile);
      ui->menuRecent_Project->addAction(FileAction);
      connect(FileAction, &QAction::triggered, this , &MainWindow::on_action1_triggered);
     }
    if(RecentProjFile2!="")
     {
      QAction *FileAction = new QAction(this);
      FileAction->setIconText("~"+ RecentProjFile2);
      ui->menuRecent_Project->addAction(FileAction);
      connect(FileAction, &QAction::triggered, this , &MainWindow::on_action2_triggered);
     }
    if(RecentProjFile3!="")
     {
      QAction *FileAction = new QAction(this);
      FileAction->setIconText("~"+ RecentProjFile3);
      ui->menuRecent_Project->addAction(FileAction);
      connect(FileAction, &QAction::triggered, this , &MainWindow::on_action3_triggered);
     }
}


bool ConvertSlpDevFlag = 0;
/*!
* \fn MainWindow::SaveFile_GUI_Preprocessing()
* \brief This function saves the changes made in current page opened in
*        textbrowser.
*
* \sa SaveTimeLog(),DisplayTimeLog(),on_viewComments_clicked(),updateAverageAccuracies(),
*/
void MainWindow::SaveFile_GUI_Preprocessing()
{

    SaveTimeLog();
    if (!mProject.isProjectOpen())
        return;
    //! Adding entries in Timelog.json about the elapsed time
    QString currentVersion = mProject.get_version();
    gSeconds = newTimeLog.value(mRole +":"+ gCurrentPageName +":V-"+ currentVersion).at(0).toInt();
//    int nMilliseconds = myTimer.elapsed();
//    gSeconds += nMilliseconds / 1000;
/*    int nMilliseconds = myTimer.elapsed();
    gSeconds = nMilliseconds/1000; */                                //!Converting milliseconds to seconds
    if(mRole == "Verifier" && mRole != currentVersion)
        currentVersion = QString::number(currentVersion.toInt() - 1);   //!Version is decremented for Verifier

//    timeLog[mRole +":"+ gCurrentPageName +":V-"+ currentVersion]=gSeconds;
    QString dateTime = QDateTime::currentDateTime().toString();
    newTimeLog[mRole +":"+ gCurrentPageName +":V-"+ currentVersion] = {gSeconds, dateTime};

    SaveTimeLog();
    DisplayTimeLog();
    //! When changes are made by the verifier the following values are also updated.
    if(isVerifier)
    {
        gSaveTriggered = 1;
        on_viewComments_clicked();
        gSaveTriggered = 0;
        updateAverageAccuracies();
    }
    ConvertSlpDevFlag =1;
    QTextCharFormat fmt;
    //fmt.setForeground(QBrush(QColor(0,0,0)));           //!Setting foreground brush to render text
    QTextCursor cursor = curr_browser->textCursor();
    cursor.beginEditBlock();
    cursor.select(QTextCursor::Document);
    cursor.mergeCharFormat(fmt);
    cursor.endEditBlock();

    QString output = curr_browser->toHtml();

    QTextDocument doc;
    doc.setHtml( gInitialTextHtml[currentTabPageName] );
    s1 = doc.toPlainText();          //!before Saving
    s2 = curr_browser->toPlainText();       //!after Saving
}

/*!
* \fn MainWindow::SaveFile_Backend()
* \brief This function saves the changes made in current page opened in
*        textbrowser and also perform all backend task like saving files on loaction,
*        commits of the changes and perform global replce changes
*
*/
void MainWindow::SaveFile_Backend()
{
    slpNPatternDict slnp;
    QVector <QString> changedWords;
    QString tempPageName = gCurrentPageName;

    //! Selecting the location where file is to be saved
    QString changefiledir = filestructure_fw[gCurrentDirName];
    QString localFilename = gDirTwoLevelUp + "/" +changefiledir +"/" + tempPageName;

    localFilename.replace(".txt",".html");

    QFile sFile(localFilename);
    edit_Distance ed;
    changedWords = ed.editDistance(s1, s2);             // Update CPair by editdistance
    QVectorIterator<QString> i(changedWords);
    while (i.hasNext())
        qDebug() << i.next()<<endl;

    //! Do commit when there are some changes in previous and new html file on the basis of editdistance.
    if(changedWords.size())
    {
        if(mProject.get_version().toInt())     //Check version number
        {
            QString commit_msg = "Corrector Turned in Version: " + mProject.get_version();
            //!Check commit condition
            if(!mProject.commit(commit_msg.toStdString()))
            {
                //cout<<"Commit Unsuccessful"<<endl;
                return;
            }
            else
            {
                mProject.commit(commit_msg.toStdString());
                //cout<<"Commit Successful"<<endl;
            }
        }
    }

    //CPair.insert(CPair_editDis.begin(), CPair_editDis.end());
    //! Enters entries in CPairs through CPair_editDis; allows multiple entries for a incorrent word entry
    for(auto elem : CPair_editDis)
    {
       std::cerr << elem.first << " " << elem.second << "\n";
       std::cerr << slnp.toslp1(elem.first) << " " << slnp.toslp1(elem.second) << "\n";
       //CPair.insert(make_pair(toslp1(elem.first), toslp1(elem.second)));
       if ( CPairs.find(slnp.toslp1(elem.first)) != CPairs.end())
       {
           std::set< std::string>& s_ref = CPairs[slnp.toslp1(elem.first)];
           s_ref.insert(slnp.toslp1(elem.second));
       }
       else
       {
           CPairs[slnp.toslp1(elem.first)].insert(slnp.toslp1(elem.second));
       }
    }

    //! Reflecting CPairs entries in the file /Dicts/CPair; Making it dynamic
    QString filename12 = mProject.GetDir().absolutePath() + "/Dicts/" + "CPair";
    QFile file12(filename12);
    if(!file12.exists())
    {
       qDebug() << "No exist file "<<filename12;
    }
    else
    {
       qDebug() << filename12<<"exists";
    }

    //! Insert entries in Correct Formatting Hello (/t) hi,(comma)hiii
    if (file12.open(QIODevice::ReadWrite  | QIODevice::Text | QIODevice::Append))
    {
        QTextStream out(&file12);
        out.setCodec("UTF-8");
        map<string, set<string>>::iterator itr;
        set<string>::iterator set_it;

        for (itr = CPairs.begin(); itr != CPairs.end(); ++itr)
        {
            out <<  QString::fromStdString(slnp.toDev(itr->first)) << '\t';
            for (set_it = itr->second.begin(); set_it != itr->second.end(); ++set_it)
            {
                if(set_it != prev(itr->second.end()))
                {
                    out << QString::fromStdString(slnp.toDev(*set_it)) << ",";
                }
                else {
                    out << QString::fromStdString(slnp.toDev(*set_it));
                }

            }
            out <<"\n";
        }
         file12.close();
    }
}

/*!
* \fn MainWindow::SaveFile_GUI_Postprocessing()
* \brief This function saves the changes made in current page opened in
*        textbrowser and after selcting the loaction to save the file this funtion
*        convert all html output to plain text then perform saving.
*
*/
void MainWindow::SaveFile_GUI_Postprocessing()
{
    QString tempPageName = gCurrentPageName;

    //! Selecting the location where file is to be saved
    QString changefiledir = filestructure_fw[gCurrentDirName];
    QString localFilename = gDirTwoLevelUp + "/" +changefiledir +"/" + tempPageName;

    localFilename.replace(".txt",".html");

    //! Don't create and save new file if output file already exists.
    if (gCurrentDirName == "Inds" || isVerifier && gCurrentDirName == "CorrectorOutput")
    {
        QFileInfo check_file(localFilename);
        if (check_file.exists() && check_file.isFile())
        {
            return;
        }
    }

    QFile sFile(localFilename);
    //restoreBbox(&sFile); //getting title tag before saving the file
    QString output = curr_browser->toHtml();

    if(sFile.open(QFile::WriteOnly))
    {
        QTextStream out(&sFile);
        out.setCodec("UTF-8");          //!Sets the codec for this stream
        gInitialTextHtml[currentTabPageName] = output;
//        output = "<style> body{ width: 21cm; height: 29.7cm; margin: 30mm 45mm 30mm 45mm; } </style>" + output;     //Formatting the output using CSS <style> tag
        output = "<style> body{ width: 21cm; height: 29.7cm; margin: 30mm 45mm 30mm 45mm; } </style><head>"
                 "<script src=\"https://polyfill.io/v3/polyfill.min.js?features=es6\"></script>"
                 "<script id=\"MathJax-script\" async src=\"https://cdn.jsdelivr.net/npm/mathjax@3/es5/tex-mml-chtml.js\"></script></head>" + output;//for showing math equations in browser using MathJax library

        /* Doing equation png to equaton latex mapping
         * we are showing png in our tool and saving Latex form in html page */

        QRegularExpression rex("<img(.*?)>",QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatchIterator itr;
        itr = rex.globalMatch(output);
        while(itr.hasNext()){
            QRegularExpressionMatch match = itr.next();
            QString img = match.captured();
            if(img.contains("Equations_") && img.contains(".png")){
                qDebug()<<img;
                string img_ = img.toStdString();
                int ind = img_.find("/");
                int lindex = img_.find("png");
                string str = img_.substr(ind, lindex-ind);
                QString path = QString::fromStdString(str) + "tex";
                QFile f(path);
                if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    qDebug() << "Cannot open file"<<f;
                    continue;
                }
                QString line = f.readAll();
                f.close();
                output.replace(img,"<a name=\""+path+"\"></a>$$ "+line+" $$");

            }
        }
        // Formatting the output using CSS <style> tag
        // Add style tag just before head or add styling properties in the pre-made style tag
        int inputDataIndex = -1;
        if ((inputDataIndex = output.indexOf("</style>")) != -1) {
            output.insert(inputDataIndex - 1, "\nbody { width: 21cm; height: 29.7cm; margin: 30mm 45mm 30mm 45mm; }");
        } else if ((inputDataIndex = output.indexOf("</head>")) != -1) {
            output.insert(inputDataIndex - 1, "<style>\nbody { width: 21cm; height: 29.7cm; margin: 30mm 45mm 30mm 45mm; }\n</style>");
        }

        //removing empty p tags inserted by Qt.
//        QRegularExpression rex_empty("<p style=\"-qt-paragraph-type:empty;(.*?)</p>",QRegularExpression::DotMatchesEverythingOption);
//        output = output.remove(rex_empty);
        out << output;
        sFile.flush();      //!Flushes any buffered data waiting to be written in the \a sFile
        sFile.close();      //!Closing the file

//        // Fixing Word breaking problem after saving file
//        filterHtml(&sFile);
        //Inserting back bbox info
//        bboxInsertion(&sFile);
//		insertBboxes(&sFile);
        handleBbox->insertBboxes(&sFile);
    }

    //! Converting html output into plain text.
    QTextDocumentFragment qtextdocfragment;
    QString plain = qtextdocfragment.fromHtml(output).toPlainText();

    std::stringstream ss(plain.toStdString());
    std::string to;
    //! Appending the plain text in QVector<QString> object.
    QVector<QString> s;
    if (plain != NULL)
    {
        while(std::getline(ss,to,'\n'))
        {
            QString qstr = QString::fromStdString(to);
            s.append(qstr);
        }
    }

    //! Inserting string values in \a qjsonobj.
    QJsonObject qjsonobj;
    for(int i = 0;i < s.size(); i++)
    {
        QString z = QString::number(i);
        qjsonobj.insert(z, QJsonValue(s[i]));
    }
    int len = qjsonobj.length();

    localFilename.replace(".html",".json");         //!Replacing extension of file from .html to .json
    QFile sFile2(localFilename);

    //! Sets codec value and then adding values in file
    if(sFile2.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&sFile2);
        out.setCodec("UTF-8");
        out << "{\n";
        for(int x = 0; x<len; x++)
        {
            QString z = QString::number(x);
            out << "\"" << x << "\"" << ":" << "\"" << qjsonobj[z].toString() << "\"" <<","<< '\n';
        }
        out << "}";

        sFile2.flush();
        sFile2.close();
    }

    //! Set Inds file readonly after saving - Corrector mode
    if (!isVerifier && gCurrentDirName == "Inds")
    {
        if(QFile::exists(localFilename))
        {
            curr_browser->setReadOnly(true);
        }

    }

    //! Set Inds and CorrectorOutput files readonly after generating output file - Verifier mode
    if (isVerifier && (gCurrentDirName == "Inds" || gCurrentDirName == "CorrectorOutput"))
    {
        if(QFile::exists(localFilename))
        {
            QString Inds_file = gCurrentPageName;
            Inds_file.replace(".html", ".txt");
            QString Corr_file = Inds_file;
            Corr_file.replace(".txt", ".html");
//            for (int i = 0; i < ui->tabWidget_2->count(); i++)
//            {
//                QString tab_name = ui->tabWidget_2->tabText(i);
//                if (tab_name == Inds_file || tab_name == Corr_file)
//                {
                    auto b = (CustomTextBrowser*)ui->textBrowser;
                    b->setReadOnly(true);
//                }
//            }
        }
    }

    QString currentDirAbsolutePath = gDirTwoLevelUp + "/" + gCurrentDirName;


}



/*!
 * \fn MainWindow::on_actionSave_triggered()
 * \brief This function will save any changes made in the current file.
 *
 * \details
 * In this function we first do the preprocessing needed for the save to happen.
 *
 * Then we run the function SaveFile_Backend() which will perform the actual saving and commiting of files
 * in a separate worker thread. We synchronise this with the loading window as well.
 *
 * We call doSaveBackend() of worker class which calls the SaveFile_Backend() function and emits the signal when
 * done. This signal is needed to indicate that the saving is complete and the saving window can be closed.
 *
 * We quit the thread once the thread is finished and we also deallocate the loading window from memory
 *
 * We then perform SaveFile_GUI_PostProcessing() and also call the WriteSettings() function
 * (refer functions for more details)
 *
 *
 * \sa SaveFile_GUI_Preprocessing(), SaveFile_Backend(), SaveFile_GUI_Postprocessing(), writeSettings()
*/

void MainWindow::on_actionSave_triggered()
{
    // check if file is new
    if (mFilename=="Untitled")
    {
        on_actionSave_As_triggered();
    }
    else
    {
        SaveFile_GUI_Preprocessing(); // GUI Preprocessing

        Worker *worker = new Worker(nullptr,
                                    &mProject,
                                    gCurrentPageName,
                                    gCurrentDirName,
                                    gDirTwoLevelUp,
                                    s1,
                                    s2,
                                    CPair_editDis,
                                    &CPairs,
                                    filestructure_fw);
        QThread *thread = new QThread;

        connect(thread, SIGNAL(started()), worker, SLOT(doSaveBackend()));
        connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
        connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
        connect(worker, SIGNAL(finished()), this, SLOT(stopSpinning()));
        worker->moveToThread(thread);
        thread->start();

        spinner = new LoadingSpinner(this);
        spinner->SetMessage("Saving...", "Saving File");
        spinner->setModal(false);
        spinner->exec();

        SaveFile_GUI_Postprocessing(); // GUI Postprocessing

        writeSettings();
    }
    // Run Global Replace

    GlobalReplace();

}

/*!
* \fn    MainWindow::stopSpinning
* \brief This function stops the sppiner whenever called.
*
*/
void MainWindow::stopSpinning()
{
    spinner->close();
    spinner->deleteLater();
}

/*!
 * \fn MainWindow::GlobalReplace
 * \brief This function runs the comparison check on the previously saved file and current document for getting the changed words for global replace
 */
void MainWindow::GlobalReplace()
{
    edit_Distance ed;
    QVector <QString> changedWords;
    changedWords = ed.editDistance(s1, s2);             // Update CPair by editdistance
    QString currentDirAbsolutePath = gDirTwoLevelUp + "/" + gCurrentDirName;
    runGlobalReplace(currentDirAbsolutePath, changedWords);
    ConvertSlpDevFlag =0;
}

/*!
 * \fn MainWindow::on_actionSave_As_triggered()
 * \brief This function saves the file which has never been saved once.
 */
void MainWindow::on_actionSave_As_triggered()
{
    QString file(QFileDialog::getSaveFileName(this, "Open a File"));

    //! Sets the file name and saves the file.
    if (!file.isEmpty())
    {
        setMFilename(file);
        UpdateFileBrekadown();

        on_actionSave_triggered();
    }
}

/*!
 * \fn on_actionSpell_Check_triggered()
 * \brief Converts the whole html page to the original text page in orange color
 * \note works on the basis of a pre-defined flag which helps to always convert the text to Devanagari
 * \sa findDictEntries(), find_and_replace_oddInstancesblue(), find_and_replace_oddInstancesorange(),hasM40PerAsci()
 */
map<string, int> wordLineIndex;

void MainWindow::on_actionSpell_Check_triggered()
{
    slpNPatternDict slnp;
    if(!curr_browser || curr_browser->isReadOnly())
        return;

    QString textBrowserText = curr_browser->toPlainText();
    QChar ch;
    ch=textBrowserText[1];
    textBrowserText+=" ";
    string str1=textBrowserText.toUtf8().constData();

    //! load number of words
    istringstream iss1(str1);
    size_t WordCount = 0;
    string word1;
    while(iss1 >> word1) WordCount++;

    //str1 = toslp1(str1);
    istringstream iss(str1);
    string strHtml = "<html><body>";
    string line;

    int value = 0;
    while (getline(iss, line))
    {
        istringstream issw(line);
        string word;

        while(issw >> word)
        {
            if(ConvertSlpDevFlag)
            {
                string word1 = word;
                word = slnp.toslp1(word);
                string wordNext;
                if(slnp.hasM40PerAsci(word1))
                {
                    wordNext = word1;
                }
                else
                {
                    wordNext = slnp.toDev(word);
                }
                strHtml += wordNext; strHtml += " ";
                value ++;
            }
            else
            {
                string word1 = word;
                word = slnp.toslp1(word);
                string wordNext;
                //! checks if the word exists in the English language, Seconday OCR, Pwords, Dict and CPair; convert its color coding
                if(slnp.hasM40PerAsci(word1))
                    wordNext = word1;

                else if(GBook[(word)] > 0 )
                {
                    wordNext = slnp.toDev(word);
                    PWords[word]++;
                }

                else if(PWords[word] > 0)
                {
                    wordNext = "<font color=\'gray\'>" + slnp.toDev(word) + "</font>";
                }
                else if((Dict[word] ==0) && (PWords[word] == 0) && (CPair[word].size() > 0))
                {
                    wordNext = "<font color=\'purple\'>" + slnp.toDev(CPair[word]) + "</font>";
                }
                else
                {
                    wordNext = slnp.findDictEntries(slnp.toslp1(word),Dict,PWords, word.size());     //replace m1 with m2,m1 for combined search
                    wordNext = slnp.find_and_replace_oddInstancesblue(wordNext);
                    wordNext = slnp.find_and_replace_oddInstancesorange(wordNext);
                }
                strHtml += wordNext;
                strHtml += " ";
                value ++;
            }
        }
        strHtml +="<br>";  // To add new line
    }
    strHtml += "</body></html>";
    curr_browser->setHtml(QString::fromStdString(strHtml));

    str1=textBrowserText.toUtf8().constData();

    istringstream iss2(str1);
    size_t WordCount2 = 0;

    //! clean(word) instead of word
    while (getline(iss2, line))
    {
        istringstream issw(line);
        string word;
        while(issw >> word)
        {
            wordLineIndex[(word + "###" + line)] = WordCount2; WordCount2++;
        }
    }
}

int isProjectOpen = 0;
/*!
 * \fn MainWindow::on_actionLoad_Next_Page_triggered
 * \brief Sets the browser window to display the next page
 *
 * \sa on_actionSave_triggered() ,get_version(), SaveTimeLog(), GetPageNumber(), LoadDocument()
 */
void MainWindow::on_actionLoad_Next_Page_triggered()
{
    /*Description
     * 1. Check if the file is saved else save the file
     * 2. Add Entries in Timelog.json about the elapsed time
     *    a) If the present mode is verifier, adds entries as verifer. Eg: "Verifier:page-2.txt:V-0"
     *    b) If the present mode is corrector, adds entries as corrector Eg: "Corrector:page-1.txt:V-1"
     * 3. Increment the page number extracted from the localFilename by value one. Terminates function if file doesn't exist
     * 4. Page number extracted from the tab name is incremented and set as the new tab name
     * 5. Loads the file with the incremented page number
     * */

//! Checking if the file is saved else saves the file
    if(curr_browser) {
        string localFilename = mFilename.toUtf8().constData();

        //! Extract page number from the localFilename and checks if the incremented page exists
        string no = "";

        size_t loc;
        QString ext = "";

        if(!mProject.GetPageNumber(localFilename, &no, &loc, &ext))
            return;

        QFile *file = new QFile(QString::fromStdString(localFilename));
        QFileInfo finfo(file->fileName());

        if(!(finfo.exists() && finfo.isFile())){
            return; }

        ui->treeView->selectionModel()->clearSelection();
        QModelIndex currentTreeItemIndex = ui->treeView->selectionModel()->currentIndex();
        QModelIndex parentIndex = currentTreeItemIndex.parent();
        auto model = ui->treeView->model();
        int rowCount = ui->treeView->model()->rowCount(parentIndex);

        QString treeItemLabel;
        for (int i = 0; i <= rowCount-1; i++)
        {
            QModelIndex index = model->index(i, 0, parentIndex);
            treeItemLabel = index.data(Qt::DisplayRole).toString();
            if (index.isValid())
            {
                if (treeItemLabel == currentTabPageName)
                {
                    if(i==rowCount-1) i=-1;
                    index = model->index(i+1, 0, parentIndex);
                    treeItemLabel = index.data(Qt::DisplayRole).toString();
                    file_click(index);
                    break;
                }
             }
         }
    }
}

/*!
 * \fn MainWindow::on_actionLoad_Prev_Page_triggered()
 * \brief Sets the browser window to display the previous page
 *
 * \sa on_actionSave_triggered() ,get_version(), SaveTimeLog(), GetPageNumber(), LoadDocument()
 */
void MainWindow::on_actionLoad_Prev_Page_triggered()
{
    /*Description
     * 1. Check if the file is saved else save the file
     * 2. Add Entries in Timelog.json about the elapsed time
     *    a) If the present mode is verifier, adds entries as verifer. Eg: "Verifier:page-2.txt:V-0"
     *    b) If the present mode is corrector, adds entries as corrector Eg: "Corrector:page-1.txt:V-1"
     * 3. Extract page number from the localfileName and decrements the page number by one. Terminates function if file doesn't exist
     * 4. Decrement the page number extracted from tab name and sets it as new tab name
     * 5. Loads the file with the decremented page number
     * */

//! Check if the file is saved or not
    if(curr_browser) {
        string localFilename = mFilename.toUtf8().constData();
        //! Extract page number from the localFilename
        string no = "";
        size_t loc;
        QString ext = "";
        if(!mProject.GetPageNumber(localFilename, &no, &loc, &ext))
            return;

        //!checks if the decremented page exists
        QFile *file = new QFile(QString::fromStdString(localFilename));
        QFileInfo finfo(file->fileName());
        if(!(finfo.exists() && finfo.isFile())) // Check if file exists
            return;

        ui->treeView->selectionModel()->clearSelection();
        QModelIndex currentTreeItemIndex = ui->treeView->selectionModel()->currentIndex();
        QModelIndex parentIndex = currentTreeItemIndex.parent();
        auto model = ui->treeView->model();
        int rowCount = ui->treeView->model()->rowCount(parentIndex);

        QString treeItemLabel;
        for (int i = 0; i < rowCount; i++)
        {
            QModelIndex index = model->index(i, 0, parentIndex);
            treeItemLabel = index.data(Qt::DisplayRole).toString();
            if (index.isValid())
            {
                if (treeItemLabel == currentTabPageName)
                {
                    if(i==0) i = rowCount;
                    index = model->index(i-1, 0, parentIndex);
                    treeItemLabel = index.data(Qt::DisplayRole).toString();
                    file_click(index);
                    break;
                }
             }
         }
    }
}

/*!
 * \fn MainWindow::on_actionToDevnagri_triggered()
 * \brief Converts transliterated text to devanagri text
 * Transliterated here means Hindi/ Sanskrit written in English.
 * This function converts selected translitrate text or last written translitrate word to Devanagri.
*/
void MainWindow::on_actionToDevanagari_triggered()
{
    slpNPatternDict slnp;
    if(!curr_browser || curr_browser->isReadOnly())
        return;
    QTextCursor cursor = curr_browser->textCursor();

    if(!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);

    QString str1 = cursor.selectedText();
    selectedStr = str1.toUtf8().constData();
    cursor.beginEditBlock();
    cursor.removeSelectedText();
    cursor.insertText(QString::fromStdString(slnp.toDev(slnp.toslp1(selectedStr))));
    cursor.endEditBlock();
}

/*!
 * \fn MainWindow::on_actionToSlp1_triggered()
 * \brief Converts devanagri/Sanskrit text to transliterated(english) text
 * This function converts selected Devanagri text to transliterated text.
*/
void MainWindow::on_actionToSlp1_triggered()
{
    if(!curr_browser || curr_browser->isReadOnly())
            return;
    QTextCursor cursor = curr_browser->textCursor();

    if(!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);

    QString str1 = cursor.selectedText();
    selectedStr = str1.toUtf8().constData();
    cursor.beginEditBlock();
    cursor.removeSelectedText();
    slpNPatternDict slnp;
    cursor.insertText(QString::fromStdString((slnp.toslp1(selectedStr))));
    cursor.endEditBlock();
}

/*!
 * \fn MainWindow::on_actionLoadGDocPage_triggered()
 * \brief Loads PWords and its associated trie data structure
 * \sa on_actionSave_As_triggered(), loadMap(), loadmaptoTrie(), generateCorrectionPairs(), loadConfusionsFont, loadTopConfusions()
*/
void MainWindow::on_actionLoadGDocPage_triggered()
{
    slpNPatternDict slnp;
    trieEditDis trie;
    /*! If file name is Untitled do nothing*/
    if (mFilename == "Untitled")
    {

    }
    else
    {
        /*! Create an html file for current ind file*/
        on_actionSave_As_triggered();
        QString changefiledir = filestructure_fw[gCurrentDirName];
        QString str1 = gDirTwoLevelUp + "/" + changefiledir + "/" + gCurrentPageName;
        str1.replace(".txt", ".html");

        QFile sFile(str1);
        if (sFile.open(QFile::WriteOnly | QFile::Text))
        {
            QTextStream out(&sFile);
            out.setCodec("UTF-8");
            out << curr_browser->toHtml(); //toPlainText(); //Modified
            sFile.flush();
            sFile.close();
        }

        /*! Load PWord and Top Confusion Words*/
        slnp.loadMap(str1.toUtf8().constData(), PWords, "PWords");

        map<string, int> PWordspage;
        slnp.loadMap(str1.toUtf8().constData(), PWordspage, "PWordspage");
        trie.loadmaptoTrie(TPWords, PWordspage);

        vector<string> wrong, right;
        QString str2 = mFilename;

        slnp.generateCorrectionPairs(wrong, right, str2.toUtf8().constData(), str1.toUtf8().constData());

        slnp.loadConfusionsFont(wrong, right, ConfPmapFont);
        slnp.loadConfusionsFont(wrong, right, ConfPmap);

        TopConfusions.clear();
        TopConfusionsMask.clear();
        slnp.loadTopConfusions(ConfPmap, TopConfusions, TopConfusionsMask);
    }
}



QString mFilename1, loadStr, loadStr1;

/*!
 * \fn    MainWindow::WordCount
 * \brief This function is called whenever there is any key pressed after opening the project.
 *        This function counts the number of word in the page and shows it in the statusbar at bottom
 *        of the window.
 */
void MainWindow::WordCount()
{
    if(curr_browser){
        QString extText = curr_browser->toPlainText();
        //!Removes these symbol while counting
        extText.remove("?");
        extText.remove("|");
        extText.remove("`");
        extText.remove("[");
        extText.remove("]");
        extText.remove("'");
        extText.remove(",");

        int wordcnt = extText.split(QRegExp("(\\s|\\n|\\r)+"), QString::SkipEmptyParts).count();
        QString toshow = QString::number(wordcnt)+" Words";
        ui->lineEdit_3->setText(toshow);
    }
}

/*!
 * \fn    MainWindow::on_actionLoadData_triggered()
 * \brief Loads the dictionary files only once
 *        This function is only called once per project to load the dictionary file of the project,
 *        dictionary files can be used to work on suggestions.
 */
void MainWindow::on_actionLoadData_triggered()
{
    if (mProject.isProjectOpen())
    {
        if (LoadDataFlag)
        {
            ui->actionLoadData->setDisabled(true);
            ui->actionLoadData->setDisabled(true);
            QString initialText = ui->lineEdit->text();
            ui->lineEdit->setText("Loading Data...");
            QString  localmFilename1 = mFilename;
            string localmFilename1n = localmFilename1.toUtf8().constData();
            localmFilename1n = localmFilename1n.substr(0, localmFilename1n.find("page"));
            localmFilename1 = QString::fromStdString(localmFilename1n);

            LoadDataWorker *worker = new LoadDataWorker(
                        nullptr,
                        &mProject,
                        mFilename,
                        mFilename1,
                        &LSTM,
                        &CPairs,
                        &Dict,
                        &GBook,
                        &IBook,
                        &PWords,
                        &ConfPmap,
                        &vGBook,
                        &vIBook,
                        &TDict,
                        &TGBook,
                        &TGBookP,
                        &TPWords,
                        &TPWordsP,
                        &synonym,
                        &synrows
                        );
            QThread *thread = new QThread;

            connect(thread, SIGNAL(started()), worker, SLOT(LoadData()));
            connect(worker, SIGNAL(finishedLoadingData()), thread, SLOT(quit()));
            connect(worker, SIGNAL(finishedLoadingData()), worker, SLOT(deleteLater()));
            connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
            connect(worker, SIGNAL(finishedLoadingData()), this, SLOT(stopSpinning()));
            worker->moveToThread(thread);
            thread->start();

            spinner = new LoadingSpinner(this);
            spinner->SetMessage("Loading Data...", "Loading...");
            spinner->setModal(false);
            spinner->exec();

            ui->lineEdit->setText(initialText);
            LoadDataFlag = 0;
            qDebug() << "done loading ....";
            QMessageBox messageBox;
            messageBox.information(0, "Load Data", "Data has been loaded.");
        }
    }
}

/*!
 * \fn loadDict()
 * \param current project file location
 * \brief The path of the dictionary file is fetched and the files are returned in the map
 * these words are then fetched depending upon the word selected
 * \sa loadMap()
 */
bool loadDict(Project & project) {
    slpNPatternDict slnp;
    QString localmFilename1 = project.GetDir().absolutePath() + "/Dicts/" + "Dict";
    if (!QFile::exists(localmFilename1)) return false;
    slnp.loadMap(localmFilename1.toUtf8().constData(), Dict, "Dict");
    return true;
}

/*!
 * \fn on_actionLoadDict_triggered()
 * \brief The path of the dictionary file is fetched and the files are returned in the map
 * these words are then fetched depending upon the word selected
 * \sa loadDict()
 */
void MainWindow::on_actionLoadDict_triggered()
{
    loadDict(mProject);
}

/*!
 * \fn MainWindow::on_actionLoadOCRWords_triggered()
 * \brief Loads the OCR files
 * The path of the GEROCR and IEROCR file is fetched and the files are returned in the map which is again,
 * used as a suggestion depending upon the word selected
 * \sa LoadMapNV()
 */
void MainWindow::on_actionLoadOCRWords_triggered()
{
    slpNPatternDict slnp;
    QString localmFilename1 = mProject.GetDir().absolutePath() + "/Dicts/" + "GEROCR";
    cout << localmFilename1.toUtf8().constData() << endl;
    slnp.loadMapNV(localmFilename1.toUtf8().constData(), GBook, vGBook, "GBook"); localmFilename1 = mFilename1;
    cout << localmFilename1.toUtf8().constData() << endl;
    localmFilename1 = mProject.GetDir().absolutePath() + "/Dicts/" + "IEROCR";
    slnp.loadMapNV(localmFilename1.toUtf8().constData(), IBook, vIBook, "IBook");
    cout << GBook.size() << " " << IBook.size() << endl;

}

/*!
  \fn MainWindow::on_actionLoadDomain_triggered()
  \brief loads the common OCR files
 The path of the PWords file is fetched and the files are returned in the map which can be used
 for the suggestion feature
 \sa loadMapPWords()
 */
void MainWindow::on_actionLoadDomain_triggered()
{
    QString localmFilename1 = mProject.GetDir().absolutePath() + "/Dicts/" + "/PWords";
    slpNPatternDict slnp;
    slnp.loadMapPWords(vGBook, vIBook, PWords);
}

/*!
  \fn MainWindow::on_actionLoadSubPS_triggered()
  \brief the CPair files
    The path of the CPair files are fetched and the files are returned in the map which is returned
    to load the suggestions
  \sa loadmaptoTrie(), loadPwordsPatternstoTrie(), loadCPair()
 */
void MainWindow::on_actionLoadSubPS_triggered()
{
    slpNPatternDict slnp;
    trieEditDis trie;
    size_t count = trie.loadPWordsPatternstoTrie(TPWordsP, PWords);// justsubstrings not patterns exactly // PWordsP,
    QString localmFilename1 = mProject.GetDir().absolutePath() + "/Dicts/" + "CPair";

    slnp.loadCPairs(localmFilename1.toUtf8().constData(), CPairs, Dict, PWords);
    localmFilename1 = mFilename1;

    localmFilename1 = mProject.GetDir().absolutePath() + "/Dicts/" + "LSTM";
    ifstream myfile(localmFilename1.toUtf8().constData());
    if (myfile.is_open())
    {
        string str1, str2, line;
        while (getline(myfile, line))
        {
            istringstream slinenew(line); slinenew >> str1; slinenew >> str2;
            if (str2.size() > 0) LSTM[str1] = str2;
        }
    }
    cout << LSTM.size() << "LSTM Pairs Loaded";
    localmFilename1 = mFilename1;

    trie.loadmaptoTrie(TPWords, PWords);
    trie.loadmaptoTrie(TDict, Dict);
    trie.loadmaptoTrie(TGBook, GBook);
    trie.loadPWordsPatternstoTrie(TGBookP, GBook);
}

/*!
 * \fn MainWindow::on_actionLoadConfusions_triggered
 * \brief Loads the confusions for CPair
 */
void MainWindow::on_actionLoadConfusions_triggered()
{
    slpNPatternDict slnp;
    QString localmFilename1 = mProject.GetDir().absolutePath() + "/Dicts/" + "CPair";
    slnp.loadConfusions(localmFilename1.toUtf8().constData(), ConfPmap);
    localmFilename1 = mFilename;
}

/*!
 * \fn MainWindow::on_actionSugg_triggered
 * \brief Displays the context menu that has suggestion item
 * \note It works only when the data is loaded
 */
void MainWindow::on_actionSugg_triggered()
{
    /*Description
     * The function loads the context menu and waits for mouse event.
     * As soon as the mouse event is captured, the context menu is unloaded.
     */
    RightclickFlag = 1;
    QMouseEvent *ev1;
    mousePressEvent(ev1);      //to capture mouse events
    RightclickFlag = 0;

}

/*!
 * \fn MainWindow::on_actionUndo_triggered()
 * \brief undo the changes made QTextBrowser
 */
void MainWindow::on_actionUndo_triggered()
{
    if(!curr_browser || curr_browser->isReadOnly())
        return;
    curr_browser->undo();
}

/*!
 * \fn MainWindow::on_actionRedo_triggered()
 * \brief Redo the changes made in QTextBrowser
 */
void MainWindow::on_actionRedo_triggered()
{
    if(!curr_browser || curr_browser->isReadOnly())
        return;
    curr_browser->redo();
}

/*!
 * \fn MainWindow::on_actionFind_and_Replace_triggered()
 * \brief helps to find particular text and replaces them with new user entered text
 * \sa openFindAndReplace
 */
void MainWindow::on_actionFind_and_Replace_triggered()
{
    TextFinder *dialog = TextFinder::openFindAndReplace(this);
    dialog->show();
}

/*!
 * \fn MainWindow::on_actionAccuracyLog_triggered()
 * \brief Logs various accuracies to AccuracyLog.csv
 * Stores the following values in the csv:
 * Page Name
 * Percent Difference in Corrector and Verifier Text
 * Percent Character Level Accuracy (Corrector)
 * Percent Word Level Accuracy (Corrector)
 * Percent Change made by Corrector wrt OCR Text
 * Percent Word Errors
 * Percent Accuracy of OCR
 * \sa diff_match_patch::diff_main(),  Project::LevenshteinWithGraphemes(), Project::GetGraphemesCount()
*/
void MainWindow::on_actionAccuracyLog_triggered()
{
    QString qs1="", qs2="",qs3="";

    file = QFileDialog::getOpenFileName(this,"Open File from VerifierOutput Folder"); //open file
    int loc =  file.lastIndexOf("/");
    QString folder = file.mid(0,loc);      //fetch parent tdirectory

    QDir directory(folder);
    QStringList textFiles = directory.entryList((QStringList()<<"*.txt", QDir::Files)); //fetch all files in the parent directory

    int loc1 = folder.lastIndexOf("/");
    QString qcsvfolder =  folder.mid(0,loc1) +"/AccuracyLog.csv";
    string csvfolder = qcsvfolder.toUtf8().constData();

    std::ofstream csvFile(csvfolder);
    csvFile<<"Page Name,"<<"Errors (Word level),"<<"Errors (Character-Level),"<< "Accuracy of Corrector (Word level),"<<"Accuracy of Corrector (Character-Level)," <<"Changes made by Corrector(%)," <<"OCR Accuracy(w.rt. Verified Text)"<<"\n";

    foreach(QString fileName, textFiles)
    {

        string pageName = fileName.toUtf8().constData();
        fileName = folder + "/" + fileName;

        QString verifierText = fileName;
        QString correctorText = fileName.replace("VerifierOutput","CorrectorOutput"); //CAN CHANGE ACCORDING TO FILE STRUCTURE
        QString ocrText = fileName.replace("CorrectorOutput","Inds"); //CAN CHANGE ACCORDING TO FILE STRUCTURE
        ocrText.replace(".html",".txt");
        ocrText.replace("V1_", "");
        ocrText.replace("V2_", "");
        ocrText.replace("V3_", "");

        if(!ocrText.isEmpty())
        {
            QFile sFile(ocrText);
            if(sFile.open(QFile::ReadOnly | QFile::Text))
            {
                QTextStream in(&sFile);
                in.setCodec("UTF-8");
                qs1 = in.readAll().simplified();
            }

        }
        if(!correctorText.isEmpty())
        {
            QFile sFile(correctorText);
            if(sFile.open(QFile::ReadOnly | QFile::Text))
            {
                QTextStream in(&sFile);
                in.setCodec("UTF-8");
                qs2 = in.readAll().simplified();
            }

        }
        if(!verifierText.isEmpty())
        {
            QFile sFile(verifierText);
            if(sFile.open(QFile::ReadOnly | QFile::Text))
            {
                QTextStream in(&sFile);
                in.setCodec("UTF-8");
                qs3 = in.readAll().simplified();
                sFile.close();
            }

        }
        int l1,l2,l3, DiffOcr_Corrector,DiffCorrector_Verifier,DiffOcr_Verifier; float correctorChangesPerc,verifierChangesPerc,ocrErrorPerc;


        QTextDocument doc;

        doc.setHtml(qs2);
        qs2 = doc.toPlainText().replace(" \n","\n");

        doc.setHtml(qs3);
        qs3 = doc.toPlainText().replace(" \n","\n");

        l1 = mProject.GetGraphemesCount(qs1); l2 = mProject.GetGraphemesCount(qs2); l3 = mProject.GetGraphemesCount(qs3);
        if(qs1=="" | qs2 == "" | qs3 == "")
        {
            continue;
        }

        diff_match_patch dmp;

        auto diffs1 = dmp.diff_main(qs1,qs2);
        DiffOcr_Corrector = mProject.LevenshteinWithGraphemes(diffs1);
        correctorChangesPerc = ((float)(DiffOcr_Corrector)/(float)l2)*100;
        if(correctorChangesPerc>100)
            correctorChangesPerc = ((float)(DiffOcr_Corrector)/(float)l1)*100;
        correctorChangesPerc = (((float)lround(correctorChangesPerc*100))/100);

        auto diffs2 = dmp.diff_main(qs2,qs3);
        DiffCorrector_Verifier = mProject.LevenshteinWithGraphemes(diffs2);
        verifierChangesPerc = ((float)(DiffCorrector_Verifier)/(float)l3)*100;
        if(verifierChangesPerc>100)
            verifierChangesPerc = ((float)(DiffCorrector_Verifier)/(float)l2)*100;
        verifierChangesPerc = (((float)lround(verifierChangesPerc*100))/100);
        float correctorCharAcc =100- (((float)lround(verifierChangesPerc*100))/100); //Corrector accuracy = 100-changes mabe by Verfier

        auto diffs3 = dmp.diff_main(qs1,qs3);
        DiffOcr_Verifier = mProject.LevenshteinWithGraphemes(diffs3);
        ocrErrorPerc = ((float)(DiffOcr_Verifier)/(float)l3)*100;
        if(ocrErrorPerc>100)
            ocrErrorPerc = ((float)(DiffOcr_Verifier)/(float)l1)*100;
        float ocrAcc = 100 - (((float)lround(ocrErrorPerc*100))/100);


        auto a = dmp.diff_linesToChars(qs2, qs3); //LinesToChars modifed for WordstoChar in diff_match_patch.cpp
        auto lineText1 = a[0].toString();
        auto lineText2 = a[1].toString();
        auto lineArray = a[2].toStringList();
        int wordCount2 = qs2.simplified().count(" ");
        int wordCount3 = qs3.simplified().count(" ");
        auto diffs = dmp.diff_main(lineText1, lineText2);
        int worderrors = dmp.diff_levenshtein(diffs);
        dmp.diff_charsToLines(diffs, lineArray);

        float correctorwordaccuracy = (float)(worderrors)/(float)wordCount3*100;
        if(correctorwordaccuracy>100)
            correctorwordaccuracy = (float)(worderrors)/(float)wordCount2*100;
        correctorwordaccuracy = (((float)lround(correctorwordaccuracy*100))/100);

        csvFile<<pageName<<","<<worderrors<<","<<DiffCorrector_Verifier<<","<< correctorwordaccuracy<<","<<correctorCharAcc<<"," <<correctorChangesPerc<<","<<ocrAcc<<"\n";

    }

    csvFile.close();

}

/*!
 * \fn MainWindow::on_actionViewAverageAccuracies_triggered()
 * \brief Calculates the average accuracy of the submission based upon the comments and the csv file input which depend upon the
 * page changes. Also updates in the csv file of the current project.
 * \sa AverageAccuracies(),readJsonFile()
 */
void MainWindow::on_actionViewAverageAccuracies_triggered()
{
    QString commentFilename = gDirTwoLevelUp + "/Comments/comments.json";
    QString csvFile = gDirTwoLevelUp + "/Comments/AverageAccuracies.csv";
    QString pagename = gCurrentPageName;
    pagename.replace(".txt", "");
    pagename.replace(".html", "");

    float avgcharacc = 0, avgwordacc = 0, avgrating = 0; int avgcharerrors = 0, avgworderrors = 0;

    QJsonObject mainObj = readJsonFile(commentFilename);

    avgcharacc = mainObj["AverageCharAccuracy"].toDouble();
    avgwordacc = mainObj["AverageWordAccuracy"].toDouble();
    avgcharerrors = mainObj["AverageCharErrors"].toInt();
    avgworderrors = mainObj["AverageWordErrors"].toInt();

    AverageAccuracies *aa = new AverageAccuracies(csvFile, avgwordacc, avgcharacc, avgworderrors, avgcharerrors);
    aa->show();
}

/*!
 * \fn MainWindow::on_actionAllFontProperties_triggered()
 * \brief Opens a pop up font properties selector and applies chosen properties to font.
 * The font selector lets you choose:
 * Font face, font size, variations of those fonts (regular, bold, italic).
 * The font selector is the Operating System's native font selector.
 * All fonts installed in the system will appear in the selector
 * (a software restart after font installation/ reopening the selector might be required).
*/
void MainWindow::on_actionAllFontProperties_triggered()
{
    if(!curr_browser || curr_browser->isReadOnly())
        return;

    auto cursor = curr_browser->textCursor();
        auto selected = cursor.selection();
        QString sel = selected.toHtml();

  QFont initialFont=curr_browser->currentFont();                                                   // initial font face

    auto pointsize = curr_browser->fontPointSize();

    if(pointsize) initialFont.setPointSize(pointsize);      // initial font size
    bool ok;
    QFont font = QFontDialog::getFont(&ok, initialFont, this, tr("Font Properties"), QFontDialog::DontUseNativeDialog);

    /*! If user clicks OK then change to selected font with properties*/
    if (!ok) {
        return;
    }

    int ret = QMessageBox::question(this, tr("Question"), tr("Do you want to apply this font to all pages ? (CAN'T UNDO THIS OPERATION)"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        QString fontFamily = font.family();
        int fontSize = font.pointSize();
        QVector<QString> styleProperties = {"font-family:", "font-size:"};
        QVector<QString> stylePropertyValues = {QString("\'" + fontFamily + "\'"), QString::number(fontSize) + "pt"};
        int totalFontProperties = styleProperties.size();
        qDebug() << "apply to all pages";
        /*!
         * 1. Loop through each page except the current one
         * 2. Apply font size and font family
         */

        QString directory = gDirTwoLevelUp + "/" + gCurrentDirName;
        QString exception = gDirTwoLevelUp + "/" + gCurrentDirName + "/" + gCurrentPageName;

        QDirIterator filesIt(directory, {"*.html"}, QDir::Files | QDir::NoDotAndDotDot);

        while (filesIt.hasNext()) {
            QString filename = filesIt.next();
            if (filename == exception) {
                continue;
            }
            QFile file(filename);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                qDebug() << "Cannot open file in read mode";
            }
            QString fileText = file.readAll();
            file.close();

            QRegularExpression regex_style("(<span[^>]*>)");
            QRegularExpressionMatchIterator itr = regex_style.globalMatch(fileText);

            while (itr.hasNext()) {
                QRegularExpressionMatch match = itr.next();
                QString capString = match.captured(1);
                int capStart = match.capturedStart(1);

                for (int i = 0; i < totalFontProperties; i++) {
                    QString property = styleProperties[i];
                    QString value = stylePropertyValues[i];
                    int propIndex = -1;

                    if ((propIndex = capString.indexOf(property)) != -1) { // If value of the property is different
                        int endIndexOfProperty = capString.indexOf(";", propIndex);
                        int replacementLen = endIndexOfProperty - (propIndex + property.length());
                        fileText.replace(capStart + propIndex + property.length(), replacementLen, value);
                        capString.replace(propIndex + property.length(), replacementLen, value);
                    } else if (capString.indexOf("style=\"") != -1) { // If property is not present
                        int indexOfStyle = capString.indexOf("style=\"");
                        fileText.insert(capStart + indexOfStyle + QString("style=\"").length(), " " + property + value + ";");
                        capString.insert(indexOfStyle + QString("style=\"").length(), " " + property + value + ";");
                    } else { // If style tag is not present
                        fileText.insert(capStart + QString("<span ").length(), "style=\" " + property + value + ";\" ");
                        capString.insert(QString("<span ").length(), "style=\" " + property + value + ";\" ");
                    }
                }

                itr = regex_style.globalMatch(fileText, capStart + capString.length());
            }
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                qDebug() << "Cannot open file in write mode";
            }
            QTextStream out(&file);
            out.setCodec("UTF-8");
            out << fileText;
            file.close();
        }

    }

    //!Filter the font properities
    QTextCharFormat format_to_be_applied;

    qreal wgt = font.pointSize();
    QString fam = font.family();
    bool strike = font.strikeOut();
    bool underline = font.underline();
    qreal LetterSpacing=font.letterSpacing();
    qreal WordSpacing = font.wordSpacing();
    int stretch = font.stretch();
    auto styleHint = font.styleHint();
    auto styleStrategy = font.styleStrategy();
    auto letterSpacingType = font.letterSpacingType();
    bool overline = font.overline();
    bool fixedpitch = font.fixedPitch();
    auto hintingpref = font.hintingPreference();

    format_to_be_applied.setFontPointSize(wgt);
    format_to_be_applied.setFontFamily(fam);
    format_to_be_applied.setFontStrikeOut(strike);
    format_to_be_applied.setFontUnderline(underline);
    format_to_be_applied.setFontLetterSpacing(LetterSpacing);
    format_to_be_applied.setFontWordSpacing(WordSpacing);
    format_to_be_applied.setFontStretch(stretch);
    format_to_be_applied.setFontStyleHint(styleHint,styleStrategy);
    format_to_be_applied.setFontLetterSpacingType(letterSpacingType);
    format_to_be_applied.setFontOverline(overline);
    format_to_be_applied.setFontFixedPitch(fixedpitch);
    format_to_be_applied.setFontHintingPreference(hintingpref);


    //! Apply bold and italics if present
    if(font.bold())
    {
        qreal weight = font.weight();
        format_to_be_applied.setFontWeight(weight);
    }
    if(font.italic())
    {
        bool Italics = font.italic();
        format_to_be_applied.setFontItalic(Italics);
    }

    curr_browser->setCurrentFont(font);
}

/*!
 * \fn MainWindow::on_actionBold_triggered()
 * \brief Sets the font weight to bold
*/
void MainWindow::on_actionBold_triggered()
{
    if(!curr_browser || curr_browser->isReadOnly())
        return;
    QTextCursor cursor = curr_browser->textCursor();
    /*
     * charFormat returns the format of the character before the position
     * So, we interchange the ancr and position
    */
    int pos = cursor.position();
    int ancr = cursor.anchor();
    if (pos < ancr) {
        cursor.setPosition(pos, QTextCursor::MoveAnchor);
        cursor.setPosition(ancr, QTextCursor::KeepAnchor);
    }
    //qDebug()<<"pos : ancr"<<pos<<ancr;
    bool isBold = cursor.charFormat().font().bold();
    /*
     * If the font-weight value is bold then
     * it will change it to normal else bold.
    */
    QTextCharFormat fmt;
    fmt.setFontWeight(isBold ? QFont::Normal : QFont::Bold);
    cursor.mergeCharFormat(fmt);
    curr_browser->mergeCurrentCharFormat(fmt);
}

/*!
 * \fn MainWindow::on_actionUnBold_triggered()
 * \brief Sets the font weight to regular
*/
void MainWindow::on_actionUnBold_triggered()
{
    if(!curr_browser || curr_browser->isReadOnly())
        return;
    QTextCharFormat format;
    format.setFontWeight(QFont::Normal);
    curr_browser->textCursor().mergeCharFormat(format);
}

/*!
 * \fn MainWindow::on_actionItalic_triggered()
 * \brief Sets the font style to italic
*/
void MainWindow::on_actionItalic_triggered()
{
    if(!curr_browser || curr_browser->isReadOnly())
        return;

    QTextCursor cursor = curr_browser->textCursor();                        // initialize cursor position at text cursor's position
    /*
     * charFormat returns the format of the character before the position
     * So, we interchange the ancr and position
    */
    int pos = cursor.position();
    int ancr = cursor.anchor();
    if (pos < ancr) {
        cursor.setPosition(pos, QTextCursor::MoveAnchor);
        cursor.setPosition(ancr, QTextCursor::KeepAnchor);
    }
    bool isItalic = cursor.charFormat().font().italic();                    // check if character under cursor is italic or not

    QTextCharFormat fmt;
    fmt.setFontItalic(isItalic ? false : true);                             // if font is italic set font to regular, else set it to italic
    cursor.mergeCharFormat(fmt);
    curr_browser->mergeCurrentCharFormat(fmt);                              // Merge current character format to character under cursor's format (previous properties + italic/non italic)
}

/*!
 * \fn MainWindow::on_actionNonitalic_triggered()
 * \brief Resets the font style to non italic
*/
void MainWindow::on_actionNonitalic_triggered()
{
    if(!curr_browser || curr_browser->isReadOnly())
        return;
    QTextCharFormat format;
    format.setFontItalic(false);
    curr_browser->textCursor().mergeCharFormat(format);
}

/*!
 * \fn MainWindow::on_actionHighlight_triggered()
 * \brief Highlights selected text in text browser and then calls LogHighlights()
 * Only Verifiers can highlight the text, correctors can only remove highlights.
 * Takes the start and end position of cursor and goes to the middle of the word,
 * then formats the word.LogHighlights() is called only when a highlight is made.
 * \note Text Highlight works by changing the background colour to yellow
 * \sa LogHighlights()
*/
void MainWindow::on_actionHighlight_triggered()
{
     //! Check if text browser exists AND it is NOT in read only mode
     if(curr_browser && !curr_browser->isReadOnly())
     {
        //! Verifier gets to add and remove highlight the text
        if(isVerifier)
        {
            QTextCursor cursor = curr_browser->textCursor();
            QString text = cursor.selectedText().toUtf8().constData();
            int pos1 = curr_browser->textCursor().selectionStart();
            int pos2 = curr_browser->textCursor().selectionEnd();

            int cursorpos = round(((float)(pos1 + pos2)) / 2);      // goes to mid of the selection
            cursor.setPosition(cursorpos);

            QTextCharFormat  format = cursor.charFormat();         // gets word character format properties

            //! If word background is already yellow, set it to transparent, else set it to yellow (yellow is the highlight colour).
            if (format.background() == Qt::yellow)
            {
                format.setBackground(Qt::transparent);
            }
            else
            {
                format.setBackground(Qt::yellow);

                LogHighlights(text);       // Add log to HighlightsLog file if word is highlighted
            }
            curr_browser->textCursor().mergeCharFormat(format);        // Set format at current cursor
        }
        else
        {
            curr_browser->setTextBackgroundColor(Qt::transparent); //Correctors are only allowed to remove highlights.
        }
    }
}

/*!
 * \fn MainWindow::LogHighlights()
 * \param QString text
 * \brief Stores highlight metadata to JSON files
 * \note Meta data includes: Word highlighted, timestamp, time elapsed, page name
*/
void MainWindow::LogHighlights(QString word) //Verifier Only
{
    //!Highlighted Time meta data is stored
    QString dir = mProject.GetDir().absolutePath();

    QString highlightsFilename = gDirTwoLevelUp + "/Comments/HighlightsLog.json"; // load the highlight file name
    QString pagename = gCurrentPageName;                                          // load the current page name (in the text browser)

    pagename.replace(".txt", "");                                                 // remove extension from file name to use it as a key
    pagename.replace(".html", "");

    QJsonObject mainObj = readJsonFile(highlightsFilename);                       // read JSON file into mainObj
    QJsonObject page = mainObj.value(pagename).toObject();                        // set values for page name
    QJsonObject highlights;                                                       // make a new JSON object to store new metadata

    int nMilliseconds = myTimer.elapsed();
    secs = nMilliseconds / 1000;

    QString time = QTime::currentTime().toString();
    highlights["Word"] = word;
    highlights["Timestamp"] = time;
    highlights["Time Elapsed (s)"] = secs;
    highlights["Page Name"] = pagename;

    page.remove(time);
    page.insert(time, highlights);
    mainObj.remove(pagename);
    mainObj.insert(pagename, page);

    writeJsonFile(highlightsFilename, mainObj);
}

/*!
 * \fn MainWindow::on_actionLeftAlign_triggered()
 * \brief Sets the browser text alignment to Left Align
*/
void MainWindow::on_actionLeftAlign_triggered()
{
    if(!curr_browser || curr_browser->isReadOnly())
        return;
    curr_browser->setAlignment(Qt::AlignLeft);
}

/*!
 * \fn MainWindow::on_actionRightAlign_triggered()
 * \brief Sets the browser text alignment to Right Align
*/
void MainWindow::on_actionRightAlign_triggered()
{
    if(!curr_browser || curr_browser->isReadOnly())
        return;
    curr_browser->setAlignment(Qt::AlignRight);
}

/*!
 * \fn MainWindow::on_actionCentreAlign_triggered()
 * \brief Sets the browser text alignment to Center Align
*/
void MainWindow::on_actionCentreAlign_triggered()
{
    if(!curr_browser || curr_browser->isReadOnly())
        return;
    curr_browser->setAlignment(Qt::AlignCenter);
}

/*!
 * \fn    MainWindow::on_action_JustifiedAlign_triggered
 * \brief Justifies the text.
 *        Here, whenever there is a break in the text, that is being replaced by space
 *        and then text is placed in-between paragraph tags. After that, the following
 *        selected text is justisfied aligned.
 * \note  Not used, does not work as intended
*/
void MainWindow::on_actionJusitfiedAlign_triggered(){
    if(!curr_browser || curr_browser->isReadOnly())
        return;
    auto cursor = curr_browser->textCursor();
    auto selected = cursor.selection();
    cursor.removeSelectedText();
    QString sel = selected.toHtml();
    sel.replace("<br />" ," ");
    sel = "</p><p>" + sel + "</p><p>";
    auto newFrag = selected.fromHtml(sel);
    cursor.insertFragment(newFrag);
    curr_browser->setAlignment(Qt::AlignJustify);
}

/*!
 *  \fn MainWindow::on_actionSuperscript_triggered()
 *  \brief toggles superscript for selected text/ character at cursor
*/
void MainWindow::on_actionSuperscript_triggered() {

    if(!curr_browser || curr_browser->isReadOnly())
        return;

    auto cursor = curr_browser->textCursor();

    QTextCharFormat fmt;

    fmt.setVerticalAlignment((cursor.charFormat().verticalAlignment() == QTextCharFormat::AlignSuperScript)?\
                                 QTextCharFormat::AlignNormal : \
                                 QTextCharFormat::AlignSuperScript
                                 ); // Ternary operator for isSuperscripted ? alignNormal : alignSuperscript

    cursor.mergeCharFormat(fmt);
    curr_browser->mergeCurrentCharFormat(fmt);
}

/*!
 *  \fn MainWindow::on_actionSubscript_triggered()
 *  \brief toggles subscript for selected text/ character at cursor
*/
void MainWindow::on_actionSubscript_triggered() {

    if(!curr_browser || curr_browser->isReadOnly())
        return;

    auto cursor = curr_browser->textCursor();

    QTextCharFormat fmt;
    fmt.setVerticalAlignment((cursor.charFormat().verticalAlignment() == QTextCharFormat::AlignSubScript)? QTextCharFormat::AlignNormal : QTextCharFormat::AlignSubScript);

    cursor.mergeCharFormat(fmt);
    curr_browser->mergeCurrentCharFormat(fmt);
}

/*!
 * \fn MainWindow::on_actionInsert_Horizontal_Line_triggered()
 * \brief Draws a horizontal rule a line below the cursor.
 * Uses html to insert a horizontal rule using <hr> tag.
 * Selecting text and then using this fuctionality will clear the text,
 * and then insert a horizontal rule below it.
 * \note The text browser is a rich text field, so html tags and styling can be used seamlessly.
*/
void MainWindow::on_actionInsert_Horizontal_Line_triggered()
{
    if(!curr_browser || curr_browser->isReadOnly())
        return;
    curr_browser->insertHtml("</span></p><hr /><p><span>");
}

/*!
 * \fn MainWindow::on_actionFontBlack_triggered()
 * \brief Sets the current font colour to black.
 * \note There are no further colour properties provided by the editor.
*/
void MainWindow::on_actionFontBlack_triggered()
{
    if(!curr_browser || curr_browser->isReadOnly())
        return;
    curr_browser->setTextColor(Qt::black);
}

/*!
 * \fn MainWindow::on_actionInsert_Tab_Space_triggered()
 * \brief inserts tab (4 whitespaces) at cursor
*/
void MainWindow::on_actionInsert_Tab_Space_triggered()
{
    if(!curr_browser || curr_browser->isReadOnly())
        return;
    curr_browser->insertPlainText("    ");
}

/*!
 * \fn MainWindow::on_actionInsert_Table_2_triggered()
 * \brief Inserts table at text cursor
 * Creates a dialog to insert the numbr of rows and columns.
*/
void MainWindow::on_actionInsert_Table_2_triggered()
{
    if(!curr_browser || curr_browser->isReadOnly())
        return;

    QDialog dialog(this);
    QFormLayout form(&dialog);      // Use a layout allowing to have a label next to each field
    form.addRow(new QLabel("Insert Table", this));                                  // Create a dialog for asking table dimensions

    //! Add the lineEdits with their respective labels
    QLineEdit *rows = new QLineEdit(&dialog);
    QLineEdit *columns = new QLineEdit(&dialog);                                    // Add lineEdits to get Rows
    form.addRow("Rows", rows);                                                      // Add lineEdits to get Columns
    form.addRow("Columns", columns);

    //! Add some standard buttons (Cancel/Ok) at the bottom of the dialog
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog); // Add some standard buttons (Cancel/Ok) at the bottom of the dialog
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    //! Show the dialog as modal
    if (dialog.exec() == QDialog::Accepted)
    {

        QTextTableFormat tf;
        tf.setBorderBrush(Qt::black);
        tf.setCellSpacing(0);
        tf.setCellPadding(7);
        QTextCursor cursor = curr_browser->textCursor();
        cursor.insertTable(rows->text().toInt(),columns->text().toInt(),tf);
    }
}

/*!
 * \fn MainWindow::on_actionInsert_Columnleft_triggered()
 * \brief Inserts Column to the left of table
*/
void MainWindow::on_actionInsert_Columnleft_triggered()
{
    if(!curr_browser || curr_browser->isReadOnly())
        return;
    if(curr_browser->textCursor().currentTable())
    {
        QTextTable *currentTable = curr_browser->textCursor().currentTable();
        QTextTableCell currentCell = currentTable->cellAt(curr_browser->textCursor());
        currentTable->insertColumns(currentCell.column(),1);
    }
}

/*!
 * \fn MainWindow::on_actionInsert_Columnright_triggered()
 * \brief Inserts Column to the right of table
*/
void MainWindow::on_actionInsert_Columnright_triggered()
{
    if(!curr_browser || curr_browser->isReadOnly())
        return;
    if(curr_browser->textCursor().currentTable())
    {
        QTextTable *currentTable = curr_browser->textCursor().currentTable();
        QTextTableCell currentCell = currentTable->cellAt(curr_browser->textCursor());
        currentTable->insertColumns(currentCell.column()+1,1);
    }
}

/*!
 * \fn MainWindow::on_actionInsert_Rowabove_triggered()
 * \brief Inserts Row above the table
*/
void MainWindow::on_actionInsert_Rowabove_triggered()
{
    if(!curr_browser || curr_browser->isReadOnly())
        return;
    if(curr_browser->textCursor().currentTable())
    {
        QTextTable *currentTable = curr_browser->textCursor().currentTable();
        QTextTableCell currentCell = currentTable->cellAt(curr_browser->textCursor());
        currentTable->insertRows(currentCell.row(),1);
    }
}

/*!
 * \fn MainWindow::on_actionInsert_Rowbelow_triggered()
 * \brief Inserts Row below the table
*/
void MainWindow::on_actionInsert_Rowbelow_triggered()
{
    if(!curr_browser || curr_browser->isReadOnly())
        return;
    if(curr_browser->textCursor().currentTable())
    {
        QTextTable *currentTable = curr_browser->textCursor().currentTable();
        QTextTableCell currentCell = currentTable->cellAt(curr_browser->textCursor());
        currentTable->insertRows(currentCell.row()+1,1);
    }
}

/*!
 * \fn MainWindow::on_actionRemove_Column_triggered()
 * \brief Delete selected column
*/
void MainWindow::on_actionRemove_Column_triggered()
{
    if(!curr_browser || curr_browser->isReadOnly())
        return;
    if(curr_browser->textCursor().currentTable())
    {
        QTextTable *currentTable = curr_browser->textCursor().currentTable();
        QTextTableCell currentCell = currentTable->cellAt(curr_browser->textCursor());
        currentTable->removeColumns(currentCell.column(),1);
    }
}

/*!
 * \fn MainWindow::on_actionRemove_Row_triggered()
 * \brief Delete selected row
*/
void MainWindow::on_actionRemove_Row_triggered()
{
    if(!curr_browser || curr_browser->isReadOnly())
        return;
    if(curr_browser->textCursor().currentTable())
    {
        QTextTable *currentTable = curr_browser->textCursor().currentTable();
        QTextTableCell currentCell = currentTable->cellAt(curr_browser->textCursor());
        currentTable->removeRows(currentCell.row(),1);
    }
}

/*!
 * \brief MainWindow::on_actionFetch_2_triggered()
 * Not using this function
 */
void MainWindow::on_actionFetch_2_triggered()
{
    //check whether user is logged in or not
    QSettings settings("IIT-B", "OpenOCRCorrect");
    settings.beginGroup("loginConsent");
    QString value = settings.value("consent").toString();
    settings.endGroup();
    if(value != "loggedIn"){
        QMessageBox msg;
        msg.setText("Please login to sync cloud data");
        int cnt = 2;
        //showing the message box for 2 seconds only.
        QTimer cntDown;
        QObject::connect(&cntDown, &QTimer::timeout, [&msg,&cnt, &cntDown]()->void{
             if(--cnt < 0){
                 cntDown.stop();
                 msg.close();
             }
            });
        cntDown.start(1000);
        msg.exec();
        return;
    }
    settings.beginGroup("login");
    QString email = settings.value("email").toString();
    settings.endGroup();
    QProcess process;
    process.execute("curl -d -X -k -POST --header "
                    "\"Content-type:application/x-www-form-urlencoded\" https://udaaniitb.aicte-india.org/udaan/email/ -d \"email="+email+"\" -o validate.json");

    QStringList list = gDirTwoLevelUp.split("/");
    QString repo = list[list.size()-1];
    QJsonObject mainObj = readJsonFile("validate.json");
    QJsonArray repos = mainObj.value("repo_list").toArray();
    QJsonArray::iterator itr; int flag = 0;
    for(itr = repos.begin(); itr != repos.end(); itr++){
        //qDebug()<<"itr->toString()"<<itr->toString()<<":"<<repo;
        if(itr->toString() == repo){
            flag = 1;
            break;
        }
    }
    QFile::remove("validate.json");
    if(repos.size() == 0 || flag == 0){
        QMessageBox msg;
        msg.setText("You don't have access to this project on cloud.");
        msg.exec();
        return;
    }

    QString stage = mProject.get_stage();
    QString prvs_stage = (stage=="Verifier")?"Verifier":"Corrector";
    QString prvs_output_dir = prvs_stage + "Output"; //"VerifierOutput" or "CorrectorOutput"

    QMessageBox forPullBox;
    forPullBox.setWindowTitle("Pull ?");
    forPullBox.setIcon(QMessageBox::Question);
    forPullBox.setInformativeText("This will overwrite files in " + prvs_output_dir + " directory. Do you want to Continue?");
    QPushButton *okButton = forPullBox.addButton(QMessageBox::StandardButton::Yes);
    QPushButton *noButton = forPullBox.addButton(QMessageBox::StandardButton::No);
    forPullBox.exec();

    int error;
    if (forPullBox.clickedButton() == okButton)
    {
        if ((error = mProject.fetch()) != 0) {
            qDebug() << "Fetch failed with error code " << error;
        }
        if(mProject.get_version().toInt())
        {
            QMessageBox::information(0, "Sync Success", "Synced Succesfully");
        }
        else
        {
            QMessageBox::information(0, "Sync Error", "Cloud Sync Un-successful, Please Check Your Internet Connection");
        }

        ui->lineEdit_2->setText("Version " + mProject.get_version());
    }
    else
        return;
}

/*!
 * \fn MainWindow::on_actionTurn_In_triggered()
 * \brief To turn-in the corrector's file to git repository when user clicks "submit corrector" button.
 *
 * \note This function turn-in files only for corrector and not for verifier.
 * \sa checkUnsavedWork(), saveAllWork(), get_version(), commit(), push(), set_stage_verifier() and enable_push()
 */
void MainWindow::on_actionTurn_In_triggered()
{
    //! Checking if the files are saved or not.
    if (checkUnsavedWork())
    {
        QMessageBox checkUnsavedBox;
        checkUnsavedBox.setWindowTitle("Unsaved Work");
        checkUnsavedBox.setIcon(QMessageBox::Question);
        checkUnsavedBox.setInformativeText("You have unsaved files. Save it before turn-in.\n");
        QPushButton *cancelButton = checkUnsavedBox.addButton(QMessageBox::Cancel);
        QPushButton *saveButton = checkUnsavedBox.addButton(QMessageBox::Save);
        checkUnsavedBox.exec();



        if (checkUnsavedBox.clickedButton() == cancelButton)
        {
            QMessageBox::information(0, "Turn In", "Turn In Cancelled");
            return;
        }
        else
        {
            saveAllWork();      //saves all the file
        }
    }

    /*
     * \description
     * Checks whether user is logged in or not
    */
    QSettings settings("IIT-B", "OpenOCRCorrect");
    settings.beginGroup("loginConsent");
    QString value = settings.value("consent").toString();
    settings.endGroup();
    if(value != "loggedIn"){
        QMessageBox msg;
        msg.setText("Please login to save your changes on cloud");
        int cnt = 2;
        //showing the message box for 2 seconds only.
        QTimer cntDown;
        QObject::connect(&cntDown, &QTimer::timeout, [&msg,&cnt, &cntDown]()->void{
             if(--cnt < 0){
                 cntDown.stop();
                 msg.close();
             }
            });
        cntDown.start(1000);
        msg.exec();
        return;
    }

    //retrieve details from database and check if user has access to push into this repo
    settings.beginGroup("login");
    QString email = settings.value("email").toString();
    settings.endGroup();
    QProcess process;
    process.execute("curl -d -X -k -POST --header "
                    "\"Content-type:application/x-www-form-urlencoded\" https://udaaniitb.aicte-india.org/udaan/email/ -d \"email="+email+"\" -o validate.json");

    QStringList list = gDirTwoLevelUp.split("/");
    QString repo = list[list.size()-1];
    QJsonObject mainObj = readJsonFile("validate.json");
    QJsonArray repos = mainObj.value("repo_list").toArray();
    QJsonArray::iterator itr; int flag = 0;
    for(itr = repos.begin(); itr != repos.end(); itr++){
       // qDebug()<<itr->toString();
        if(itr->toString() == repo){
            flag = 1;
            break;
        }
    }
    QFile::remove("validate.json");
    if(repos.size() == 0 || flag == 0){
        QMessageBox msg;
        msg.setText("You don't have access to this project on cloud.");
        msg.exec();
        return;
    }

    //!Checking whether all the file are there in CorrectorOutput directory.
    if(mProject.get_version().toInt())
    {
//         if(mProject.findNumberOfFilesInDirectory(mProject.GetDir().absolutePath().toStdString() + R"(/CorrectorOutput/)")
//                 != 2* mProject.findNumberOfFilesInDirectory(mProject.GetDir().absolutePath().toStdString() + R"(/Inds/)"))
//         {
//             QMessageBox::information(0, "Couldn't Turn In", "Make sure all files are there in CorrectorOutput directory");
//             return;
//         }
        QString commit_msg = "Corrector Turned in Version: " + mProject.get_version();     // append current version

        QMessageBox submitBox;
        submitBox.setWindowTitle("Submit ?");
        submitBox.setIcon(QMessageBox::Question);
        submitBox.setInformativeText("Are you ready to submit your changes?");
        QPushButton *yButton = submitBox.addButton(QMessageBox::StandardButton::Yes);
        QPushButton *nButton = submitBox.addButton(QMessageBox::StandardButton::No);
        submitBox.exec();



        if (submitBox.clickedButton() == yButton)
        {
            bool ok;
//            branchName = QInputDialog::getText(this, tr("Branch Name"),
//                                               tr("Enter the branch name:"), QLineEdit::Normal,
//                                               "", &ok );
            branchName = "master";
//            if ( ok && !branchName.isEmpty() ) {
                // user entered something and pressed OK
                // mProject.set_stage_verifier();    // set_stage_verifier()inherited from project.cpp updates the stage in xml file to "verifier"

                //! commits and pushes the file. commit() and push() from Project.cpp creates a commit and pushes the file to git repo
            if(mProject.commit(commit_msg.toStdString()))
            {
//                    if(!mProject.push(branchName))
//                mProject.enable_push(false);      // enable_push() increments version and sets stage in xml file
//                QMessageBox::information(0, "Turn In", "Turn In Cancelled");
//                return;
                threadingPush *tp=new threadingPush(nullptr);
                QThread *thread = new QThread;

                QObject::connect(thread, SIGNAL(started()), tp, SLOT(ControlPush(branchName,repo,
                                                                      login_tries,is_cred_cached,
                                                                      mEmail,mName,
                                                                    user,pass)));
                QObject::connect(tp, SIGNAL(finishedLoadingData()), thread, SLOT(quit()));
                QObject::connect(tp, SIGNAL(finishedLoadingData()), tp, SLOT(deleteLater()));
                QObject::connect(tp, SIGNAL(finished()), thread, SLOT(deleteLater()));
                QObject::connect(tp, SIGNAL(finishedLoadingData()), this, SLOT(stopSpinning()));
                tp->moveToThread(thread);
                thread->start();

                spinner = new LoadingSpinner(this);
                spinner->SetMessage("Loading Data...", "Loading...");
                spinner->setModal(false);
                spinner->exec();
            }
//            }
//            else {
//                // user entered nothing or pressed Cancel
//                QMessageBox::information(0, "Turn In", "Turn In Cancelled");
//                return;
//            }
            mProject.set_corrector();
        }
        else
        {
            QMessageBox::information(0, "Turn In", "Turn In Cancelled");
            return;
        }

        ui->lineEdit_2->setText("Version " + mProject.get_version());      //Update the version of file on ui.

        QString emailText =  "Book ID: " + mProject.get_bookId()
                + "\nSet ID: " + mProject.get_setId()
                + "\n" + commit_msg ;       //Send an email if turn-in failed

       // ui->actionTurn_In->setEnabled(false);        // Deactivating the "Submit Corrector" button on ui
        QMessageBox::information(0, "Turn In", "Turned In Successfully");
        //deleteEditedFilesLog();
    }
    else
    {
        QMessageBox::information(0, "Turn In Error", "Please Open Project Before Turning In");
    }
}

/*!
 * \fn MainWindow::on_actionVerifier_Turn_In_triggered
 * \brief To turn-in the verifier's file to git repository when user clicks ""Submit Verifier" button.
 *
 * \note This function turn-in files for verifiers not correctors.
 * \sa checkUnsavedWork(), saveAllWork(), get_version(), get_stage(), readJsonFile(), writeJsonFile(),set_stage_verifier() and enable_push.
 */
void MainWindow::on_actionVerifier_Turn_In_triggered()
{
    /*!
     * \fn checkUnsavedWork
     * If there's unsaved work, a message box will appear on the screen
     * asking to save the usaved work or not.
     * \return boolean value, true if there is unsaved work and false if not.
    */
    if (checkUnsavedWork())
    {

        QMessageBox checkUnsavedBox2;
        checkUnsavedBox2.setWindowTitle("Unsaved Work");
        checkUnsavedBox2.setIcon(QMessageBox::Question);
        checkUnsavedBox2.setInformativeText("You have unsaved files. Save it before turn-in.\n");
        QPushButton *cancelButton2 = checkUnsavedBox2.addButton(QMessageBox::Cancel);
        QPushButton *saveButton2 = checkUnsavedBox2.addButton(QMessageBox::Save);
        checkUnsavedBox2.exec();



        if (checkUnsavedBox2.clickedButton() == cancelButton2)
        {
            QMessageBox::information(0, "Turn In", "Turn In Cancelled");
            return;
        }
        else
        {
            saveAllWork();
        }
    }

    /*
     * \description
     * Checks whether user is logged in or not
    */
    QSettings settings("IIT-B", "OpenOCRCorrect");
    qDebug() << settings.fileName();
    settings.beginGroup("loginConsent");
    QString value = settings.value("consent").toString();
    settings.endGroup();
    if(value != "loggedIn"){
        QMessageBox msg;
        msg.setText("Please login to save your changes on cloud");
        int cnt = 2;
        //showing the message box for 2 seconds only.
        QTimer cntDown;
        QObject::connect(&cntDown, &QTimer::timeout, [&msg,&cnt, &cntDown]()->void{
             if(--cnt < 0){
                 cntDown.stop();
                 msg.close();
             }
            });
        cntDown.start(1000);
        msg.exec();
        return;
    }
    /*
     * \description
     * 1. Checks if any project is opened or not.
     * 2. Reads the comments.json file in Comments folder in the opened project.
     * 3. Calculates AverageCharAccuracy.
     * 4. mRoleCheck
     * 5. Sets the rating and formatting.
    */

    if(mProject.get_version().toInt())
    {

        int ver = mProject.get_version().toInt();
        QString commit_msg;

        QString commentFilename = gDirTwoLevelUp + "/Comments/comments.json";
        float avgcharacc = 0;
        bool formatting = false;
        int rating = 0;

        QJsonObject mainObj = readJsonFile(commentFilename);

        avgcharacc = mainObj["AverageCharAccuracy"].toDouble();

        //! Calcuates the rating for the current set.
        if(mProject.get_stage() != mRole)
            rating = mainObj["Rating-V"+ QString::number(mProject.get_version().toInt() - 1)].toInt();
        else
            rating = mainObj["Rating-V"+ mProject.get_version()].toInt();
        if(((!mainObj["Formatting"].isNull())) || (! mainObj["Formatting"].isUndefined()))
            formatting = mainObj["Formatting"].toBool();

        /*
         * \description
         * 1. Check formatting dialog box will be opened
         * 2. Dialog box will show the current rating out of 4 and a checkbox.
        */
        QDialog dialog(this);
        dialog.setWindowTitle("Check Formatting");

        QFormLayout form(&dialog);
        form.addRow(new QLabel("Average Rating of Current Set  : " + QString::number(rating) + " out of 4"));

        QCheckBox *cb = new QCheckBox("Perfect Formatting?" ,&dialog);
        cb->setChecked(formatting);
        form.addRow(cb);

        QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                   Qt::Horizontal, &dialog);
        form.addRow(&buttonBox);

        //! Checking which signal has been passed i.e. accept or reject.
        QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));     //when ok is pressed.
        QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));     //when cancel is pressed.

        if (dialog.exec() == QDialog::Accepted)
        {
            formatting = cb->isChecked();

            //! If the checkbox is checked, increment rating by 1 else decrement the rating value by 1.
            if(rating == 4 && formatting)
                rating = 5;
            else if(rating == 5 && (!formatting))
                rating = 4;
        }
        else
        {
            QMessageBox::information(0, "Turn In", "Turn In Cancelled");
            return;
        }

        //! Updating the formatting and rating parameters value in the comments.json file, if any
        mainObj["Formatting"] = formatting;

        //! Calcuates the rating for the current set.
        if(mProject.get_stage() != mRole)
            mainObj["Rating-V"+ QString::number(mProject.get_version().toInt() - 1)] = rating;
        else
            mainObj["Rating-V"+ mProject.get_version()] = rating;

        writeJsonFile(commentFilename, mainObj);

        QMessageBox messageBox(this);
        QString msg1 = QString(
                    "Rating for Current Version Based on the Formatting Input: " + QString::number(rating) + " out of 5"

                    + "\n\nDo you want to Return the Set to the Corrector or Finalise the set?"

                    + "\n\nClick \"Return Set\" to Increment the Version from "
                    + QString::number(ver) +" to "+QString::number(ver + 1)

                    + "\nClick \"Finalise\" to Approve the set as the Final Version"
                    );

        QString msg2 = QString(
                    "Rating for Current Version Based on the Formatting Input: " + QString::number(rating) + " out of 5"

                    + "\n\nDo you want to Return or Resubmit or Finalise the set?"

                    + "\n\nClick \"Return Set\" to Turnin and Increment the Version from "
                    + QString::number(ver) +" to "+QString::number(ver + 1)

                    + " \nClick \"Resubmit\" to Turn In without Incrementing Version."

                    + "\nClick \"Finalise\" to Approve the set as the Final Version"
                    );

        messageBox.setWindowTitle("Turn In");
        QAbstractButton *resubmitButton =
                messageBox.addButton(tr("Resubmit"), QMessageBox::ActionRole);
        QAbstractButton *returnSetButton =
                messageBox.addButton(tr("Return Set"), QMessageBox::ActionRole);
        QAbstractButton *finaliseButton =
                messageBox.addButton(tr("Finalise"), QMessageBox::ActionRole);
        QAbstractButton *cancelButton =
                messageBox.addButton(tr("Cancel"), QMessageBox::RejectRole);

        /*
             * \description
             * 1.Checks if the project content is added to the staging area or not
             * 2. If no, display \a msg1 and remove the resubmit button.
             * 3. If yes, display \a msg2.
        */
        if(mRole != mProject.get_stage())
        {
            messageBox.setText(msg1);
            messageBox.removeButton(resubmitButton);
        }
        else
        {
            messageBox.setText(msg2);
        }

        messageBox.exec();

        /*!
         * \enum class SubmissionType
         *
         * This enum describes the type of submission type.
         *
         * \value resubmit To turn in without incrementing version.
         * \value return_set To turn-in and increment the version.
         * \value finalise To approve the set as the final version.
        */
        enum class SubmissionType {resubmit, return_set, finalise};
        SubmissionType s ;

        /*
            * Checking the condition: CorrectorOutputFiles != 2*IndsFiles
            * If true, then \a s and \a commit_msg are updated.
        */
        if (messageBox.clickedButton() == resubmitButton)
        {
            //mProject.enable_push( false ); //Increment = false
//             if(mProject.findNumberOfFilesInDirectory(mProject.GetDir().absolutePath().toStdString() + R"(/VerifierOutput/)")
//                     != 2* mProject.findNumberOfFilesInDirectory(mProject.GetDir().absolutePath().toStdString() + R"(/Inds/)"))
//             {
//                 QMessageBox::information(0, "Couldn't Turn In", "Make sure all files are there in VerifierOutput directory");
//                 return;
//             }
            s = SubmissionType::resubmit;
            commit_msg = "Verifier Resubmitted Version:" + mProject.get_version();
        }

        //! \a s and \a commit_msg are updated
        else if (messageBox.clickedButton() == returnSetButton)
        {
            //mProject.enable_push( true ); //Increment = true
            s = SubmissionType::return_set;
            commit_msg = "Verifier has Turned in the Next Version:" + mProject.get_version();
        }

        /*
         * Checking the condition: CorrectorOutputFiles != 2*IndsFiles
         * If true, a message box of \value Couldn't Turn in will be displayed,
         * else \a s and \a commit_msg are updated.
        */
        else if (messageBox.clickedButton() == finaliseButton)
        {
            //mProject.enable_push( false ); //Increment = false
//             if(mProject.findNumberOfFilesInDirectory(mProject.GetDir().absolutePath().toStdString() + R"(/VerifierOutput/)")
//                     != 2* mProject.findNumberOfFilesInDirectory(mProject.GetDir().absolutePath().toStdString() + R"(/Inds/)"))
//             {
//                 QMessageBox::information(0, "Couldn't Turn In", "Make sure all files are there in VerifierOutput directory");
//                 return;
//             }
            s = SubmissionType::finalise;
            commit_msg = "Verifier Finalised Version:" + mProject.get_version();
        }
        else
        {
            QMessageBox::critical(0, "Turn In", "Turn In Cancelled");
            return;
        }

        QMessageBox submitBox2;
        submitBox2.setWindowTitle("Submit ?");
        submitBox2.setIcon(QMessageBox::Question);
        submitBox2.setInformativeText("Are you ready to submit your changes?");
        QPushButton *yButton2 = submitBox2.addButton(QMessageBox::StandardButton::Yes);
        QPushButton *nButton2 = submitBox2.addButton(QMessageBox::StandardButton::No);
        submitBox2.exec();



       if (submitBox2.clickedButton() == yButton2)
       {
            bool ok;
//            branchName = QInputDialog::getText(this, tr("Branch Name"),
//                                                 tr("Enter the branch name:"), QLineEdit::Normal,
//                                                 "", &ok );
            branchName ="master";
            if (!branchName.isEmpty() ) {
                // user entered something and pressed OK
                if(s == SubmissionType::return_set)   //If yes button is clicked and submission type is return_set then enable push
                {
                    mProject.enable_push( true );
                }
                else if (s == SubmissionType::resubmit)    //If yes button is clicked and submission type is resubmit then enable push
                {
                    mProject.enable_push( false );
                }
                if(!mProject.commit(commit_msg.toStdString()) || !mProject.push(branchName))
                {
                    if(s == SubmissionType::return_set)
                    {
                        mProject.set_version( mProject.get_version().toInt() - 1 );
                    }
                   // mProject.set_stage_verifier();
                    QMessageBox::critical(0, "Turn In", "Turn In Cancelled");
                    return;
                }
            }
            else {
                // user entered nothing or pressed Cancel
                QMessageBox::critical(0, "Turn In", "Turn In Cancelled");
                return;
            }
            mProject.set_verifier();
        }
        else
        {
            QMessageBox::critical(0, "Turn In", "Turn In Cancelled");
            return;
        }

        //! Sending email with the following information
        QString emailText =  "Book ID: " + mProject.get_bookId()
                + "\nSet ID: " + mProject.get_setId()
                + "\nRating Provided: " + QString::number(rating)
                + "\n" + commit_msg ;

        //! Updating the Project Version
        ui->lineEdit_2->setText("Version " + mProject.get_version());
        QMessageBox::information(0, "Turn In", "Turned In Successfully");
        //deleteEditedFilesLog();
    }
    else
    {
        QMessageBox::critical(0, "Turn In Error", "Please Open Project Before Turning In");
    }
}

/*!
 * \fn MainWindow::on_actionSymbols_triggered
 * \brief It shows the table for various symbols
 * \sa Symbols.cpp
 */
void MainWindow::on_actionSymbols_triggered()
{
    SymbolsView *dialog = SymbolsView::openSymbolTable(this);
    dialog->show();
}

/*!
 * \fn MainWindow::on_actionZoom_In_triggered
 * \brief for zoom-in operation
 */
void MainWindow::on_actionZoom_In_triggered()
{
//    if (z)
//        z->gentle_zoom(z->getDefaultZoomInFactor());
    if(!curr_browser || curr_browser->isReadOnly())
        return;
    QTextCursor cursor = curr_browser->textCursor();
    /*
     * charFormat returns the format of the character before the position
     * So, we interchange the ancr and position
    */
//    int pos = cursor.position();
//    int ancr = cursor.anchor();
//    if (pos < ancr) {
//        cursor.setPosition(pos, QTextCursor::MoveAnchor);
//        cursor.setPosition(ancr, QTextCursor::KeepAnchor);
//    }
    //qDebug()<<"pos : ancr"<<pos<<ancr;
    qreal fontSize = cursor.charFormat().fontPointSize();
    if(fontSize == 0){
        fontSize = 16;
    }
    //qDebug()<<"Font size returned is:"<<fontSize;
    /*
     * Increase the font size by 1
    */
    QTextCharFormat fmt;
    fmt.setFontPointSize(fontSize+1);
    cursor.mergeCharFormat(fmt);
}

/*!
 * \fn MainWindow::on_actionZoom_Out_triggered
 * \brief for zoom-out operation
 */
void MainWindow::on_actionZoom_Out_triggered()
{
//    if (z)
//        z->gentle_zoom(z->getDefaultZoomOutFactor());
    if(!curr_browser || curr_browser->isReadOnly())
        return;
    QTextCursor cursor = curr_browser->textCursor();
    /*
     * charFormat returns the format of the character before the position
     * So, we interchange the ancr and position
    */
//    int pos = cursor.position();
//    int ancr = cursor.anchor();
//    if (pos < ancr) {
//        cursor.setPosition(pos, QTextCursor::MoveAnchor);
//        cursor.setPosition(ancr, QTextCursor::KeepAnchor);
//    }
    //qDebug()<<"pos : ancr"<<pos<<ancr;
    qreal fontSize = cursor.charFormat().fontPointSize();
    if(fontSize == 0){
        fontSize = 16;
    }
    //qDebug()<<"Font size returned is:"<<fontSize;
    /*
     *decrease the font size by 1
    */
    QTextCharFormat fmt;
    fmt.setFontPointSize(fontSize - 1);
    cursor.mergeCharFormat(fmt);
    //curr_browser->mergeCurrentCharFormat(fmt);
}

/*!
 * \fn MainWindow::on_pushButton_clicked
 * \brief When button is clicked, then we can add placeholders for figure/table/equation.
 * \sa eventFilter
 */
void MainWindow::on_pushButton_clicked()
{
    if(loadimage)                   //Check image is loaded or not.
    {
        ui->graphicsView->setDragMode(QGraphicsView::NoDrag);
        shouldIDraw=true;
        auto p = (QPushButton*)ui->pushButton;       //get the pushButton
        p->setStyleSheet("QPushButton { background-color:rgb(227, 228, 228);border:0px; color: rgb(32, 33, 72); height:26.96px; width: 109.11px; padding-top:1px; border-radius:4.8px; padding-left:1.3px; }\n"
                          "QPushButton:enabled { background-color: rgb(136, 138, 133);color:white; }\n");      //apply style on button when it is triggered
     }
}

/*!
 * \fn MainWindow::keyPressEvent
 * \brief This function handles "Ctrl+C" for storing the copied values and showing when user right clicks. It also handles "Ctrl+D" which is used for converting english text to devanagari text
 * \param e
 */
void MainWindow::keyPressEvent(QKeyEvent *e)
{
    if ( (e->key() == Qt::Key_C)  && QApplication::keyboardModifiers() == Qt::ControlModifier)
    {
        QTextCursor cursor = curr_browser->textCursor();
        QString text = cursor.selectedText().toUtf8().constData();
        if(text!=""){
            QSettings settings("IIT-B", "OpenOCRCorrect");
        settings.beginGroup("Clipboard");
        QString s1,s2,s3;
        s1 = settings.value("copy1").toString();
        s2 = settings.value("copy2").toString();
        s3 = settings.value("copy3").toString();
        if(text == s1){
            settings.setValue("copy1",text);
        }
        else if(text == s2){
            settings.setValue("copy2",settings.value("copy1").toString());
            settings.setValue("copy1",text);
        }
        else{
            settings.setValue("copy3",settings.value("copy2").toString());
            settings.setValue("copy2",settings.value("copy1").toString());
            settings.setValue("copy1",text);
        }
        settings.endGroup();
        }
    }

    if ( (e->key() == Qt::Key_D)  && QApplication::keyboardModifiers() == Qt::ControlModifier)
    {
       QString convertedText;
       string selectedStr;

       selectedStr = ui->lineEdit_4->text().toUtf8().constData();
       convertedText = toDevanagari(selectedStr);
       ui->lineEdit_4->setText(convertedText);
    }
}

QString MainWindow::toDevanagari(string text) {
    slpNPatternDict slnp;
    return QString::fromStdString(slnp.toDev(slnp.toslp1(text)));
}

/*!
 * \fn MainWindow::eventFilter
 * \brief event: ToolTip and ImageMarkingRegion
      1. Responsible for drawing rectangular region
      2. Placing a PlaceHolder for figure/table/equation entries
      3. Set a MessageBox for figure/table/equation/cancel
      4. Set counter for pagewise for each entry
      5. Mark multiple image regions in a loaded image.
      6. Set various flag: a)drawRectangleFlag: is to prevent triggering of this function twice
      b) loadimage: check image is loaded on not; c) pressedFlag: resposible for dynamic rectangular
      drawing.
 * \param object, event
 * \return QMainWindow::eventFilter(object, event);
 * \sa MainWindow::displayHolder, MainWindow::updateEntries, MainWindow::createImageInfoXMLFile, findStringSimilarity,
 */
bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
    //! Tooltip documentation
    markRegion objectMarkRegion;
    QString bboxf = currentTabPageName;
    QFile bbox_file(gDirTwoLevelUp + "/bboxf/"+bboxf.replace(".html", ".bbox"));
    //! When user moves his mouse the system will ask user to download new update.

    if(event->type() == QEvent::MouseButtonPress)
    {

        if(isProjectOpen)
        {
            if(curr_browser != NULL){

            curr_browser->setStyleSheet("CustomTextBrowser{selection-background-color: #3297fd; selection-color: #ffffff;}");
            }
        }
    }

    //! When the user moves the cursor over unedited html file text, it shows a rectangle box with some text value.
    if (event->type() == QEvent::ToolTip)
    {
          event->accept();

         if(QToolTip::text() != "")
         {

             QString qs =  QToolTip :: text();

             int x0, y0, x1, y1;

             QStringList list;
             list=qs.split(" ");
             int len = list.count();
             if (len>=5)
             {

                 x0 = list[1].toInt();
                 y0 = list[2].toInt();
                 x1 = list[3].toInt();
                 y1 = list[4].replace(";", "").toInt();
                 if(x1!=0 && x0!=0 && y1!=0 && y0!=0)
                 {
                     QColor blue40 = Qt::blue;
                     blue40.setAlphaF( 0.4 );

                     item1->setBrush(blue40);

                     item1->setRect(x0, y0, x1-x0, y1-y0);
                  }
             }
          }
      }
    edit_Distance edit;


    //! ImageMarkingRegion feature
    if(loadimage)                   //Check image is loaded or not.
    {
     static int x1, y1;             //top & left coordinate values
     int x2, y2;                    //bottom & right coordinate values
     int x_temp , y_temp;           // dynamic coordinate values

     //! Apply event on graphicsview (image loaded part)
     if( object->parent() == ui->graphicsView)
     {
            installEventFilter(this);
            //! Capturing mouse press event on graphicsview
            if (event->type() == QEvent::MouseButtonPress && shouldIDraw)
            {
                QMouseEvent *mEvent = static_cast<QMouseEvent*>(event);
                QPointF pos =  ui->graphicsView->mapToScene( mEvent->pos()); //Capturing the coordinates values according to the image.
                QRgb rgb = imageOrig.pixel( ( int )pos.x(), ( int )pos.y());

                x1 = ( int )pos.x();      //left coordinate value
                y1 = ( int )pos.y();      //top coordinate value
                pressedFlag=1;            //drawing is on until it becomes 0 or for continuous pressing event
                event->accept();
            }
            //! Capturing mouse release event on graphicsview
            if (event->type() == QEvent::MouseButtonRelease)
            {
                //! reponsible for preventing the event second time.
                if(drawRectangleFlag==true)
                {
                    drawRectangleFlag=false;
                    pressedFlag =0;        //for stopping the drawing
                    event->accept();
                    return true;
                }
                if(shouldIDraw){

                drawRectangleFlag=true;     //set the flag true when occuring for first time
                static int i,j,k;           //for storing the counter values for figure/equation/table for each page
                static QString a;           //pagecounter

                //! Getting PageNo string from gCurrentPageName
                QStringList PageNo=gCurrentPageName.split(QRegExp("[-.]"));
                QString PageNumber = PageNo[1];

                //!Getting i,j,k values from image.xml file
                //! first reading the file
                QDomDocument document;
                QString filename12 = mProject.GetDir().absolutePath() + "/image.xml";
                QFile f(filename12);

                //! throws an error if file is not in readonly mode
                if (!f.open(QIODevice::ReadOnly ))
                {
                    std::cerr << "Error while loading file" << std::endl;
                    return 1;
                }
                document.setContent(&f);       // Set data into the QDomDocument before processing
                f.close();

                //!for this you can refer image.xml file
                QDomElement root=document.documentElement();       //Item: BookSet
                QDomElement Component=root.firstChild().toElement();      //Item: Page(No)

                //! Loop while there is a child
                while(!Component.isNull())
                {
                    //! Check if the child tag name is Page(No)
                    if (Component.tagName()=="page"+PageNo[1])
                    {
                        a = Component.attribute("count");        //get counter value for each page starts with 1.
                        QDomElement Child=Component.firstChild().toElement();      //Item: figure
                        while (!Child.isNull())
                        {
                            //! Read tagNames and values
                            if (Child.tagName()=="figure") i=Child.firstChild().toText().data().toInt();
                            if (Child.tagName()=="table") j=Child.firstChild().toText().data().toInt();
                            if (Child.tagName()=="equation") k=Child.firstChild().toText().data().toInt();

                            Child = Child.nextSibling().toElement();        // Next child
                        }
                    }
                    Component = Component.nextSibling().toElement();        // Next component
                 }

                QMouseEvent *mEvent = static_cast<QMouseEvent*>(event);
                QPointF pos =  ui->graphicsView->mapToScene( mEvent->pos() );
                QRgb rgb = imageOrig.pixel( ( int )pos.x(), ( int )pos.y() );

                x2 = ( int )pos.x();         //right coordinate value
                y2 = ( int )pos.y();         //bottom coordinate value
                pressedFlag =0;              // stop rectangular drawing


                QColor blue40 = Qt::blue;     //sets its color
                blue40.setAlphaF( 0.4 );      //for transparency

                crop_rect->setBrush(blue40);   //set brush

                //qDebug() << x1 << " " << y1 << " " << x2 - x1 << " " << y2 - y1;   //getting the coordinates

                crop_rect->setRect(x1, y1, x2 - x1, y2 - y1);       //set final coordinates for rectangular region
                QRect rect(x1, y1, x2 - x1, y2 - y1);              //set QRect
                QPixmap image=QPixmap::fromImage(imageOrig);       //set QPixmap image
                QPixmap cropped=image.copy(rect);                   //get cropped image according to coordinates

                //! Set a messagebox for choosing what do you want to add: Figure/Table/Equation/Cancel
                QMessageBox messageBox;          //isisde this argument is remove need to be tested
                messageBox.setWindowTitle("Do you want to add");
                QAbstractButton *figureButton = messageBox.addButton(tr("Figure"), QMessageBox::ActionRole);
                QAbstractButton *tableButton = messageBox.addButton(tr("Table"), QMessageBox::ActionRole);
                QAbstractButton *equationButton = messageBox.addButton(tr("Equation"), QMessageBox::ActionRole);
                QAbstractButton *cancelButton = messageBox.addButton(tr("Cancel"), QMessageBox::RejectRole);

                QString msg = "Select an option\n";
                messageBox.setText(msg);
                messageBox.exec();

                //! settings for a figureholder
                if (messageBox.clickedButton() == figureButton)
                {
                    QString s1 = "IMGHOLDER";
                    QString s2 = "Figure";

                    //graphic->removeItem(crop_rect);

                    //!Saving Image Regions to their respective folder(Figure/Table/Equation)
                    saveImageRegion(cropped,a,s1,i,x2-x1,y2-y1);

                    i++;       //increment values when a figure is inserted in the textBrowser

                    crop_rect->setRect(0,0,1,1);       //settings this for dynamic rectangular region

                    //! updating entries for figure entries in xml file
                   objectMarkRegion.updateEntries(document, filename12, PageNo[1], s2, i);

                    shouldIDraw=false;
                    ui->pushButton->setStyleSheet("background-color:rgb(227, 228, 228);border:0px; color: rgb(32, 33, 72);height:26.96px; width: 109.11px; padding-top:1px; border-radius:4.8px; padding-left:1.3px;");     //remove the style once the operation is done
                }
                //! settings for a tableholder
                else if (messageBox.clickedButton() == tableButton)
                {
                    QString s1 = "TBHOLDER";
                    QString s2 = "Table";

                    //graphic->removeItem(crop_rect);

                    //!Saving Image Regions to their respective folder(Figure/Table/Equation)
                    saveImageRegion(cropped,a,s1,j,x2-x1,y2-y1);

                    j++;         //increment values when a table is inserted in the textBrowser

                    crop_rect->setRect(0,0,1,1);         //settings this for dynamic rectangular region

                    //! updating entries for table entries in xml file
                   objectMarkRegion.updateEntries(document, filename12, PageNo[1], s2, j);

                    shouldIDraw=false;
                    ui->pushButton->setStyleSheet("background-color:rgb(227, 228, 228);border:0px; color: rgb(32, 33, 72);height:26.96px; width: 109.11px; padding-top:1px; border-radius:4.8px; padding-left:1.3px;");       //remove the style once the operation is done
                }
                //! settings for a equationholder
                else if(messageBox.clickedButton() == equationButton)
                {
                    QString s1 = "EQHOLDER";
                    QString s2 = "Equation";

//                    //! for placing a equation placeholder
//                    (s1,s2,a,x1,y1,x2,y2,k);

                    //graphic->removeItem(crop_rect);

                    //!Saving Image Regions to their respective folder(Figure/Table/Equation)
                    saveImageRegion(cropped,a,s1,k,x2-x1,y2-y1);

                    k++;       //increment values when a equation is inserted in the textBrowser

                    crop_rect->setRect(0,0,1,1);       //settings this for dynamic rectangular region

                    //! updating entries for equation entries in xml file
                    objectMarkRegion.updateEntries(document, filename12, PageNo[1], s2, k);

                    shouldIDraw=false;
                    ui->pushButton->setStyleSheet("background-color:rgb(227, 228, 228);border:0px; color: rgb(32, 33, 72);height:26.96px; width: 109.11px; padding-top:1px; border-radius:4.8px; padding-left:1.3px;");       //remove the style once the operation is done
                }
                //! setting for cancelbutton
                else
                {
                    QMessageBox::information(0, "Not saved", "Cancelled");
                    crop_rect->setRect(0,0,1,1);
                    shouldIDraw=false;
                    ui->pushButton->setStyleSheet("background-color:rgb(227, 228, 228);border:0px; color: rgb(32, 33, 72);height:26.96px; width: 109.11px; padding-top:1px; border-radius:4.8px; padding-left:1.3px;");       //remove the style once the operation is done
                }
                ui->graphicsView->setDragMode( QGraphicsView::DragMode::ScrollHandDrag );
                event->accept();
                //return true;
            }
            }
        }
        //! Capturing mousemove event & creating single dynamic rectangle & Updating the temporary coordinates until pressedFlag is true
        if (event->type() == QEvent::MouseMove)
        {
             QMouseEvent *mEvent = static_cast<QMouseEvent*>(event);

             if (pressedFlag == 1)
             {
                 //statusBar()->showMessage(QString("Mouse move (%1,%2)").arg(mEvent->pos().x()).arg(mEvent->pos().y()));
                 QPointF position =  ui->graphicsView->mapToScene( mEvent->pos() );
                 QRgb rgb = imageOrig.pixel( ( int )position.x(), ( int )position.y() );

                 QColor blue40 = Qt::blue;
                 blue40.setAlphaF( 0.4 );
                 crop_rect->setBrush(blue40);
                 x_temp = ( int )position.x();
                 y_temp = ( int )position.y();

                 crop_rect->setRect(x1, y1, x_temp-x1, y_temp-y1);
         }
         event->accept();
      }
    }

    if (event->type() == QEvent::ShortcutOverride) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

        if ( keyEvent->key()) {
            //keyPressEvent(keyEvent);
//            event->ignore();
            WordCount();
        }
    }
    if (event->type() == QEvent::ShortcutOverride) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->modifiers().testFlag(Qt::ControlModifier) && keyEvent->key() == 'C') {
            keyPressEvent(keyEvent);
            event->ignore();
            return true;
        }
    }
    if (event->type() == QEvent::ShortcutOverride) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->modifiers().testFlag(Qt::ControlModifier) && keyEvent->key() == 'D') {
            keyPressEvent(keyEvent);
            event->ignore();
            return true;
        }
    }
    return QMainWindow::eventFilter(object, event);
}

/*!
 * \fn MainWindow::saveImageRegion
 * \brief Saving Image  cropped regions to their respective folder(Figure/Table/Equation)
 * \param cropped
 * \param a
 * \param s1
 * \param z
 * \param h
 * \param w
 */
void MainWindow::saveImageRegion(QPixmap cropped, QString a, QString s1,int z, int w, int h)
{
    //! If directory exists then create the folders
    if(!QDir(gDirTwoLevelUp+"/Cropped_Images").exists()){
        QDir(gDirTwoLevelUp).mkdir("Cropped_Images");
    }
    if(!QDir(gDirTwoLevelUp+"/Cropped_Images/Figures").exists())
    {
        QDir(gDirTwoLevelUp).mkdir("Cropped_Images/Figures");
        QDir(gDirTwoLevelUp).mkdir("Cropped_Images/Tables");
        QDir(gDirTwoLevelUp).mkdir("Cropped_Images/Equations");
    }

    //! Adding picture to the respective directory
    if(QDir(gDirTwoLevelUp+"/Cropped_Images").exists())
    {
        if(s1 == "IMGHOLDER")
        {
            QString path = "/Cropped_Images/Figures/Figure"+a+"-"+QString::number(z)+".jpg";

            cropped.save(gDirTwoLevelUp+path,"JPG",100);       //100 is storing the image in uncompressed high resolution

            QString src = ".."+path;
            QString html = QString("\n <img src='%1' width='%2' height='%3'>").arg(src).arg(w).arg(h); //Creating an img tag for image resize in latek
            QTextCursor cursor = curr_browser->textCursor();
            cursor.insertHtml(html);
        }
        else if(s1 == "TBHOLDER")
        {
            QString path = "/Cropped_Images/Tables/Table"+a+"-"+QString::number(z)+".jpg";

            cropped.save(gDirTwoLevelUp+path,"JPG", 100);

            QString src = ".."+path;

            QString html = QString("\n <img src='%1' width='%2' height='%3'>").arg(src).arg(w).arg(h);
            QTextCursor cursor = curr_browser->textCursor();
            cursor.insertHtml(html);

        }
        else if(s1 == "EQHOLDER")
        {
            QString path = "/Cropped_Images/Equations/Equation"+a+"-"+QString::number(z)+".jpg";

            cropped.save(gDirTwoLevelUp+path,"JPG",100);

            QString src = ".."+path;
            QString html = QString("\n <img src='%1' width='%2' height='%3'>").arg(src).arg(w).arg(h);
            QTextCursor cursor = curr_browser->textCursor();
            cursor.insertHtml(html);
        }
        else
        {
            //empty
        }
    }
}


/*!
 * \brief MainWindow::on_pushButton_2_clicked
 * Button for resizing an image
 * Captures src, width and height attributes, modifies height and width and change the image size
 * it does not chnage the quality of an image
 */
void MainWindow::on_pushButton_2_clicked()
{
    if(!curr_browser) return;

    auto cursor = curr_browser->textCursor();
    auto selected = cursor.selection();
    QString sel = selected.toHtml();
    QRegularExpression rex("<img(.*?)>",QRegularExpression::DotMatchesEverythingOption);
    //    QRegularExpression rex("(<img[^>]*>)",QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatchIterator itr;
    itr = rex.globalMatch(sel);
    int height=0;
    int width=0;
    if(!sel.contains("<img")){
        QMessageBox::critical(this,"Error","Image Not selected");
        return;
    }
        QDialog dialog(this);
        QFormLayout form(&dialog);

        form.addRow(new QLabel("Insert Height and Width",this));

        QLineEdit *height_textLine= new QLineEdit(&dialog);
         QLineEdit *width_textLine= new QLineEdit(&dialog);

         form.addRow("Height",height_textLine);
           form.addRow("Width",width_textLine);

           QDialogButtonBox buttonbox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,Qt::Horizontal,&dialog);
           form.addRow(&buttonbox);

           QObject::connect(&buttonbox,SIGNAL(accepted()),&dialog,SLOT(accept()));
           QObject::connect(&buttonbox,SIGNAL(rejected()),&dialog,SLOT(reject()));

           if(dialog.exec() ==QDialog::Accepted){
               height=height_textLine->text().toInt();
               width=width_textLine->text().toInt();
           }

    while(itr.hasNext())
    {
        QRegularExpressionMatch match = itr.next();
        QString ex = match.captured(1);
        string str = ex.toStdString();
        int ind = str.find("src=");
        ind+=5;
        int start = ind;
        int end = 0;

        if (str.find(".jpg") != -1) {
            end = str.find(".jpg");
            end += 3;
        } else if (str.find(".png") != -1) {
            end = str.find(".png");
            end += 3;
        } else if (str.find(".jpeg") != -1) {
            end = str.find(".jpeg");
            end += 4;
        } else {
            qDebug() << "File extension not recognisable";
        }

        str = str.substr(start,end-start+1);
        QString imgname = QString::fromStdString(str);

        if(height>0 && width>0)
        {
            QString html = QString("\n <img src='%1' width='%2' height='%3'>").arg(imgname).arg(width).arg(height);
            cursor.insertHtml(html);      //insert new image with modified attributes height and width
        }
    }
}

/*!
 * \fn MainWindow::on_viewComments_clicked
 * \brief This function is used for viewing the Comments. It opens up the comment box for showing comments.
 */
void MainWindow::on_viewComments_clicked()
{
    if (curr_browser)
    {
        QString correctorOutput,currentpagetext;
        QString correctorText = gDirTwoLevelUp + "/CorrectorOutput/" + gCurrentPageName;
        QFile sFile(correctorText);
        if(sFile.open(QFile::ReadOnly | QFile::Text))
        {
            QTextStream in(&sFile);
            in.setCodec("UTF-8");
            correctorOutput = in.readAll().simplified();
            sFile.close();
        }

        map<int, int> wordCount;
        QString commentFilename = gDirTwoLevelUp + "/Comments/comments.json";
        QString pageName = gCurrentPageName;
        pageName.replace(".txt", "");
        pageName.replace(".html", "");
        int totalCharErrors = 0, totalWordErrors = 0, rating = 0;
        QString comments = "";
        QString avgAcc = "100%";
        float wordAccuracy=100, charAccuracy=100, avgCharAcc = 100;
        QString version = mProject.get_version();

        QJsonObject mainObj = readJsonFile(commentFilename);
        QJsonObject pages = mainObj.value("pages").toObject();
        QJsonObject page = pages.value(pageName).toObject();

        if( !mainObj.isEmpty() )
        {
            rating = mainObj["Rating-V"+ mProject.get_version()].toInt();
            avgCharAcc = mainObj["AverageCharAccuracy"].toDouble();
            avgAcc = QString::number((((float)lround(avgCharAcc*100))/100)) + "%";

            if(!page.isEmpty())
            {
                comments = page.value("comments").toString();
                totalCharErrors = page.value("charerrors").toInt();
                totalWordErrors = page.value("worderrors").toInt();
                wordAccuracy = page.value("wordaccuracy").toDouble();
                charAccuracy = page.value("characcuracy").toDouble();
            }
        }

        if(!isVerifier)
        {
            CommentsView *cv = new CommentsView(totalWordErrors,totalCharErrors,wordAccuracy,charAccuracy,comments,commentFilename, pageName, rating, avgAcc, mRole,version);
            cv->show();
            return;
        }

        //HIGHLIGHTS FOR Accuracy Calculation
        auto textCursor = curr_browser->textCursor();
        QString textBrowserText = curr_browser->toPlainText();
        textCursor.setPosition(0);
        QString highlightedChars = "", selectedChar; // to store all the highlighted characters
        int prevHighlightPos = -2; // Used as an indicator to separate non contigous highlighted text with a space

        while(!textCursor.atEnd())
        {
            int anchor = textCursor.position();
            QTextCharFormat format = textCursor.charFormat();
            if(anchor!=0)
                selectedChar = QString(textBrowserText[anchor-1]);
            if(!selectedChar.contains(" "))
            {
                textCursor.select(QTextCursor::WordUnderCursor);
                QString wordundercursor = textCursor.selectedText();
                int key = textCursor.selectionStart();

                if(format.background() == Qt::yellow && anchor>=(key+1))
                {
                    //totalCharErrors++;
                    if((prevHighlightPos != -2) && (anchor != prevHighlightPos + 1))
                        highlightedChars += " ";
                    highlightedChars += selectedChar;
                    wordCount[key]++;
                    prevHighlightPos = anchor;
                }
            }
            textCursor.setPosition(anchor+1);
            //textCursor.movePosition(QTextCursor::NextCharacter , QTextCursor::MoveAnchor, 1);
        }
        totalCharErrors = mProject.GetGraphemesCount(highlightedChars);

        int totalChars=0;
        QString currentText = curr_browser->toPlainText();
        int totalWords = currentText.simplified().count(" ") + 1;
        QTextBoundaryFinder finder1 = QTextBoundaryFinder(QTextBoundaryFinder::BoundaryType::Grapheme, currentText);
        while (finder1.toNextBoundary() != -1)
        {
            totalChars++;
        }

        totalWordErrors = wordCount.size();
        charAccuracy = (float)(totalChars - totalCharErrors)/(float)totalChars*100;
        wordAccuracy = (float)(totalWords - totalWordErrors)/(float)totalWords*100 ;
        wordAccuracy = ((float)lround(wordAccuracy*100))/100;
        charAccuracy = ((float)lround(charAccuracy*100))/100;

        page["comments"] = comments;
        page["charerrors"] = totalCharErrors;
        page["worderrors"] = totalWordErrors;
        page["characcuracy"] = charAccuracy;
        page["wordaccuracy"] = wordAccuracy;
        page["pagename"] = pageName;

        pages.remove(pageName);
        pages.insert(pageName, page);
        mainObj.remove("pages");
        mainObj.insert("pages",pages);

        mainObj = getAverageAccuracies(mainObj);

        if(mProject.get_stage() != mRole)
            rating = mainObj["Rating-V"+ QString::number(mProject.get_version().toInt() - 1)].toInt();
        else
            rating = mainObj["Rating-V"+ mProject.get_version()].toInt();
        avgCharAcc = mainObj["AverageCharAccuracy"].toDouble();
        avgAcc = QString::number((((float)lround(avgCharAcc*100))/100)) + "%";

        writeJsonFile(commentFilename, mainObj);

        if(!gSaveTriggered)
        {
            CommentsView *cv = new CommentsView(totalWordErrors,totalCharErrors,wordAccuracy,charAccuracy,comments,commentFilename,pageName, rating, avgAcc, mRole, version);
            cv->show();
        }
    }
}

/*!
 * \fn MainWindow::on_compareCorrectorOutput_clicked
 * \brief Compares the changes made by the Corrector in OCR generated text file.
 * \sa InternDiffView(), LevenshteinWithGraphemes(), GetGraphemesCount()
 */
void MainWindow::on_compareCorrectorOutput_clicked()
{

    if(mProject.get_version().toInt())   //checks if a project is open or not
    {
    //QString qs1="", qs2="";
    QString page = gCurrentPageName;
    //QString page = currentTabPageName;

    //!checks whether users have selected a page
    if(page.isEmpty())
      {
           QMessageBox::information(0, "Error", "Please open a page ");
           return;
      }

    QString fpath = gDirTwoLevelUp;
    QString file = gDirTwoLevelUp + "/CorrectorOutput/" + page;

    //! Opens corresponding OCR text file and image
    if(!file.isEmpty())
    {
        InternDiffView *dv = new InternDiffView(this, page, fpath);   //Fetch OCR Image in DiffView2 and Set
        if (dv->validFilePath())
            dv->show();
        else
            dv->close();
      }
    }
    else{
         QMessageBox::information(0, "Error", "Please Open a Project");
    }
}

/*!
 * \fn MainWindow::on_compareVerifierOutput_clicked
 * \brief Compares Verifier's Output, Corrector's Output and OCR text.
 * This function also displays the percentage of changes made by the Corrector and Verifier, and the accuracy of the OCR text w.r.t. the verified text.
 * \sa DiffView()
 */
void MainWindow::on_compareVerifierOutput_clicked() //Verifier-Version
{

  if(mProject.get_version().toInt())
   {
    QString page =gCurrentPageName;

    //!Check whether the user has clicked a page
    if(page.isEmpty())
      {
           QMessageBox::information(0, "Error", "Please open a page ");
           return;
      }

    //! Open a Verifier's Output File
    QString fpath = gDirTwoLevelUp;
    QString file = gDirTwoLevelUp + "/VerifierOutput/" + page;

    //! Opens corresponding Corrector's Output and OCR text file
    if(!file.isEmpty())
    {
        DiffView *dv = new DiffView(this,page,fpath);
        if (dv->validFilePath())
            dv->show();
        else
            dv->close();
    }
  }
  else{
       QMessageBox::information(0, "Error", "Please Open a Project");
  }
}

//Global CPair Starts

/*!
 * \fn MainWindow::dumpStringToFile
 * \brief dumps given QString to file at file_path
 * \param file_path
 * \param string
 */
void MainWindow::dumpStringToFile(QString file_path, QString string){
    QFile file(file_path);
    if(file.open(QIODevice::WriteOnly | QIODevice::Append)){
            QTextStream outputStream(&file);
            outputStream << string << endl;
    }
    file.close();
}

/*!
 * \fn MainWindow::isStringInFile
 * \brief Checks if a QString is in file at file_path
 * \param file_path
 * \param searchString
 * \return 0 if not present and 1 if it is present
 */
bool MainWindow::isStringInFile(QString file_path, QString searchString){

    QFile fileToSearchIn(file_path);
    bool textFound = false;

    if(fileToSearchIn.open(QIODevice::ReadOnly | QIODevice::Text)){
        QTextStream in(&fileToSearchIn);
        QString line;

        //! Check in everyline if string exists
        do{
            line = in.readLine();
            if(line.contains(searchString)){
                textFound = true;
                break;
            }
        }while(!line.isNull());
    }
    fileToSearchIn.close();

    return textFound;

}

/*!
 * \fn MainWindow::addCurrentlyOpenFileToEditedFilesLog
 * \brief adds currently opened file in editor in .EditedFiles.txt to mark it as dirty
 */
void MainWindow::addCurrentlyOpenFileToEditedFilesLog(){
    QString editedFilesLogPath = gDirTwoLevelUp + "/Dicts/" + ".EditedFiles.txt";
    QString currentFilePath = gDirTwoLevelUp + "/" + gCurrentDirName+ "/" + gCurrentPageName;

    bool fileFound = isStringInFile(editedFilesLogPath, currentFilePath);

    if(fileFound)
        qDebug() << gCurrentPageName <<" already found in Edited Files Log. No need to update.";
    else
    {
        qDebug() << gCurrentPageName <<" not found in Edited Files Log."<<endl;
        qDebug()<< "Writing " <<currentFilePath << " to file." << endl;
        dumpStringToFile(editedFilesLogPath, currentFilePath);
    }
}

/*!
 * \fn MainWindow::deleteEditedFilesLog
 * \brief for now I am calling this everytime window closes
 */
void MainWindow::deleteEditedFilesLog(){
    QString editedFilesLogPath = gDirTwoLevelUp + "/Dicts/" + ".EditedFiles.txt";
    QFile file(editedFilesLogPath);
    file.remove();
}

/*!
 * \fn MainWindow::writeGlobalCPairsToFiles
 * \brief writes CPairs by iterating over all files
 * \param file_path
 * \param globalReplacementMap
 */
int MainWindow::writeGlobalCPairsToFiles(QString file_path, QMap <QString, QString> globalReplacementMap){
    QMap <QString, QString>::iterator grmIterator;
    QFile *f = new QFile(file_path);

    f->open(QIODevice::ReadOnly);

    //!Set encoding and read the file content
    QTextStream in(f);
    in.setCodec("UTF-8");
    QString s1 = in.readAll();
    f->close();
    f->open(QIODevice::WriteOnly);

    int replaced = 0, tot_replaced = 0;

    //create new text browser for html files(such that replacement works on text instead of html)
    CustomTextBrowser * browser = new CustomTextBrowser();
    browser->setReadOnly(false);

    QFont font("Chandas");
    font.setWeight(16);
    font.setPointSize(16);
//    font.setFamily("Shobhika");
    browser->setFont(font);
    browser->setHtml(s1);

    //!Replacing words by iterating the map
    for (grmIterator = globalReplacementMap.begin(); grmIterator != globalReplacementMap.end(); ++grmIterator)
    {
        QString grmItrKey = grmIterator.key();
        QString pattern = ("(\\b)")+grmItrKey.trimmed()+("(\\b)"); // \b is word boundary, for cpp compilers an extra \ is required before \b, refer to QT docs for details
        QRegExp re(pattern);
        QString replacementString = re.cap(1) + grmIterator.value() + re.cap(2); // \1 would be replace by the first paranthesis i.e. the \b  and \2 would be replaced by the second \b by QT Regex
        //   if(!mapOfReplacements.contains(grmIterator.key()))
        string str = replacementString.toStdString();
        QString::fromStdString(str).toUtf8();
        QString replacementString1 = QString::fromStdString(str).trimmed();
        mapOfReplacements[grmIterator.key()] = grmIterator.value().trimmed();

        browser->moveCursor(QTextCursor::Start);
        while(browser->find(re))
        {
            QTextCursor cursor = browser->textCursor(); //get the cursor
            QTextCharFormat fmt;
            int pos = cursor.position(); //get the cursor position
            int ancr = pos - replacementString.size() + 1; //anchor is now cursor position - length of old word to be replaced
            //qDebug()<<"pos : ancr"<<pos<<ancr;
            if (pos < ancr) {
                cursor.setPosition(pos, QTextCursor::MoveAnchor);
                cursor.setPosition(ancr, QTextCursor::KeepAnchor);
            }
            fmt = cursor.charFormat(); //get the QTextCharFormat of old word/phrase to be replaced
            browser->textCursor().insertText(replacementString1);
            cursor = browser->textCursor(); //get new cursor position after old word is replaced by new one

            pos = cursor.position();
            ancr = pos - replacementString1.size();//anchor is cursor position - new word/phrase length
            cursor.setPosition(pos, QTextCursor::MoveAnchor);
            cursor.setPosition(ancr, QTextCursor::KeepAnchor);
            //qDebug()<<"pos : ancr"<<pos<<ancr;
            cursor.mergeCharFormat(fmt); //apply the text properties captured earlier

            tot_replaced = tot_replaced + 1;
        }
//qDebug()<<"word ="<<grmIterator.key()+":"+replacementString1;
//        s1.replace(re, replacementString1);
//        replaced = s1.count(replacementString1);
//        tot_replaced = tot_replaced + replaced;
    }

    s1 = browser->toHtml();
    in << s1;
    //f->flush();
    f->close();
    browser->close();
    GlobalReplaceWorker grw;
    grw.bboxInsertion(file_path);
    return tot_replaced;
}

/*!
 * \fn MainWindow::globalReplaceQueryMessageBox
 * \brief spawns a MessageBox and returns true if Replace is chosen
 * \param old_word
 * \param new_word
 * \return bool
 *
 */
bool MainWindow::globalReplaceQueryMessageBox(QString old_word, QString new_word, int &chk){

    //!Disable global replace on special symbols
    //QRegularExpression ss("[^a-zA-Z0-9]+");
    QString special_symbols = "~`!@#$%^&*()-+={}[]|\ \"/:;'<>,.?;";
    int replaceFlag = 0;
    //qDebug ()<<"Reg exp:"<<special_symbols;
    for(int i=0;i<old_word.size();i++){
        int count =0;
        for(int k=0;k<special_symbols.size();k++){
            if(old_word[i] != special_symbols[k]){
                count +=1;
                //replaceFlag = 1;
                //qDebug ()<<"Replace should work";
                //break;
            }
            if(count == special_symbols.size()){
                replaceFlag = 1;
                break;
            }
        }
        if(replaceFlag == 1)
            break;
    }
        if (replaceFlag == 0){
            return false;}
    chk=0;

    //!Declaring messagebox and the associated buttons
    QMessageBox messageBox(this);
   // QDialog dialog(this);
    QCheckBox *cb = new QCheckBox("Make changes to all pages");
    //dialog.setWindowTitle("Check Formatting");
    //QAbstractButton *escButton = messageBox.addButton(tr("Esc"), QMessageBox::ActionRole);
    QAbstractButton *previewButton = messageBox.addButton(tr("Preview"), QMessageBox::ActionRole);
    QAbstractButton *replaceButton = messageBox.addButton(tr("Yes"), QMessageBox::ActionRole);
    QAbstractButton *cancelButton = messageBox.addButton(tr("No"), QMessageBox::RejectRole);

    messageBox.setCheckBox(cb);
    QString msg = "Do you want to replace \"" + old_word + "\" with \"" + new_word + "\" in rest of the pages?\n"
                + "\n\nClick \"Yes\" to save the changes and replace the word in the unedited pages."
                + "\nClick \"No\" to save the changes and not replace the word in the unedited page.";

    QMap <QString, QString> obj;
    obj[old_word] = new_word;
   // QVector<int> allPages;
    //qDebug()<<"cb->checkState():"<<cb->checkState();
    //!Get checkbox State
    /*if(cb->checkState() == Qt::Checked){
        qDebug()<<"Make changes in all pages?";
        chk=1;
        allPages.push_back(1);
    }
    else if(cb->checkState() == Qt::Unchecked){
        qDebug()<<"Make in all pages not clicked";
        //allPages.push_back(0);
    }*/
 //allPages.push_back(1);

    //!Disconnecting button from message box
    previewButton->disconnect();
    connect(previewButton,&QAbstractButton::clicked, this,[=](){
        QVector<int> allPages;
        //!Get checkbox State
        if(cb->checkState() == Qt::Checked){
           // chk=1;
            allPages.push_back(1);
        }
        else if(cb->checkState() == Qt::Unchecked){
            allPages.push_back(0);
        }
        globalReplacePreviewfn(obj,allPages);   //!preview for single replace
    } );

    messageBox.setWindowTitle("Global Replace");
    messageBox.setText(msg);
    messageBox.exec();

    if (messageBox.clickedButton() == replaceButton){
        if(cb->checkState() == Qt::Checked){
            chk=1;
            qDebug()<<"Checkbox Checked";
        }


    return true;
    }

    //!Writing logs
    if (messageBox.clickedButton() == cancelButton){
        QDir directory(gDirTwoLevelUp);
        QString setName=directory.dirName();
        QString filename = gDirTwoLevelUp+"/"+setName+"_logs.csv";
        QFile csvFile(filename);
        if(!csvFile.exists())
        {
            csvFile.open(QIODevice::ReadWrite | QIODevice::Append);
            QTextStream output(&csvFile);
            output.setCodec("UTF-8");
            output << "Source Word,Target Word,Type of Replacement,Time of Replacement,Page Name,Set name";
        }
        else
        {
            csvFile.open(QIODevice::ReadWrite | QIODevice::Append);
        }
        QString sourceString = old_word;
        QString replaceString = new_word;
        QString typeOfReplacement = "No Replacement";

        QDateTime current = QDateTime::currentDateTime();
        QString time = current.toString();

        QTextStream output(&csvFile);
        output.setCodec("UTF-8");
        //qDebug() << "csv Contents" << output.readAll();
        output << "\n";
        output<<sourceString<<","<<replaceString<<","<<typeOfReplacement<<","<<time<<","<<gCurrentPageName<<","<<setName;
        csvFile.close();
        //to clear the CPair entries when "no" is clicked, else they will be added to cpair later
        CPair_editDis.clear();
    }
    return false;

}



/*!
 * \fn MainWindow::getGlobalReplacementMapFromChecklistDialog
 * \brief spawns a checklist and returns a Qmap of selected pairs
 * \param changedWords
 * \returns QMap
 */
QMap <QString, QString> MainWindow::getGlobalReplacementMapFromChecklistDialog(QVector <QString> changedWords, QVector<int> *replaceInAllPages){
    QMap <QString, QString> globalReplacementMap;
    QMap<QString, QString> uncheckedItemsListMap;
    GlobalReplaceDialog grDialog(changedWords, this);
    currentGlobalReplaceDialog = &grDialog;

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect  screenGeometry = screen->geometry();
    float height = screenGeometry.height() * 0.5;
    float width = screenGeometry.width() * 0.5;

    //!Initiating and executing globalreplacedialog for getting multiple repalcement pairs
    grDialog.setModal(true);
    grDialog.setFixedSize(width, height);
    grDialog.exec();

    if(grDialog.clicked_applyButton())
    {
        *replaceInAllPages = grDialog.getStatesOfCheckboxes();
        globalReplacementMap = grDialog.getFilteredGlobalReplacementMap();
        uncheckedItemsListMap = grDialog.uncheckedItemsList();
        if(uncheckedItemsListMap.values().length()>0)
        {
            QMap <QString, QString>::iterator grmIterator;

            //!Writing logs
            QDir directory(gDirTwoLevelUp);
            QString setName=directory.dirName();
            QString filename = gDirTwoLevelUp+"/"+setName+"_logs.csv";
            QFile csvFile(filename);
            if(!csvFile.exists())     //for first time creation
            {
                csvFile.open(QIODevice::ReadWrite | QIODevice::Append);
                QTextStream output(&csvFile);
                output.setCodec("UTF-8");
                output << "Source Word,Target Word,Type of Replacement,Time of Replacement,Page Name,Set name";
            }
            else
            {
                csvFile.open(QIODevice::ReadWrite | QIODevice::Append);
            }

            //!Writing all changed words to logs
            for (grmIterator = uncheckedItemsListMap.begin(); grmIterator != uncheckedItemsListMap.end(); ++grmIterator)
            {
                QString sourceString = grmIterator.key();
                QString replaceString= grmIterator.value();
                QString typeOfReplacement = "No Replacement";

                QDateTime current = QDateTime::currentDateTime();
                QString time = current.toString();

                QTextStream output(&csvFile);
                output.setCodec("UTF-8");
                //qDebug() << "csv Contents" << output.readAll();
                output << "\n";
                output<<sourceString<<","<<replaceString<<","<<typeOfReplacement<<","<<time<<","<<gCurrentPageName<<","<<setName;
            }
            csvFile.close();
        }
    }
    else{

        uncheckedItemsListMap = grDialog.uncheckedItemsList();
        //qDebug()<<"uncheckedItemsListMap"<<uncheckedItemsListMap;
        if(uncheckedItemsListMap.values().length()>0)
        {
            QMap <QString, QString>::iterator grmIterator;

            QDir directory(gDirTwoLevelUp);
            QString setName=directory.dirName();
            QString filename = gDirTwoLevelUp+"/"+setName+"_logs.csv";
            QFile csvFile(filename);
            if(!csvFile.exists())
            {
                csvFile.open(QIODevice::ReadWrite | QIODevice::Append);
                QTextStream output(&csvFile);
                output.setCodec("UTF-8");
                output << "Source Word,Target Word,Type of Replacement,Time of Replacement,Page Name,Set name";
            }
            else
            {
                csvFile.open(QIODevice::ReadWrite | QIODevice::Append);
            }

            for (grmIterator = uncheckedItemsListMap.begin(); grmIterator != uncheckedItemsListMap.end(); ++grmIterator)
            {
                QString sourceString = grmIterator.key();
                QString replaceString= grmIterator.value();
                QString typeOfReplacement = "No Replacement";

                QDateTime current = QDateTime::currentDateTime();
                QString time = current.toString();

                QTextStream output(&csvFile);
                output.setCodec("UTF-8");
                //qDebug() << "csv Contents" << output.readAll();
                output << "\n";
                output<<sourceString<<","<<replaceString<<","<<typeOfReplacement<<","<<time<<","<<gCurrentPageName<<","<<setName;
            }
            csvFile.close();
        }
    }
    //qDebug()<<"globalReplacementMap"<<globalReplacementMap<<endl;
    return globalReplacementMap;

}

/*!
 * \fn MainWindow::closeProgressBar
 * \brief This function closes the progress bar.
 */
void MainWindow::closeProgressBar()
{
    progressBarDialog->close();
    progressBarDialog->deleteLater();
}

/*!
 * \fn MainWindow::setProgressBarPerc
 * \brief Sets the progress bar value to the passed value
 * \param value
 */
void MainWindow::setProgressBarPerc(int value)
{
    progressBarDialog->setPercentage(value);
}

/*!
 * \fn MainWindow::runGlobalReplace
 * \brief It calls the function which replaces the words and add the words into the CPair file in a multithreaded fashion
 * \details 1. Opens up dialog box for asking the user if he wants to do global-replace or not.
 *          2. Then it runs another thread for doing global replace.
 *          3. After completion of global replace, it runs another thread for adding the replaced words to CPair.
 * \param currentFileDirectory
 * \param changedWords
 */
void MainWindow::runGlobalReplace(QString currentFileDirectory , QVector <QString> changedWords)
{
    QMap <QString, QString> globalReplacementMap;

    QMap<QString, QString> replaceInAllPages_Map;
    QMap<QString, QString> replaceInUneditedPages_Map;

    /*
     * Stores values in 0s and 1s. Eg: {1, 0, 0, 1}. 1 means that the word corresponding the map should be replaced in all pages
     * and if 0 then replace in unedited pages
    */
    QVector<int> replaceInAllPages;

    QString editedFilesLogPath = gDirTwoLevelUp + "/Dicts/" + ".EditedFiles.txt";

    int noOfChangedWords = changedWords.size();
    int files = 0;
    int r1 = 0, r2 = 0;
    int x1 = 0;
    int check=2;

    QRegExp separat("\\s*=>*");
    QMap<QString, QString> replacementMap; // This map is used for removing duplicate entries from changedWords
    for (int i = 0; i < noOfChangedWords; i++) {
        QStringList mapping = changedWords[i].split(separat, QString::SkipEmptyParts);

        // Removing source words which have more than 5 words
        int numOfWordsInSource = 0;
        bool lastWasSpace = true;
        QString source = mapping[0].trimmed();
        for (int i = 0; i < source.size(); i++) {
            if (source[i] == ' ') {
                if (!lastWasSpace) {
                    numOfWordsInSource++;
                    lastWasSpace = true;
                }
            } else {
                if (source[i] != '\n')
                    lastWasSpace = false;
            }
        }
        if (!lastWasSpace)
            numOfWordsInSource++;
        if (numOfWordsInSource > 5) {
            changedWords.remove(i);
            noOfChangedWords--;
        }

        if (replacementMap.find(mapping[0]) != replacementMap.end()) {
            changedWords.remove(i);
            noOfChangedWords--;
        } else {
            replacementMap.insert(mapping[0], mapping[1]);
        }
    }

    //! if only one change spawn checkbox
    if (noOfChangedWords == 1){
        grdFlag = 1;
        QRegExp sep("\\s*=>*");
        QStringList changesList = changedWords[0].split(sep, QString::SkipEmptyParts );
        bool updateGlobalCPairs = globalReplaceQueryMessageBox(changesList[0], changesList[1],check);
        if (updateGlobalCPairs){
            globalReplacementMap[changesList[0]] = changesList[1];
            if(check==1)
                replaceInAllPages_Map.insert(changesList[0],changesList[1]);
            else
                replaceInUneditedPages_Map.insert(changesList[0],changesList[1]);
        }

    }
    //! if there is more than 1 change spawn a checklist and get the checked pairs only
    else if(noOfChangedWords > 1){
        grdFlag = 2;
        globalReplacementMap = getGlobalReplacementMapFromChecklistDialog(changedWords, &replaceInAllPages);
        QMap<QString, QString>::iterator it;
        it = globalReplacementMap.begin();
        for (int i = 0; i < globalReplacementMap.size(); i++)
        {
            if (replaceInAllPages.at(i) == 1)
                replaceInAllPages_Map.insert(it.key(), it.value());
            else
                replaceInUneditedPages_Map.insert(it.key(), it.value());
            it++;
        }
    }

    if(!globalReplacementMap.isEmpty())
    {
        globallyReplacedWords = globalReplacementMap;

        int pairMap = 1;
        if(changesCheckedInPreviewMap.size()==0) pairMap = 0;
        qDebug()<<"Pair map ="<<pairMap;

        /*START MULTITHREADING IMPLEMENTATION HERE*/
        GlobalReplaceWorker *grWorker = new GlobalReplaceWorker(
                    nullptr,
                    &filesChangedUsingGlobalReplace,
                    &mapOfReplacements,
                    globalReplacementMap,
                    changesCheckedInPreviewMap,
                    replaceInAllPages_Map,
                    replaceInUneditedPages_Map,
                    currentFileDirectory,
                    gDirTwoLevelUp,
                    gCurrentPageName,
                    noOfChangedWords,
                    check,
                    &r1,
                    &r2,
                    &x1,
                    &files,
                    pairMap
                    );

        QThread *thread = new QThread;
        connect(thread, SIGNAL(started()), grWorker, SLOT(replaceWordsInFiles()));
        connect(grWorker, SIGNAL(finishedReplacingWords()), grWorker, SLOT(writeLogs()));
        connect(grWorker, SIGNAL(finishedWritingLogs()), thread, SLOT(quit()));
        connect(grWorker, SIGNAL(finishedWritingLogs()), grWorker, SLOT(deleteLater()));
        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
        connect(grWorker, SIGNAL(finishedWritingLogs()), this, SLOT(closeProgressBar()));
        connect(grWorker, SIGNAL(changeProgressBarValue(int)), this, SLOT(setProgressBarPerc(int)));
        grWorker->moveToThread(thread);
        thread->start();

        progressBarDialog = new ProgressBarDialog(this);
        progressBarDialog->setMessage("Replacing words...");
        progressBarDialog->setModal(false);
        progressBarDialog->exec();

    }
    map<string, string> new_cpair;
        QMapIterator<QString, QString>i(globalReplacementMap);
        map<string, set<string>>::iterator itr;
        while (i.hasNext())
        {
            i.next();
            QString tmp = i.key();
            new_cpair[i.key().toStdString()]=i.value().toStdString();

        }
        Worker *worker = new Worker(nullptr,
                                    &mProject,
                                    gCurrentPageName,
                                    gCurrentDirName,
                                    gDirTwoLevelUp,
                                    s1,
                                    s2,
                                    new_cpair,
                                    &CPairs,
                                    filestructure_fw);
        QThread *thread = new QThread;

        connect(thread, SIGNAL(started()), worker, SLOT(addCpair()));
        worker->moveToThread(thread);
        thread->start();
    QString msg  = QString::fromStdString(std::to_string(globalReplacementMap.values().length()) + " words changed" + "\n" + std::to_string(r2) + " instances replaced" + "\n" + std::to_string(files) + " files modified");
    QMessageBox messageBox;
    if(globalReplacementMap.values().length()>0) //{
        messageBox.information(0, "Replacement Successful", msg);
    changesCheckedInPreviewMap.clear();
        //globalReplacementMap.clear();}

    addCurrentlyOpenFileToEditedFilesLog();
}

/*!
 * \fn MainWindow::globalReplacePreviewfn()
 * \brief This feature allows user to view the changes in advance that are going to be done by global replace
 * \details
 * This function will fetch the required data and put in a window where the users will be able to see in a
 * tabular format, the page name, the sentences where the global replace word is/are present and the same sentence
 * after making the change. This allows the user to better make decisions whether to perform global replace or not
 *
 * When the user saves the file, a dialog box for global replace appears with words to be selected and globally
 * replaced. If no word is selected and preview button is clicked then a message will be shown that no word
 * was selected.
 *
 * If words are selected then all files in the project are scanned using dirIterator and the result is fetched
 * by called getBeforeAndAfterWords() which returns the page name and the relevant sentences.
 *
 * Once the data is fetched, we loop through the data and split the returned sentence in the following fashion:
 * 1. Before the change was made
 * 2. Same sentence after change
 *
 * Then we initialize QStandardItem model and feed this data to it and also set the relevant headings to it
 *
 * \param QMap <QString, QString> previewMap , QVector<int> allPages
 * \sa getBeforeAndAfterWords()
 */
void MainWindow::globalReplacePreviewfn(QMap <QString, QString> previewMap , QVector<int> allPages)
{
  QStandardItemModel *model = new QStandardItemModel;
  int lineindex = 0;
    //qDebug()<<"previewMap:"<<previewMap;sadam
    if(previewMap.size() == 0)
  {
       QMessageBox::warning(this, "Error", "No words are selected for replacement");
  }
  else {

      QMap<QString, QString> replaceInAllPages_Map;
      QMap<QString, QString> replaceInUneditedPages_Map;
      QMap<QString, QString>::iterator it = previewMap.begin();
      for (int i = 0; i < previewMap.size(); i++)
      {
          if (allPages.at(i) == 1)
            { replaceInAllPages_Map.insert(it.key(), it.value());}
          else
            { replaceInUneditedPages_Map.insert(it.key(), it.value());}
          it++;
      }


     if(previewMap.size() >= 1)
     {
        QString editedFilesLogPath = gDirTwoLevelUp + "/Dicts/" + ".EditedFiles.txt";
        QString currentFileDirectory =gDirTwoLevelUp + "/" + gCurrentDirName;;
        QDirIterator dirIterator(currentFileDirectory, QDirIterator::Subdirectories);
        QMap<QString,QStringList> lines;
        //Unedited pages
        while (dirIterator.hasNext())
        {
            QString it_file_path = dirIterator.next();

            bool isFileInEditedFilesLog = isStringInFile(editedFilesLogPath, it_file_path);
            QString suff = dirIterator.fileInfo().completeSuffix();
            if (!isFileInEditedFilesLog)
            {
                if(suff == "html")
                {
                  lines.unite(getBeforeAndAfterWords(it_file_path, replaceInUneditedPages_Map));
                }
             }
        }

        QDirIterator dirIterator_2(currentFileDirectory, QDirIterator::Subdirectories);
        //!all pages
        while (dirIterator_2.hasNext())
        {
            QString it_file_path = dirIterator_2.next();
            QString suff = dirIterator_2.fileInfo().completeSuffix();
            if(suff == "html")
            {
               lines.unite(getBeforeAndAfterWords(it_file_path, replaceInAllPages_Map));
            }
         }

           QMap<QString,QStringList>::iterator ite;
            for(ite=lines.begin(); ite!=lines.end();++ite )
            {
                QString pages = ite.key();
                QStringList sentences = ite.value();
                for(int i=0;i<sentences.size();i++)
                {
                    QStandardItem *Item = new QStandardItem(pages);
                    Item->checkState();
                    Item->setCheckState(Qt::Unchecked);
                    Item->setCheckable(1);
                    model->setItem(lineindex, 0,Item);
                    string sent = sentences.at(i).toStdString();
                    string newSentence = sent.substr(sent.find("==>")+3);
                    string oldSentence = sent.substr(0, sent.find("==>"));
                    QStandardItem *Item1 = new QStandardItem(QString::fromStdString(oldSentence));
                    model->setItem(lineindex, 1,Item1);
                    QStandardItem *Item2 = new QStandardItem(QString::fromStdString(newSentence));
                    model->setItem(lineindex, 2,Item2);
                    lineindex++;
                }
            }

            model->setHeaderData (0,Qt::Horizontal, QObject::tr ("Page"));
            model->setHeaderData (1,Qt::Horizontal, QObject::tr ("Before Replace"));
            model->setHeaderData (2,Qt::Horizontal, QObject::tr ("After Replace"));
      }
  globalReplacePreview gp(this, model);
  gp.exec();



  int n = model->rowCount();
  int checkCount = 0;

 // cout<<"::::::::::::::::::::::::::::::::"<<endl;
  for(int i=0 ; i<n ; i++)
  {
      if(model->item(i)->checkState()==Qt::Checked)
      {

          changesCheckedInPreviewMap.insert({model->item(i,2)->text(),model->item(i,0)->text()},model->item(i,1)->text());
          checkCount++;

          //qDebug()<<model->item(i,1)->text()<<"------->"<<changesCheckedInPreviewMap[model->item(i,1)->text()].first<<"++++"<<changesCheckedInPreviewMap[model->item(i,1)->text()].second<<endl;
      }
  }

  //qDebug()<<changesCheckedInPreviewMap<<endl;
  if(grdFlag == 2){
    GlobalReplaceDialog *grd = currentGlobalReplaceDialog;
    qDebug()<<"Check count:"<<checkCount;
    if(checkCount>0){
        grd->disableCheckboxes(1,&changesCheckedInPreviewMap);
    }
    else if(checkCount == 0){
        grd->disableCheckboxes(0,&changesCheckedInPreviewMap);
    }
    }
  }
}

/*!
 * \fn MainWindow::getBeforeAndAfterWords()
 *
 * \brief This function is used by MainWindow::globalReplacePreviewfn() for fetching the context sentences where
 * the global replace is to be applied.
 * \details
 * The function first opens the file passed in parameter fPath and opens the stream.
 * Then it runs a for loop wherein we loop through the global replace word map and otain the key and value and
 * store them as oldword and newword respectively
 *
 * We run a regex which will allow us to get handful of words before and after the replaced word which will be
 * shown in the preview dialog box.
 *
 * We run the regex for occurances wherein we capture the matched pattern and replace the old word with the new word,
 * and return both the sentences along with the file name.
 *
 *
 * \param QString fPath, QMap <QString, QString> globalReplacementMap
 */

QMap<QString,QStringList> MainWindow::getBeforeAndAfterWords(QString fPath,QMap <QString, QString> globalReplacementMap)
{
    //qDebug()<<"grm:"<<globalReplacementMap;
  QStringList sentences;
  QMap <QString, QString>::iterator grmIterator;
  QFile *f = new QFile(fPath);
  QMap <QString, QStringList> previewPagesMap;

  f->open(QIODevice::ReadOnly);
  QTextStream in(f);
  in.setCodec("UTF-8");
  QString s1 = in.readAll();
  QTextDocumentFragment fragment;
  QString plain = fragment.fromHtml(s1).toPlainText();
  f->close();

  for (grmIterator = globalReplacementMap.begin(); grmIterator != globalReplacementMap.end(); ++grmIterator)
  {
      QString oldWord = grmIterator.key();
      QString newWord = grmIterator.value();

      QRegularExpression rx(".*"+oldWord+"*"); //"[^.]*"+oldWord+"[^.]*\."
      for(int i=0;i<rx.captureCount()+1;++i)
      {
         QRegularExpressionMatchIterator match = rx.globalMatch(plain);
          while(match.hasNext())
           {
              QRegularExpressionMatch Extmatch = match.next();
              QString matched = Extmatch.captured(i);
              //qDebug()<<"Matched:"<<matched;
              string no_dn[]={"०","१","२","३","४","५","६","७","८","९","॥","।","–","—"};
              QStringList list = matched.split(QString::fromStdString(no_dn[11]), QString::SkipEmptyParts);
              //qDebug()<<"list:"<<list;
              for(i=0;i<list.size();i++)
              {
                  if(list[i].contains(oldWord))
                  {
                      matched = list[i];
                      break;
                  }
              }
              QString newSentence = matched;
              //qDebug()<<oldWord<<newWord;
              oldWord = oldWord.trimmed();
              newSentence = newSentence.replace(oldWord,newWord,Qt::CaseSensitive);
              QString finalSentence = matched + "==>" + newSentence;
//              qDebug() << "Final Sentence" << finalSentence;
              if(newSentence.length() >0 )
              {
                  sentences << finalSentence;
              }
           }
       }
  }
  QFile file(fPath);
  QFileInfo fileInfo(file);
  QString fileName = fileInfo.fileName();

  previewPagesMap[fileName] = sentences;

  return previewPagesMap;

}



//Global CPair End

/*!
 * \fn MainWindow::DisplayJsonDict
 * \brief Load and display *.dict files
 */
void MainWindow::DisplayJsonDict(CustomTextBrowser *b, QString input)
{
    QVector<QString> dictionary;
    QJsonDocument doc;
    QJsonObject obj;
    QByteArray data_json;
    QStringList list1;
    QSet<QString> dict_set;
    QSet<QString> dict_set1;
    //! Get dict file from current opened file
    QString dictFilename;
    if(mRole=="Verifier")
    {
        dictFilename = gDirTwoLevelUp + "/" + "VerifierOutput" + "/" + gCurrentPageName;
    }
    else if(mRole=="Corrector")
    {
        dictFilename = gDirTwoLevelUp + "/" + "CorrectorOutput" + "/" + gCurrentPageName;
    }
    dictFilename.replace(".txt", ".dict");
    dictFilename.replace(".html", ".dict");
    QFile dictQFile(dictFilename);

    ui->textEdit_dict->clear();
    ui->textEdit_dict->setFontPointSize(14);
    //! Open the dict file and display it in textedit view
    if(QFile::exists(dictFilename))
    {
            QFile dictQFile(dictFilename);
            if(dictQFile.open(QIODevice::ReadOnly | QIODevice::Text))
            {
               data_json = dictQFile.readAll();
               dictQFile.close();
               doc = doc.fromJson(data_json);
               obj = doc.object();
               if( obj.size() == 0){
                   QMessageBox::information(0, "Error !", "Dictionary of current page can't be loaded, please correct the syntax of corresponding Json file.");
                   return;
               }
               QJsonValue jv = obj.value(obj.keys().at(0));
               QJsonObject item = jv.toObject();
               for(int i = 0; i < item.count(); i++)
               {
                  ui->textEdit_dict->append(item.keys().at(i)+":");
                  QJsonValue subobj = item.value(item.keys().at(i));
                  QJsonArray test = subobj.toArray();
                  for(int k = 0; k < test.count(); k++)
                  {
                     if(test[k].toString()!=NULL){
                         QString jsonDi;

                         for(int i=0;i<test[k].toString().length();i++){
                             QString newStr=test[k].toString();
                             list1 = newStr.split(",");
                         }

                     }

                     ui->textEdit_dict->moveCursor(QTextCursor::End);
                     ui->textEdit_dict->insertPlainText(" "+test[k].toString());

                     if(k<test.count()-1)
                     {
                        ui->textEdit_dict->insertPlainText(",");
                     }
                     ui->textEdit_dict->moveCursor(QTextCursor::End);
                   }
                  foreach(auto &x,list1){
                      dict_set.insert(x);
                  }

               }

               foreach(auto &x,dict_set){
                   std::string string1= x.toStdString();
                   std::string string2;
                   string2=string1.substr(0, string1.find("(", 0));
                   QString qstr = QString::fromStdString(string2);
                   dict_set1.insert(qstr);
               }

          }
    }

    QTextCharFormat fmt;
    fmt.setBackground(Qt::green);
    QTextCursor cursor(b->document());
    int indexOfReplacedWord;
    int from=0;
    int count;
    int numReplaced=0;
    foreach(auto &x, dict_set1)
    {
        count = input.count(x, Qt::CaseInsensitive);

        numReplaced=0;
        from=0;
        int flag=0;

        while(numReplaced<count)
        {
//            if(x.size()<count){
//                break;
//            }
            int endIndex;
            indexOfReplacedWord = input.indexOf(x,from , Qt::CaseInsensitive);
            endIndex = indexOfReplacedWord;
            int len = x.length();

            while(len > 0)
            {
                endIndex++;
                len--;
            }

            int start = indexOfReplacedWord;

            if(indexOfReplacedWord == 0){
                input[start] == " ";
            }
            else{
                start=start-1;
            }

            QRegExp regex("[$&+,:;=?@#|'\"<>.^*()%!-\n\t]");

            QString test1=input.at(start);
            QString test2=input.at(endIndex);
            if((input[endIndex] == " " || test2.contains(regex)) && (input[start] == " " || test1.contains(regex))){
                flag=1;
            }

            if(flag==1)
            {
                cursor.setPosition(indexOfReplacedWord, QTextCursor::MoveAnchor);
                cursor.setPosition(endIndex, QTextCursor::KeepAnchor);
                cursor.mergeCharFormat(fmt);
                QTextEdit::ExtraSelection h;
                h.format.setBackground(Qt::green);
            }
            from = endIndex;
            numReplaced+=1;
            flag=0;
        }
    }

}

/*!
 * \fn MainWindow::getAverageAccuracies
 * \brief Calculate average char, word accuracy of .html files w.r.t to the .txt files
 * \param mainObj
 * \return QJsonObject
 */
QJsonObject MainWindow::getAverageAccuracies(QJsonObject mainObj)
{
    float totalcharacc=0, totalwordacc = 0; int totalcharerrors = 0, totalworderrors = 0, count = 0, rating = 0;

    //!Navigate to the Json object named "pages" and extract the parent object within them
    QJsonObject pages = mainObj.value("pages").toObject();
    QJsonObject page;

    //!for every parent object or pages in json file
    foreach(const QJsonValue &val, pages)
    {
        //!Extract the values from Json file
        QString page = val.toObject().value("pagename").toString();
        float charAccuracy    = val.toObject().value("characcuracy").toDouble();
        float wordAccuracy    = val.toObject().value("wordaccuracy").toDouble();
        int charerrors = val.toObject().value("charerrors").toInt();
        int worderrors = val.toObject().value("worderrors").toInt();

        //!Store the fetched values from Json locally for calculating average and error values
        totalcharacc    += charAccuracy;
        totalwordacc    += wordAccuracy;
        totalcharerrors += charerrors;
        totalworderrors += worderrors;

        count++;
    }
    if(count)
    {
        //!Rate the average character accuracy and update it to csv file as 4,3,2 and 1
        double avgcharacc = totalcharacc/count;
        if(avgcharacc>98.5) rating = 4;
        else if(avgcharacc > 97.5) rating = 3;
        else if(avgcharacc > 96.5) rating = 2;
        else if(avgcharacc <= 96.5) rating = 1;

        if(mProject.get_stage() != mRole)
            mainObj["Rating-V"+ QString::number(mProject.get_version().toInt() - 1)] = rating;    //Decreases version and updates rating if stage in xml file and currect user is same
        else
            mainObj["Rating-V"+ mProject.get_version()] = rating;     //Update the rating in csv if stage in project.xml is different from current user Eg: "Rating-V1": 1

        //!Calculate and update the value of average accuracy and error to Json on Character and Word level
        mainObj["AverageCharAccuracy"] = avgcharacc;
        mainObj["AverageWordAccuracy"] = totalwordacc/count;
        mainObj["AverageCharErrors"] = totalcharerrors/count;
        mainObj["AverageWordErrors"] = totalworderrors/count;

    }
    return mainObj;
}

/*!
 * \fn MainWindow::updateAverageAccuracies
 * \brief The function updates accuracy and error on word and charater level to
 * the files named comments.json and AverageAccuracies.csv
 *
 * \sa readJsonFile(), writeJsonFile()
 */
void MainWindow::updateAverageAccuracies() //Verifier only
{
    //! Get the file path of comments.json and AverageAccuracies.csv
    QString commentFilename = gDirTwoLevelUp + "/Comments/comments.json";

    string csvfolder = gDirTwoLevelUp.toUtf8().constData();
    csvfolder += "/Comments/AverageAccuracies.csv";

    //!Write the column name to AverageAccuracies.csv
    std::ofstream csvFile(csvfolder);
    csvFile<<"Page Name,"<< "Word-Level Accuracy,"<<"Character-Level Accuracy," <<"Word-Level Errors,"<<"Character-Level Errors"<<"\n";

    //!Read the Json Objects and terminates functions if the file is empty
    QJsonObject mainObj = readJsonFile(commentFilename);
    if(mainObj.isEmpty())
        return;
    float totalcharacc=0, totalwordacc = 0; int totalcharerrors = 0, totalworderrors = 0, count = 0, rating = 0;

    //!Navigate to the Json object named "pages" and extract the parent object within them
    QJsonObject pages = mainObj.value("pages").toObject();
    QJsonObject page;

    //!for every parent object or pages in json file
    foreach(const QJsonValue &val, pages)
    {
        //!Extract the values from Json file
        QString page = val.toObject().value("pagename").toString();
        float charAccuracy = val.toObject().value("characcuracy").toDouble();
        float wordAccuracy = val.toObject().value("wordaccuracy").toDouble();
        int charerrors = val.toObject().value("charerrors").toInt();
        int worderrors = val.toObject().value("worderrors").toInt();

        //!Write those fetched values from Json to csv
        csvFile << page.toUtf8().constData() <<"," << wordAccuracy << "," << charAccuracy << "," << worderrors<< "," << charerrors<<"\n";

        //!Store the fetched values from Json locally for calculating average and error values
        totalcharacc    += charAccuracy;
        totalwordacc    += wordAccuracy;
        totalcharerrors += charerrors;
        totalworderrors += worderrors;

        count++;
    }
    if(count)
    {
        //!Rate the average character accuracy and update it to csv file as 4,3,2 and 1
        double avgcharacc = totalcharacc/count;
        if(avgcharacc>98.5) rating = 4;
        else if(avgcharacc > 97.5) rating = 3;
        else if(avgcharacc > 96.5) rating = 2;
        else if(avgcharacc <= 96.5) rating = 1;

        if(mProject.get_stage() != mRole)
            mainObj["Rating-V"+ QString::number(mProject.get_version().toInt() - 1)] = rating;   //Decreases version and updates rating if stage in xml file and currect user is same
        else
            mainObj["Rating-V"+ mProject.get_version()] = rating;    //Update the rating in csv if stage in project.xml is different from current user Eg: "Rating-V1": 1

        //!Calculate and update the value of average accuracy and error to Json on Character and Word level
        mainObj["AverageCharAccuracy"] = avgcharacc;
        mainObj["AverageWordAccuracy"] = totalwordacc/count;
        mainObj["AverageCharErrors"] = totalcharerrors/count;
        mainObj["AverageWordErrors"] = totalworderrors/count;

        //!Calculate and update the value of accuracy and error to csv on Character and Word level
        csvFile<<" ,"<< "Average Accuracy (Word level),"<<"Average Accuracy (Character-Level)," <<"Average Errors (Word level),"<<"Average Errors (Character-Level),"<<"\n";
        csvFile <<" " <<"," << totalwordacc/count << "," << totalcharacc/count << "," << totalworderrors/count<< "," << totalcharerrors/count<<"\n";
        writeJsonFile(commentFilename, mainObj);

    }
}

//end

/*!
 * \brief MainWindow::on_actionLineSpace_triggered
 */
void MainWindow::on_actionLineSpace_triggered() //Not used, does not work as intended
{
    if(!curr_browser)
        return;
    QTextCursor cursor = curr_browser->textCursor();
    QTextBlockFormat format = cursor.blockFormat();
    double lineHeight = format.lineHeight()/100;
    bool False = false;
    bool *ok = &False;
    if(lineHeight == 0)
        lineHeight = 1;
    double inputLineSpace = QInputDialog::getDouble(this, "Custom Line Space", "Line Space", lineHeight, 0, 10, 2,ok);
    if(*ok) {
        // LineHeight(x,1) sets x as a percentage with base as 100
        //200 is Double LineSpace and 50 is half LineSpace
        format.setLineHeight(inputLineSpace*100, 1);
        cursor.setBlockFormat(format);
    }
}

/*!
 * \fn MainWindow::on_actionAdd_Image_triggered
 * \brief This function is called whenever user wishes to add an image in the document in our tool.
 */

void MainWindow::on_actionAdd_Image_triggered()
{
    //! Ask user to select image from file dialog.
    if(curr_browser) {
        QString file = QFileDialog::getOpenFileName(this, tr("Select an image"),
                                                    "./", tr("Bitmap Files (*.bmp)\n"
                                                             "JPEG (*.jpg *jpeg)\n"
                                                             "GIF (*.gif)\n"
                                                             "PNG (*.png)\n"));


        //! If file is not empty then a copy is created in a temporary location in our project set.
        if(!file.isEmpty()){
            QFileInfo fileInfo(file);
            QString fileName = fileInfo.fileName();
            QString destinationFileName =  mProject.GetDir().absolutePath() + "/Images/Inserted/" + fileName;
            QString copiedFileName;
            if(QFileInfo::exists(destinationFileName)) {
                QString temp = destinationFileName;
                int i =0;
                while(QFileInfo::exists(temp)) {
                    temp = destinationFileName ;
                    temp.insert(destinationFileName.lastIndexOf("."),  ("(" + QString::number(++i) + ")"));
                }
                destinationFileName = temp;
                QFileInfo finfo(destinationFileName);
            }
            QFile::copy(file, destinationFileName);
            copiedFileName = QDir::current().relativeFilePath(destinationFileName);

            //QUrl Uri ( QString ( "file://%1" ).arg ( file ) );

            //! After copy is done the image is now added to the current browser to the current cursor
            //! position.

            QImage image = QImageReader ( copiedFileName ).read();
            QTextDocument * textDocument = curr_browser->document();
            textDocument->addResource( QTextDocument::ImageResource, copiedFileName, QVariant ( image ) );
            QTextCursor cursor = curr_browser->textCursor();
            QTextImageFormat imageFormat;
            imageFormat.setWidth( image.width() );
            imageFormat.setHeight( image.height() );
            imageFormat.setName( copiedFileName );
            cursor.insertImage(imageFormat);
        }
    }
}

/*!
 * \fn MainWindow::on_actionResize_Image_triggered
 * \brief This function resizes the images in the document. User has to input the new height or width
 * and thus the image is appropriately resized.
 */
void MainWindow::on_actionResize_Image_triggered()
{
    //! gets the block position of the image in the current browser
    QTextBlock currentBlock = curr_browser->textCursor().block();
    QTextBlock::iterator it;

    //! The for loop scans through the block in the text browser and _captures_ the image
    //! It gets the new size from resizeImage view class where user has inputted the new height & width
    //! Then it sets this new height and width by setting setWidth setHeight and also sets formatting
    //! using helper object

    for (it = currentBlock.begin(); !(it.atEnd()); ++it) {
        QTextFragment fragment = it.fragment();
        if (fragment.isValid()) {
            if(fragment.charFormat().isImageFormat ()) {
                QTextImageFormat newImageFormat = fragment.charFormat().toImageFormat();
                QPair<double, double> size = ResizeImageView::getNewSize(this, newImageFormat.width(), newImageFormat.height());
                newImageFormat.setWidth(size.first);
                newImageFormat.setHeight(size.second);
                if (newImageFormat.isValid()) {
                    //QMessageBox::about(this, "Fragment", fragment.text());
                    //newImageFormat.setName(":/icons/text_bold.png");
                    QTextCursor helper = curr_browser->textCursor();
                    helper.setPosition(fragment.position());
                    helper.setPosition(fragment.position() + fragment.length(), QTextCursor::KeepAnchor);
                    helper.setCharFormat(newImageFormat);
                }
            }
        }
    }
}

/*!
 * \brief MainWindow::on_actionPush_triggered
 */
void MainWindow::on_actionPush_triggered()
{
    mProject.push(branchName);
}

/*!
 * \fn GetFilter
 * \brief Return the filter with name and list provided to the function
 * \param Name
 * \param list
 * \return Filter
 */
QString GetFilter(QString & Name, const QStringList &list) {

    QString Filter = Name;
    Filter += " ( ";
    for (auto ext : list) {
        //int loc = ext.lastIndexOf(".");
        QString s = "*";
        if (ext.size() > 1) {
            if (ext[0] != '.') {
                s += ".";
            }
            s += ext;
            Filter += s + " ";
        }
    }
    Filter += ")";
    return Filter;
}

/*!
 * \fn    MainWindow::LoadDocument
 * \brief This function is called whenever user wants to load a document of the project.
 *        This function  checks for the current directory of the document and finds the filepath
 *        then opens that document into the curr browser. Also before opening the document it
 *        checks the type of file and saves its path into the log and removes its extension and saves it for later use
 *        then updates the word count UI to display the count of the word.
 *
 * \param f
 * \param ext
 * \param name
 *
 * \sa    setMFilename(), UpdateFileBrekadown(), DisplayJsonDict(), highlight(), LoadImageFromFile(), WordCount(), readSettings(), tabchanged()
 */
void MainWindow::LoadDocument(QFile * f, QString ext, QString name)
{
    if(curr_browser) {
        if(gInitialTextHtml[currentTabPageName].compare(curr_browser->toHtml())) {   //fetching the text from the key(tab name) and comparing it to current browser text
            QMessageBox currBox2;
            currBox2.setWindowTitle("Save?");
            currBox2.setIcon(QMessageBox::Question);
            currBox2.setInformativeText("Do you want to save " + currentTabPageName + " file?");
            QPushButton *okButton2 = currBox2.addButton(QMessageBox::StandardButton::Ok);
            QPushButton *noButton2 = currBox2.addButton(QMessageBox::StandardButton::No);
            currBox2.exec();

            if (currBox2.clickedButton() == okButton2){
                on_actionSave_triggered();
            }
        }
    }

    f->open(QIODevice::ReadOnly);
    QFileInfo finfo(f->fileName());

    if(!(finfo.exists() && finfo.isFile())){
        return;
    }

    //!Retreives current folder details
    current_folder = finfo.dir().dirName();
    QString fileName = finfo.fileName();
    setMFilename(mFilename = f->fileName());
    UpdateFileBrekadown();
    CustomTextBrowser * b = new CustomTextBrowser();
    b->setReadOnly(false);
    b->setStyleSheet("background-color:white; color:black;");

    if (!isVerifier && current_folder == "Inds") {     //checks if role is not verifier
        QString output_file = mProject.GetDir().absolutePath() + "/" + filestructure_fw[current_folder] + "/" + fileName;
        output_file.replace(".txt", ".html");
        if (QFile::exists(output_file)) {
            b->setReadOnly(true);
        }
    }
    if (isVerifier && (current_folder == "Inds" || current_folder == "CorrectorOutput")) {
        QString output_file = mProject.GetDir().absolutePath() + "/" + filestructure_fw[current_folder] + "/" + fileName;
        output_file.replace(".txt", ".html");
        if (QFile::exists(output_file)) {
            b->setReadOnly(true);
        }
    }
    // Saves the current opened page

    QSettings settings("IIT-B", "OpenOCRCorrect");
    settings.beginGroup("RecentPageLoaded");
    QString tmp1 = settings.value("projectName1").toString();
    QString tmp2 = settings.value("projectName2").toString();
    QString tmp3 = settings.value("projectName3").toString();

    if(ProjFile == tmp1){
        settings.setValue("projectName1",ProjFile );
        settings.setValue("name1",name );
        settings.setValue("pageParent1",gCurrentDirName );}
    else if(ProjFile == tmp2){
        settings.setValue("projectName2",settings.value("projectName1").toString() );
        settings.setValue("name2",settings.value("name1").toString() );
        settings.setValue("pageParent2",settings.value("pageParent1").toString() );
        settings.setValue("projectName1",ProjFile );
        settings.setValue("name1",name );
        settings.setValue("pageParent1",gCurrentDirName );}
    else{
        settings.setValue("projectName3",settings.value("projectName2").toString() );
        settings.setValue("name3",settings.value("name2").toString() );
        settings.setValue("pageParent3",settings.value("pageParent2").toString() );
        settings.setValue("projectName2",settings.value("projectName1").toString() );
        settings.setValue("name2",settings.value("name1").toString() );
        settings.setValue("pageParent2",settings.value("pageParent1").toString() );
        settings.setValue("projectName1",ProjFile );
        settings.setValue("name1",name );
        settings.setValue("pageParent1",gCurrentDirName );
    }
    settings.endGroup();

    isProjectOpen = 1;

    doc = b->document();

    //!Display format by setting font size and styles
    QTextStream stream(f);
    stream.setCodec("UTF-8");
    QString input = stream.readAll();
    QFont font("Chandas");
    setWindowTitle(name);
    font.setPointSize(16);
    if(ext == "txt") {
        istringstream iss(input.toUtf8().constData());
        string strHtml = "<html><body><p>";
        string line;
        while (getline(iss, line)) {
            QString qline = QString::fromStdString(line);
            if((line == "\n") || (line == "") || (qline.contains("\r")) )
                strHtml+=line + "</p><p>";    //for html view
            else strHtml += line + "<br />";
        }
        strHtml += "</p></body></html>";
        QString qstrHtml = QString::fromStdString(strHtml);
        qstrHtml.replace("<br /></p>", "</p>");

        QFont font("Chandas");
        font.setWeight(16);
        font.setPointSize(16);
//        font.setFamily("Shobhika");
        b->setFont(font);
        b->setHtml(qstrHtml);
    }
    if (ext == "html") {
        QSize graphicsViewSize = ui->graphicsView->size();
        int graphicsViewHeight = graphicsViewSize.height()/4;
        int graphicsViewWidth = graphicsViewSize.width()/3;
        QRegularExpression rex("(<img[^>]*>)",QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatchIterator itr;
        itr = rex.globalMatch(input);
        int height=graphicsViewHeight;
        int width=graphicsViewWidth;

        while(itr.hasNext())
        {
            QRegularExpressionMatch match = itr.next();
            QString ex = match.captured(1);

            if(!ex.contains("width") && !ex.contains("height")){
                string str = ex.toStdString();
                int ind = str.find("src=");
                ind += 5;
                int start = ind;

                int end = 0;
                if (str.find(".jpg") != -1) {
                    end = str.find(".jpg");
                    end += 3;
                } else if (str.find(".png") != -1) {
                    end = str.find(".png");
                    end += 3;
                } else if (str.find(".jpeg") != -1) {
                    end = str.find(".jpeg");
                    end += 4;
                } else {
                    qDebug() << "File extension not recognisable";
                }

                string ttstr = str.substr(end+2,str.length()-end-3);// title tag string
                str = str.substr(start,end-start+1);
                QString imgname = QString::fromStdString(str);
                QString titleString = QString::fromStdString(ttstr);
                QString html = QString("\n <img src='%1' width='%2' height='%3'%4>").arg(imgname).arg(width).arg(height).arg(titleString);
                input.replace(ex,html);
            }
        }
//		b->setHtml(input);

        f->close();

        if (!f->open(QIODevice::WriteOnly | QIODevice::Text)) {
            qDebug() << "Cannot open file in write mode";
        }
        QTextStream out(f);
        out.setCodec("utf-8");
        out << input;
        out.flush();
        f->close();

        if (handleBbox != nullptr) {
            delete handleBbox;
        }
        handleBbox = new HandleBbox();
        QTextDocument *curDoc = handleBbox->loadFileInDoc(f);
        if (curDoc == nullptr) {
            qDebug() << "Cannot load file";
            return;
        }
        curDoc = curDoc->clone(static_cast<QObject*>(b));
        b->setDocument(curDoc);
        doc = b->document();
//		loadHtmlInDoc(f);
//        preprocessing(); //for removing dangling mathras
        connect(b->document(), SIGNAL(blockCountChanged(int)), this, SLOT(blockCountChanged(int)));
        blockCount = b->document()->blockCount();
        if (!f->open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "Cannot open file for reading";
            return;
        }

    }
    QDir::setCurrent(gDirOneLevelUp);   //changing application path to load document in a relative path
    b->setFont(font);
    input = b->toPlainText();


    DisplayJsonDict(b,input);
    //highlight(b , input);

    b->setMouseTracking(true);
    b->setLineWrapColumnOrWidth(QTextEdit::NoWrap);
    b->setUndoRedoEnabled(true);

    curr_browser = (CustomTextBrowser*)ui->splitter->widget(1);
    curr_browser->setDocument(b->document()->clone(curr_browser));

    QFileInfo info(*f);
    currentTabPageName = info.fileName();

    gInitialTextHtml[currentTabPageName] = b->toHtml();

    f->close();

    QString imageFilePath = mProject.GetDir().absolutePath()+"/Images/" + gCurrentPageName;

    QString temp = imageFilePath;
    int flag=0;

    //!removing extention from the document name
    temp.replace(".txt", ".jpeg");
    if (QFile::exists(temp) && flag==0)
    {
        imageFilePath=temp;

        QFile *pImageFile = new QFile(imageFilePath);
        flag=1;
        LoadImageFromFile(pImageFile);
    }
    else
    {
        temp=imageFilePath;
    }

    temp.replace(".html", ".jpeg");
    if (QFile::exists(temp) && flag==0)
    {
        imageFilePath=temp;
        QFile *pImageFile = new QFile(imageFilePath);
        flag=1;
        LoadImageFromFile(pImageFile);
    }
    else
    {
        temp = imageFilePath;
    }
    temp.replace(".html", ".png");
    if (QFile::exists(temp) && flag==0)
    {
        imageFilePath=temp;
        QFile *pImageFile = new QFile(imageFilePath);
        flag=1;
        LoadImageFromFile(pImageFile);
    }
    else
    {
        temp = imageFilePath;
    }
    temp.replace(".html", ".jpg");
    if (QFile::exists(temp) && flag==0)
    {
        imageFilePath=temp;
        QFile *pImageFile = new QFile(imageFilePath);
        flag=1;
        LoadImageFromFile(pImageFile);
    }
    else
    {
        temp = imageFilePath;
    }
    NextPrevTrig =0;

    //! Enabling Selection in treeView
    ui->treeView->selectionModel()->clearSelection();
    QModelIndex currentTreeItemIndex = ui->treeView->selectionModel()->currentIndex();
    QModelIndex parentIndex = currentTreeItemIndex.parent();
    auto model = ui->treeView->model();
    int rowCount = ui->treeView->model()->rowCount(parentIndex);

    QString treeItemLabel;
    for (int i = 0; i < rowCount; i++)
    {
        QModelIndex index = model->index(i, 0, parentIndex);
        treeItemLabel = index.data(Qt::DisplayRole).toString();

        if (index.isValid())
        {
            if (treeItemLabel == currentTabPageName)
            {
                ui->treeView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::Select);
                break;
            }
        }
    }

    // Deleting temporarily created CustomTextBrowser
    delete b;
    myTimer.start();
    WordCount();     //for counting no of words in the document
    readSettings();
}

/*!
 * \fn    MainWindow::LoadImageFromFile
 * \brief This functions loads the image file in the image tab and provides all the functionality
 *        like zoom , cut and resize on that image file.
 *
 * \param f
 */
void MainWindow::LoadImageFromFile(QFile * f)
{


    QString localFileName = f->fileName();
    loadimage = true;
    ui->horizontalSlider->setEnabled(true);
    ui->horizontalSlider->setValue(100);
    ui->zoom_level_value->setText("100%");

    imageOrig.load(localFileName);
    if (graphic)delete graphic;
    graphic = new QGraphicsScene(this);
    graphic->addPixmap(QPixmap::fromImage(imageOrig));
    ui->graphicsView->setScene(graphic);
    ui->graphicsView->fitInView(graphic->itemsBoundingRect(), Qt::KeepAspectRatio);
    if (z)delete z;
    z = new Graphics_view_zoom(ui->graphicsView, graphic,800);
    z->set_modifiers(Qt::NoModifier);
    z->zoom_level = 100;                            //default zoom level
    connect(z, SIGNAL(zoomed()), this, SLOT(zoomedUsingScroll()));

    item1 =new QGraphicsRectItem(0, 0, 1, 1);
    graphic->addItem(item1);

    //! while loading an image; create crop_rect and add it to graphic; so we can track & capture mouse press and mouse release event
    crop_rect = new QGraphicsRectItem(0, 0, 1, 1);
    graphic->addItem(crop_rect);
    ui->graphicsView->setMouseTracking(true);
    ui->graphicsView->viewport()->installEventFilter(this);

    // Zooming upto currentZoomLevel
    if (currentZoomLevel == z->zoom_level) {
        return;
    }
    double factor_to_be_applied = 1.0;
    if (currentZoomLevel < z->zoom_level) {
        factor_to_be_applied = 1 - ((z->zoom_level - currentZoomLevel) / 100.0);
    } else {
        factor_to_be_applied = 1 + ((currentZoomLevel - z->zoom_level) / 100.0);
    }
    z->gentle_zoom(factor_to_be_applied);
}

/*!
 * \fn    MainWindow::file_click
 * \brief This function checks for the type of file user clicked in the tree view then loads that file into
 *        the text browser if its not image else loads into image tab.
 * \param indx
 * \sa    LoadDocument(), LoadImageFromFile()
 */
void MainWindow::file_click(const QModelIndex & indx)
{
    auto item = (TreeItem*)indx.internalPointer();
    auto qvar = item->data(0).toString();
    if(qvar == "Document" || qvar == "Image" || qvar=="CorrectorOutput" || qvar=="VerifierOutput")
        return;
    auto file = item->GetFile();
    QString fileName = file->fileName();          //gets filename
    NodeType type = item->GetNodeType();
    switch (type) {
    case NodeType::_FILETYPE:
    {
        QFileInfo f(*file);
        QString suff = f.completeSuffix();
        if (suff == "txt" || suff == "html") {
            LoadDocument(file,suff,qvar);     //loads not image files
        }

        if (suff == "jpeg" || suff == "jpg" || suff == "png")
        {
            LoadImageFromFile(file);          //loads image files
        }
        break;
    }
    default:
        break;
    }

}

/*!
 * \fn    MainWindow::OpenDirectory
 * \brief This function is used to a folder into the tree at the current index. Also it adds
 *        that folder to the project and imports all of its files  and at it into the tree as
 *        well as project.
 */
void MainWindow::OpenDirectory()
{
    auto item = (TreeItem*)curr_idx.internalPointer();
    auto filtr = item->GetFilter();
    auto name = filtr->name();
    auto list = filtr->extensions();

    std::string extn = GetFilter(name, list).toStdString();
    QList<QString> files = QFileDialog::getOpenFileNames(this, "Open File", "./", tr(extn.c_str()));
    for (QString file : files) {
        QFile f(file);
        mProject.addFile(*filtr, f);
    }
}

/*!
 * \fn    MainWindow::RemoveFile
 * \brief This function provides functionality to remove a file from the project as well as
 *        from the tree at that index.
 */
void MainWindow::RemoveFile()
{
    //std::cout << "Test";
    auto item = (TreeItem*)curr_idx.internalPointer();
    Filter * filtr = item->GetFilter();
    QFile * file = item->GetFile();
    if (file->exists())
    {
        mProject.removeFile(curr_idx, *filtr, *file);
        ui->treeView->reset();
    }
}

/*!
 * \fn MainWindow::AddNewFile
 * \brief This function is used to add a new file into the tree to do this this function
 *        first checks current index of tree at which user wants new file then it adds the file
 *        at that point in tree as well as it updates project.
 */
void MainWindow::AddNewFile()
{
    auto item = (TreeItem*)curr_idx.internalPointer();
    Filter * filtr = item->GetFilter();
    QString name = filtr->name();
    QStringList list = filtr->extensions();
    QString filter = GetFilter(name, list);
    std::string str = filter.toStdString();
    QFile fileo(QFileDialog::getOpenFileName(this, "Open File", "./", tr(str.c_str())));
    if (fileo.exists())
    {
        mProject.addFile(*filtr, fileo);
    }
}

/*!
 * \fn    MainWindow::CustomContextMenuTriggered
 * \brief This function is called when user wants to change Context menu. This function maps the
 *        tree view of then it regenrates it according to filetype or filter.
 * \param p
 */
void MainWindow::CustomContextMenuTriggered(const QPoint & p)
{
    curr_idx = ui->treeView->indexAt(p);

    if (curr_idx.isValid())
    {
        auto item = (TreeItem*)curr_idx.internalPointer();
        switch (item->GetNodeType())
        {
        case _FILETYPE:                //for filetype view
        {
            QMenu * m = new QMenu(this);
            QAction * act = new QAction("Remove File", this);
            connect(act, &QAction::triggered, this, &MainWindow::RemoveFile);
            m->addAction(act);
            m->move(ui->treeView->mapToGlobal(p));
            m->show();
        }
            break;
        case FILTER:
        {
            QMenu * m = new QMenu(this);

            QAction * act = new QAction("Add New File", this);
            connect(act, &QAction::triggered, this, &MainWindow::AddNewFile);
            m->addAction(act);
            QAction * act2 = new QAction("Add Files", this);
            connect(act2, &QAction::triggered, this, &MainWindow::OpenDirectory);
            m->addAction(act2);
            m->move(ui->treeView->mapToGlobal(p));
            m->show();
            break;
        }
        }
    }
}

/*!
 * \fn    MainWindow::closetab
 * \brief This function is called when tab is closed or when the page is closed. This function
 *        checks if page is in readonly mode and if condition fails then it popups a messagebox
 *        asking to save any unsaved works left.
 * \param idx
 *
 * \sa    on_actionSave_triggered()
 */
void MainWindow::closetab(int idx)
{

//    CustomTextBrowser *closing_browser = (CustomTextBrowser*)ui->tabWidget_2->widget(idx);
//    QString closing_browserHtml = closing_browser->toHtml();
//    QString qstr = ui->tabWidget_2->tabText(idx);

//    string str = qstr.toStdString();
//    str.erase(remove(str.begin(), str.end(), ' '), str.end());
//   QString closingTabPageName = QString::fromStdString(str);

//    if(!closing_browser->isReadOnly() && (closing_browserHtml != gInitialTextHtml[closingTabPageName]))
//    {

//        QMessageBox saveBox2;
//        saveBox2.setWindowTitle("Save ?");
//        saveBox2.setIcon(QMessageBox::Question);
//        saveBox2.setInformativeText("Do you want to save " + closingTabPageName + " file?");
//        QPushButton *OKButton = saveBox2.addButton(QMessageBox::StandardButton::Ok);
//        QPushButton *NOButton = saveBox2.addButton(QMessageBox::StandardButton::No);
//        saveBox2.exec();



//        if (saveBox2.clickedButton() == OKButton)
//            on_actionSave_triggered();
//    }
//    delete ui->tabWidget_2->widget(idx);
//    //deleteEditedFilesLog();
}

/*!
 * \fn    MainWindow::tabchanged
 * \brief When new page is opened without closing the current page or new tab is opened without closing
 *        current tab or tabs/pages gets switched then this function is called.
 * \param idx
 *
 * \sa    setMFilename(), UpdateFileBrekadown(), LoadImageFromFile(), DisplayTimeLog()
 */
void MainWindow::tabchanged(int idx)
{
//    currentTabIndex = idx;
//    curr_browser = (CustomTextBrowser*)ui->tabWidget_2->widget(currentTabIndex);
//    QString qstr = ui->tabWidget_2->tabText(currentTabIndex);
//    string str = qstr.toStdString();
//    str.erase(remove(str.begin(), str.end(), ' '), str.end());
//    currentTabPageName=QString::fromStdString(str);

//    if(mRole=="Corrector" | mRole=="Verifier"){
//        setMFilename(mProject.GetDir().absolutePath() + "/" + gCurrentDirName + "/" + currentTabPageName);
//    }
//    else{
//        setMFilename(mProject.GetDir().absolutePath() + "/Inds/" + currentTabPageName);
//          }
//    UpdateFileBrekadown();


//    QString imagePathFile = mFilename;
//    imagePathFile.replace("CorrectorOutput", "Images");
//    imagePathFile.replace("VerifierOutput", "Images");
//    imagePathFile.replace("Inds", "Images");
//    QString temp = imagePathFile;
//    int flag=0;
//    temp.replace(".txt", ".jpeg");
//    if (QFile::exists(temp) && flag==0)
//    {
//        imagePathFile=temp;
//        QFile *pImageFile = new QFile(imagePathFile);
//        flag=1;
//        LoadImageFromFile(pImageFile);
//    }
//    else
//    {
//        temp=imagePathFile;
//    }
//    temp.replace(".html", ".jpeg");
//    if (QFile::exists(temp) && flag==0)
//    {
//        imagePathFile=temp;
//        QFile *pImageFile = new QFile(imagePathFile);
//        flag=1;
//        LoadImageFromFile(pImageFile);
//    }
//    else
//    {
//        temp=imagePathFile;
//    }
//    temp.replace(".html", ".png");
//    if (QFile::exists(temp) && flag==0)
//    {
//        imagePathFile=temp;
//        QFile *pImageFile = new QFile(imagePathFile);
//        flag=1;
//        LoadImageFromFile(pImageFile);
//    }
//    else
//    {
//        temp=imagePathFile;
//    }
//    temp.replace(".html", ".jpg");
//    if (QFile::exists(temp) && flag==0)
//    {
//        imagePathFile=temp;
//        QFile *pImageFile = new QFile(imagePathFile);
//        flag=1;
//        LoadImageFromFile(pImageFile);
//    }
//    else
//    {
//        temp=imagePathFile;
//    }

//    myTimer.start();
//    DisplayTimeLog();
//    //DisplayJsonDict(b);
}

/*!
 * \fn    MainWindow::setMFilename
 * \brief This function is used to set the file name or image file name of the project tree when
 *        opened. Also this functions removes extentions of those file and keep only name of that file.
 * \param name
 *
 */
void MainWindow::setMFilename( QString name )
{
    mFilename = name;
    QString tempName = mFilename;

    //!Replacing all the extensions from the filename
    tempName.replace("Inds", "Images");
    tempName.replace("CorrectorOutput", "Images");
    tempName.replace("VerifierOutput", "Images");
    tempName.replace(".txt", ".jpeg").replace(".html", ".jpeg");

    //! select the image. look for jpeg, jpg and png(select first whichever is found)
    QFileInfo check_file(tempName);
    if (check_file.exists() && check_file.isFile())
    {
        mFilenameImage = tempName;
        return;
    }

    tempName.replace(".jpeg", ".jpg");
    check_file.setFile(tempName);
    if (check_file.exists() && check_file.isFile())
    {
        mFilenameImage = tempName;
        return;
    }

    tempName.replace(".jpg", ".png");
    check_file.setFile(tempName);
    if (check_file.exists() && check_file.isFile())
    {
        mFilenameImage = tempName;
        return;
    }
}

/*!
 * \fn    MainWindow::directoryChanged
 * \brief This funcction is called when directory of project is changed. This function sets all
 *        file paths of corrector output and verifier output to the new loaction.
 * \param path
 */
void MainWindow::directoryChanged(const QString &path)
{

    QDir d(path);

    QString dirstr = d.dirName();
    auto list = d.entryList(QDir::Files);
    QSet<QString> s;
    for (auto file : list)    //iterating on set
    {
        s.insert(file);
    }
    if (dirstr == "CorrectorOutput")
    {
        QSet<QString> added = s - corrector_set;
        QSet<QString> removed = corrector_set - s;
        QString str = mProject.GetDir().absolutePath() + "/CorrectorOutput/";  //new location
        Filter * filter = mProject.getFilter("CorrectorOutput");
        for (auto f : added)
        {
            QString t = str + "/" + f;
            QFile f2(t);
            mProject.AddTemp(filter, f2, "");
            corrector_set.insert(f);
        }
    }
    else
    {
        QSet<QString> added = s - verifier_set;
        QString str = mProject.GetDir().absolutePath() + "/VerifierOutput/"; //new location
        Filter * filter = mProject.getFilter("VerifierOutput");
        for (auto f : added)
        {
            QString t = str + "/" + f;
            QFile f2(t);
            mProject.AddTemp(filter, f2, "");
            verifier_set.insert(f);
        }
    }
}

/*!
* \fn    MainWindow::checkUnsavedWork
* \brief This function is called when user closes a tab or page then this function checks for the
*        unsaved changes in that tab or page and if any unsaved work found then it returns true else
*        false.
*
* \return true, false
*/
bool MainWindow::checkUnsavedWork() {
    //!iterate over tab counts and checks for wok in the text browser of that tab

    if(!curr_browser) return false;

    QString closing_browserHtml = curr_browser->toHtml();
    if(closing_browserHtml != gInitialTextHtml[currentTabPageName]) {
        return true;
    }
    else
        return false;
}

/*!
* \fn    MainWindow::saveAllWork
* \brief This function is called when user closes a tab or page and it has some unsaved work containg in that
*        page or tab and this function saves all those changes in that page or tab.
*
* \sa    on_actionSave_triggered()
*/
void MainWindow::saveAllWork()
{
    QString closing_browserHtml = curr_browser->toHtml();
    if(!curr_browser->isReadOnly() && closing_browserHtml != gInitialTextHtml[currentTabPageName]) {
        on_actionSave_triggered();
    }
}

/*!
* \fn    MainWindow::on_actionSave_All_triggered
* \brief This is called when user cloases a tab, page or the application. It saves all the unsaved work
*        of that page.
*
* \sa    on_actionSave_triggered(), UpdateFileBrekadown()
*/
void MainWindow::on_actionSave_All_triggered()  //enable when required
{

    UpdateFileBrekadown();
    on_actionSave_triggered();

}

/*!
 * \fn MainWindow::closeEvent
 * \brief event filter that tracks close event and prompts user to save file if they didn't
 * \param event
 */
void MainWindow::closeEvent (QCloseEvent *event)
{
    bool isUnsaved = checkUnsavedWork();

    if (isUnsaved)
    {

        //!confusion
        QMessageBox saveBox;
        saveBox.setWindowTitle("Close");
        saveBox.setIcon(QMessageBox::Question);
        saveBox.setInformativeText("You have unsaved files. Your changes will be lost if you don't save them.\n");
        QPushButton *svButton = saveBox.addButton(QMessageBox::Save);
        QPushButton *discardButton = saveBox.addButton(QMessageBox::Discard);
        QPushButton *cncButton = saveBox.addButton(QMessageBox::Cancel);
        saveBox.exec();



        if (saveBox.clickedButton() == cncButton)
        {
            event->ignore();
        }
        else if (saveBox.clickedButton() == discardButton)
        {
            event->accept();
        }
        else
        {
            saveAllWork();
            event->accept();
        }
    }
}

/*!
 * \fn MainWindow::highlight
 * \brief This function highlights words of Globally Replaced Words
 *
 * We have a map where we store all global replace word list and we get this data from CPair File in
 * the project set folder.
 *
 * We loop through this map and we also loop through all the files in the set.
 *
 * Now we get the index of the globally replaced word, we check if the word is a whole word and not a
 * substring and after passing all the condition we set the background of the word in yellow.
 *
 */

void MainWindow:: highlight(CustomTextBrowser *b , QString input)
{

    QMap <QString, QString>::iterator grmIterator;
    QTextCursor cursor(b->document());

    QTextCharFormat fmt;
//    if(uploadReplaceFlag)
//        fmt.setBackground(QColor("#ffa000"));
//    else
    fmt.setBackground(Qt::yellow);


    int indexOfReplacedWord;
    int from=0;
    int count;
    int numReplaced=0;
    //qDebug()<<"mapOfReplacements"<<mapOfReplacements;
    for (grmIterator = mapOfReplacements.begin(); grmIterator != mapOfReplacements.end(); ++grmIterator)
    {
        count = input.count(grmIterator.value(),Qt::CaseInsensitive);
        numReplaced=0;
        from=0;
        int flag=0;

        while(numReplaced<count)
        {

            int endIndex;
            indexOfReplacedWord = input.indexOf(grmIterator.value(),from , Qt::CaseInsensitive);
            endIndex = indexOfReplacedWord;
//            qDebug() << indexOfReplacedWord << " " <<endIndex;
//            while(input[endIndex]!=" ")
//                endIndex++;
//            qDebug() << indexOfReplacedWord << " " <<endIndex;
            int len = grmIterator.value().length();
            while(len > 0)
            {
                endIndex++;
                len--;
            }

            int start = indexOfReplacedWord;

            if(indexOfReplacedWord == 0){
                input[start] == " ";
            }
            else{
                start=start-1;
            }

            QRegExp regex("[$&+,:;=?@#|'\"<>.^*()%!-\n\t]");

            QString test1=input.at(start);
            QString test2=input.at(endIndex);

            //qDebug()<<"input[start-1]="<<input[start];
            //qDebug()<<"input[endIndex]="<<input[endIndex];
            if((input[endIndex] == " " || test2.contains(regex)) && (input[start] == " " || test1.contains(regex))){
                flag=1;
            }
            //qDebug()<<"flag : "<<flag;

            if(flag==1)
            {
                cursor.setPosition(indexOfReplacedWord, QTextCursor::MoveAnchor);
                cursor.setPosition(endIndex, QTextCursor::KeepAnchor);
                cursor.mergeCharFormat(fmt);
            }
            from = endIndex;
            numReplaced+=1;
            flag=0;
        }
    }


}

/*!
 * \fn MainWindow::on_actionas_PDF_triggered
 * \brief This function gets the book project set in PDF format for easy reading.
 *
 * Depending on whether the user has opened the tool in corrector or verifier mode the appropriate
 * folder is opened and we scan the html contents of only html files in the folder.
 *
 * We then print the html contents using QPrinter and output it in the project set folder with the
 * name "Bookset.pdf".
 *
 */

void MainWindow::on_actionas_PDF_triggered()
{
    //! We set the dir path to CorrectorOutput or Verifier output depending on whether it is opened
    //! in Corrector or Verifier Mode.
    QString currentDirAbsolutePath;
    if(mRole=="Verifier")
        currentDirAbsolutePath = gDirTwoLevelUp + "/VerifierOutput/";
    else if (mRole=="Corrector") {
        currentDirAbsolutePath = gDirTwoLevelUp + "/CorrectorOutput/";
    }

    //! We then open this directory and set sorting preferences.
    QDir dir(currentDirAbsolutePath);
    dir.setSorting(QDir::SortFlag::DirsFirst | QDir::SortFlag::Name);
    QDirIterator dirIterator(dir,QDirIterator::NoIteratorFlags);

    //! Set count of files in directory

    QString html_contents=""; // Final HTML Content
    QString mainHtml;
    int count = dir.entryList(QStringList("*.html"), QDir::Files | QDir::NoDotAndDotDot).count();
    int counter=0;

    int stIndex, startFrom = 0;

    //! Set the background of the pdf to be printed to be white
    QString searchString = "background-color:#"; // string to be searched
    int l = searchString.length();
    QString whiteColor = "ffffff";

    int itr = 0;
    PdfRangeDialog *pdfRangeDialog = new PdfRangeDialog(this, count, 100);
    pdfRangeDialog->exec();
    int startPage = 0;
    int endPage = 0;
    if (pdfRangeDialog->isOkClicked()) {
        startPage = pdfRangeDialog->getStartPage() - 1;
        endPage = pdfRangeDialog->getEndPage();
    } else {
        startPage = 0;
        endPage = count;
    }
    qDebug() << startPage << " : " << endPage;


    //! Loop through all files
    foreach(auto a, dir.entryList())
    {
        QString x = currentDirAbsolutePath + a;

        startFrom = 0; // The position from which searchString will be scanned
        //! if condition makes sure we extract only html files for PDF Processing
        //! (folder has hocr, dict, htranslate, and other such files)
        if(x.contains("."))
        {
            QStringList html_files = x.split(QRegExp("[.]"));

            //! condition to check if file is html
            if(html_files[1]=="html")
            {
                if (itr < startPage) {
                    itr++;
                    continue;
                }

                QFile file(x);
                if (!file.open(QIODevice::ReadOnly)) qDebug() << "Error reading file main.html";
                QTextStream stream(&file);
                stream.setCodec("UTF-8");

                //! Read the file

                mainHtml=stream.readAll();
                //! Changing the text background to white by setting the background to #fffff
                while (true){
                    stIndex = mainHtml.indexOf(searchString, startFrom);
                    if (stIndex == -1)
                        break;
                    stIndex += l; // increment line
                    mainHtml.replace(stIndex, 6, whiteColor); // Here, 6 is used because length of whiteColor is 6
                    startFrom = stIndex + 6;
                }
                //! append counter when one file is fully scanned
                counter++;

                //! Search for Latex code in html files and replace it by corresponding png images
                //! We save latext for mathematical equations in html, and show png in our tool as our tool can't render Latex
                if(mainHtml.contains("$$")){

                    QRegularExpression rex_lat("<a(.*?)</a>",QRegularExpression::DotMatchesEverythingOption);
                    QRegularExpressionMatchIterator itr_lat;
                    itr_lat = rex_lat.globalMatch(mainHtml);
                    while(itr_lat.hasNext()){

                        QRegularExpressionMatch match = itr_lat.next();
                        QString text = match.captured(1);

                        if(text.contains("Equations_"))
                        {
                            int sindex = match.capturedStart(1);
                            int l_index = match.capturedEnd(1);
                            std::string inputText_ = text.toStdString();
                            int ind = inputText_.find("/");
                            int lindex = inputText_.find(".tex");

                            std::string str = inputText_.substr(ind,lindex-ind);
                            QString path = QString::fromStdString(str) + ".png";
                            QString html = "<img src=\""+path+"\">";
                            text = "<a"+text+"</a>";
                            mainHtml.replace(text,html);
                        }

                    }
                    mainHtml = mainHtml.replace("$$","dne_nqe"); //where dne_nqe is a random string used as end delimiter here.
                    //Note that this string should not appear as an original text - else it will cause parsing issues.
                    QRegularExpression rex_dollar("dne_nqe(.*?)dne_nqe",QRegularExpression::DotMatchesEverythingOption);
                    mainHtml = mainHtml.remove(rex_dollar);
                }

                //! Once page html is extracted ... before we move to next page we add html tag
                //! for page break so that the PDF printer separates the pages
                //! We do this till all pages done
                if(counter<count){
                    mainHtml+="<P style=\"page-break-before: always\"></P>";
                }
                file.close();
                html_contents.append(mainHtml);
                itr++;

                if (itr == endPage) {
                    break;
                }

            }
            //! if file is not html
            else {
                continue;
            }
        }
    }

    // Asking the path where to save the PDF
    QString saveFileName = QFileDialog::getSaveFileName(this, "Save File", gDirTwoLevelUp, tr("PDF(*.pdf)"));

    // New way of printing pdf
    QPrinter printer(QPrinter::ScreenResolution);
    printer.setPaperSize(QPrinter::A4);
    printer.setPageMargins(QMarginsF(5, 5, 5, 5));
    printer.setOutputFileName(saveFileName);
    printer.setOutputFormat(QPrinter::NativeFormat);

    QPrintDialog printDialog(&printer, this);

    if (printDialog.exec() == QDialog::Accepted) {
        PrintWorker *workerPrint = new PrintWorker(nullptr, html_contents);
        QThread *thread = new QThread;
        workerPrint->printer = printDialog.printer(); // Assigning the printer for printing (VERY IMPORTANT)

        connect(thread, SIGNAL(started()), workerPrint, SLOT(printPDF()));
        connect(workerPrint, SIGNAL(finishedPrintingPDF()), thread, SLOT(quit()));
        connect(workerPrint, SIGNAL(finishedPrintingPDF()), workerPrint, SLOT(deleteLater()));
        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
        connect(workerPrint, SIGNAL(finishedPrintingPDF()), this, SLOT(stopSpinning()));

        workerPrint->moveToThread(thread);
        thread->start();
        spinner = new LoadingSpinner(this);
        spinner->SetMessage("Printing PDF...", "Printing...");
        spinner->setModal(false);
        spinner->exec();

        QMessageBox::information(this, "Print Successful", "Printed PDF successfully", QMessageBox::Ok, QMessageBox::Ok);
    }
}

/*!
 * \fn MainWindow::on_actionGet_Help_triggered
 * \brief Redirects user to OCR Tool Help documentation
 */
void MainWindow::on_actionGet_Help_triggered()
{
    QDesktopServices::openUrl(QUrl("https://docs.google.com/document/d/1PAQKz3Vwu5EN850uxZUeSejvmwF2293j/edit?usp=sharing&ouid=114703528031965332802&rtpof=true&sd=true", QUrl::TolerantMode));
}

/*!
 * \fn MainWindow::on_actionTutorial_triggered
 * \brief Redirects user to Team OCR Youtube channel
 */
void MainWindow::on_actionTutorial_triggered()
{
    QDesktopServices::openUrl(QUrl("https://www.youtube.com/channel/UCrViL9ay1RO9lS7FIlnh8BQ", QUrl::TolerantMode));
}

/*!
 * \fn MainWindow::on_actionLinux_triggered
 * \brief Redirects user to OpenOCRCorrect Linux Installation Guide
 */
void MainWindow::on_actionLinux_triggered()
{
    QDesktopServices::openUrl(QUrl("https://docs.google.com/document/d/15PbeYfdMl1eMypAMoqibG6Z5dxipfx_aZBSAhifTlec/edit?usp=sharing", QUrl::TolerantMode));
}

/*!
 * \fn MainWindow::on_actionWindows_triggered
 * \brief Redirects user to OpenOCRCorrect Windows Installation Guide
 */
void MainWindow::on_actionWindows_triggered()
{
    QDesktopServices::openUrl(QUrl("https://docs.google.com/document/d/16P1UZ2t1Dd8qhAsl2UqL5hTkrsOkBJqr/edit?usp=sharing&ouid=105473566501828143797&rtpof=true&sd=true", QUrl::TolerantMode));
}

/*!
 * \fn MainWindow::on_actionShortcut_Guide_triggered
 * \brief Opens shortcut guide for all available features
 */
void MainWindow::on_actionShortcut_Guide_triggered()
{
    ShortcutGuideDialog dialog;
    dialog.setModal(true);
//    dialog.setWindowFlags(Qt::FramelessWindowHint);

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect  screenGeometry = screen->geometry();
    float height = screenGeometry.height() * 0.6;
    float width = screenGeometry.width() * 0.6;

    dialog.setFixedSize(width, height);
    dialog.exec();
}

/*!
 * \fn MainWindow::on_textBrowser_textChanged
 * \brief Updates the word count based on browser text change
 */
void MainWindow::on_textBrowser_textChanged()
{
    WordCount();
}

/*!
 * \fn MainWindow::on_zoom_Out_Button_clicked
 * \brief sets zoom out to graphics view
 */
void MainWindow::on_zoom_Out_Button_clicked()
{
    if (z)
        z->gentle_zoom(z->getDefaultZoomOutFactor());
}

/*!
 * \fn MainWindow::on_zoom_In_Button_clicked
 * \brief sets zoom in to graphics view
 */
void MainWindow::on_zoom_In_Button_clicked()
{
    if (z)
        z->gentle_zoom(z->getDefaultZoomInFactor());
}

/*!
 * \fn MainWindow::zoom_slider_valueChanged
 * \brief Updates the zoom percentage to QLabel
 * \param value
 */
void MainWindow::zoom_slider_valueChanged(int value)
{
    ui->zoom_level_value->setText(QString::number(z->zoom_level) + "%");
}

/*!
 * \fn MainWindow::zoom_slider_moved
 * \brief Updates the zoom value when the slider is moved
 * \param value
 */
void MainWindow::zoom_slider_moved(int value)
{
    //!Calculating value to check whether it is zoom in or zoom out
    if (value % 10 != 0) {
        value = (value / 10)*10 + 10;
    }
    double zoomFactor;

    //!increses zoom level by one
    if (value > z->zoom_level) {
        zoomFactor = 1 + ((value - z->zoom_level) / 100.0);
    }
    //!Decreases zoom level by one
    else if (value < z->zoom_level) {
        zoomFactor = 1 - ((z->zoom_level - value) / 100.0);
    }
    else return;

    //!Sets the final zoom value to the slider
    z->gentle_zoom(zoomFactor);
    ui->horizontalSlider->setValue(value);
}

/*!
 * \fn MainWindow::zoomedUsingScroll
 * \brief Set the zoom level of horizontal slider based on scroll
 */
void MainWindow::zoomedUsingScroll()
{
    currentZoomLevel = z->zoom_level;
    ui->horizontalSlider->setValue(z->zoom_level);
}

/*!
 * \fn MainWindow::createActions
 * \brief sets Icon Size of mainToolbar
 */
void MainWindow::createActions()
{
    ui->mainToolBar->setIconSize(QSize(100, 100));
}

/*!
* \fn MainWindow::on_actionUndo_Global_Replace_triggered()
* \brief This feature allows user to undo global replacements when it has been done previously
*        This function checks if last global replace was for single word or multiple word and
*        accordingly the appropriate function is called.
*        This function retrives a map for undo global replace in a variable "UndoGRMap" and
*        writes all the new words back to the old words thereby undoing the last global replace
*
*\sa undoGlobalReplace_Single_Word(), writeGlobalCPairsToFiles(), getUndoGlobalReplaceMap_Multiple_Words()

*/
void MainWindow::on_actionUndo_Global_Replace_triggered()
{
    QMap<QString, QString> undoGRMap;
    int r1 = 0;
    int r2 = 0;
    int files = 0;

    reverseGlobalReplacedWordsMap();
    if ( globallyReplacedWords.size() == 1 )
    {
        QString oldWord = globallyReplacedWords.firstKey();
        QString newWord = globallyReplacedWords.value(oldWord);
        oldWord=oldWord.trimmed();
        newWord=newWord.trimmed();
        bool replace = undoGlobalReplace_Single_Word(oldWord, newWord);

        if ( replace )
            undoGRMap.insert(oldWord, newWord);
     }
     else if ( globallyReplacedWords.size() > 1 )
     {
        //qDebug() << "For Multiple Words";
        undoGRMap = getUndoGlobalReplaceMap_Multiple_Words(globallyReplacedWords);
     }

     QString currentDirAbsolutePath = gDirTwoLevelUp + "/" + gCurrentDirName;
     QDirIterator dirIterator(currentDirAbsolutePath, QDirIterator::Subdirectories);

     if ( !undoGRMap.isEmpty() )
     {
        //for (auto itFile : filesChangedUsingGlobalReplace)
        while(dirIterator.hasNext())
        {
            //QString suff = dirIterator.fileInfo().completeSuffix();
            QString itFile = dirIterator.next();
            if(itFile.contains(".html")){
                r1 = writeGlobalCPairsToFiles(itFile, undoGRMap);
                r2 = r2+r1;
                if(r1 > 0)
                files++;
             }
//             else
//                writeGlobalCPairsToFiles(itFile, undoGRMap);
        }

        QDir directory(gDirTwoLevelUp);
        QString setName=directory.dirName();
        QString filename = gDirTwoLevelUp+"/"+setName+"_logs.csv";
        //qDebug()<<filename;
        QFile csvFile(filename);
        if(!csvFile.exists())
        {
        }

        else
        {
            csvFile.open(QIODevice::ReadOnly);
            QMap <QString, QString>::iterator grmIterator;
            QStringList s1;
            QTextStream s2(&csvFile);
            s2.setCodec("UTF-8");
            while (!s2.atEnd()) {
            QString line = s2.readLine();
                //qDebug()<<line;
                //s1.append(line.split(',').first());
                s1.append(line);
            }
            //qDebug()<<"S1"<<s1;

            csvFile.close();
            csvFile.open(QIODevice::WriteOnly);
            QTextStream output(&csvFile);
            output.setCodec("UTF-8");
            QStringList s4;
            for(grmIterator = undoGRMap.begin(); grmIterator != undoGRMap.end();++grmIterator)
            {
                QString value=grmIterator.value();
                //qDebug()<<value;
                for(int i=0;i<s1.length();i++)
                {
                    QString s3;
                    s3.append(s1[i].split(',').first());
                    if(s3==value)
                    {
                     s4.append(s1[i]);
                    }
                }

            }
            //qDebug()<<"S4"<<s4;
            for(int i=0;i<s1.length();i++)
            {
                for(int j=0;j<s4.length();j++)
                {
                    if(s1[i]==s4[j])
                    {
                    s1.removeAll(s1[i]);
                    }
                }
            }
            //qDebug()<<"S1 Final"<<s1;

            for(int i=0;i<s1.length();i++){
                output<<s1[i];
                output << "\n";

            }
            csvFile.close();
        }
    }
     QString msg  = QString::fromStdString(std::to_string(undoGRMap.values().length()) + " words changed" + "\n" + std::to_string(r2) + " instances replaced" + "\n" + std::to_string(files) + " files modified");
     QMessageBox messageBox;
     if(undoGRMap.values().length()>0)
        messageBox.information(0, "Undo Global Replacement Successful", msg);
}

/*!
* \fn MainWindow::reverseGlobalReplacedWordsMap()
* \brief This function adds the words requested by the user for global replace undo change to a map "reversedMap"
*/
void MainWindow::reverseGlobalReplacedWordsMap()
{
    QMap<QString, QString>::iterator i;
    QMap<QString, QString> reversedMap;

    for (i = globallyReplacedWords.begin(); i != globallyReplacedWords.end(); ++i)
        reversedMap[i.value()] = i.key();

    globallyReplacedWords = reversedMap;
}

/*!
* \fn MainWindow::undoGlobalReplace_Single_Word()
* \brief This function shows a dialog box asking the user whether the global replacement for the word has to be undone
*        This function is only called when the last global replace was done for a single word ONLY.

*/
bool MainWindow::undoGlobalReplace_Single_Word(QString oldWord, QString newWord)
{
    QMessageBox messageBox(this);

    QAbstractButton *undo = messageBox.addButton(tr("Yes"), QMessageBox::ActionRole);
    QAbstractButton *cancel = messageBox.addButton(tr("No"), QMessageBox::RejectRole);

    QString msg = "Do you want to undo the changes you made previously using global replace feature ?\nUndo by replacing \"" + oldWord + "\" with \"" + newWord + "\"\n";
    messageBox.setWindowTitle("Undo Global Replace");
    messageBox.setText(msg);
    messageBox.setModal(true);
    messageBox.exec();

    if ( messageBox.clickedButton() == undo )
        return true;
    return false;
}

/*!
* \fn MainWindow::undoGlobalReplace_Multiple_Words()
* \brief This function shows a dialog box asking the user whether the global replacement for the word has to be undone
*        This function is only called when the last global replace was done for more than ONE word.
*\sa UndoGlobalReplace::on_applyButton_clicked(),UndoGlobalReplace::getFinalUndoMap .
*/
QMap<QString, QString> MainWindow::getUndoGlobalReplaceMap_Multiple_Words(QMap<QString, QString> GRMap)
{

    QMap<QString, QString> undoGRMap;
    UndoGlobalReplace ugrWindow(GRMap, this);

    ugrWindow.setModal(true);
    ugrWindow.exec();

    if ( ugrWindow.on_applyButton_clicked() )
        undoGRMap = ugrWindow.getFinalUndoMap();
    return GRMap;
}

/*!
* \fn    MainWindow::replaceInAllFilesFromTSVfile
* \brief This feature allows user to perform global replace by uploading a tsv file
*        Here the function checks if the file being uploaded by the user is valid or invalid by calling checkForValidTSVfile()
*        If it is valid then it maps the words in the file to global replace map and asks user whether to perform global replace or not
*        If the user clicks on Ok then the words are globally replaced
*
* \sa     checkForValidTSVfile(), writeGlobalCPairsToFiles()
*/
void MainWindow::replaceInAllFilesFromTSVfile()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open a file", gDirTwoLevelUp);
    QFile file(filename);

    if (!file.exists() )
    {
        //QMessageBox::warning(this, "Error", "Incorrect file format", QMessageBox::Ok, QMessageBox::Ok);
        return;
    }
    if ( !checkForValidTSVfile(file) )
    {
        QMessageBox::warning(this, "Error", "Incorrect file format", QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Error", "Error in opening file", QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    QTextStream in(&file);
    QVector<QString> phrasesToBeReplaced;
    while (!in.atEnd())
    {
        QString line = in.readLine();
        if (line == "")
            continue;
        int indexOfFirstTab = line.indexOf("\t");
        QString sourcePhrase = line.left(indexOfFirstTab);
        QString targetPhrase = line.right(line.length() - indexOfFirstTab - 1);

        phrasesToBeReplaced.push_back(sourcePhrase + " =>" + targetPhrase + " ");
    }
    file.close();

    QString currentDirAbsPath = gDirTwoLevelUp + "/" + gCurrentDirName;
    runGlobalReplace(currentDirAbsPath, phrasesToBeReplaced);
}


/*!
* \fn MainWindow::checkForValidTSVfile()
* \brief This function checks whether the tsv file uploaded for global replace is valid or not
* This function opens the file, and starts scanning it.
* A tsv file is a valid only when each and every line contains one tab space between text and no single spaces
* The function loops through each line checking for the above condition. If any line violates above condition
* then it returns false and terminates indicating that the file is invalid.
* If the scan is successful and every line satisfies the condition, it returns true and exits indicating that file is valid.

*/

bool MainWindow::checkForValidTSVfile(QFile & file)
{
    int singleSpaces, tabSpaces;

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream in(&file);

    while (!in.atEnd())
    {
        QString line = in.readLine();
        if (line == "")
            continue;

        if (!line.contains("\t"))
        {
            file.close();
            return false;
        }
    }
    file.close();

    return true;
}

/*!
* \fn    MainWindow::on_actionUpload_triggered
* \brief When uploading any file or folder this function is called which calls replaceInAllFilesFromTSVfile function
*
* \sa    replaceInAllFilesFromTSVfile()
*/
void MainWindow::on_actionUpload_triggered()
{
    //uploadReplaceFlag = 1;

    if(ProjFile==""){
        QMessageBox::information(0, "Error", "Please open a Project first.");
        return;
    }
    QString closing_browserHtml = curr_browser->toHtml();
    if (closing_browserHtml != gInitialTextHtml[currentTabPageName])
    {
        QMessageBox::information(0, "Error", "Please save the current opened file first.");
    }
    else
        replaceInAllFilesFromTSVfile();
}

/*!
* \fn    MainWindow::on_justify_triggered
* \brief This function checks if page is opened in readonly mode if false then it allows the
*        user to change the alignment of the text in the current opened page or tab.
*
*/
void MainWindow::on_justify_triggered()
{
    if(!curr_browser || curr_browser->isReadOnly())
        return;
    curr_browser->setAlignment(Qt::AlignJustify);
}

/*!
* \fn    MainWindow::on_actionFont_Color_triggered
* \brief This function checks if page is opened in readonly mode if false then it allows the
*        user to select a color for the font and then changes the text font color in the page.
*
*/
void MainWindow::on_actionFont_Color_triggered()
{
    if(!curr_browser || curr_browser->isReadOnly())
      {
        QMessageBox::information(0, "Error", "Please check whether the page is opened and you are editing in appropriate role.");
        return;
      }

    QTextCursor cursor = curr_browser->textCursor();
    QColor choosencolor = QColorDialog::getColor();

    QTextCharFormat charFormat;
    charFormat.setForeground(QBrush(choosencolor));
    cursor.mergeCharFormat(charFormat);
}

/*!
* \fn    MainWindow::reLoadTabWindow
* \brief This function reloads the tab window that is it loads the document again into the
*        current tab.
*
* \sa    LoadDocument()
*/
void MainWindow::reLoadTabWindow()
{
    string localFilename = mFilename.toUtf8().constData();
    QFile *file = new QFile(QString::fromStdString(localFilename));
    QFileInfo f(*file);
    QString suff = f.completeSuffix();
    if (suff == "txt" || suff == "html") {
      (file,suff,currentTabPageName );
    }

}

/*!
 * \fn MainWindow::on_lineEditSearch_textChanged
 * \brief This function is a part of Project File Search feature.
 *
 * If set is very large then user can enter page number or any keyword related to file name and get
 * it filtered. The tool will show him that page and thus he will be able to open more easily.
 *
 * This function scans the whole files and stores in a list. Whenever the user types on the search
 * text box it will check the keyword to match in the list. The respective files are highlighted.
 *
 */

void MainWindow::on_lineEditSearch_textChanged(const QString &arg1)
{
    ui->treeView->selectionModel()->clearSelection();
    QModelIndex currentTreeItemIndex=ui->treeView->selectionModel()->currentIndex();
    QModelIndex parentIndex = currentTreeItemIndex.parent();

    auto *model = ui->treeView->model();
    int rowCount = ui->treeView->model()->rowCount(parentIndex);
    //qDebug()<<"rowCount"<<rowCount;
    QModelIndexList children;

    QString item;
    for(int i=0;i<model->rowCount();i++){
        children<<model->index(i,0);
        item=children[i].data(Qt::DisplayRole).toString();
//        qDebug()<<"Item"<<item;
    }

    //qDebug()<<"Children size"<<children.size();
    for(int i=0;i<children.size();i++){
        for(int j=0;j<model->rowCount(children[i]);j++){
            children<<children[i].child(j,0);

            //qDebug()<<"Item"<<item;
            //if(item.contains(arg1)){
            //ui->treeView->selectionModel()->setCurrentIndex(children[j],QItemSelectionModel::Select);
            //}
        }
    }
    //qDebug()<<"Children size"<<children.size();
    for(int i=0;i<children.size();i++){
        item=children[i].data(Qt::DisplayRole).toString();
        //qDebug()<<"Item"<<item;
        if(item.contains(arg1)){
        ui->treeView->selectionModel()->setCurrentIndex(children[i],QItemSelectionModel::Select);
        }
    }
}

/*! Cursor Highlight Feature
 *
 *  If user is working on a page in a book and closes the tool and returns back later, when he opens the
 *  page he will be able to know in which line he was last working on by highlighting that position of
 *  cursor.
 *
 *  We do it by saving the cursor position to a file when users saves it and quits the tool or changes
 *  page, and then when he returns we retrive that position and highlight it.
 *
 *  We use readSettings() and writeSettings() in this feature
 *
 */

/*!
 * \fn MainWindow::writeSettings
 * \brief We use function write settings to write the positions of the cursor to a binary file
 *
 * We retrieve the cursor position and put it in a QMap first and then write it in the file
 * and subsequently flush it in persistent storage
 *
 */
void MainWindow::writeSettings()
{
    int pos = curr_browser->textCursor().position();
    QString f = gDirTwoLevelUp + "/cursor.txt";
    if(QFile(f).exists())
    {
        QFile::remove(f);
    }
    QString filename = gDirTwoLevelUp + "/.cursor.txt";
      QFile myFile (filename);
      myFile.open(QIODevice::ReadWrite);
      QDataStream in (&myFile);
      in.setVersion(QDataStream::Qt_5_3);
      QMap <QString ,int> curpos;
      in >> curpos;
      if(curpos.find(gCurrentPageName)==curpos.end()){
            curpos.insert(gCurrentPageName,pos);
            //qDebug()<<curpos[gCurrentPageName];
        }
      else{
            curpos[gCurrentPageName]=pos;
        }
      myFile.resize(0);
      QDataStream out (&myFile);
      out.setVersion(QDataStream::Qt_5_3);
      out<<curpos;
      myFile.flush();
      //qDebug() << "cursor.txt written .. ";
      myFile.close();

}

/*!
 * \fn MainWindow::readSettings
 * \brief We use function read settings to read the positions of the cursor from a binary file and
 * thus get the cursor position of that file.
 *
 * We read the file by first opening it and we feed it into QMap and get the cursor position of that
 * file. Then we hughlight that area by setting stylesheet of text browser.
 *
 */

void MainWindow::readSettings()
{
    int pos1;

    QString filename = gDirTwoLevelUp + "/.cursor.txt";
        QFile myFile (filename);
        myFile.open(QIODevice::ReadOnly);
        QMap<QString,int> map;
        QDataStream in (&myFile);
        in.setVersion(QDataStream::Qt_5_3);
        in >> map;
        //qDebug()<<map;
        pos1=map[gCurrentPageName];
        //qDebug()<<"pos1"<<pos1;
        myFile.close();
        curr_browser->setStyleSheet("CustomTextBrowser{selection-background-color: #ffa500; selection-color: #ffffff;}");

    auto cursor = curr_browser->textCursor();
    cursor.setPosition(pos1);
    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
    curr_browser->setTextCursor(cursor);


}

void MainWindow::on_action1_triggered()
{
    proj_flag = '0';
    isRecentProjclick = true;
    on_actionOpen_Project_triggered();
}

void MainWindow::on_action2_triggered()
{
    proj_flag = '1';
    isRecentProjclick = true;
    on_actionOpen_Project_triggered();
}

void MainWindow::on_action3_triggered()
{
    proj_flag = '2';
    isRecentProjclick = true;
    on_actionOpen_Project_triggered();
}

/*!
 * \fn MainWindow::RecentPageInfo
 * \brief Used for storing recent page info in QSettings, so that last page opened for the set gets opened when that set is opened again
 */
void MainWindow::RecentPageInfo()
{
    ui->treeView->selectionModel()->clearSelection();
    QModelIndex currentTreeItemIndex=ui->treeView->selectionModel()->currentIndex();
    QModelIndex parentIndex = currentTreeItemIndex.parent();
    auto *model = ui->treeView->model();
    int rowCount = ui->treeView->model()->rowCount(parentIndex);
    //qDebug()<<"rowCount"<<rowCount;
    QModelIndexList children;
    QString var1,var2;
    QSettings settings("IIT-B", "OpenOCRCorrect");
    settings.beginGroup("RecentPageLoaded");
    QString stored_project = settings.value("projectName1").toString();
    QString stored_project2 = settings.value("projectName2").toString();
    QString stored_project3 = settings.value("projectName3").toString();
    if(ProjFile == stored_project){
    var1 = settings.value("name1").toString();
    var2 = settings.value("pageParent1").toString();}
    else if(ProjFile == stored_project2){
        var1 = settings.value("name2").toString();
        var2 = settings.value("pageParent2").toString();
    }
    else if(ProjFile == stored_project3){
        var1 = settings.value("name3").toString();
        var2 = settings.value("pageParent3").toString();
    }
    settings.endGroup();
    QString item,item1;
    for(int i=0;i<model->rowCount();i++){
        children<<model->index(i,0);
        item=children[i].data(Qt::DisplayRole).toString();
        if(item == var2){
            for(int j=0;j<model->rowCount(children[i]);j++){
                children<<children[i].child(j,0);
                item1 = children[i].child(j,0).data(Qt::DisplayRole).toString();
                //qDebug ()<<"Item1"<<item1;
                if(item1 == var1){
                    auto location = children[i].child(j,0);
                    ui->treeView->selectionModel()->setCurrentIndex(children[i].child(j,0),QItemSelectionModel::Select);
                    file_click(location);
                }
        }
    }}
}



/*!
 * \fn MainWindow::on_actionCheck_for_Updates_triggered()
 * \brief Checks for the update of the following File from Github-Repo
 *Timer is set to 5000ms which starts the sprite animation.
 * If the animation is already running, calling this method has no effect.
 * Stores the response from github as string.
 * Compares Latest Version to the Current version. If found Same then functions is returned.
 * Latest Version is download if Current version is not equal to Latest Version.
 * (Later) Button is also set if user does not want to upgrade this time.
 */
void MainWindow::on_actionCheck_for_Updates_triggered()
{
    QUrl url("https://api.github.com/repos/IITB-OpenOCRCorrect/iitb-openocr-digit-tool/releases");
    qInfo() << url.toString();
    QNetworkRequest request(url);               //requesting url over the network
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkAccessManager nam;                  //sending network request
    QNetworkReply * reply = nam.get(request);
    QTimer *timer = new QTimer();
    timer->start(5000);

    while(true){
        qApp->processEvents();
        if(reply->isFinished()) break;
    }

    if(reply->isFinished()){
        QByteArray response_data = reply->readAll();
        QJsonDocument json = QJsonDocument::fromJson(response_data);
        qDebug() << json[0]["name"].toString();
        if(json[0]["name"].toString() == "")
        {
            qDebug() << QString("Timeout .... Internet Not Available");
           // return "";
        }
        QString latestVersion=json[0]["name"].toString();
        QString newFeatures = json[0]["body"].toString();
        qDebug()<<latestVersion;
        QString curr_version = qApp->applicationVersion();
        //QString latestVersion = UpdateInfo();
        qDebug() << curr_version;
        if(curr_version==latestVersion)
        {
            QMessageBox box;
            box.setText("Software is already on the latest version");
            box.exec();
        }
        else{
            QMessageBox msg;
            msg.setWindowTitle("Update Available");
            msg.setIcon(QMessageBox::Information);
            msg.setText("A New Version of Udaan Editing Tool is Available!!\n\nUdaan Editing Tool "+latestVersion+"\nTo Download the latest version of this software click 'Go to Download Page' button below\nWhat's New:-\n\n" + newFeatures);
            QAbstractButton *download = msg.addButton(tr("Go to Download Page"), QMessageBox::ActionRole);
            download->setMinimumWidth(160);
            QAbstractButton *rml = msg.addButton(tr("Later"), QMessageBox::RejectRole);
            rml->setMaximumWidth(80);
            msg.exec();

            if(msg.clickedButton() == download){
                QDesktopServices::openUrl(QUrl("https://drive.google.com/drive/folders/1DZn72n6gH0r459hTGsL2f7qhoZnHQPEI"));
            }
            else {
                msg.close();
            }
        }
    }
}

/*!
 * \brief MainWindow::on_find_clicked
 */
void MainWindow::on_find_clicked()
{
    QRegExp searchExpr = QRegExp(ui->lineEdit_4->text());
    searchExpr.setCaseSensitivity(Qt::CaseInsensitive);
    QTextCursor cursor = ui->textEdit_dict->textCursor();
    int pos1=cursor.position();
    //qDebug()<<pos1;

    if(ui->textEdit_dict->find(searchExpr, QTextDocument::FindFlags()))
    {
        QPalette p = ui->textEdit_dict->palette();
        p.setColor(QPalette::Highlight, QColor(Qt::yellow));
        p.setColor(QPalette::HighlightedText, QColor(Qt::black));
        ui->textEdit_dict->setPalette(p);
        //ui->textEdit_dict->moveCursor(QTextCursor::EndOfWord);

    }
    else
    {
        if (pos1==0)
        {
            QMessageBox::warning(0,"Error","No such Word Found");
            // qDebug()<<"Word Not found";
        }
        else
        {
            ui->textEdit_dict->moveCursor(QTextCursor::Start);
            // qDebug()<<"Moving to start of FIle";
        }
    }
}


/*!
 * \brief MainWindow::on_actionPDF_Preview_triggered
 */
void MainWindow::on_actionPDF_Preview_triggered()
{
    QPrinter printer(QPrinter::PrinterResolution);
    QPrintPreviewDialog preview(&printer,this);
    preview.setMinimumHeight(800);
    preview.setMinimumWidth(800);
    connect(&preview, SIGNAL(paintRequested(QPrinter*)),this, SLOT(print(QPrinter*)));
    preview.exec();
}

/*!
 * \brief MainWindow::print
 * \param printer
 */
void MainWindow::print(QPrinter *printer)
{
    QTextDocument *document = new QTextDocument();
    QString htmlFile = gDirTwoLevelUp + "/"+gCurrentDirName+"/"+currentTabPageName;

    QString html_contents="";
    QString mainHtml ;
    int startFrom,stIndex = 0;

    //! Set the background of the pdf to be printed to be white
    QString searchString = "background-color:#";
    int l = searchString.length();
    QString whiteColor = "ffffff";

    startFrom = 0;

    QFile file(htmlFile);
    if (!file.open(QIODevice::ReadOnly)) qDebug() << "Error reading file main.html";
    QTextStream stream(&file);
    stream.setCodec("UTF-8");

    //! Read the file
    mainHtml=stream.readAll();

    //! Changing the text background to white by setting the background to #fffff
    while (true){
        stIndex = mainHtml.indexOf(searchString, startFrom);
        if (stIndex == -1)
            break;
        stIndex += l; // increment line
        mainHtml.replace(stIndex, 6, whiteColor); // Here, 6 is used because length of whiteColor is 6
        startFrom = stIndex + 6;
    }
    //latex to png mapping
    if(mainHtml.contains("$$")){

        QRegularExpression rex_lat("<a(.*?)</a>",QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatchIterator itr_lat;
        itr_lat = rex_lat.globalMatch(mainHtml);
        while(itr_lat.hasNext()){

            QRegularExpressionMatch match = itr_lat.next();
            QString text = match.captured(1);

            if(text.contains("Equations_"))
            {
                int sindex = match.capturedStart(1);
                int l_index = match.capturedEnd(1);
                std::string inputText_ = text.toStdString();
                int ind = inputText_.find("/");
                int lindex = inputText_.find(".tex");

                std::string str = inputText_.substr(ind,lindex-ind);
                QString path = QString::fromStdString(str) + ".png";
                QString html = "<img src=\""+path+"\">";
                text = "<a"+text+"</a>";
                mainHtml.replace(text,html);
                qDebug()<<text<<html;
            }

        }
        mainHtml = mainHtml.replace("$$","dne_nqe"); //where dne_nqe is a random string used as end delimiter here.
        //Note that this string should not appear as an original text - else it will cause parsing issues.
        QRegularExpression rex_dollar("dne_nqe(.*?)dne_nqe",QRegularExpression::DotMatchesEverythingOption);
        mainHtml = mainHtml.remove(rex_dollar);
    }

    file.close();
    html_contents.append(mainHtml);
    document->setHtml(html_contents);
    document->print(printer);
    //curr_browser->print(printer);
}

/*!
 *fn  MainWindow::on_actionChange_Role_triggered
 * \brief Changes the role of the user.
 *  provides application settings
 *   prefix of the current group is set to(SETROLE)
 */
void MainWindow::on_actionChange_Role_triggered()
{
    QSettings settings("IIT-B", "OpenOCRCorrect");
    settings.beginGroup("Set Role");
    settings.remove("");
    settings.endGroup();
}

//this function then re-introduces bbox's to saved file
void MainWindow::bboxInsertion(QFile *f){
    QFile *file = f;

    if (!file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "File not opened for reading";
        return;
    }

    QTextStream in(file);
    in.setCodec("UTF-8");
    QString text = in.readAll();
    file->close();

    QRegularExpression rex("<p(.*?)</p>",QRegularExpression::DotMatchesEverythingOption);
    QRegularExpression rex_("<span(.*?)</span>",QRegularExpression::DotMatchesEverythingOption);

    QRegularExpression rex2("(<p[^>]*>|<span[^>]*>)");

    QString bboxf = currentTabPageName;
    QFile bbox_file(gDirTwoLevelUp + "/bboxf/"+bboxf.replace(".html", ".bbox"));
    if(bbox_file.exists())
    {
        QRegularExpressionMatchIterator itr,itr_;
        itr = rex.globalMatch(text);
        itr_ = rex_.globalMatch(text);
        if (!bbox_file.open(QIODevice::ReadOnly)) {
            qDebug() << "Unable to open bbox_file!";
            return;
        }

        QDataStream in_ (&bbox_file);
        in_.setVersion(QDataStream::Qt_5_3);
        //QDataStream out (&bbox_file);
        //out.setVersion(QDataStream::Qt_5_3);
        QMap<QString,QString> coordinates;

        in_ >> coordinates;
        bbox_file.close();
        QString bbox_coordinates;
        QStringList bbox_list,bbox_list_;
        QMap<QString, QString>::iterator ci;
        edit_Distance edit;
        //qDebug()<<"in_ : map="<<in_<<":"<<coordinates;
        QRegularExpression rex3("(<[^>]*>|[^>]*>)");
        while (itr.hasNext())
        {
            QRegularExpressionMatch match = itr.next();
            QString ex = match.captured(1);

            ex.remove(rex3);//ex.remove("</p>");ex.remove("</span>");
            //qDebug()<<"text now = "<<ex;
            double max = 0;
            for(ci = coordinates.begin(); ci!=coordinates.end(); ++ci)
            {
                double similarity = edit.getSimilarityValue(ex.toStdString(), ci.value().toStdString());
                if(similarity>max)
                {
                    bbox_coordinates = ci.key();
                    max = similarity;
                }
            }
            bbox_coordinates.remove("\">");
            if(bbox_coordinates != "")
            bbox_list.append(bbox_coordinates);
        }
        //itr_ is for span tags
        while (itr_.hasNext())
        {
            QRegularExpressionMatch match_ = itr_.next();
            QString ex_ = match_.captured(1);
            ex_.remove(rex3);//ex_.remove("</p>");ex_.remove("</span>");
            //qDebug()<<"for spans text = "<<ex_;
            double max = 0;
            for(ci = coordinates.begin(); ci!=coordinates.end(); ++ci)
            {
                double similarity = edit.DiceMatch(ex_.toStdString(), ci.value().toStdString());

                //qDebug()<<"| matching above with ::"<<ci.value()<<" Similarity = "<<similarity;
                // qDebug()<<"similarity="<<;
                if(similarity>max)
                {
                    bbox_coordinates = ci.key();
                    max = similarity;
                }
            }
           // qDebug() <<"max similarity is = "<<max;
            bbox_coordinates.remove("\">");
            if(bbox_coordinates != "")
            bbox_list_.append(bbox_coordinates);
        }
        //now just insert the bbox coordinates into the file saved
        QRegularExpressionMatchIterator itr2;
        itr2 = rex2.globalMatch(text,0);
        int i=0,j=0;
        //qDebug()<<"bbox_list="<<bbox_list;
        if(bbox_list.size() == 0 && bbox_list_.size() == 0){
            return;
        }
        while (itr2.hasNext()) {
            QRegularExpressionMatch match2 = itr2.next();
            QString ex = match2.captured(1);
            int endIndex = match2.capturedEnd(1);
            if((ex[1] == "p" || ex[1] == "P") && i<bbox_list.size()){
                endIndex = endIndex-1;
                text.insert(endIndex," title=\""+bbox_list[i]+"\"");//index+1
                i++;
            }
            else if((ex[1] == "s" || ex[1] == "S") && j<bbox_list_.size()){
                endIndex = endIndex-1;
                text.insert(endIndex," title=\""+bbox_list_[j]+"\"");
                j++;
            }

            itr2 = rex2.globalMatch(text,endIndex+1);

        }
        if(file->open(QFile::WriteOnly))
        {
            QTextStream out2(file);
            out2.setCodec("UTF-8");          //!Sets the codec for this stream
            //text = "<style> body{ width: 21cm; height: 29.7cm; margin: 30mm 45mm 30mm 45mm; } </style>" + text;     //Formatting the output using CSS <style> tag
            out2 << text;
            file->flush();      //!Flushes any buffered data waiting to be written in the \a sFile
            file->close();      //!Closing the file
        }
    bbox_list.clear();bbox_list_.clear();
    }

}
/*!
 *fn  MainWindow::finishedPdfCreation
 * param int->exitCode
 * QProcess::ExitStatus->exitStatus
 * \brief Function is called When PDF Creation is successfull.
 * Comapres exitcode to zero(0).If found True message is set to(PDF created Successfully)
 * if exit code is EQUAL to -1,255,9 then PDF creation was Stopped by user.
 * if other cases failed then PDF creation was unsuccessfull.
 * File is Closed.
 * Message Box of Title is Previewed.
 */
void MainWindow::finishedPdfCreation(int exitCode, QProcess::ExitStatus exitStatus)
{
    QString msg, title;

    if (exitCode == 0) {
        qDebug() << "PDF created Successfully";
        title = "Success";
        msg = "PDF created Successfully";
    } else if (exitCode == -1 || exitCode == 255 || exitCode == 9) {
        qDebug() << "User cancelled PDF creation";
        title = "Cancelled";
        msg = "PDF creation cancelled";
    } else {
        qDebug() << "PDF creation failed";
        title = "Error";
        msg = "Error in creating PDF";
    }

    // Closing the dialog box shown after the PDF dialog box is ready
    tempMsgBox->close();

    QFile file(toolDirAbsolutePath + "/.html_for_pdf.html");
    if (file.exists()) {
        file.remove();
    }
//    stopSpinning();

    qDebug() << "Exit code is " << QString::number(exitCode);
    if (title != "Error") {
        QMessageBox::information(this, title, msg, QMessageBox::Ok, QMessageBox::Ok);
    } else {
        QMessageBox::warning(this, title, msg, QMessageBox::Ok, QMessageBox::Ok);
    }
}
/*!
 *fn  MainWindow::pdfPrintIsReady()
 * \brief Pdf Creation is in the Process
 * Loading Spinner is Stopped
 * Users are given choice wheather to close the dialog box or wait.
 * Window Title is set to Wait.
 * CLose option is Added in Push Button.
 * if close buttton is clicked then Print PDF process is called of.
 */
void MainWindow::pdfPrintIsReady()
{
    stopSpinning();
    QString title = "Wait";
    QString msg = "Print PDF dialog box is opened.\nDeal with it OR if you want to close it, click the close button";
    QMessageBox msgBox(this);
    tempMsgBox = &msgBox;
    msgBox.setWindowTitle(title);
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setInformativeText(msg);
    QPushButton *closeButton = msgBox.addButton("Close", QMessageBox::AcceptRole);
    msgBox.exec();

    if (msgBox.clickedButton() == closeButton) {
        // close the print dialog box
        mPrintPdfProcess->close();
    }
}
/*!
 *fn  MainWindow::readOutputFromPdfPrint()
 * \brief OutPut from Pdf is read.
 * if Check String is Ready the  pdfPrintIsReady function is called.
 */
void MainWindow::readOutputFromPdfPrint()
{
    QByteArray data = mPrintPdfProcess->readAllStandardOutput();
    QString checkString;
#ifdef Q_OS_WIN
    checkString = "Ready\r\n";
#else
    checkString = "Ready\n";
#endif
    if (QString(data) == checkString) {
        pdfPrintIsReady();
    }
}

/*!
 *fn  MainWindow::SearchOnGoogle()
 * \brief selected text is searched over google to find it's meaning
 */
void MainWindow::SearchOnGoogle()
{
    QTextCursor cursor = curr_browser->textCursor();
    QString str = cursor.selectedText();
    QDesktopServices::openUrl(QUrl("https://www.google.com/search?q="+str, QUrl::TolerantMode));
}


/*!
 *fn  MainWindow::GoogleTranslation()
 * \brief selected Text translation is searched over google.
 */
void MainWindow::GoogleTranslation()
{
    QTextCursor cursor = curr_browser->textCursor();
    QString str = cursor.selectedText();
    QDesktopServices::openUrl(QUrl("https://translate.google.co.in/?sl=auto&tl=en&text=" +str+ "&op=translate", QUrl::TolerantMode));
}

/*!
 *fn  MainWindow::insertImageAction()
 * \brief Image is inserted in the opened file.
 * File Dialog is opened,giving user choice to choose the images from their PC.
 * if image file is empty then fucntion terminates.
 * File information is stored(File Path)
 * Image height and Width is set.
 */
void MainWindow::insertImageAction()
{
    //qDebug()<<"Image Will Be Inserted"<<endl;
    QString imgFileName;
    QString imgFile = QFileDialog::getOpenFileName(this, "Select an Image");
    if(imgFile.isEmpty())
        return;

    QFileInfo imgFileInfo(imgFile);
    imgFileName = imgFileInfo.fileName();
    QString imgFilePath = imgFileInfo.filePath();

    QString copiedImgFilePath(gDirTwoLevelUp + "/Inserted_Images/"+imgFileName);
    if(!QDir(gDirTwoLevelUp + "/Inserted_Images").exists())
            QDir().mkdir(gDirTwoLevelUp + "/Inserted_Images");
    QFile::copy(imgFilePath,copiedImgFilePath);
     qDebug()<<imgFilePath<<"\n"<<copiedImgFilePath;
    int height =0;
    int width = 0;
    QDialog dialog(this);
    QFormLayout form(&dialog);

    form.addRow(new QLabel("Insert Height and Width",this));

    QLineEdit *height_textLine= new QLineEdit(&dialog);
     QLineEdit *width_textLine= new QLineEdit(&dialog);

     form.addRow("Height",height_textLine);
       form.addRow("Width",width_textLine);

       QDialogButtonBox buttonbox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,Qt::Horizontal,&dialog);
       form.addRow(&buttonbox);

       QObject::connect(&buttonbox,SIGNAL(accepted()),&dialog,SLOT(accept()));
       QObject::connect(&buttonbox,SIGNAL(rejected()),&dialog,SLOT(reject()));

       if(dialog.exec() ==QDialog::Accepted){
           height=height_textLine->text().toInt();
           width=width_textLine->text().toInt();
       }



   //setting width
//    int n = QInputDialog::getInt(this, "Set Width","Width",width,-2147483647,2147483647,1);
//    //!setting height
//    int n1 = QInputDialog::getInt(this, "Set Height","height",height,-2147483647,2147483647,1);



    QString html = QString("\n <img src='%1' width='%2' height='%3'>").arg(copiedImgFilePath).arg(width).arg(height);
    auto cursor = curr_browser->textCursor();
    cursor.insertHtml(html);

}


/*!
 *fn  MainWindow::createMenu()
 * \brief Menu is Created
 */
void MainWindow::createMenu()
{
    QAction *exitAction = new QAction(tr("Exit"), this);

   connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);

    QMenu *fileMenu = menuBar()->addMenu(tr("File"));
    fileMenu->addAction(exitAction);
}

void MainWindow::blockCountChanged(int numOfBlocks)
{
    QTextCursor cur = curr_browser->textCursor();
    int currentBlockNum = cur.blockNumber();
    QPair<QString, QString> bbox_value;

    if (numOfBlocks > handleBbox->blockCount) {
        if (currentBlockNum == 0) {
            bbox_value = bboxes[0];
            for (int i = 0; i < (numOfBlocks - blockCount); i++) {
                bboxes.insert(currentBlockNum, {"p", bbox_value.second});
            }
        } else {
            int pos = currentBlockNum - (numOfBlocks - blockCount);
            bbox_value = bboxes[pos];
            bbox_value.first = "p";
            for (int i = 0; i < (numOfBlocks - blockCount); i++) {
                bboxes.insert(pos + 1, bbox_value);
            }
        }
    } else if (numOfBlocks < blockCount) {
        int pos;
        if (cur.atBlockEnd()) {
            pos = currentBlockNum + 1;
        } else if (cur.atBlockStart()) {
            pos = currentBlockNum;
        }

        for (int i = 0; i < (blockCount - numOfBlocks); i++) {
            bboxes.remove(pos);
        }
    }
    blockCount = numOfBlocks;
}


/*!
 *fn  MainWindow::on_actionLogin_triggered()
 * \brief authenticate function is called.
 * user is reDirected to Google login page
 */

void MainWindow::on_actionLogin_triggered()
{
    authenticate();
}

/*!
 *fn  MainWindow::on_actionLogout_triggered()
 * \brief Function is executed when user logouts.
 * Logout button is not visible
 * login button is previewed so that user can login
 *
 */
void MainWindow::on_actionLogout_triggered()
{
    QSettings settings("IIT-B", "OpenOCRCorrect");
    ui->actionLogout->setVisible(false);
    ui->actionLogin->setVisible(true);
    //QDesktopServices::openUrl(QUrl("https://myaccount.google.com/permissions?continue=https%3A%2F%2Fmyaccount.google.com%2Fsecurity", QUrl::TolerantMode));
    settings.beginGroup("login");
    QString token = settings.value("token").toString();
    settings.endGroup();
    QProcess process;
   //qDebug()<<"curl -d -X -POST --header \"Content-type:application/x-www-form-urlencoded\" https://oauth2.googleapis.com/revoke?token="+token;
    process.execute("curl -d -X -POST --header \"Content-type:application/x-www-form-urlencoded\" https://oauth2.googleapis.com/revoke?token="+token);
    settings.beginGroup("loginConsent");
    settings.remove("");
    settings.endGroup();
    settings.beginGroup("login");
    settings.remove("");
    settings.endGroup();

}
/*!
 *fn  MainWindow::on_actionClone_Repository_triggered()
 * \brief This function helps user to clone repositories from their account.
 * Checks if user is already logined or not. If not then user is made to login
 * POST request is send to udaaniitb.aicte-india.org to get the files.
 * Json Object File is read and converted to Json Array.
 * "validate.json" is removed from output
 * Table is formed to preview how many file does user have in their repository
 */
void MainWindow::on_actionClone_Repository_triggered()
{
    /*
     * \description
     * Checks whether user is logged in or not
    */
    QSettings settings("IIT-B", "OpenOCRCorrect");
    settings.beginGroup("loginConsent");
    QString value1 = settings.value("consent").toString();
    settings.endGroup();
    if(value1 != "loggedIn"){
        QMessageBox msg;
        msg.setText("Please login to go to your dashboard");
        int cnt = 2;
        //showing the message box for 2 seconds only.
        QTimer cntDown;
        QObject::connect(&cntDown, &QTimer::timeout, [&msg,&cnt, &cntDown]()->void{
             if(--cnt < 0){
                 cntDown.stop();
                 msg.close();
             }
            });
        cntDown.start(1000);
        msg.exec();
        return;
    }
    //
    //retrieve details from database and check if user has access to push into this repo
    settings.beginGroup("login");
    QString email = settings.value("email").toString();
    settings.endGroup();
    QProcess process;
    process.execute("curl -d -X -k -POST --header "
                    "\"Content-type:application/x-www-form-urlencoded\" https://udaaniitb.aicte-india.org/udaan/email/ -d \"email="+email+"\" -o validate.json");

    QJsonObject mainObj = readJsonFile("validate.json");
    QJsonArray repos = mainObj.value("repo_list").toArray();
    QFile::remove("validate.json");
    if(repos.size() == 0){
        QMessageBox msg;
        msg.setText("There is nothing to show on dashboard");
        msg.exec();
        return;
    }
    QJsonArray::iterator itr; int flag = 0;
    int lineindex = 0;
    QString importHtml="<table><tr><th>#Project ID</th><th>#Project name</th></tr>";
    QStandardItemModel *model = new QStandardItemModel;
    QMap<int, QString> repoMap;
    for(itr = repos.begin(); itr != repos.end(); itr++){
        lineindex++;
        repoMap[lineindex] = itr->toString();
        QString num = QString::number(lineindex);
        importHtml += QString::fromStdString("<tr><td>")+num+"</td><td>"+itr->toString()+"</td></tr>";
    }
    importHtml += "</table>";
    dashboard d(this, importHtml, repos.size(), repoMap);
    d.exec();
}



/*!
 *fn  MainWindow::on_actionClose_project_triggered()
 * \brief Closes the current opened Project
 * Checks wheather if projected is opened or not.If not Function terminates
 * Clears Tree view,Current browser,graphicsView.
 * All buttons which were enabled during the Open project are closed,Like
 * Zoom,Compare Corrector output,save etc.
 * Dictornary is cleared
 */
void MainWindow::on_actionClose_project_triggered()
{
    if(!mProject.isProjectOpen()){
//        QMessageBox::critical(this,"Error","Project Not Opened");
        return;                                                                  //checking if the project is already
        // opened or not
    }
    mProject.setProjectOpen(false);
    ui->actionLoadDict->setVisible(false);
    ui->actionLoadOCRWords->setVisible(false);
    ui->actionLoadDomain->setVisible(false);
    ui->actionLoadSubPS->setVisible(false);
    ui->actionLoadConfusions->setVisible(false);
    ui->actionLoadGDocPage->setVisible(false);
    ui->menuSelectLanguage->setTitle("");
    ui->menuCreateReports->setTitle("");

    //disableing the buttons after project is closed
    // File Menu
    ui->actionSave->setEnabled(false);
    ui->actionSave_As->setEnabled(false);
    ui->actionSpell_Check->setEnabled(false);
    ui->actionLoad_Prev_Page->setEnabled(false);
    ui->actionLoad_Next_Page->setEnabled(false);
    ui->actionToDevanagari->setEnabled(false);
    ui->actionToSlp1->setEnabled(false);
    ui->actionLoadGDocPage->setEnabled(false);
    ui->actionLoadData->setEnabled(false);
    ui->actionLoadDict->setEnabled(false);
    ui->actionLoadOCRWords->setEnabled(false);
    ui->actionLoadDomain->setEnabled(false);
    ui->actionLoadSubPS->setEnabled(false);
    ui->actionLoadConfusions->setEnabled(false);
    ui->actionSugg->setEnabled(false);

    // Edit Menu
    ui->actionUndo->setEnabled(false);
    ui->actionRedo->setEnabled(false);
    ui->actionFind_and_Replace->setEnabled(false);
    ui->actionUndo_Global_Replace->setEnabled(false);
    ui->actionUpload->setEnabled(false);

    // Language Menu
    ui->actionSanskrit_2->setEnabled(false);
    ui->actionEnglish->setEnabled(false);
    ui->actionHindi->setEnabled(false);

    // Reports Menu
    ui->actionAccuracyLog->setEnabled(false);
    ui->actionViewAverageAccuracies->setEnabled(false);

    // View Menu
    ui->actionAllFontProperties->setEnabled(false);
    ui->actionBold->setEnabled(false);
    ui->actionItalic->setEnabled(false);
    ui->actionLeftAlign->setEnabled(false);
    ui->actionRightAlign->setEnabled(false);
    ui->actionCentreAlign->setEnabled(false);
    ui->actionJusitfiedAlign->setEnabled(false);
    ui->actionSuperscript->setEnabled(false);
    ui->actionSubscript->setEnabled(false);
    ui->actionInsert_Horizontal_Line->setEnabled(false);
    ui->actionFontBlack->setEnabled(false);
    ui->actionInsert_Tab_Space->setEnabled(false);
    ui->actionPDF_Preview->setEnabled(false);
    if (isVerifier)
        ui->actionHighlight->setEnabled(false);

    // Table Menu inside View Menu
    ui->actionInsert_Table_2->setEnabled(false);
    ui->actionInsert_Columnleft->setEnabled(false);
    ui->actionInsert_Columnright->setEnabled(false);
    ui->actionInsert_Rowabove->setEnabled(false);
    ui->actionInsert_Rowbelow->setEnabled(false);
    ui->actionRemove_Column->setEnabled(false);
    ui->actionRemove_Row->setEnabled(false);

    // Versions Menu
    ui->actionFetch_2->setEnabled(false);
    ui->actionTurn_In->setEnabled(false);
    ui->actionVerifier_Turn_In->setEnabled(false);

    // Download Menu
    ui->actionas_PDF->setEnabled(false);

    ui->actionSymbols->setEnabled(false);
    ui->actionZoom_In->setEnabled(false);
    ui->actionZoom_Out->setEnabled(false);
    //Reset loadData flag
    LoadDataFlag = 1;
    //reset data
    mFilename.clear();
    mFilename1.clear();
    // mFile.clear();
    LSTM.clear();
    CPairs.clear();
    Dict.clear();
    GBook.clear();
    IBook.clear();
    PWords.clear();
    ConfPmap.clear();
    vGBook.clear();
    vIBook.clear();
    TDict.clear();
    TGBook.clear();
    TGBookP.clear();
    TPWords.clear();
    TPWordsP.clear();
    synonym.clear();
    synrows.clear();

    if(ui->lineEdit_3->text()!="" && ui->lineEdit_3->text()!="Words 0" && ui->lineEdit_3->text()!="0 Words"){
        curr_browser->clear();
    }
    ui->treeView->setModel(nullptr);
    ui->graphicsView->setScene(nullptr);
    ui->lineEdit_2->clear();
    ui->lineEdit->clear();
    ui->lineEdit_3->clear();
    ui->pushButton->setDisabled(true);
    ui->pushButton_2->setDisabled(true);
    ui->viewComments->setDisabled(true);
    ui->compareCorrectorOutput->setDisabled(true);
    ui->groupBox->setDisabled(true);
//    QMessageBox::information(this,"Success","Project Closed Successfully");
    curr_browser=0;
}

/*!
 * \fn MainWindow::on_actionMerge_Cells_triggered
 * \brief Merges the cells selected in the table.
 * \details Tables are manipulated using QTextTable module from Qt.
 */
void MainWindow::on_actionMerge_Cells_triggered()
{
    if (!curr_browser || curr_browser->isReadOnly()) { return; }
    if (curr_browser->textCursor().currentTable())
    {
        QTextTable *table = curr_browser->textCursor().currentTable();
        table->mergeCells(curr_browser->textCursor());
    }
}

/*!
 * \fn MainWindow::on_actionSplit_Cell_triggered
 * \brief Splits the current cell into the rows and columns specified by the user.
 */
void MainWindow::on_actionSplit_Cell_triggered()
{
    if (!curr_browser || curr_browser->isReadOnly()) { return; }
    if (curr_browser->textCursor().currentTable())
    {
        QDialog dialog(this);
        QFormLayout form(&dialog);      // Use a layout allowing to have a label next to each field
        form.addRow(new QLabel("Split Cells", this));

        //! Add the lineEdits with their respective labels
        QLineEdit *rows = new QLineEdit(&dialog);
        QLineEdit *columns = new QLineEdit(&dialog);                                    // Add lineEdits to get Rows
        form.addRow("Rows", rows);                                                      // Add lineEdits to get Columns
        form.addRow("Columns", columns);

        //! Add some standard buttons (Cancel/Ok) at the bottom of the dialog
        QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog); // Add some standard buttons (Cancel/Ok) at the bottom of the dialog
        form.addRow(&buttonBox);
        QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
        QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

        if (dialog.exec() == QDialog::Accepted) {
            QTextTable *table = curr_browser->textCursor().currentTable();
            QTextTableCell currentCell = table->cellAt(curr_browser->textCursor());
            table->splitCell(currentCell.row(), currentCell.column(), rows->text().toInt(), columns->text().toInt());
        }
    }
}

/*!
 * \fn MainWindow::on_actionInsert_Bulleted_List_triggered
 * \brief Adds the bulleted list to the text browser
 */
void MainWindow::on_actionInsert_Bulleted_List_triggered()
{
    QDialog dialog(this);
    QFormLayout form(&dialog);      // Use a layout allowing to have a label next to each field

    //! Add the lineEdits with their respective labels
    QGroupBox *groupBox = new QGroupBox(tr("Bulleted Lists"));
    QRadioButton *radioBtn1 = new QRadioButton("List Disc");
    QRadioButton *radioBtn2 = new QRadioButton("List Circle");
    QRadioButton *radioBtn3 = new QRadioButton("List Square");
    QRadioButton *radioBtn4 = new QRadioButton("List Decimal");

    radioBtn1->setChecked(true);
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(radioBtn1);
    vbox->addWidget(radioBtn2);
    vbox->addWidget(radioBtn3);
    vbox->addWidget(radioBtn4);
    vbox->addStretch(1);
    groupBox->setLayout(vbox);

    form.addRow(groupBox);

    //! Add some standard buttons (Cancel/Ok) at the bottom of the dialog
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog); // Add some standard buttons (Cancel/Ok) at the bottom of the dialog
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    //! Show the dialog as modal
    if (dialog.exec() == QDialog::Accepted)
    {
        if (radioBtn1->isChecked()) {
            insertList(QTextListFormat::ListDisc);
        } else if (radioBtn2->isChecked()) {
            insertList(QTextListFormat::ListCircle);
        } else if (radioBtn3->isChecked()) {
            insertList(QTextListFormat::ListSquare);
        } else if (radioBtn4->isChecked()) {
            insertList(QTextListFormat::ListDecimal);
        }
    }
}

/*!
 * \fn MainWindow::on_actionInsert_Numbered_List_triggered
 * \brief Adds the Numbered list to the text browser
 */
void MainWindow::on_actionInsert_Numbered_List_triggered()
{
    QDialog dialog(this);
    QFormLayout form(&dialog);      // Use a layout allowing to have a label next to each field

    //! Add the lineEdits with their respective labels
    QGroupBox *groupBox = new QGroupBox(tr("Bulleted Lists"));
    QRadioButton *radioBtn1 = new QRadioButton("List Lower Alpha");
    QRadioButton *radioBtn2 = new QRadioButton("List Upper Alpha");
    QRadioButton *radioBtn3 = new QRadioButton("List Lower Roman");
    QRadioButton *radioBtn4 = new QRadioButton("List Upper Roman");

    radioBtn1->setChecked(true);
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(radioBtn1);
    vbox->addWidget(radioBtn2);
    vbox->addWidget(radioBtn3);
    vbox->addWidget(radioBtn4);
    vbox->addStretch(1);
    groupBox->setLayout(vbox);

    form.addRow(groupBox);

    //! Add some standard buttons (Cancel/Ok) at the bottom of the dialog
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog); // Add some standard buttons (Cancel/Ok) at the bottom of the dialog
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    //! Show the dialog as modal
    if (dialog.exec() == QDialog::Accepted)
    {
        if (radioBtn1->isChecked()) {
            insertList(QTextListFormat::ListLowerAlpha);
        } else if (radioBtn2->isChecked()) {
            insertList(QTextListFormat::ListUpperAlpha);
        } else if (radioBtn3->isChecked()) {
            insertList(QTextListFormat::ListLowerRoman);
        } else if (radioBtn4->isChecked()) {
            insertList(QTextListFormat::ListUpperRoman);
        }
    }
}

/*!
 * \fn MainWindow::insertList
 * \brief Acts as a helper for adding ordered and unordered lists
 * \details Inserts the list according to the styleIndex provided
 * \param styleIndex
 */
void MainWindow::insertList(QTextListFormat::Style styleIndex)
{
    QTextCursor cursor(curr_browser->textCursor());
    cursor.beginEditBlock();
    QTextBlockFormat blockFmt = cursor.blockFormat();
    QTextListFormat listFmt;

    if (cursor.currentList()) {
        listFmt = cursor.currentList()->format();
    } else {
        listFmt.setIndent(blockFmt.indent() + 1);
        blockFmt.setIndent(0);
        cursor.setBlockFormat(blockFmt);
    }

    listFmt.setStyle(styleIndex);
    cursor.createList(listFmt);
    cursor.endEditBlock();
}


void MainWindow::on_actionInsert_Equation_triggered()
{
    if(!mProject.isProjectOpen()){
                 QMessageBox::critical(this,"Error","Please open the project first");
                 return;                                                                  //checking if the project is already
        }
        if(gCurrentPageName.isEmpty()){
            QMessageBox::critical(this,"Error","Please open the html file first");
            return;
        }
        equationeditor *w = new equationeditor(this,gDirTwoLevelUp,curr_browser,"0");
        w->show();
}


void MainWindow::on_actionEdit_Equation_triggered()
{
    if(!curr_browser) return;

    auto cursor = curr_browser->textCursor();
    auto selected = cursor.selection();
    QString sel = selected.toHtml();

    if(!sel.contains("<img") || !sel.contains(".png") || !sel.contains("Equations_")){
        QMessageBox::critical(this,"Error","Please select an equation to edit.");
        return;
    }
    QString path;
    QRegularExpression rex("<img(.*?)>",QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatchIterator itr;
    itr = rex.globalMatch(sel);
    if(itr.hasNext()){
    QRegularExpressionMatch match = itr.next();
    QString img = match.captured();
    string img_ = img.toStdString();
    int ind = img_.find("/");
    int lindex = img_.find("png");
    string str = img_.substr(ind, lindex-ind);
    path = QString::fromStdString(str) + "txt";
    }
    equationeditor *w = new equationeditor(this,gDirTwoLevelUp,curr_browser,path);
    w->show();
}

/*!
 *fn  MainWindow::on_actionExit_triggered()
 * \brief Application is exited and Closed.
 */
void MainWindow::on_actionExit_triggered()
{
    QCoreApplication::quit();
}

