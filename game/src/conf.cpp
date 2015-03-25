#include "stdafx.h"
#include "conf.h"

Conf::Conf()
{
}

std::string Conf::GetBackground() const
{
   return "img/BackGround.jpg";
}

std::vector<std::string> Conf::GetGems() const
{
   std::vector<std::string> gems;
	static const char* images[] = {"Red", "Green", "Purple", "Blue", "Yellow"};

	for(int i = 0; i < sizeof(images) / sizeof(*images); i++) 
	{
		const std::string resRoot = std::string("./img/") 
         + std::string(images[i]) 
         + std::string(".png");
		gems.push_back(resRoot);
	}	
	return gems;
}

std::string Conf::GetFontFile() const
{
	return "fonts/Anonymous Pro.ttf";
}