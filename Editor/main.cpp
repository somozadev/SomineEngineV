#include "Application.h"
#include<cstdio>

int main()
{
	Application app; 
	if (!app.init())
	{
		std::fprintf(stderr, "Failed to initialise application \n");
		return 1;
	}
	app.run();
	app.shutdown();
	return 0;
}