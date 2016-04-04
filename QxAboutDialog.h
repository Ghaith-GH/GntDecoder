#ifndef _QX_ABOUT_DIALOG_H_
#define _QX_ABOUT_DIALOG_H_

#include <QDialog>

/*
	Dialog showing basic information about this application.
*/
class QxAboutDialog : public QDialog
{
    Q_OBJECT
public:
	QxAboutDialog(QWidget* parent = NULL);
	virtual ~QxAboutDialog();

private:
	//Init the dialog.
	void initDialog();
};

#endif
