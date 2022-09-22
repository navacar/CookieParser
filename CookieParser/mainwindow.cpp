#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include<QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , settingsWindow(nullptr)
    , settingsWindowButton(nullptr)
    , isDarkMode(false)
    , isFileDialogOpened(false)
    , cookieCount(0)
{
    ui->setupUi(this);
    setFixedSize(800, 600);

    connect(ui->settingsButton, &QPushButton::clicked, this, &MainWindow::onSettingsButton);
    connect(ui->uploadButton, &QPushButton::clicked, this, &MainWindow::onUploadButton);

    setModeFromFile();
    configureSettingWindow();
    changeMode();
}

MainWindow::~MainWindow()
{
    delete settingsWindowButton;
    settingsWindowButton = nullptr;

    delete settingsWindow;
    settingsWindow = nullptr;

    delete ui;
}

void MainWindow::closeEvent (QCloseEvent *event)
{
    this->~MainWindow();
}

QString MainWindow::formatFileName(const QString &fileName)
{
    QString name;
    size_t i = fileName.size() - 1;

    while (fileName[i] != '/' && fileName[i] != '\\')
    {
        name = fileName[i] + name;
        i--;
    }

    return name;
}

void MainWindow::onUploadButton()
{
    settingsWindow->hide();

    QFileDialog* dialog = new QFileDialog(ui->centralwidget);
    dialog->exec();

    if (!dialog->selectedFiles().isEmpty())
    {
        QString fileName = dialog->selectedFiles().first();
        QFile file(fileName);
        file.open(QIODevice::ReadOnly);

        int size = fileNames.size();
        fileNames << fileName;

        if (fileNames.size() > size)
        {
            QString name = formatFileName(fileName);
            ui->filesNames->addItem(name);
            QString data = QString(file.readAll());
            workWithData(name, data);
        }
        else
        {
            QMessageBox::information(this, "Ошибка", "Такой файл уже используеться \nвыберите другой");
        }
    }

    delete dialog;
    dialog = nullptr;
}

void MainWindow::workWithData(QString &fileName, QString &data)
{
    QJsonDocument json = QJsonDocument::fromJson(data.toUtf8());
    QSet<QString> domainsName;
    QVector<Cookie> cookies;
    cookies.reserve(json.array().size());

    for (int i = 0; i < json.array().size(); i++)
    {
        QJsonObject object = json[i].toObject();
        Cookie cookie;

        cookie.domain = object.value(QString("domain")).toString();
        domainsName << cookie.domain;

        cookie.isHttpOnly = object.value(QString("httpOnly")).toBool() ? "yes" : "no";
        cookie.isSecure = object.value(QString("secure")).toBool() ? "yes" : "no";

        cookie.path = object.value(QString("path")).toString();
        cookie.val = object.value(QString("value")).toString();
        cookie.name = object.value(QString("name")).toString();

        cookie.timestamp = object.value(QString("expirationDate")).toInt();

        cookies.append(cookie);
    }

    createCookieTree(fileName, cookies, domainsName);
}

void MainWindow::createCookieTree(QString &fileName, QVector<Cookie> &cookies, QSet<QString> &domainsName)
{
    cookieCount += cookies.size();
    ui->cookieCount->setText(QString("Колличество \nCookie: %1").arg(cookieCount));

    QTreeWidgetItem *root = new QTreeWidgetItem(ui->treeWidget);
    root->setText(0, fileName);

    for (int i = 0; i < domainsName.size(); i++)
    {
        QTreeWidgetItem *domain = new QTreeWidgetItem();
        QString name = domainsName.values()[i];
        domain->setText(0, name);

        for (int j = 0; j < cookies.size(); j++)
        {
            if (cookies[j].domain == name)
            {
                QTreeWidgetItem* name = new QTreeWidgetItem();
                name->setText(0, cookies[j].name);

                QTreeWidgetItem* val = new QTreeWidgetItem();
                val->setText(0, "val: " + cookies[j].val);
                name->addChild(val);

                QTreeWidgetItem* path = new QTreeWidgetItem();
                path->setText(0, "path: " + cookies[j].path);
                name->addChild(path);

                QDateTime timestamp;
                timestamp.setSecsSinceEpoch(cookies[j].timestamp);
                QTreeWidgetItem* expire = new QTreeWidgetItem();
                expire->setText(0, "Expires: " + timestamp.toString());
                name->addChild(expire);

                QTreeWidgetItem* isHttpOnly = new QTreeWidgetItem();
                isHttpOnly->setText(0, "isHttpOnly: " + cookies[j].isHttpOnly);
                name->addChild(isHttpOnly);

                QTreeWidgetItem* isSecure = new QTreeWidgetItem();
                isSecure->setText(0, "isSecure: " + cookies[j].isSecure);
                name->addChild(isSecure);

                QTreeWidgetItem* isSession = new QTreeWidgetItem();
                isSession->setText(0, "isSession: " + cookies[j].isSession);
                name->addChild(isSession);

                QTreeWidgetItem* samesite = new QTreeWidgetItem();
                samesite->setText(0, "samesite: " + cookies[j].samesite);
                name->addChild(samesite);

                domain->addChild(name);

                cookies.removeAt(j);
                j--;
            }

            root->addChild(domain);
        }

        ui->treeWidget->addTopLevelItem(root);
    }
}

void MainWindow::onSettingsButton()
{
    settingsWindow->hide();
    settingsWindow->show();
}

void MainWindow::onChangeModeButton()
{
    isDarkMode = !isDarkMode;
    changeMode();
    settingsWindow->hide();
}

void MainWindow::configureSettingWindow()
{
    settingsWindow = new QWidget;
    settingsWindow->setFixedSize(150, 70);
    settingsWindowButton = new QPushButton(settingsWindow);

    QString buttonTitle = isDarkMode ? "Поменять тему на\n светлую" : "Поменять тему на\n тёмную";
    settingsWindowButton->resize(150, 50);
    settingsWindowButton->setText(buttonTitle);
    connect(settingsWindowButton, &QPushButton::clicked, this, &MainWindow::onChangeModeButton);
}

void MainWindow::setModeFromFile()
{
    QString path = "mode.txt";
    QFile file(path);

    if (QFile::exists(QDir::currentPath().append("/" + path)))
    {
        file.open(QIODevice::ReadOnly);
        isDarkMode = file.readLine() == "Dark" ? true : false;
    }
    else
    {
        saveMode("Lite");
    }
}

void MainWindow::changeMode()
{
    if (isDarkMode)
    {
        saveMode("Dark");

        QFile styleSheetFile(":/Themes/Themes/Diffnes.qss");
        styleSheetFile.open(QFile::ReadOnly);
        QString styleSheet = QLatin1String(styleSheetFile.readAll());
        setStyleSheet(styleSheet);

        if (settingsWindow)
        {
            settingsWindow->setStyleSheet(styleSheet);
            settingsWindowButton->setText("Поменять тему на\n светлую");
        }
    }
    else
    {
//        QFile styleSheetFile(":/Themes/Themes/Integrid.qss");
//        styleSheetFile.open(QFile::ReadOnly);
//        QString styleSheet = QLatin1String(styleSheetFile.readAll());

        // Закоменченный код здесь для пункта 3 из ТЗ про дизайн.
        // Не использую потому что больше нравиться внешний вид стандартного интерфейся
        // Чтобы включить этот просто раскоментите строчки и в ставте в функции setStyleSheet переменную styleSheet
        saveMode("Lite");
        setStyleSheet(""); //styleSheet

        if (settingsWindow)
        {
            settingsWindow->setStyleSheet(""); //styleSheet
            settingsWindowButton->setText("Поменять тему на\n тёмную");
        }
    }
}

void MainWindow::saveMode(QString mode)
{
    QString path = "mode.txt";
    QFile file(path);
    file.open(QIODevice::WriteOnly);
    QTextStream stream( &file );
    stream << mode;
}

