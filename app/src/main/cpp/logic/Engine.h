/*
 * Engine.h
 *
 *  Created on: 10.11.2012
 *      Author: Denis Zagayevskiy
 */

#ifndef ENGINE_H_
#define ENGINE_H_

#include "log.h"
#include "actions_events.h"

#include "StateMachine.h"
#include "Game.h"
#include "graphics/ui/menu/MainMenu.h"
#include "graphics/ui/menu/GameMenu.h"
#include "graphics/ui/menu/SwipeGameMenu.h"
#include "graphics/ui/menu/GameOverMenu.h"
#include "graphics/ui/menu/PauseMenu.h"
#include "graphics/ui/menu/WinMenu.h"

#include "managers/Art.h"
#include "managers/Audio.h"
#include "managers/Statistics.h"


class Engine: public StateMachine, public IRenderable {
public:

	void initGraphics(int width, int height);
	void initLogic();
	void step(double elapsedTime);
	bool stop();
	void render(double elapsedTime);
	void save();
	void load();

	void performAction(Action act, float x, float y);

	virtual ~Engine();

private:

	static const char* NAME_STATE;
	static const char* NAME_LEVEL_TO_LOAD_NUMBER;
	static const char* NAME_SAVE_OK;

	enum EngineState{
		STATE_LOADING = 0,
		STATE_AFTER_LOADING = 1,
		STATE_MAIN_MENU = 2,
		STATE_PLAY = 3,
		STATE_PAUSE = 4,
		STATE_GAME_OVER = 5,
		STATE_WIN = 6
	};

	EngineState state;
	EngineEvent lastEvent;
	int levelToLoadNumber;
	bool exitOnStop;

	GLuint stableProgram, vertexHandle, textureHandle, matrixHandle, mapHandle;
	GLuint shiftProgram, shiftMapHandle, shiftMatrixHandle;

	float screenPixelWidth, screenPixelHeight;
	float maxX, maxY;

	Menu* currentMenu;
	MainMenu* mainMenu;
	SwipeGameMenu* gameMenu;
	GameOverMenu* gameOverMenu;
	PauseMenu* pauseMenu;
	WinMenu* winMenu;

	Game* game;

	void setState(EngineState nextState);
	inline void setMenu(Menu* menu){
		currentMenu = menu;
		menu->onShow();
	}

	bool setupGraphics(int width, int height);
};

#endif /* ENGINE_H_ */
