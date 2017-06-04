#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <QApplication>
#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QImage>
#include <QLabel>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QPixmap>
#include <QProgressDialog>
#include <QResizeEvent>
#include <QSplitter>
#include <QString>
#include <QTextStream>
#include <QToolBar>

#include "QxAboutDialog.h"
#include "QxMainWindow.h"

// Manage the directories.
static QDir g_dirManager;
// Create a sub-folder named "images" in the selected folder to save the decoded images.
static const QString g_ImageFolderName = "images";
// Use a .txt file named "image_labels.txt" to save path of images and corresponding labels.
static const QString g_LabelFileName = "image_labels.txt";
// Use a local file named "code_labels.txt" to save the mapping relationship between image labels and gbk code of Chinese characters.
static const QString g_MappingFileName = "code_label.txt";


//Decoded .gnt files based on the parameters. Return true when successfully decoding the files.
bool QxMainWindow::decodeFiles(const QStringList& fileList, const QString& strDestinationPath, const QString& imageFormat, const cv::Size& imageSize, const QxDecodeOptionDlg::ApplicationType appType)
{
	// Create a sub-folder in the selected folder to save the decoded images.
	// If the folder already exists, ask user whether to append decoded images to it.
	QString strImagePath = strDestinationPath + "/" + g_ImageFolderName;
	if (!g_dirManager.exists(strImagePath)) 
	{ 
		g_dirManager.mkdir(strImagePath); 
	}
	else 
	{
		QString strTitle("Folder already exists");
		QString strMessage;
		strMessage.append("There is already a floder named \"").append(g_ImageFolderName).append("\" in the selected folder.\n");
		strMessage.append("Do you still want to save the decoded images to it?");
		QMessageBox::StandardButton button = QMessageBox::question(this, strTitle, strMessage, QMessageBox::Yes | QMessageBox::No);
		if (button != QMessageBox::Yes) { return false;	} 
	}

	// Remove former information, just in case the user does twice or more times decoding without restart the software.
	m_LabelCodeMap.clear();
	m_ImageLabelMap.clear();

	// decode files
	QStringList::size_type uFileAmount = fileList.size();
	QProgressDialog progressDlg("Decoding files...", "Cancel", 0, uFileAmount, this);
	progressDlg.setWindowModality(Qt::WindowModal);
	progressDlg.setMinimumDuration(0);
	for (QStringList::size_type i = 0; i != fileList.size(); ++i)
	{
		//Update progress dialog
		progressDlg.setValue(i);
		QApplication::processEvents();
		if (progressDlg.wasCanceled())
		{
			saveMappingFile(strDestinationPath, appType);
			saveLabelFile(strDestinationPath, appType);
			return false;
		}

		//Error handling
		QString strFileName = fileList.at(i);
		QFile file(strFileName);
		if (!file.open(QIODevice::ReadOnly))
		{
			QString strTitle("Open file error");
			QString strErrorMessage = "Cannot open selected file:\nFile name: " + strFileName + "\nContinue decoding the remaining files? ";
			QMessageBox::StandardButton button = QMessageBox::information(this, strTitle, strErrorMessage, QMessageBox::Yes | QMessageBox::No);
			if (button == QMessageBox::Yes) { continue; }
			else { return false; }
		}

		//Decode .gnt file specified by strFileName.
		quint64 uDecodedByteNum = 0;
		quint64 uDecodedByteNumTotal = 0;
		quint64 uTotalByteNum = file.bytesAvailable();
		QByteArray rawData;
		while (uDecodedByteNumTotal != uTotalByteNum)
		{
			rawData = file.read(4); // get data size of this character
			quint32 uDataLen = uchar(rawData.at(0)) + quint32(uchar(rawData.at(1))) * (1 << 8) + quint32(uchar(rawData.at(2))) * (1 << 16) + quint32(uchar(rawData.at(3))) * (1 << 24);
			rawData = file.read(uDataLen - 4); //4 bytes have beed decoded already
			quint32 uTagCode = uchar(rawData.at(0)) + quint32(uchar(rawData.at(1))) * (1 << 8);
			quint32 uWidth = uchar(rawData.at(2)) + quint32(uchar(rawData.at(3))) * (1 << 8);
			quint32 uHeight = uchar(rawData.at(4)) + quint32(uchar(rawData.at(5))) * (1 << 8);
			quint32 uArcLen = uWidth > uHeight ? uWidth : uHeight;
			uDecodedByteNum = 6;
			/* A character should be presented in a square. However, decoded images are, in most cases, rectangle.
			  Therefore, a decoded character image is padded to a square whose length of the side is uArcLen (longer edge of the rectangle). 
			  As the background of the decoded images is white (pixel value 255 for 8-bit grayscale images), all the padded pixels are set to be 255.  */
			cv::Mat img = 255 * cv::Mat::ones(uArcLen, uArcLen, CV_8UC1);
			int iHalfPadRowNum = (uArcLen - uHeight) / 2;
			int iHalfPadColNum = (uArcLen - uWidth) / 2;
            for (quint32 row = iHalfPadRowNum; row < uHeight + iHalfPadRowNum; ++row)
			{
				uchar *pRow = img.ptr<uchar>(row);
                for (quint32 col = iHalfPadColNum; col < uWidth + iHalfPadColNum; ++col)
				{
					pRow[col] = uchar(rawData.at(uDecodedByteNum++));
				}
			}
			// update mapping table, because label of the input images should, optimally, start from 0 and be consecutive.
			if (!m_LabelCodeMap.contains(uTagCode))
			{
				quint32 uNewLabel = m_LabelCodeMap.size();
				m_LabelCodeMap[uTagCode] = uNewLabel;
			}
			// image normalization and saving
			QString strSaveFileName = getSaveImageName(strImagePath, uTagCode, i + 1, appType);
			strSaveFileName += "." + imageFormat;
			cv::resize(img, img, imageSize);
			cv::imwrite(strSaveFileName.toStdString(), img);
			m_ImageLabelMap[strSaveFileName] = m_LabelCodeMap[uTagCode];
			uDecodedByteNumTotal += uDataLen;
		}
		file.close();
	}
	// Save the .txt files for different software
	saveMappingFile(strDestinationPath, appType);
	saveLabelFile(strDestinationPath, appType);
	progressDlg.setValue(uFileAmount); 
	return true;
}

//Generate a proper file name according to application type.
QString QxMainWindow::getSaveImageName(const QString& strImagePath, const quint32 uCode, const quint32 uIndex, QxDecodeOptionDlg::ApplicationType appType)
{
	QString strImageName;
	if (appType == QxDecodeOptionDlg::Caffe)
	{
		strImageName = strImagePath + "/" + QString::number(uCode) + "-" + QString::number(uIndex);
	}
	else if (appType == QxDecodeOptionDlg::CNTK)
	{
		strImageName = strImagePath + "/" + QString::number(uCode) + "-" + QString::number(uIndex);
	}
	else if (appType == QxDecodeOptionDlg::TensorFlow)
	{
		strImageName = strImagePath + "/" + QString::number(uCode) + "-" + QString::number(uIndex);
	}
	else  //DIGITS
	{
		// First, check whether the subfolder for this class exists.
		strImageName = strImagePath + "/" + QString::number(uCode);
		if (!g_dirManager.exists(strImageName))
		{
			g_dirManager.mkdir(strImageName);
		}
		g_dirManager.setPath(strImageName);
		g_dirManager.setFilter(QDir::Files);
		//For example, if there are already 5 images in the folder, the new image will be named 6 (plus image suffix).
		strImageName = strImageName + "/" + QString::number(g_dirManager.count()+1);
	}

	return strImageName;
}

//Generate the "image name   label" format string.
QString QxMainWindow::getLabelInfo(const QString& strImageName, const quint32 uLabel, QxDecodeOptionDlg::ApplicationType appType)
{
	QString strToken;
	switch (appType)
	{
	case QxDecodeOptionDlg::Caffe:
		strToken = " "; break;
	case QxDecodeOptionDlg::CNTK:
		strToken = "\t"; break;
	case QxDecodeOptionDlg::TensorFlow:
		strToken = " "; break;
	case QxDecodeOptionDlg::DIGITS:
		strToken = " "; break;
	default:
		strToken = " ";
	}
	return strImageName + strToken + QString::number(uLabel);
}

//Constructor
QxMainWindow::QxMainWindow(QWidget* parent/*=NULL*/)
	: QMainWindow(parent)
{
	QMap<quint32, quint32> m_LabelCodeMap;
	QMap<QString, quint32> m_ImageLabelMap;
	initDialog();
}

//Destructor
QxMainWindow::~QxMainWindow()
{
}

//Init the dialog
void QxMainWindow::initDialog()
{
	// Menu
    QPointer<QMenu> pFileMenu = menuBar()->addMenu("File");
    QPointer<QMenu> pViewMenu = menuBar()->addMenu("View");
    QPointer<QMenu> pHelpMenu = menuBar()->addMenu("Help");
	m_pOpenAction = pFileMenu->addAction("Open");
	m_pDecodeAction = pFileMenu->addAction("Decode Selected");
	m_pDecodeAllAction = pFileMenu->addAction("Decode All");
	m_pRemoveAction = pFileMenu->addAction("Remove Selected");
	m_pClearListAction = pFileMenu->addAction("Remove All");
	m_pHidePreviewAction = pViewMenu->addAction("Hide Preview");
	m_pAboutAction = pHelpMenu->addAction("About gntDecoder");

	m_pOpenAction->setIcon(QIcon(":/Resources/open.ico"));
	m_pDecodeAllAction->setIcon(QIcon(":/Resources/save_all.ico"));
	m_pAboutAction->setIcon(QIcon(":/Resources/about.ico"));
	m_pDecodeAction->setIcon(QIcon(":/Resources/save.png"));
	m_pRemoveAction->setIcon(QIcon(":/Resources/remove.ico"));
	m_pClearListAction->setIcon(QIcon(":/Resources/clear.ico"));
	m_pHidePreviewAction->setIcon(QIcon(":/Resources/hide.png"));

	m_pOpenAction->setToolTip("Add .gnt files to file list");
	m_pDecodeAction->setToolTip("Decode selected file");
	m_pDecodeAllAction->setToolTip("Decode all files in the file list");
	m_pRemoveAction->setToolTip("Remove selected file from file list(Do nothing with local files)");
	m_pClearListAction->setToolTip("Clear file list(Do nothing with local files)");
	m_pHidePreviewAction->setToolTip("Hide preview image");
	
	// Toolbar
	m_pToolBar = new QToolBar;
	addToolBar(m_pToolBar);
	m_pToolBar->addAction(m_pOpenAction);
	m_pToolBar->addAction(m_pDecodeAction);
	m_pToolBar->addAction(m_pDecodeAllAction);
	m_pToolBar->addAction(m_pRemoveAction);
	m_pToolBar->addAction(m_pClearListAction);
	m_pToolBar->addAction(m_pHidePreviewAction);

	// window widgets
	m_pFileListWidget = new QListWidget;
	m_pPreviewLabel = new QLabel;
	m_pPreviewLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QPointer<QSplitter> pSplitter = new QSplitter(Qt::Horizontal);
	pSplitter->addWidget(m_pFileListWidget);
	pSplitter->addWidget(m_pPreviewLabel);

	// window style
	setCentralWidget(pSplitter);
	setWindowFlags(windowFlags() & ~Qt::WindowMinMaxButtonsHint);

	// connect signals/slots
    connect(m_pOpenAction.data(), &QAction::triggered, this, &QxMainWindow::setFileList);
    connect(m_pDecodeAction.data(), &QAction::triggered, this, &QxMainWindow::decodeSelected);
    connect(m_pDecodeAllAction.data(), &QAction::triggered, this, &QxMainWindow::decodeAll);
    connect(m_pClearListAction.data(), &QAction::triggered, this, &QxMainWindow::clearFileList);
    connect(m_pRemoveAction.data(), &QAction::triggered, this, &QxMainWindow::removeSelectedFile);
    connect(m_pHidePreviewAction.data(), &QAction::triggered, this, &QxMainWindow::closePreview);
    connect(m_pFileListWidget.data(), &QListWidget::itemSelectionChanged, this, &QxMainWindow::preview);
    connect(m_pAboutAction.data(), &QAction::triggered, this, &QxMainWindow::showAboutDialog);
}

//Update file list.
void QxMainWindow::setFileList()
{
	QStringList fileList = QFileDialog::getOpenFileNames(this, "Select Files to decode", "..", "*.gnt");

	//If some files are already in the file list.
	QStringList::size_type originalSize = m_FileList.size();
	if (!fileList.isEmpty())
	{
		m_FileList.append(fileList);
		//Remove duplicate items, this will guarantee a safe behavior when adding new items below
		m_FileList.removeDuplicates();
	}
	for (QStringList::iterator itr = m_FileList.begin() + originalSize; itr != m_FileList.end(); ++itr)
	{
		QListWidgetItem *pNewItem = new QListWidgetItem(*itr);
		m_pFileListWidget->addItem(pNewItem);
	}
}

//Decode selected .gnt file. in the file list.
void QxMainWindow::decodeSelected()
{
	QList<QListWidgetItem*> selectedItems = m_pFileListWidget->selectedItems();
	if (m_FileList.isEmpty() || selectedItems.size()==0)
	{
		QString strTitle("No selected files");
		QString strMessage("No files selected yet. Please select the file you want to decode.\n");
		strMessage.append("If there are no files listed in the file list, please press \"Open\" to select files first.");
		QMessageBox::information(this, strTitle, strMessage, QMessageBox::Ok);
		return;
	}
	QStringList fileList;
	for (QList<QListWidgetItem*>::iterator itr = selectedItems.begin(); itr != selectedItems.end(); ++itr)
	{
		fileList.push_back((*itr)->text());
	}
	QxDecodeOptionDlg dlg(fileList, this);
	if (dlg.exec() != QxDecodeOptionDlg::Accepted)
	{
		return;
	}

	// get selected application, image size, as well as image format
	unsigned uImageSize = dlg.imageSize();
	cv::Size imageSize(uImageSize, uImageSize);
	QString strImageFormat = dlg.imageFormat();
	QString strDestinationPath = dlg.filePath();
	QxDecodeOptionDlg::ApplicationType appType = dlg.application();

	//If files are successfully decode, show a messagebox to inform user.
	if (decodeFiles(fileList, strDestinationPath, strImageFormat, imageSize, appType))
	{
		QString strMessage("Files successfully decoded.");
		QMessageBox::information(this, "Result", strMessage, QMessageBox::Ok);
	}
}

//Decode all the .gnt files in the file list.
void QxMainWindow::decodeAll()
{
	if (m_FileList.isEmpty())
	{
		QString strTitle("No selected files");
		QString strMessage("No files selected yet. Please press \"Open\" to select files first.");
		QMessageBox::information(this, strTitle, strMessage, QMessageBox::Ok);
		return;
	}

	// If user clicks "Cancel" button when selection decoding options, do not decode files.
	QxDecodeOptionDlg dlg(m_FileList, this);
	if (dlg.exec() != QxDecodeOptionDlg::Accepted)
	{
		return;
	}

	// get selected application, image size, as well as image format
	unsigned uImageSize = dlg.imageSize();
	cv::Size imageSize(uImageSize, uImageSize);
	QString strImageFormat = dlg.imageFormat();
	QString strDestinationPath = dlg.filePath();
	QxDecodeOptionDlg::ApplicationType appType = dlg.application();

	//If files are successfully decode, show a messagebox to inform user.
	if (decodeFiles(m_FileList, strDestinationPath, strImageFormat, imageSize, appType))
	{
		QString strMessage("Files successfully decoded.");
		QMessageBox::information(this, "Result", strMessage, QMessageBox::Ok);
	}
}

//Save the mapping relationship between image labels and the GBK code of Chinese characters into a .txt file.
void QxMainWindow::saveMappingFile(const QString& strFilePath, QxDecodeOptionDlg::ApplicationType appType)
{
	// There's no need to store label files for DIGITS.
	if (appType == QxDecodeOptionDlg::DIGITS)
	{
		return;
	}

	QString strFileName = strFilePath + "/" + g_MappingFileName;
	QFile file(strFileName);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		return;
	}

	const int iWordWidth = 10;
	QTextStream textStream(&file);
	textStream << qSetFieldWidth(iWordWidth) << left << "Code" << "Label" << qSetFieldWidth(0) << "\n";
	for (QMap<quint32, quint32>::iterator itr = m_LabelCodeMap.begin(); itr != m_LabelCodeMap.end(); ++itr)
	{
		textStream << qSetFieldWidth(iWordWidth) << left << itr.key() << itr.value() << qSetFieldWidth(0) << "\n";
	}
	file.close();
}

//Save the image names and corresponding labels into a .txt file for Caffe/CNTK/TensorFlow
void QxMainWindow::saveLabelFile(const QString& strFilePath, QxDecodeOptionDlg::ApplicationType appType)
{
	// There's no need to store label files for DIGITS.
	if (appType == QxDecodeOptionDlg::DIGITS)
	{
		return;
	}

	// Use a local file to save path of images and corresponding labels.
	QString strLabelFileName = strFilePath + "/" + g_LabelFileName;
	QFile labelFile(strLabelFileName);
	if (!labelFile.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		QString strTitle("Open file error");
		QString strMessage("Can not open label file:\n");
		strMessage.append(strLabelFileName);
		strMessage.append("\nMaybe you do not have permission to create a new file in the selected folder?");
		strMessage.append("\nOr maybe there is a Read-Only file named \"").append(g_LabelFileName).append("\" in the selected folder?");
		QMessageBox::critical(this, strTitle, strMessage, QMessageBox::Ok);
		return;
	}

	QTextStream textStream(&labelFile);
	for (QMap<QString, quint32>::iterator itr = m_ImageLabelMap.begin(); itr != m_ImageLabelMap.end(); ++itr)
	{
		textStream << getLabelInfo(itr.key(), itr.value(), appType) << "\n";
	}
	labelFile.close();
}

//Clear file list.
void QxMainWindow::clearFileList()
{
	m_pFileListWidget->clear();
	m_FileList.clear();
	m_pPreviewLabel->hide();
}

//Remove selected files from filelist
void QxMainWindow::removeSelectedFile()
{
	// Safety check.
	QList<QListWidgetItem *> selectedItems = m_pFileListWidget->selectedItems();
	if (selectedItems.isEmpty())
	{
		QMessageBox::information(this, "No selected file", "Please select the file you want to remove", QMessageBox::Ok);
		return;
	}

	QListWidgetItem  *pCurrentItem = m_pFileListWidget->currentItem();
	int iCurrentIndex = m_pFileListWidget->row(pCurrentItem);

	m_FileList.removeOne(pCurrentItem->text());
	m_pFileListWidget->removeItemWidget(pCurrentItem);
	delete pCurrentItem;

	if (!m_pFileListWidget->count()) //no items left after removing this one
	{
		m_pPreviewLabel->hide();
		return;
	}
	else if (iCurrentIndex == m_pFileListWidget->count()) //removed item was the last item
	{
		m_pFileListWidget->setCurrentRow(iCurrentIndex - 1);
	}
	else
	{
		m_pFileListWidget->setCurrentRow(iCurrentIndex);
	}
	preview();
}

//Preview selected file
void QxMainWindow::preview()
{
	QListWidgetItem *pCurrentItem = m_pFileListWidget->currentItem();
	if (!pCurrentItem)
	{
		return;
	}
	QString strFileName = pCurrentItem->text();
	QFile file(strFileName);
	if (!file.open(QIODevice::ReadOnly))
	{
		return;
	}

	cv::Size imageSize(640,448);	//the image used to preview some of the characters
	const unsigned uCharacterWidth = 64;	//the width of each character in the preview image
	const unsigned uCharacterHeight = 64;	//the height of each character in the preview image
	cv::Size smallerSize(54, 54);; //to guarantee a certain distance among characters, characters are presented in smaller size than they are.
	const quint32 uInterval = 5;  //this should be calculated by (characterSize.width-smallerSize.width)/2;
	quint32 uCharacterPerRow = imageSize.width / uCharacterWidth;
	quint32 uCharacterPerCol = imageSize.height / uCharacterHeight;
	cv::Mat img = 255 * cv::Mat::ones(imageSize, CV_8UC1);

	quint32 uDecodedByteNum = 0;	//As the data is read as a stream, this variable tells how many bytes are processed already
	for (quint32 i = 0; i != uCharacterPerRow; ++i)
	{
		for (quint32 j = 0; j != uCharacterPerCol; ++j)
		{
			QByteArray rawData = file.read(4);
			quint32 uDataLen = uchar(rawData.at(0)) + quint32(uchar(rawData.at(1))) * (1 << 8) + quint32(uchar(rawData.at(2))) * (1 << 16) + quint32(uchar(rawData.at(3))) * (1 << 24);
			rawData = file.read(uDataLen - 4); //4 bytes have beed decoded already
			quint32 uWidth = uchar(rawData.at(2)) + quint32(uchar(rawData.at(3))) * (1 << 8);
			quint32 uHeight = uchar(rawData.at(4)) + quint32(uchar(rawData.at(5))) * (1 << 8);
			quint32 uArcLen = uWidth > uHeight ? uWidth : uHeight;
			uDecodedByteNum = 10;

			// save data to a pre-defined white image(all pixel values are pre-defined to be 255)
			cv::Mat characterImage = 255 * cv::Mat::ones(uArcLen, uArcLen, CV_8UC1);
			quint32 uHalfPadRowNum = (uArcLen - uHeight) / 2;
			quint32 uHalfPadColNum = (uArcLen - uWidth) / 2;
			for (quint32 row = uHalfPadRowNum; row != uHeight + uHalfPadRowNum; ++row)
			{
				uchar *pRow = characterImage.ptr<uchar>(row);
				for (quint32 col = uHalfPadColNum; col != uWidth + uHalfPadColNum; ++col)
				{
					pRow[col] = uchar(rawData.at(uDecodedByteNum++));
				}
			}
			// image normalization and filling
			cv::resize(characterImage, characterImage, smallerSize);
			cv::Range rowRange(j*uCharacterWidth, (j + 1)*uCharacterWidth);
			cv::Range colRange(i*uCharacterHeight, (i + 1)*uCharacterHeight);
			cv::Mat roi = img(rowRange, colRange);
			//cv::Mat::copyTo() calls cv::Mat::create(), which breaks the original image.
			//Also, cv::Mat::clone() fails to copy the values properly somehow, thus a pixelwise copy is implemented here.
            for (int row = 0; row != smallerSize.height; ++row)
			{
				uchar *pDst = roi.ptr<uchar>(row + uInterval);
				uchar *pSrc = characterImage.ptr<uchar>(row);
                for (int col = 0; col != smallerSize.width; ++col)
				{
					pDst[col + uInterval] = pSrc[col];
				}
			}
		}
	}
	file.close();

	QImage previewImage(img.data, img.cols, img.rows, img.step, QImage::Format_Grayscale8);
	QPixmap pixmap = QPixmap::fromImage(previewImage);
	m_pPreviewLabel->setPixmap(pixmap);
	m_pPreviewLabel->show();
}

//Hide preview image
void QxMainWindow::closePreview()
{
	m_pPreviewLabel->hide();
}

//Show "about" dialog
void QxMainWindow::showAboutDialog()
{
	QxAboutDialog dlg(this);
	dlg.exec();
}
