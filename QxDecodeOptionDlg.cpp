#include "QxDecodeOptionDlg.h"

#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QStringList>

static const unsigned UINT_DEFAULT_IMAGE_SIZE = 64;

QxDecodeOptionDlg::QxDecodeOptionDlg(const QStringList& fileList, QWidget* parent /*= NULL*/)
    : QDialog(parent)
{
	initDialog(fileList);
}

void QxDecodeOptionDlg::initDialog(const QStringList& fileList)
{
	// show selected files.
    QPointer<QListWidget> pFileListWidget = new QListWidget;
	pFileListWidget->addItems(fileList);

	// select file path to save decoded images.
    QPointer<QHBoxLayout> pSavePathLayout = new QHBoxLayout;
    QPointer<QLabel> pSavePathLabel = new QLabel("Save to: ");
	m_pFilePathEdit = new QLineEdit;
    QPointer<QPushButton> pSelectSavePathButton = new QPushButton("Browse...");
	pSavePathLayout->addWidget(pSavePathLabel);
	pSavePathLayout->addWidget(m_pFilePathEdit);
	pSavePathLayout->addWidget(pSelectSavePathButton);

	// select application from {Caffe, DIGITS, CNTK and TensorFlow}
	m_pApplicationGroupBox = new QGroupBox("Application:");
    QPointer<QVBoxLayout> pApplicationBoxLayout = new QVBoxLayout;
	m_pCaffe = new QRadioButton("Caffe");
	m_pCNTK = new QRadioButton("Microsoft CNTK");
	m_pDigits = new QRadioButton("Nvidia Digits");
	m_pTensorFlow = new QRadioButton("TensorFlow");
	pApplicationBoxLayout->addWidget(m_pCaffe);
	pApplicationBoxLayout->addWidget(m_pCNTK);
	pApplicationBoxLayout->addWidget(m_pDigits);
	pApplicationBoxLayout->addWidget(m_pTensorFlow);
	m_pApplicationGroupBox->setLayout(pApplicationBoxLayout);
	
	// select image format for given application
	m_pImageFormatGroupBox = new QGroupBox("Format: ");
    QPointer<QVBoxLayout> pImageFormatBoxLayout = new QVBoxLayout;
	m_pPngFormat = new QRadioButton("PNG");
	m_pJpegFormat = new QRadioButton("JPEG");
	m_pPpmFormat = new QRadioButton("PPM");
	m_pBmpFormat = new QRadioButton("BMP");
	pImageFormatBoxLayout->addWidget(m_pPngFormat);
	pImageFormatBoxLayout->addWidget(m_pPpmFormat);
	pImageFormatBoxLayout->addWidget(m_pBmpFormat);
	pImageFormatBoxLayout->addWidget(m_pJpegFormat);
	m_pImageFormatGroupBox->setLayout(pImageFormatBoxLayout);

	// select image size for given application
	m_pImageSizeGroupBox = new QGroupBox("Size: ");
    QPointer<QVBoxLayout> pImageSizetBoxLayout = new QVBoxLayout;
    QPointer<QHBoxLayout> pImageSizeLayout = new QHBoxLayout;
	m_pSmall = new QRadioButton("32 (32 by 32)");
	m_pMedium = new QRadioButton("64");
	m_pLarge = new QRadioButton("128");
	m_pCustomize = new QRadioButton("Other: ");
	m_pImageSizeEdit = new QLineEdit;
	m_pImageSizeEdit->setVisible(false);
	pImageSizeLayout->addWidget(m_pCustomize);
	pImageSizeLayout->addWidget(m_pImageSizeEdit);
	pImageSizetBoxLayout->addWidget(m_pSmall);
	pImageSizetBoxLayout->addWidget(m_pMedium);
	pImageSizetBoxLayout->addWidget(m_pLarge);
	pImageSizetBoxLayout->addLayout(pImageSizeLayout);
	m_pImageSizeGroupBox->setLayout(pImageSizetBoxLayout);

	// put the groupboxes together
    QPointer<QHBoxLayout> pButtonLayout = new QHBoxLayout;
	pButtonLayout->addWidget(m_pApplicationGroupBox);
	pButtonLayout->addWidget(m_pImageFormatGroupBox);
	pButtonLayout->addWidget(m_pImageSizeGroupBox);

	// OK and Cancel Button
    QPointer<QPushButton> pOkButton = new QPushButton("Decode");
    QPointer<QPushButton> pCancelButton = new QPushButton("Cancel");
    QPointer<QHBoxLayout> pOptionLayout = new QHBoxLayout;
	pOptionLayout->addStretch();
	pOptionLayout->addWidget(pOkButton);
	pOptionLayout->addWidget(pCancelButton);

	// main layout of the dialog
    QPointer<QVBoxLayout> pLayout = new QVBoxLayout;
	pLayout->addWidget(pFileListWidget);
	pLayout->addLayout(pSavePathLayout);
	pLayout->addLayout(pButtonLayout);
	pLayout->addLayout(pOptionLayout);
	setLayout(pLayout);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint &~Qt::WindowCloseButtonHint);

	// connect signals and slots
    connect(pOkButton.data(), &QPushButton::clicked, this, &QxDecodeOptionDlg::accept);
    connect(pCancelButton.data(), &QPushButton::clicked, this, &QxDecodeOptionDlg::reject);
    connect(pSelectSavePathButton.data(), &QPushButton::clicked, this, &QxDecodeOptionDlg::setSaveFilePath);
    connect(m_pCustomize.data(), &QRadioButton::toggled, this, &QxDecodeOptionDlg::setImageSize);
    connect(m_pLarge.data(), &QRadioButton::toggled, this, &QxDecodeOptionDlg::setImageSize);
    connect(m_pMedium.data(), &QRadioButton::toggled, this, &QxDecodeOptionDlg::setImageSize);
    connect(m_pSmall.data(), &QRadioButton::toggled, this, &QxDecodeOptionDlg::setImageSize);
    connect(m_pCaffe.data(), &QRadioButton::toggled, this, &QxDecodeOptionDlg::setImageFormatOption);
    connect(m_pCNTK.data(), &QRadioButton::toggled, this, &QxDecodeOptionDlg::setImageFormatOption);
    connect(m_pDigits.data(), &QRadioButton::toggled, this, &QxDecodeOptionDlg::setImageFormatOption);
    connect(m_pTensorFlow.data(), &QRadioButton::toggled, this, &QxDecodeOptionDlg::setImageFormatOption);

	// default status
	m_pCaffe->setChecked(true);
	m_pPngFormat->setChecked(true);
	m_pMedium->setChecked(true);
}

void QxDecodeOptionDlg::setSaveFilePath()
{
	QString strFilePath = QFileDialog::getExistingDirectory(this, "Select a directory");

	// If user selected any existing path.
	if (!strFilePath.isEmpty())
	{
		m_pFilePathEdit->setText(strFilePath);
	}
}

void QxDecodeOptionDlg::setImageSize()
{
	if (m_pCustomize->isChecked())
	{
		m_pImageSizeEdit->setVisible(true);
	}
	else
	{
		m_pImageSizeEdit->setVisible(false);
	}
}

void QxDecodeOptionDlg::accept()
{
	// Check if filepath is specified
    if (m_pFilePathEdit->text().isEmpty())
	{
		QMessageBox::information(this, "Wrong path", "Please select a directory to save decoded files!", QMessageBox::Ok);
		return;
	}
	// Check customized image size
	if (m_pCustomize->isChecked())
	{
		bool bOk = false;
		quint32 uImageSize = m_pImageSizeEdit->text().toUInt(&bOk);
		if (!bOk || !uImageSize) //bOK == false or uImageSize == 0
		{
			QMessageBox::information(this, "Invalid image size", "Please input valid image size (positive integer only) !", QMessageBox::Ok);
			return;
		}
	}

	QDialog::accept();
}

QxDecodeOptionDlg::ApplicationType QxDecodeOptionDlg::application() const
{
	ApplicationType selectedApp = Caffe;
	if (m_pCNTK->isChecked())
	{
		selectedApp = CNTK;
	}
	else if (m_pDigits->isChecked())
	{
		selectedApp = DIGITS;
	}
	else if (m_pTensorFlow->isChecked())
	{
		selectedApp = TensorFlow;
	}

	return selectedApp;
}

QString QxDecodeOptionDlg::imageFormat() const
{
	QString strFormat = "png";
	if (m_pJpegFormat->isChecked())
	{
		strFormat = "jpeg";
	}
	else if (m_pPpmFormat->isChecked())
	{
		strFormat = "ppm";
	}
	else if (m_pBmpFormat->isChecked())
	{
		strFormat = "bmp";
	}

	return strFormat;
}

unsigned QxDecodeOptionDlg::imageSize() const
{
	unsigned int uImageSize = 32;
	if (m_pMedium->isChecked())
	{
		uImageSize = 64;
	}
	else if (m_pLarge->isChecked())
	{
		uImageSize = 128;
	}
	else if (m_pCustomize->isChecked())
	{
		bool bOK(false);
		int iImageSize = m_pImageSizeEdit->text().toInt(&bOK);
		if (bOK)
		{
			uImageSize = iImageSize;
		}
		else
		{
			uImageSize = UINT_DEFAULT_IMAGE_SIZE;
		}
	}

	return uImageSize;
}

void QxDecodeOptionDlg::setImageFormatOption()
{
	if (m_pCaffe->isChecked())
	{
		m_pPngFormat->setEnabled(true);
		m_pJpegFormat->setEnabled(true);
		m_pPpmFormat->setEnabled(true);
		m_pBmpFormat->setEnabled(true);
	}
	else if (m_pCNTK->isChecked())
	{
		m_pPngFormat->setEnabled(true);
		m_pJpegFormat->setEnabled(true);
		m_pPpmFormat->setEnabled(true);
		m_pBmpFormat->setEnabled(true);
	}
	else if (m_pDigits->isChecked())
	{
		m_pPngFormat->setEnabled(true);
		m_pJpegFormat->setEnabled(true);
		m_pPpmFormat->setEnabled(true);
		m_pBmpFormat->setEnabled(true);
	}
	else if (m_pTensorFlow->isChecked())
	{
		m_pPngFormat->setEnabled(true);
		m_pJpegFormat->setEnabled(true);
		m_pPpmFormat->setEnabled(false);
		m_pBmpFormat->setEnabled(false);
		if (!m_pPngFormat->isChecked() && !m_pJpegFormat->isChecked())
		{
			m_pPngFormat->setChecked(true);
		}
	}
}

QString QxDecodeOptionDlg::filePath() const
{
	return m_pFilePathEdit->text();
}
