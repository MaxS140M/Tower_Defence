#include "header.h"

#include <ctime>


// Needs one of the following #includes, to include the class definition
#include "SimpleDemo.h"
#include "BouncingBallMain.h"
#include "MazeDemoMain.h"

#include "FlashingDemo.h"
#include "StarfieldDemo.h"
#include "ImageMappingDemo.h"
#include "ZoomingDemo.h"
#include "DraggingDemo.h"

//MyDemos
#include "MyDemoA.h"

//my cw 1 engine
#include "PsymsEngine.h"

// my cw 2 engine

#include "MainEngine.h"



// These are passed to initialise to determine the window size
const int BaseScreenWidth = 1300;
const int BaseScreenHeight = 800;

int doProgram(int argc, char *argv[])
{ 
	int iResult = 0;
	MainEngine oMainDemoObject;


	char buf[1024];
	// Screen caption can be set on following line...
	snprintf(buf, sizeof(buf), "BuildTowers : Size %d x %d", BaseScreenWidth, BaseScreenHeight);
	iResult = oMainDemoObject.initialise(buf, BaseScreenWidth, BaseScreenHeight, "Cornerstone Regular.ttf", 24);

	iResult = oMainDemoObject.mainLoop();
	oMainDemoObject.deinitialise();
	return iResult;
}

int main(int argc, char *argv[])
{

	::srand( (unsigned int)time(0));

	int iResult = doProgram( argc, argv );

	ImageManager::destroyImageManager();

#if defined(_MSC_VER)
#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif
#endif

	return iResult;
}
