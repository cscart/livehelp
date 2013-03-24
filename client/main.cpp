#include "stdafx.h"
#include "lh3.h"
#include <QtGui/QApplication>

#include "Translator.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	// settings params
	a.setOrganizationName("SimTech");
	a.setApplicationName("LiveHelp");

	// look style
#if !defined(Q_OS_MAC)
	a.setStyle("cleanlooks");
#endif

	CTranslator &translator(GetTranslator());
	a.installTranslator(&translator);

	lh3 w;
	w.show();
	return a.exec();
}
