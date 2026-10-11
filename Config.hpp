#pragma once
#include <Geode/Geode.hpp>
#include <algorithm>

using namespace geode::prelude;

namespace tuff {

struct Config {
	// Player
	bool noclip = false;
	bool deathCounter = false;
	bool jumpHack = false;

	// Level
	bool speedhack = false;
	float speed = 1.f;
	bool autoClicker = false;
	int cps = 10;
	bool hitboxes = false;

	// Cosmetics
	bool noDeathEffect = false;
	bool noSpiderDash = false;
	bool noWaveTrail = false;
	bool noTrail = false;
	bool noShake = false;
	bool hideAttempts = false;

	// Labels
	bool labelFps = false;
	bool labelCps = false;
	bool labelTps = false;
	bool labelRunFrom = false;
	bool labelAttempt = false;
	bool labelPercent = false;
	bool labelClock = false;

	// Bypass
	bool unlockIcons = false;

	// Interface color (RGB)
	int r = 40;
	int g = 120;
	int b = 255;

	// runtime only (not saved)
	int deaths = 0;
};

inline Config& cfg() {
	static Config c;
	return c;
}

template <class T>
void sync(bool load, char const* key, T& value) {
	auto mod = Mod::get();
	if (load) value = mod->getSavedValue<T>(key, value);
	else mod->setSavedValue<T>(key, value);
}

inline void io(bool load) {
	auto& c = cfg();
	sync(load, "noclip", c.noclip);
	sync(load, "death-counter", c.deathCounter);
	sync(load, "jump-hack", c.jumpHack);
	sync(load, "speedhack", c.speedhack);
	sync(load, "speed", c.speed);
	sync(load, "auto-clicker", c.autoClicker);
	sync(load, "cps", c.cps);
	sync(load, "hitboxes", c.hitboxes);
	sync(load, "no-death-effect", c.noDeathEffect);
	sync(load, "no-spider-dash", c.noSpiderDash);
	sync(load, "no-wave-trail", c.noWaveTrail);
	sync(load, "no-trail", c.noTrail);
	sync(load, "no-shake", c.noShake);
	sync(load, "hide-attempts", c.hideAttempts);
	sync(load, "label-fps", c.labelFps);
	sync(load, "label-cps", c.labelCps);
	sync(load, "label-tps", c.labelTps);
	sync(load, "label-runfrom", c.labelRunFrom);
	sync(load, "label-attempt", c.labelAttempt);
	sync(load, "label-percent", c.labelPercent);
	sync(load, "label-clock", c.labelClock);
	sync(load, "unlock-icons", c.unlockIcons);
	sync(load, "ui-r", c.r);
	sync(load, "ui-g", c.g);
	sync(load, "ui-b", c.b);
	if (load) {
		c.speed = std::clamp(c.speed, 0.01f, 20.f);
		c.cps = std::clamp(c.cps, 1, 100);
		c.r = std::clamp(c.r, 0, 255);
		c.g = std::clamp(c.g, 0, 255);
		c.b = std::clamp(c.b, 0, 255);
	}
}

inline void loadCfg() { io(true); }
inline void saveCfg() { io(false); }

inline ccColor3B accent() {
	auto& c = cfg();
	return {(GLubyte)c.r, (GLubyte)c.g, (GLubyte)c.b};
}

}
