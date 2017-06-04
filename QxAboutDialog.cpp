
#include <QLabel>
#include <QPointer>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

#include "QxAboutDialog.h"

QxAboutDialog::QxAboutDialog(QWidget *parent /* = NULL*/)
	: QDialog(parent)
{
	initDialog();
}

QxAboutDialog::~QxAboutDialog()
{
}

//Init the dialog.
void QxAboutDialog::initDialog()
{
    QPointer<QHBoxLayout> pTopLayout = new QHBoxLayout;
    QPointer<QLabel> pNameLabel = new QLabel;
	QFont font;
	font.setPointSize(16);
	font.setBold(true);
	pNameLabel->setFont(font);
	pNameLabel->setText("GNT decoder - Alpha");
	pTopLayout->addStretch();
	pTopLayout->addWidget(pNameLabel);
	pTopLayout->addStretch();

    QPointer<QLabel> pInfoLabel = new QLabel;
	QString strMessage("<br>\
<big><i>Version:</i></big>  0.1.04<br>\
<big><i>Release date:</i></big>  26.May.2017<br><br>\
<big><i>Release note:</i></big><br>\
1. Modified for reading huge files.<br>");

	pInfoLabel->setTextFormat(Qt::RichText);
	pInfoLabel->setText(strMessage);
    QPointer<QHBoxLayout> pButtonLayout = new QHBoxLayout;
    QPointer<QPushButton> pButton = new QPushButton("OK");
	pButtonLayout->addStretch();
	pButtonLayout->addWidget(pButton);

    QPointer<QVBoxLayout> pLayout = new QVBoxLayout;
	pLayout->addLayout(pTopLayout);
	pLayout->addWidget(pInfoLabel);
	pLayout->addLayout(pButtonLayout);

	setLayout(pLayout);
	setWindowFlags(windowFlags() &~Qt::WindowContextHelpButtonHint &~Qt::WindowCloseButtonHint);
    connect(pButton.data(), &QPushButton::clicked, this, &QxAboutDialog::accept);
}
