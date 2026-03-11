#include "statswindow.h"
#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QThread>
#include <QSet>
#include <QMap>
#include <algorithm>

StatsWindow::StatsWindow(QString statsFolderPath, QWidget *parent)
    : QDialog(parent), basePath(statsFolderPath)
{
    setWindowTitle(tr("TNFS Server Statistics"));
    resize(800, 600);

    csvFile = basePath + "/tnfsd_stats.csv";
    cacheFile = basePath + "/ip_cache.json";

    netManager = new QNetworkAccessManager(this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    textBrowser = new QTextBrowser(this);
    layout->addWidget(textBrowser);

    textBrowser->setHtml("<h2 align='center'>Generating Stats... Please wait.</h2>");

    // Use a slight delay to allow the window to show before blocking to process the file
    QMetaObject::invokeMethod(this, "generateReport", Qt::QueuedConnection);
}

StatsWindow::~StatsWindow()
{
}

void StatsWindow::loadCache()
{
    QFile file(cacheFile);
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        ipCache = doc.object();
        file.close();
    }
}

void StatsWindow::saveCache()
{
    QFile file(cacheFile);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(ipCache);
        file.write(doc.toJson());
        file.close();
    }
}

QString StatsWindow::getCountry(const QString &ip)
{
    if (ip.startsWith("127.") || ip.startsWith("192.168.") || ip.startsWith("10.") || ip.startsWith("172.16.")) {
        return "Local";
    }

    if (ipCache.contains(ip)) {
        return ipCache.value(ip).toString();
    }

    // Rate limiting for the free API (100ms)
    QThread::msleep(100);

    QNetworkRequest request((QUrl("http://ip-api.com/json/" + ip + "?fields=country")));
    QNetworkReply *reply = netManager->get(request);

    // Synchronously wait for the reply (acceptable for this specific background admin tool)
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QString country = "Unknown";
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        country = doc.object().value("country").toString("Unknown");
        ipCache.insert(ip, country);
    }
    reply->deleteLater();
    return country;
}

void StatsWindow::generateReport()
{
    loadCache(); // Load existing ip_cache.json

    QMap<QString, int> ipMountCounts;
    QSet<QString> uniqueMountFiles;
    QList<LogRow> processedRows;

    QFile file(csvFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        textBrowser->setHtml("<h2>Error: Could not find stats log.</h2>");
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split(',');
        if (parts.size() < 3) continue;

        QString ip = parts[1].trimmed();
        QString message = parts[2].trimmed();

        // Count mounts based on your CSV format [cite: 2]
        if (message.contains("[OPEN]") || message.contains("0x00")) {
            ipMountCounts[ip]++;
            // Extract filename after the command
            QString filename = message.section(' ', 1).trimmed();
            if (!filename.isEmpty() && !filename.toLower().contains("phantom")) {
                uniqueMountFiles.insert(filename);
            }
        }

        // Use cache only. If not there, don't wait for network!
        QString country = ipCache.value(ip).toString("Unknown");
        processedRows.append({parts[0], ip, country, parts[2], ""});
    }
    file.close();

    // Now render the HTML immediately...
    renderHtml(ipMountCounts, uniqueMountFiles, processedRows);
}


void StatsWindow::renderHtml(const QMap<QString, int> &ipMountCounts,
                             const QSet<QString> &uniqueMountFiles,
                             const QList<LogRow> &processedRows)
{
    // Calculate totals
    int total_mounts = 0;
    for (int count : ipMountCounts.values()) {
        total_mounts += count;
    }
    int total_unique_files = uniqueMountFiles.size();
    int total_unique_users = ipMountCounts.size();

    // Sort Top Users (Descending)
    std::vector<std::pair<QString, int>> sortedUsers;
    for (auto it = ipMountCounts.begin(); it != ipMountCounts.end(); ++it) {
        sortedUsers.push_back({it.key(), it.value()});
    }
    std::sort(sortedUsers.begin(), sortedUsers.end(),
              [](const std::pair<QString, int>& a, const std::pair<QString, int>& b) {
                  return a.second > b.second;
              });

    // Build the HTML String
    QString html = R"(
        <style>
            th { background-color: #2c3e50; color: white; padding: 8px; }
            td { padding: 6px; border-bottom: 1px solid #eee; }
        </style>
        <h1 align="center">TNFS Server Activity</h1>
        <hr>
        <table width="100%" cellpadding="10" cellspacing="0">
            <tr>
                <td align="center"><b>Total Mounts</b><br><font size="6" color="#2980b9">%1</font></td>
                <td align="center"><b>Unique Mounts</b><br><font size="6" color="#2980b9">%2</font></td>
                <td align="center"><b>Active Users</b><br><font size="6" color="#2980b9">%3</font></td>
            </tr>
        </table>
        <br>
        <h2>Top 20 Active Users</h2>
        <table width="100%" cellspacing="0">
            <tr><th>#</th><th>IP Address</th><th>Country</th><th align="right">Mounts</th></tr>
    )";

    html = html.arg(total_mounts).arg(total_unique_files).arg(total_unique_users);

    int rank = 1;
    for (const auto &user : sortedUsers) {
        if (rank > 20) break;
        html += QString("<tr><td align='center'><b>%1</b></td><td>%2</td><td>%3</td><td align='right'>%4</td></tr>")
                    .arg(rank).arg(user.first).arg(ipCache.value(user.first).toString("Unknown")).arg(user.second);
        rank++;
    }

    html += "</table><br><h2>Recent Activity Log (Last 20)</h2><table width='100%' cellspacing='0'>";
    html += "<tr><th>Timestamp</th><th>IP Address</th><th>Country</th><th>Message</th></tr>";

    int startIdx = qMax(0, processedRows.size() - 20);
    for (int i = processedRows.size() - 1; i >= startIdx; --i) {
        const LogRow &row = processedRows[i];
        html += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td></tr>")
                    .arg(row.time).arg(row.ip).arg(row.country).arg(row.cmd);
    }

    html += "</table><p align='center'><font color='#999'>Generated by TNFS-2k26</font></p>";

    textBrowser->setHtml(html);
}
