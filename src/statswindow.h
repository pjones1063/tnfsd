#ifndef STATSWINDOW_H
#define STATSWINDOW_H

#include <QDialog>
#include <QTextBrowser>
#include <QNetworkAccessManager>
#include <QJsonObject>

struct LogRow {
    QString time;
    QString ip;
    QString country;
    QString cmd;
    QString file;
};

class StatsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit StatsWindow(QString statsFolderPath, QWidget *parent = nullptr);
    ~StatsWindow();

private slots:
    void generateReport();

private:
    void loadCache();
    void saveCache();
    QString getCountry(const QString &ip);
    void renderHtml(const QMap<QString, int> &ipMountCounts,
                    const QSet<QString> &uniqueMountFiles,
                    const QList<LogRow> &processedRows);


    QTextBrowser *textBrowser;
    QNetworkAccessManager *netManager;
    QJsonObject ipCache;

    QString basePath;
    QString csvFile;
    QString cacheFile;

};

#endif // STATSWINDOW_H
