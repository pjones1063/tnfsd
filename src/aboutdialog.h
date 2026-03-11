#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <QTextBrowser>
#include <QPushButton>

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);

private:
    QTextBrowser *textBrowser;
    QPushButton *closeButton;
};

#endif // ABOUTDIALOG_H
