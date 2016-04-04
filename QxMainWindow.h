#ifndef _QX_MAIN_WINDOW_H_1339ECA_
#define _QX_MAIN_WINDOW_H_1339ECA_

#include <opencv2/core/core.hpp>

#include <QMainWindow>
#include <QMap>
#include <QPointer>

#include "QxDecodeOptionDlg.h"

class QLabel;
class QListWidget;

/*
	Main window of the application, allowing user to select/decode files through a graphic user interface.
*/
class QxMainWindow : public QMainWindow
{
	Q_OBJECT
public:
	QxMainWindow(QWidget* parent = NULL);
	virtual ~QxMainWindow();

private:
	//Decoded .gnt files based on the parameters. Return true when successfully decoding the files.
	bool decodeFiles(const QStringList& fileList, const QString& strDestinationPath, const QString& imageFormat, const cv::Size& imageSize, const QxDecodeOptionDlg::ApplicationType appType);

    //Generate the "image name   label" format string.
    QString getLabelInfo(const QString& strImageName, const quint32 uLabel, QxDecodeOptionDlg::ApplicationType appType);
	//Generate a proper file name according to application type.
	QString getSaveImageName(const QString& strImagePath, const quint32 uCode, const quint32 uIndex, QxDecodeOptionDlg::ApplicationType appType);

    //Init the widget.
    void initDialog();
    //Save the image names and corresponding labels into a .txt file for Caffe/CNTK/TensorFlow
    void saveLabelFile(const QString& strFilePath, QxDecodeOptionDlg::ApplicationType appType);
    //Save the mapping relationship between image labels and the GBK code of Chinese characters into a .txt file.
    void saveMappingFile(const QString& strFilePath, QxDecodeOptionDlg::ApplicationType appType);

private slots:
    //Clear file list.
    void clearFileList();
    //Hide preview image
    void closePreview();
    //Decode all the .gnt files in the file list.
    void decodeAll();
	//Decode selected .gnt file. in the file list.
	void decodeSelected();
    //Preview selected file
    void preview();
	//Remove selected files from filelist
	void removeSelectedFile();
    //Show "About" dialog
    void showAboutDialog();
    //Update file list.
    void setFileList();

private:
    QMap<quint32, quint32> m_LabelCodeMap;
    QMap<QString, quint32> m_ImageLabelMap;

	QPointer<QAction> m_pAboutAction;
    QPointer<QAction> m_pClearListAction;
	QPointer<QAction> m_pDecodeAction;
	QPointer<QAction> m_pDecodeAllAction;
    QPointer<QAction> m_pHidePreviewAction;
    QPointer<QAction> m_pOpenAction;
	QPointer<QAction> m_pRemoveAction;

	QPointer<QLabel> m_pPreviewLabel;
	QPointer<QListWidget> m_pFileListWidget;
	QPointer<QToolBar> m_pToolBar;

	QStringList m_FileList;
};
#endif
