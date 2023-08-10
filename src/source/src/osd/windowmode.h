/** @file windowmode.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.10.01

	@brief [ window mode ]
*/

#ifndef WINDOW_MODE_H
#define WINDOW_MODE_H

#include "../cptrlist.h"
#include "../res/resource.h"

class EMU;

extern EMU *emu;

#define WINDOW_MODE_MAX (ID_SCREEN_WINDOW8 - ID_SCREEN_WINDOW1 + 1)
/// window size
class CWindowMode
{
public:
	CWindowMode();
	CWindowMode(int power_, int width_, int height_);
	~CWindowMode();

	int power;	///< magnify
	int width;	///< client width
	int height;	///< client height

	void Set(int power_, int width_, int height_);
	bool Match(int power_, int width_, int height_) const;
};

/// Manage window size list
class CWindowModes : public CPtrList<CWindowMode>
{
private:
	static bool greater(const CWindowMode *a, const CWindowMode *b);

public:
	CWindowModes();

	void Sort();
	int Find(int power_, int width_, int height_) const;
};

/**
	@brief Remain showable window size on one monitor
*/
class WindowMode
{
private:
	CWindowModes window_modes;

public:
	WindowMode();
	~WindowMode();

	void Enum(int max_width, int max_height);

	int Count() const { return window_modes.Count(); }
	const CWindowMode *Get(int num) const;
	int Find(int width, int height) const;
};

#endif /* WINDOW_MODE_H */
