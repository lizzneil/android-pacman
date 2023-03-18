/*
 * actions_events.h
 *
 *  Created on: 10.11.2012
 *      Author: Denis Zagayevskiy
 */

#ifndef ACTIONS_EVENTS_H_
#define ACTIONS_EVENTS_H_

enum Action{
	ACTION_NONE = 0,
	ACTION_DOWN = 1,
	ACTION_UP = 2,
	ACTION_MOVE = 4
};

enum EngineEvent{

	EVENT_NONE = 0,
	EVENT_LOADED = 1,
	EVENT_PLAY = 2,
	EVENT_RETRY = 3,
	EVENT_MAINMENU = 4,
	EVENT_STOP = 5,
	EVENT_NEXT_LEVEL = 6,

	EVENT_MOVE_LEFT = 20,
	EVENT_MOVE_UP = 21,
	EVENT_MOVE_RIGHT = 22,
	EVENT_MOVE_DOWN = 23,

	EVENT_MUSIC_ON = 30,
	EVENT_MUSIC_OFF = 31,
	EVENT_SOUNDS_ON = 32,
	EVENT_SOUNDS_OFF = 33

};

#endif /* ACTIONS_EVENTS_H_ */
