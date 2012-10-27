/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Maratis
// main.cpp
/////////////////////////////////////////////////////////////////////////////////////////////////////////

//========================================================================
//  Maratis, Copyright (c) 2003-2011 Anael Seghezzi <www.maratis3d.com>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software Foundation,
//  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
//
//========================================================================


#include <vector>
#include <time.h>
#include <stdio.h>

#include <MEngine.h>
#include <MLoaders/MDevILLoader.h>
#include <MLog.h>
#include "MFilesUpdate/MFilesUpdate.h"
#include "Maratis/Maratis.h"
#include "Maratis/MaratisUI.h"


// logo
bool logo = true;
unsigned int logoTextureId = 0;
void drawLogo(void)
{
	MWindow * window = MWindow::getInstance();

	MGui2d quad;
	quad.setPosition(MVector2((window->getWidth()-512)*0.5f, (window->getHeight()-512)*0.5f));
	quad.setScale(MVector2(512, 512));
	quad.drawTexturedQuad(logoTextureId);
}

// update
void update(void)
{
	if(! logo)
		Maratis::getInstance()->logicLoop();
}

// draw
void draw(void)
{
	MMouse * mouse = MMouse::getInstance();
	MRenderingContext * render = MEngine::getInstance()->getRenderingContext();
	Maratis::getInstance()->graphicLoop();

	if(logo)
	{
		set2dMode(render);
		render->setBlendingMode(M_BLENDING_ALPHA);
		render->enableTexture();
		render->setColor4(MVector4(1.0f));
		drawLogo();
	}

	if(mouse->isLeftButtonPushed() || mouse->isMiddleButtonPushed() || (mouse->getWheelDirection() != 0))
		logo = false;

	MWindow::getInstance()->swapBuffer();
}


// main
int main(int argc, char **argv)
{
    MLOG(6, "Entering main...");
	setlocale(LC_NUMERIC, "C");

	// get engine (first time call onstructor)
	MEngine * engine = MEngine::getInstance();

	// get window (first time call onstructor)
	MWindow * window = MWindow::getInstance();

	// create window
	bool b=window->create("Maratis", 1024,768, 32, false);
    if(!b)
        MLOG(4, "Cannot create window");

	// set current directory
	char rep[256];
	getRepertory(rep, argv[0]);
	window->setCurrentDirectory(rep);
    MLOG(5, "Current dir set to "<<rep);

	// get Maratis (first time call onstructor)
    MLOG(6, "Getting Maratis object...");
	Maratis * maratis = Maratis::getInstance();
    if (!maratis)
        MLOG(4, "Cannot get the Maratis instance");
	MRenderingContext * render = engine->getRenderingContext();
    if (!render)
        MLOG(4, "Cannot get rendering context from engine");

	// init gui
    MLOG(5, "main: init GUI...");
	MGui * gui = MGui::getInstance();
    if (!gui)
        MLOG(4, "Cannot get MGui instance");
	gui->setRenderingContext(render);
    MLOG(6, "main: adding default.tga...");
	gui->addFont(new MGuiTextureFont("font/default.tga"));

	// init MaratisUI
    MLOG(5, "main: init Maratis UI...")
	MaratisUI * UI = MaratisUI::getInstance();
	window->setPointerEvent(MaratisUI::windowEvents);

	// logo
	{
		MImage image;
		if(! M_loadImage("gui/Title.png", &image))
		{
            MLOG(4, "Cant load title image Title.png");
            return 0;
        }

		render->createTexture(&logoTextureId);
		render->bindTexture(logoTextureId);
		render->sendTextureImage(&image, false, false, false);

		// clear window
		draw();

		MGuiText text("LOADING", MVector2(480, 280), 16, MVector4(1, 1, 1, 1));
		text.draw();

		window->swapBuffer();
	}

	// load project
	if(argc > 1)
    {
		char filename[256];
		getGlobalFilename(filename, window->getCurrentDirectory(), argv[1]);
		maratis->loadProject(filename);
	}

	// time
	unsigned int frequency = 60;
	unsigned long previousFrame = 0;
	unsigned long startTick = window->getSystemTick();

	int f = 0;
	int t = 0;

    MLOG(7, "Entering main loop...");
	// on events
	while(window->isActive())
	{
		if(! engine->isActive())
		{
			UI->endGame();
			engine->setActive(true);
		}

		// on events
		if(window->onEvents())
		{
			if(window->getFocus())
			{
				// compute target tick
				unsigned long currentTick = window->getSystemTick();

				unsigned long tick = currentTick - startTick;
				unsigned long frame = (unsigned long)(tick * (frequency * 0.001f));

				// update elapsed time
				unsigned int i;
				unsigned int steps = (unsigned int)(frame - previousFrame);


				// don't wait too much
				if(steps >= (frequency/2))
				{
					update();
					draw();
					previousFrame += steps;
					continue;
				}

				// update
				for(i=0; i<steps; i++)
				{
					update();
					previousFrame++;
					t++;
					if(t == frequency)
					{
						MGame * game = engine->getGame();
						if(game)
						{
							if(! game->isRunning())
								MFilesUpdate::update();
						}
						else
						{
							MFilesUpdate::update();
						}

						//printf("fps:%d\n", f);
						t = 0;
						f = 0;
					}
				}

				// draw
				if(steps > 0)
				{
					draw();
					f++;
				}
			}
			else
			{
				window->sleep(0.1);
			}
		}

		//window->sleep(0.001); // 1 mili sec seems to slow down on some machines...
	}

	gui->clear();
	maratis->clear();
	return 0;
}
