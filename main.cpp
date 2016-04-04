
#include <QtWidgets/QApplication>

#include "QxDecodeOptionDlg.h"
#include "QxMainWindow.h"

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);

	QxMainWindow mainWindow;
	mainWindow.show();

	return app.exec();
}
