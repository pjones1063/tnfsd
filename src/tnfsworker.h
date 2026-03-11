#ifndef TNFSWORKER_H
#define TNFSWORKER_H

#include <QObject>
#include <QString>

// Define the function pointer type for the C callback
typedef void (*tnfs_log_callback)(const char* message);

extern "C" {
int start_tnfs_server(char *root_dir, int port);
extern volatile int g_tnfs_running;
// Add this line so C++ can see the setter function
void set_log_callback(tnfs_log_callback cb);
}

class TnfsWorker : public QObject
{
    Q_OBJECT

public:
    explicit TnfsWorker(QString rootDir, int port, QObject *parent = nullptr);
    static void handleCLog(const char* msg); // The bridge function

public slots:
    void process();
    void stop();

signals:
    void finished();
    void error(QString err);
    void logMessage(QString msg); // Signal to send logs to the UI

private:
    QString m_rootDir;
    int m_port;
};

#endif // TNFSWORKER_H
