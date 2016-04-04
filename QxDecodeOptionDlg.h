#ifndef _QX_DECODE_OPTION_DIALOG_H_
#define _QX_DECODE_OPTION_DIALOG_H_

#include <QDialog>
#include <QPointer>

class QGroupBox;
class QLineEdit;
class QRadioButton;
class QString;

/*
	Dialog allowing user to select image path, image format/size, and target software(Caffe, Digits) and so on.
*/
class QxDecodeOptionDlg : public QDialog
{
public:
	QxDecodeOptionDlg(const QStringList& fileList, QWidget* parent = NULL);

	enum ApplicationType{ Caffe, CNTK, DIGITS, TensorFlow };

	// Selected target application
	ApplicationType application() const;
	// Selected image format
	QString imageFormat() const;
	// Selected file path
	QString filePath() const;
	// Selected/entered image size
	unsigned int imageSize() const;

private slots:
    // Re-implemented function, which ensures legal selections/inputs.
	void accept();
    void setImageFormatOption();
    void setSaveFilePath();
    void setImageSize();

private:
	// Init the dialog
	void initDialog(const QStringList& fileList);

private:
	QPointer<QGroupBox> m_pApplicationGroupBox;
	QPointer<QGroupBox> m_pImageFormatGroupBox;
	QPointer<QGroupBox> m_pImageSizeGroupBox;

	QPointer<QLineEdit> m_pFilePathEdit;
	QPointer<QLineEdit> m_pImageSizeEdit;

	QPointer<QRadioButton> m_pCaffe;
	QPointer<QRadioButton> m_pCNTK;
	QPointer<QRadioButton> m_pDigits;
	QPointer<QRadioButton> m_pTensorFlow;

    QPointer<QRadioButton> m_pBmpFormat;
    QPointer<QRadioButton> m_pJpegFormat;
	QPointer<QRadioButton> m_pPngFormat;
	QPointer<QRadioButton> m_pPpmFormat;

	QPointer<QRadioButton> m_pSmall;
	QPointer<QRadioButton> m_pMedium;
	QPointer<QRadioButton> m_pLarge;
	QPointer<QRadioButton> m_pCustomize;
};

#endif
