#include "settingsdialog.h"
#include "statswindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QSettings>
#include <QDir>
#include <QMessageBox>
// #include "statsgenerator.h" // We will build this next!

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("TNFS Server Settings"));
    setMinimumWidth(450);

    // --- Create Widgets ---
    QLabel *mountLabel = new QLabel(tr("Root Mount Path:"), this);
    mountPathEdit = new QLineEdit(this);
    browseMountBtn = new QPushButton(tr("Browse..."), this);

    QLabel *statsLabel = new QLabel(tr("Stats Folder (CSV/HTML):"), this);
    statsPathEdit = new QLineEdit(this);
    browseStatsBtn = new QPushButton(tr("Browse..."), this);

    generateStatsBtn = new QPushButton(tr("Generate HTML Stats Now"), this);

    saveBtn = new QPushButton(tr("Save"), this);
    cancelBtn = new QPushButton(tr("Cancel"), this);

    // --- Layouts ---
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QHBoxLayout *mountLayout = new QHBoxLayout();
    mountLayout->addWidget(mountPathEdit);
    mountLayout->addWidget(browseMountBtn);

    QHBoxLayout *statsLayout = new QHBoxLayout();
    statsLayout->addWidget(statsPathEdit);
    statsLayout->addWidget(browseStatsBtn);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(saveBtn);

    // Add everything to main layout
    mainLayout->addWidget(mountLabel);
    mainLayout->addLayout(mountLayout);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(statsLabel);
    mainLayout->addLayout(statsLayout);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(generateStatsBtn);
    mainLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

    // --- Connections ---
    connect(browseMountBtn, &QPushButton::clicked, this, &SettingsDialog::browseMountPath);
    connect(browseStatsBtn, &QPushButton::clicked, this, &SettingsDialog::browseStatsPath);
    connect(saveBtn, &QPushButton::clicked, this, &SettingsDialog::saveSettings);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(generateStatsBtn, &QPushButton::clicked, this, &SettingsDialog::generateStats);

    // Load existing settings when dialog opens
    loadSettings();
}

void SettingsDialog::loadSettings()
{
    QSettings settings("TNFS_Project", "TrayApp");
    mountPathEdit->setText(settings.value("mountPath", QDir::homePath()).toString());
    statsPathEdit->setText(settings.value("statsPath", QDir::currentPath()).toString());
}

void SettingsDialog::saveSettings()
{
    QSettings settings("TNFS_Project", "TrayApp");
    settings.setValue("mountPath", mountPathEdit->text());
    settings.setValue("statsPath", statsPathEdit->text());
    accept(); // Closes dialog returning QDialog::Accepted
}

void SettingsDialog::browseMountPath()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Root Mount Directory"),
                                                    mountPathEdit->text(),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        mountPathEdit->setText(dir);
    }
}

void SettingsDialog::browseStatsPath()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Stats Output Directory"),
                                                    statsPathEdit->text(),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        statsPathEdit->setText(dir);
    }
}

void SettingsDialog::generateStats()
{
    // Pass the currently selected stats directory to the window
    StatsWindow *statsWin = new StatsWindow(statsPathEdit->text(), this);
    statsWin->setAttribute(Qt::WA_DeleteOnClose);
    statsWin->show();
}


QString SettingsDialog::getMountPath() const { return mountPathEdit->text(); }
QString SettingsDialog::getStatsPath() const { return statsPathEdit->text(); }
