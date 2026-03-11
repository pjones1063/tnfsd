#include "aboutdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("About TNFS Server"));
    resize(520, 420);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    textBrowser = new QTextBrowser(this);
    textBrowser->setOpenExternalLinks(true);

    // HTML/CSS string to mimic the AspeQt-2k26 style exactly
    QString html = R"(
        <table width="100%" cellspacing="0" cellpadding="0" border="0">
            <tr>
                <td bgcolor="#2b2b2b" width="25"></td>

                <td bgcolor="#2b2b2b" style="padding-top: 25px; padding-bottom: 20px;">
                    <span style="color: white; font-size: 28px; font-family: sans-serif;">TNFS-2k26</span><br><br>
                    <span style="color: #4CAF50; font-size: 11px; font-weight: bold; font-family: sans-serif; letter-spacing: 1px;">FUJINET PROTOCOL SERVER FOR QT6</span>
                </td>
            </tr>
            <tr>
                <td colspan="2" bgcolor="#4CAF50" height="4"></td>
            </tr>
        </table>

        <table width="100%" cellspacing="0" cellpadding="25" border="0">
            <tr>
                <td style="color: #333; font-family: sans-serif;">
                    <p style="margin-top: 0;"><b>Bridging the Gap Between Vintage and Modern.</b></p>

                    <p>TnfsTrayApp transforms your modern PC into a powerful, stay-resident peripheral server. Rebuilt with the <b>Modern Qt6 Framework</b>, it ensures your Atari, Apple II, Coleco, and Tandy systems stay connected seamlessly in the background.</p>

                    <p><b>Key Features:</b></p>
                    <ul>
                        <li>Stay-Resident System Tray Daemon</li>
                        <li>Live CSV Stats Logging & Built-in HTML Reporting</li>
                        <li>Cross-Platform Qt6 Architecture</li>
                        <li>FujiNet Protocol Compatible</li>
                    </ul>

                    <p style="font-size: 11px; color: #999; margin-top: 30px; border-top: 1px solid #eee; padding-top: 10px;">
                        Based on the original tnfsd daemon by Dylan Smith.<br>
                        Qt6 GUI Wrapper implementation.
                    </p>
                </td>
            </tr>
        </table>
    )";


    textBrowser->setHtml(html);
    mainLayout->addWidget(textBrowser);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    closeButton = new QPushButton(tr("&Close"), this);
    // Add a nice icon to the close button if you have one, or just keep it standard
    buttonLayout->addWidget(closeButton);
    mainLayout->addLayout(buttonLayout);

    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
}
