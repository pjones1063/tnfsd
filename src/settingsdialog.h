#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);

    // Getters so TrayManager can read the saved paths
    QString getMountPath() const;
    QString getStatsPath() const;

private slots:
    void browseMountPath();
    void browseStatsPath();
    void saveSettings();
    void generateStats();

private:
    void loadSettings(); // Load from QSettings on startup

    QLineEdit *mountPathEdit;
    QLineEdit *statsPathEdit;

    QPushButton *browseMountBtn;
    QPushButton *browseStatsBtn;
    QPushButton *generateStatsBtn;
    QPushButton *saveBtn;
    QPushButton *cancelBtn;
};

#endif // SETTINGSDIALOG_H
