#include "logwindow.h"
#include <QVBoxLayout>
#include <QScrollBar>

LogWindow::LogWindow(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("TNFS Live Console"));
    resize(700, 400);

    QVBoxLayout *layout = new QVBoxLayout(this);
    logOutput = new QPlainTextEdit(this);
    logOutput->setReadOnly(true);
    logOutput->setBackgroundRole(QPalette::Base);

    // Set a dark console theme
    logOutput->setStyleSheet("background-color: #1e1e1e; color: #dcdcdc; font-family: monospace;");

    layout->addWidget(logOutput);
}

void LogWindow::addLog(QString msg)
{
    logOutput->appendPlainText(msg);
    // Auto-scroll to bottom
    logOutput->verticalScrollBar()->setValue(logOutput->verticalScrollBar()->maximum());
}
