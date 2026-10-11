#pragma once
#include "TuffMenu.hpp"

// Draggable floating button (works with a mouse on desktop and touch on mobile).
// Tap it to open TuffMenu, drag it to move it. Position is saved between sessions.
class FloatingButton : public CCNode, public CCTouchDelegate {
protected:
	bool m_dragging = false;
	float m_moved = 0.f;
	CCPoint m_startTouch;
	CCPoint m_grabOffset;

	bool init() {
		if (!CCNode::init()) return false;
		auto win = CCDirector::get()->getWinSize();
		setContentSize({50, 50});
		setAnchorPoint({0.5f, 0.5f});

		auto a = tuff::accent();
		// dark circle + blue overlay
		auto base = CCSprite::createWithSpriteFrameName("GJ_button_01.png");
		base->setColor({18, 20, 28});
		base->setScale(0.85f);
		base->setPosition({25, 25});
		addChild(base);
		auto overlay = CCSprite::createWithSpriteFrameName("GJ_button_01.png");
		overlay->setColor(a);
		overlay->setOpacity(110);
		overlay->setScale(0.85f);
		overlay->setPosition({25, 25});
		addChild(overlay);
		auto label = CCLabelBMFont::create("T", "bigFont.fnt");
		label->setScale(0.7f);
		label->setPosition({25, 26});
		addChild(label);

		auto mod = Mod::get();
		setPosition({mod->getSavedValue<float>("btn-x", 36.f), mod->getSavedValue<float>("btn-y", win.height / 2)});

		CCTouchDispatcher::get()->addTargetedDelegate(this, -700, true);
		return true;
	}

	void clampToScreen() {
		auto win = CCDirector::get()->getWinSize();
		auto p = getPosition();
		setPosition({std::clamp(p.x, 25.f, win.width - 25.f), std::clamp(p.y, 25.f, win.height - 25.f)});
	}

public:
	static FloatingButton* create() {
		auto ret = new FloatingButton();
		if (ret->init()) {
			ret->autorelease();
			return ret;
		}
		CC_SAFE_DELETE(ret);
		return nullptr;
	}

	~FloatingButton() override {
		CCTouchDispatcher::get()->removeDelegate(this);
	}

	bool ccTouchBegan(CCTouch* touch, CCEvent*) override {
		if (TuffMenu::s_open) return false;
		auto p = touch->getLocation();
		if (ccpDistance(p, getPosition()) > 28.f) return false;
		m_dragging = true;
		m_moved = 0.f;
		m_startTouch = p;
		m_grabOffset = getPosition() - p;
		return true;
	}

	void ccTouchMoved(CCTouch* touch, CCEvent*) override {
		if (!m_dragging) return;
		auto p = touch->getLocation();
		m_moved = std::max(m_moved, ccpDistance(p, m_startTouch));
		if (m_moved > 8.f) {
			setPosition(p + m_grabOffset);
			clampToScreen();
		}
	}

	void ccTouchEnded(CCTouch*, CCEvent*) override {
		if (!m_dragging) return;
		m_dragging = false;
		if (m_moved <= 8.f) {
			if (!TuffMenu::s_open) {
				if (auto menu = TuffMenu::create()) menu->show();
			}
		} else {
			auto mod = Mod::get();
			mod->setSavedValue<float>("btn-x", getPositionX());
			mod->setSavedValue<float>("btn-y", getPositionY());
		}
	}

	void ccTouchCancelled(CCTouch*, CCEvent*) override {
		m_dragging = false;
	}
};
