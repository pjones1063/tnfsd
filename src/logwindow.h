#ifndef LOGWINDOW_H
#define LOGWINDOW_H

#include <QDialog>
#include <QPlainTextEdit>

class LogWindow : public QDialog
{
    Q_OBJECT
public:
    explicit LogWindow(QWidget *parent = nullptr);

public slots:
    void addLog(QString msg); // Slot to receive the logMessage signal

private:
    QPlainTextEdit *logOutput;
};

#endif
