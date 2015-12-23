#include "Application.hpp"

int main(int argc, const char** argv)
{
#ifdef WIN32
	setlocale(LC_ALL, "");
#endif

	Application app;

	app.init(argc, argv);
	app.run();

	return 0;
}
