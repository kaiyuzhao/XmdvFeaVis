#include <RInside.h>
#include "main/XmdvTool.h"
//#define Realloc(p,n,t) (t *) R_chk_realloc( (void *)(p), (size_t)((n) * sizeof(t)) )
//#undef Realloc
//#define Realloc(p,n,t) (t *) R_chk_realloc( (void *)(p), (size_t)((n) * sizeof(t)) )



#include "main/Globals.h"
#include "main/XmdvToolMainWnd.h"

#include <QApplication>
#include <QMessageBox>
#include <QGLFormat>
#include <QGLWidget>


Globals g_globals;

//using namespace Rcpp;

int main(int argc, char *argv[])
{

    RInside R;

	QApplication a(argc, argv);

	// Check whether the current system configuration supports the OpenGL
	if (!QGLFormat::hasOpenGL() ) {
		QMessageBox::information(0, "XmdvTool",
				"This system does not support OpenGL rendering.  XmdvTool cannot start.");
		return -1;
	}

	QGLWidget* tryGLWidget = new QGLWidget( (QWidget*)0 );
	if ( !tryGLWidget->isValid() ) {
		QMessageBox::information(0, "XmdvTool",
				"This system does not support OpenGL rendering.  XmdvTool cannot start.");
		delete tryGLWidget;
		return -1;
	}
	delete tryGLWidget;

    XmdvToolMainWnd w(NULL,R);
	w.show();
	a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));
	w.openInitialDatasets();
    //delete &R;
	return a.exec();
}
