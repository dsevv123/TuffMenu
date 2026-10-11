#include "Config.hpp"
#include "Floating.hpp"

#include <Geode/modify/CCScheduler.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/GameManager.hpp>
#include <Geode/modify/GameStatsManager.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>

#include <chrono>
#include <ctime>
#include <deque>

using namespace tuff;
using namespace std::chrono_literals;

namespace {

using Clock = std::chrono::steady_clock;

struct Stats {
	Clock::time_point t0 = Clock::now();
	int frames = 0;
	int steps = 0;
	float fps = 0.f;
	float tps = 0.f;
	std::deque<Clock::time_point> clicks;
	float runFrom = 0.f;
	bool hitNow = false;
	bool hitPrev = false;
	float clickAcc = 0.f;
	bool clickDown = false;
};

Stats& st() {
	static Stats s;
	return s;
}

int cpsNow() {
	auto& s = st();
	auto now = Clock::now();
	while (!s.clicks.empty() && now - s.clicks.front() > 1s) s.clicks.pop_front();
	return static_cast<int>(s.clicks.size());
}

// Slows / speeds up the audio together with the game.
void applyPitch(float pitch) {
	static float last = 1.f;
	if (pitch == last) return;
	last = pitch;
	auto engine = FMODAudioEngine::sharedEngine();
	if (!engine || !engine->m_system) return;
	FMOD::ChannelGroup* group = nullptr;
	engine->m_system->getMasterChannelGroup(&group);
	if (group) group->setPitch(pitch);
}

bool isCube(PlayerObject* p) {
	return !(p->m_isShip || p->m_isBird || p->m_isBall || p->m_isDart || p->m_isRobot || p->m_isSpider || p->m_isSwing);
}

}

$on_mod(Loaded) {
	loadCfg();
}

// ---------------------------------------------------------------------------
// Floating button (spawned once, survives scene changes)
// ---------------------------------------------------------------------------
class $modify(TuffMenuLayer, MenuLayer) {
	bool init() {
		if (!MenuLayer::init()) return false;
		static bool spawned = false;
		if (!spawned) {
			spawned = true;
			if (auto btn = FloatingButton::create()) SceneManager::get()->keepAcrossScenes(btn);
		}
		return true;
	}
};

// ---------------------------------------------------------------------------
// Speedhack (game + music)
// ---------------------------------------------------------------------------
class $modify(TuffScheduler, CCScheduler) {
	void update(float dt) {
		auto& c = cfg();
		float speed = c.speedhack ? std::clamp(c.speed, 0.01f, 20.f) : 1.f;
		applyPitch(std::clamp(speed, 0.1f, 4.f)); // FMOD pitch is only sane within this range
		CCScheduler::update(dt * speed);
	}
};

// ---------------------------------------------------------------------------
// Camera shake
// ---------------------------------------------------------------------------
class $modify(TuffBaseLayer, GJBaseGameLayer) {
	void shakeCamera(float duration, float strength, float interval) {
		if (cfg().noShake) return;
		GJBaseGameLayer::shakeCamera(duration, strength, interval);
	}
};

// ---------------------------------------------------------------------------
// Player hooks: jump hack, auto clicker, TPS, cosmetics
// ---------------------------------------------------------------------------
class $modify(TuffPlayer, PlayerObject) {
	bool pushButton(PlayerButton btn) {
		auto& c = cfg();
		auto pl = PlayLayer::get();
		bool isP1 = pl && pl->m_player1 == this;

		if (c.jumpHack && btn == PlayerButton::Jump && isCube(this)) {
			m_isOnGround = true;
		}
		if (isP1 && btn == PlayerButton::Jump) st().clicks.push_back(Clock::now());
		return PlayerObject::pushButton(btn);
	}

	void update(float dt) {
		auto& c = cfg();
		auto& s = st();
		auto pl = PlayLayer::get();

		if (pl && pl->m_player1 == this) {
			s.steps++;

			// auto clicker: one full click (down+up) every 1/cps seconds, driven by physics steps
			if (c.autoClicker && !m_isDead) {
				s.clickAcc += dt;
				float half = 0.5f / static_cast<float>(std::clamp(c.cps, 1, 100));
				if (s.clickAcc >= half) {
					s.clickAcc = std::fmod(s.clickAcc, half);
					s.clickDown = !s.clickDown;
					pl->handleButton(s.clickDown, 1, true);
				}
			} else if (s.clickDown) {
				s.clickDown = false;
				s.clickAcc = 0.f;
				pl->handleButton(false, 1, true);
			}
		}

		PlayerObject::update(dt);

		if (m_waveTrail) m_waveTrail->setVisible(!c.noWaveTrail);
		if (m_regularTrail) m_regularTrail->setVisible(!c.noTrail);
	}

	void playDeathEffect() {
		if (cfg().noDeathEffect) return;
		PlayerObject::playDeathEffect();
	}

	void playSpiderDashEffect(CCPoint from, CCPoint to) {
		if (cfg().noSpiderDash) return;
		PlayerObject::playSpiderDashEffect(from, to);
	}
};

// ---------------------------------------------------------------------------
// PlayLayer: noclip + death counter, hitboxes, labels
// ---------------------------------------------------------------------------
class $modify(TuffPlayLayer, PlayLayer) {
	struct Fields {
		CCLabelBMFont* label = nullptr;
		std::string lastText;
		bool hitboxesWereOn = false;
	};

	bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
		if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

		auto& s = st();
		s.clicks.clear();
		s.runFrom = 0.f;
		s.clickAcc = 0.f;
		s.clickDown = false;
		cfg().deaths = 0;

		auto win = CCDirector::get()->getWinSize();
		auto label = CCLabelBMFont::create("", "bigFont.fnt");
		label->setAnchorPoint({0.f, 1.f});
		label->setAlignment(kCCTextAlignmentLeft);
		label->setScale(0.4f);
		label->setOpacity(210);
		label->setPosition({6.f, win.height - 6.f});
		m_uiLayer->addChild(label, 100);
		m_fields->label = label;
		return true;
	}

	void resetLevel() {
		PlayLayer::resetLevel();
		st().runFrom = this->getCurrentPercent();
	}

	void destroyPlayer(PlayerObject* player, GameObject* object) {
		auto& c = cfg();
		if (c.noclip && object != m_anticheatSpike) {
			auto& s = st();
			if (!s.hitPrev) c.deaths++; // count each new collision once
			s.hitNow = true;
			return;
		}
		PlayLayer::destroyPlayer(player, object);
	}

	void postUpdate(float dt) {
		PlayLayer::postUpdate(dt);
		auto& c = cfg();
		auto& s = st();

		// FPS / TPS sampling (real time)
		s.frames++;
		auto now = Clock::now();
		float elapsed = std::chrono::duration<float>(now - s.t0).count();
		if (elapsed >= 0.5f) {
			s.fps = s.frames / elapsed;
			s.tps = s.steps / elapsed;
			s.frames = 0;
			s.steps = 0;
			s.t0 = now;
		}
		s.hitPrev = s.hitNow;
		s.hitNow = false;

		// hitboxes
		if (c.hitboxes) {
			m_isDebugDrawEnabled = true;
			if (m_debugDrawNode) m_debugDrawNode->setVisible(true);
			m_fields->hitboxesWereOn = true;
		} else if (m_fields->hitboxesWereOn) {
			m_fields->hitboxesWereOn = false;
			m_isDebugDrawEnabled = false;
			if (m_debugDrawNode) {
				m_debugDrawNode->clear();
				m_debugDrawNode->setVisible(false);
			}
		}

		if (m_attemptLabel) m_attemptLabel->setVisible(!c.hideAttempts);

		// labels
		if (auto label = m_fields->label) {
			std::string txt;
			if (c.labelFps) txt += fmt::format("FPS: {:.0f}\n", s.fps);
			if (c.labelCps) txt += fmt::format("CPS: {}\n", cpsNow());
			if (c.labelTps) txt += fmt::format("TPS: {:.0f}\n", s.tps);
			if (c.labelRunFrom) txt += fmt::format("RUN FROM: {:.0f}%\n", s.runFrom);
			if (c.labelAttempt) txt += fmt::format("ATTEMPT: {}\n", m_attempts);
			if (c.labelPercent) txt += fmt::format("PERCENT: {:.1f}%\n", this->getCurrentPercent());
			if (c.labelClock) {
				std::time_t t = std::time(nullptr);
				std::tm tmv{};
#ifdef _WIN32
				localtime_s(&tmv, &t);
#else
				localtime_r(&t, &tmv);
#endif
				txt += fmt::format("TIME: {:02}:{:02}\n", tmv.tm_hour, tmv.tm_min);
			}
			if (c.noclip && c.deathCounter) txt += fmt::format("DEATHS: {}\n", c.deaths);

			if (txt != m_fields->lastText) {
				m_fields->lastText = txt;
				label->setString(txt.c_str());
			}
			label->setColor(accent());
		}
	}
};

// ---------------------------------------------------------------------------
// Bypass: unlock everything (client side only)
// ---------------------------------------------------------------------------
class $modify(TuffGameManager, GameManager) {
	bool isIconUnlocked(int id, IconType type) {
		if (cfg().unlockIcons) return true;
		return GameManager::isIconUnlocked(id, type);
	}

	bool isColorUnlocked(int id, UnlockType type) {
		if (cfg().unlockIcons) return true;
		return GameManager::isColorUnlocked(id, type);
	}
};

class $modify(TuffStats, GameStatsManager) {
	bool isItemUnlocked(UnlockType type, int id) {
		if (cfg().unlockIcons) return true;
		return GameStatsManager::isItemUnlocked(type, id);
	}
};
