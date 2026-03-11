#include "traymanager.h"
#include "settingsdialog.h"
#include "aboutdialog.h"
#include "logwindow.h"
#include "statswindow.h"
#include <QCoreApplication>
#include <QDebug>
#include <QIcon>
#include <QDir>
#include <QSettings>
#include <QTcpServer>
#include <QUdpSocket>
#include <QMessageBox>


TrayManager::TrayManager(QObject *parent) : QObject(parent),
    workerThread(nullptr), worker(nullptr), logWindow(new LogWindow()) // <-- Fix: Instantiate it here!
{
    createActions();
    createTrayIcon();

    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setIcon(QIcon(":/tnfs_badge.png"));
    trayIcon->show();
}

TrayManager::~TrayManager()
{
    // Ensure safe shutdown on destruction
    stopServer();
    delete trayIconMenu;
    delete logWindow; // <-- Fix: Clean it up here!
}

void TrayManager::createActions()
{
    startAction = new QAction(tr("&Start Server"), this);
    connect(startAction, &QAction::triggered, this, &TrayManager::startServer);

    stopAction = new QAction(tr("S&top Server"), this);
    stopAction->setDisabled(true); // Disable until started
    connect(stopAction, &QAction::triggered, this, &TrayManager::stopServer);

    settingsAction = new QAction(tr("Se&ttings..."), this);
    connect(settingsAction, &QAction::triggered, this, &TrayManager::showSettings);

    aboutAction = new QAction(tr("&About"), this);
    connect(aboutAction, &QAction::triggered, this, &TrayManager::showAbout);

    logsAction = new QAction(tr("View Live &Console"), this);
    connect(logsAction, &QAction::triggered, this, &TrayManager::showLogs);

    statsAction = new QAction(tr("View &Stats Report"), this);
    connect(statsAction, &QAction::triggered, this, &TrayManager::showStats);

    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, &QAction::triggered, this, &TrayManager::quitApp);
}

void TrayManager::createTrayIcon()
{
    trayIconMenu = new QMenu();
    trayIconMenu->addAction(startAction);
    trayIconMenu->addAction(stopAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(settingsAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(logsAction);
    trayIconMenu->addAction(statsAction);
    trayIconMenu->addAction(aboutAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setToolTip(tr("TNFS Daemon - Stopped"));
}

void TrayManager::startServer()
{
    // Prevent starting multiple threads
    if (workerThread && workerThread->isRunning()) return;

    int tnfsPort = 16384;
    if (!isPortFree(tnfsPort)) {
        qDebug() << "Port" << tnfsPort << "is already in use. Aborting start.";
        QMessageBox::critical(nullptr,
                              tr("Port in Use"),
                              tr("Cannot start TNFS Server.\nPort %1 is already in use by another application.").arg(tnfsPort));
        return; // Stop right here, don't spin up the thread
    }


    qDebug() << "Start Server clicked";
    startAction->setDisabled(true);
    stopAction->setEnabled(true);
    trayIcon->setToolTip(tr("TNFS Daemon - Running"));

    QSettings settings("TNFS_Project", "TrayApp");
    QString mountPath = settings.value("mountPath", QDir::homePath()).toString();
    QString statsPath = settings.value("statsPath", QDir::currentPath()).toString();

    // Set working directory so CSV drops in the right place
    QDir::setCurrent(statsPath);

    workerThread = new QThread(this);
    worker = new TnfsWorker(mountPath, 16384);
    worker->moveToThread(workerThread);

    // Connect logging
    connect(worker, &TnfsWorker::logMessage, logWindow, &LogWindow::addLog);

    // EXACT THREAD CLEANUP PATTERN (Fixed)
    connect(workerThread, &QThread::started, worker, &TnfsWorker::process);
    connect(worker, &TnfsWorker::finished, workerThread, &QThread::quit);
    connect(worker, &TnfsWorker::finished, worker, &QObject::deleteLater);
    connect(workerThread, &QThread::finished, workerThread, &QObject::deleteLater);

    // Handle UI Reset safely when thread actually finishes
    connect(workerThread, &QThread::finished, this, [this]() {
        worker = nullptr;
        workerThread = nullptr;
        startAction->setEnabled(true);
        stopAction->setDisabled(true);
        trayIcon->setToolTip(tr("TNFS Daemon - Stopped"));
        qDebug() << "Server thread stopped and pointers cleared.";
    });

    workerThread->start();
}

void TrayManager::stopServer()
{
    qDebug() << "Stop Server clicked";

    // Safely tell the worker to stop running its loop
    if (worker) {
        worker->stop();
        worker = nullptr; // Null it out immediately so a double-click doesn't crash
    }
}

void TrayManager::showSettings()
{
    SettingsDialog dlg;
    if (dlg.exec() == QDialog::Accepted) {
        // If they clicked save, and the server is running, you might want to restart it to use the new path!
        qDebug() << "Saved new mount path:" << dlg.getMountPath();
    }
}


void TrayManager::quitApp()
{
    // Ensure we stop the server before quitting
    stopServer();

    // Give the thread a moment to shut down gracefully before forcing a quit
    if (workerThread && workerThread->isRunning()) {
        workerThread->wait(1000);
    }

    QCoreApplication::quit();
}

void TrayManager::showAbout()
{
    qDebug() << "About clicked";
    AboutDialog dlg;
    dlg.exec();
}

void TrayManager::showLogs()
{
    if (logWindow) {
        logWindow->show();
        logWindow->raise(); // Bring to front
        logWindow->activateWindow();
    }
}

void TrayManager::showStats()
{
    qDebug() << "View Stats Report clicked";

    // Grab the current stats path from settings
    QSettings settings("TNFS_Project", "TrayApp");
    QString statsPath = settings.value("statsPath", QDir::currentPath()).toString();

    // Create and show the stats window independently
    StatsWindow *statsWin = new StatsWindow(statsPath);
    statsWin->setAttribute(Qt::WA_DeleteOnClose); // Prevents memory leaks
    statsWin->show();
}



bool TrayManager::isPortFree(int port) {
    bool tcpFree = false;
    bool udpFree = false;

    // 1. Test TCP
    QTcpServer tcpServer;
    if (tcpServer.listen(QHostAddress::Any, port)) {
        tcpFree = true;
        tcpServer.close();
    }

    // 2. Test UDP
    QUdpSocket udpSocket;
    if (udpSocket.bind(QHostAddress::Any, port)) {
        udpFree = true;
        udpSocket.close();
    }

    return tcpFree && udpFree;
}

