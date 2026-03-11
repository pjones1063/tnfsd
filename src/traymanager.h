#ifndef TRAYMANAGER_H
#define TRAYMANAGER_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QThread>
#include "tnfsworker.h"
class LogWindow;

class TrayManager : public QObject
{
Q_OBJECT

    public:
             explicit TrayManager(QObject *parent = nullptr);
    ~TrayManager();

private slots:
    void startServer();
    void stopServer();
    void showSettings();
    void quitApp();
    void showAbout();
    void showLogs();
    void showStats();


private:
    void createActions();
    void createTrayIcon();
    bool isPortFree(int port);

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;

    QAction *startAction;
    QAction *stopAction;
    QAction *settingsAction;
    QAction *quitAction;
    QAction *aboutAction;
    QAction *statsAction;
    QAction *logsAction;
    LogWindow *logWindow;
    QThread *workerThread;
    TnfsWorker *worker;

};

#endif // TRAYMANAGER_H
