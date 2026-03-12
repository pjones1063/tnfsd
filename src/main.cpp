#include <QApplication>
#include <QMessageBox>
#include <QSplashScreen>
#include <QTimer>
#include <QPixmap>
#include "traymanager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // This ensures the app doesn't quit if the user closes a settings window
    QApplication::setQuitOnLastWindowClosed(false);

    // Check if system tray is supported on this OS
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(nullptr, QObject::tr("TNFS Tray"),
                              QObject::tr("I couldn't detect any system tray "
                                          "on this system."));
        return 1;
    }

    // --- NEW: Splash Screen Logic ---
    // Load your retro badge and scale it to a nice splash size
    QPixmap splashImage(":/tnfs_badge.png");
    splashImage = splashImage.scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // Create the splash screen and force it to stay on top
    QSplashScreen *splash = new QSplashScreen(splashImage, Qt::WindowStaysOnTopHint);

    // Add a little loading text at the bottom
    splash->showMessage("", Qt::AlignBottom | Qt::AlignCenter, Qt::white);
    splash->show();

    // Force Qt to draw the splash screen immediately before doing anything else
    app.processEvents();
    // --------------------------------

    TrayManager manager;
    app.setWindowIcon(QIcon(":/tnfs_badge.png"));

    // --- NEW: Splash Screen Cleanup ---
    // Tell Qt to close and delete the splash screen after 2.5 seconds (2500 ms)
    QTimer::singleShot(2500, splash, [splash]() {
        splash->close();
        splash->deleteLater();
    });
    // ----------------------------------

    return app.exec();
}
