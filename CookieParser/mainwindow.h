#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QFileDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onUploadButton();
    void onSettingsButton();
    void onChangeModeButton();

private:

    struct Cookie
    {
        QString domain;
        QString name;
        QString val;
        QString path;
        int timestamp;
        QString isHttpOnly;
        QString isSecure;
        QString isSession = "no";
        QString samesite = "None";
    };

    void createCookieTree(QString &fileName, QVector<Cookie> &cookies, QSet<QString> &domainsName);
    void configureSettingWindow();
    void setModeFromFile();
    void changeMode();
    void saveMode(QString mode);
    void workWithData(QString &fileName, QString &data);
    void closeEvent(QCloseEvent *event);
    QString formatFileName(const QString &fileName);

private:
    Ui::MainWindow *ui;

    QWidget *settingsWindow;
    QPushButton *settingsWindowButton;
    QSet<QString> fileNames;

    bool isDarkMode;
    bool isFileDialogOpened;
    int cookieCount;
};
#endif // MAINWINDOW_H
