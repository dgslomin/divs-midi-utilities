
#include <QApplication>
#include <QMessageBox>
#include "window.h"

int main(int argc, char** argv)
{
	QApplication application(argc, argv);
	application.setOrganizationName("Sreal");
	application.setOrganizationDomain("sreal.com");
	application.setApplicationName("Seqer");
	Window window(NULL);
	window.show();
	if (argc > 1) window.open(argv[1]);
	return application.exec();
}

