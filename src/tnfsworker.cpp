#include "tnfsworker.h"
#include <QDebug>
#include <QByteArray>

static TnfsWorker* g_currentWorker = nullptr;

TnfsWorker::TnfsWorker(QString rootDir, int port, QObject *parent)
    : QObject(parent), m_rootDir(rootDir), m_port(port)
{
}

void TnfsWorker::handleCLog(const char* msg) {
    if (g_currentWorker) {
        emit g_currentWorker->logMessage(QString::fromUtf8(msg).trimmed());
    }
}


void TnfsWorker::process()
{
    g_currentWorker = this;
    set_log_callback(TnfsWorker::handleCLog);

    QByteArray rootBytes = m_rootDir.toLocal8Bit();

    // start_tnfs_server is the blocking call
    int result = start_tnfs_server(rootBytes.data(), m_port);

    // If the server returned an error code, emit the error signal
    if (result < 0) {
        emit error("TNFS Server failed to start. Check the live console for details.");
    }

    // IMPORTANT: Clear the callback BEFORE emitting finished
    set_log_callback(nullptr);
    g_currentWorker = nullptr;

    emit finished();
}



void TnfsWorker::stop()
{
    qDebug() << "Signaling C code to shut down...";
    // Flip the flag we added to datagram.c!
    g_tnfs_running = 0;
}
