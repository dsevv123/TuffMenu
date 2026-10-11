#include "TuffMenu.hpp"

using namespace tuff;

// ---------------------------------------------------------------------------
// Shared UI helpers
// ---------------------------------------------------------------------------

CCNode* ui::makeButton(char const* text, float w, float h, int overlayOpacity, float fontScale) {
	auto a = accent();
	auto root = CCNode::create();
	root->setContentSize({w, h});
	root->setAnchorPoint({0.5f, 0.5f});

	// border + dark background + blue overlay
	root->addChild(CCLayerColor::create({a.r, a.g, a.b, 200}, w, h));
	auto bg = CCLayerColor::create({16, 18, 26, 255}, w - 2, h - 2);
	bg->setPosition({1, 1});
	root->addChild(bg);
	auto over = CCLayerColor::create({a.r, a.g, a.b, (GLubyte)overlayOpacity}, w - 2, h - 2);
	over->setPosition({1, 1});
	root->addChild(over);

	if (text && text[0]) {
		auto label = CCLabelBMFont::create(text, "bigFont.fnt");
		label->setScale(fontScale);
		label->limitLabelWidth(w - 8.f, fontScale, 0.1f);
		label->setPosition({w / 2, h / 2});
		root->addChild(label);
	}
	return root;
}

void ui::skinPopup(CCNode* mainLayer, CCSize size) {
	auto a = accent();
	auto border = CCLayerColor::create({a.r, a.g, a.b, 255}, size.width + 4, size.height + 4);
	border->setPosition({-2, -2});
	mainLayer->addChild(border, -3);
	mainLayer->addChild(CCLayerColor::create({10, 12, 18, 255}, size.width, size.height), -2);
}

CCMenuItemToggler* ui::makeSwitch(bool& ref, std::function<void(bool)> onChange) {
	auto on = makeButton("ON", 54, 22, 150);
	auto off = makeButton("OFF", 54, 22, 20);
	auto toggler = CCMenuItemExt::createToggler(on, off, [&ref, onChange](CCMenuItemToggler* t) {
		ref = !t->isToggled();
		saveCfg();
		if (onChange) onChange(ref);
	});
	toggler->toggle(ref);
	return toggler;
}

// ---------------------------------------------------------------------------
// Noclip options (gear icon)
// ---------------------------------------------------------------------------

NoclipOptions* NoclipOptions::create() {
	auto ret = new NoclipOptions();
	if (ret->init()) {
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

bool NoclipOptions::init() {
	if (!Popup::init(270.f, 150.f)) return false;
	m_bgSprite->setVisible(false);
	ui::skinPopup(m_mainLayer, m_size);

	auto title = CCLabelBMFont::create("NOCLIP OPTIONS", "bigFont.fnt");
	title->setScale(0.5f);
	title->setColor(accent());
	title->setPosition({m_size.width / 2, m_size.height - 18});
	m_mainLayer->addChild(title);

	auto menu = CCMenu::create();
	menu->setPosition({0, 0});
	m_mainLayer->addChild(menu);

	auto l1 = CCLabelBMFont::create("DEATH COUNTER", "bigFont.fnt");
	l1->setScale(0.4f);
	l1->setAnchorPoint({0, 0.5f});
	l1->setPosition({16, 92});
	m_mainLayer->addChild(l1);
	auto sw = ui::makeSwitch(cfg().deathCounter);
	sw->setPosition({m_size.width - 44, 92});
	menu->addChild(sw);

	m_deathLabel = CCLabelBMFont::create(fmt::format("DEATHS THIS SESSION: {}", cfg().deaths).c_str(), "bigFont.fnt");
	m_deathLabel->setScale(0.32f);
	m_deathLabel->setAnchorPoint({0, 0.5f});
	m_deathLabel->setPosition({16, 58});
	m_mainLayer->addChild(m_deathLabel);

	auto reset = CCMenuItemExt::createSpriteExtra(ui::makeButton("RESET COUNTER", 130, 24, 60, 0.35f),
		[this](CCMenuItemSpriteExtra*) {
			cfg().deaths = 0;
			m_deathLabel->setString("DEATHS THIS SESSION: 0");
		});
	reset->setPosition({m_size.width / 2, 24});
	menu->addChild(reset);
	return true;
}

// ---------------------------------------------------------------------------
// Main menu
// ---------------------------------------------------------------------------

namespace {
	constexpr float kPopupW = 440.f;
	constexpr float kPopupH = 290.f;
	constexpr float kPageX = 108.f;
	constexpr float kPageY = 14.f;
	constexpr float kPageW = 320.f;
	constexpr float kPageH = 232.f;
	char const* kTabNames[6] = {"LEVEL", "PLAYER", "COSMETICS", "LABELS", "BYPASS", "INTERFACE"};
}

TuffMenu* TuffMenu::create() {
	auto ret = new TuffMenu();
	if (ret->init()) {
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

TuffMenu::~TuffMenu() {
	if (s_open == this) s_open = nullptr;
}

bool TuffMenu::init() {
	if (!Popup::init(kPopupW, kPopupH)) return false;
	s_open = this;
	m_bgSprite->setVisible(false);
	ui::skinPopup(m_mainLayer, m_size);

	auto a = accent();

	auto title = CCLabelBMFont::create("TUFFMENU", "bigFont.fnt");
	title->setScale(0.65f);
	title->setColor(a);
	title->setPosition({m_size.width / 2, m_size.height - 20});
	m_mainLayer->addChild(title);

	// content panel behind the pages
	auto panel = CCLayerColor::create({18, 21, 30, 255}, kPageW, kPageH);
	panel->setPosition({kPageX, kPageY});
	m_mainLayer->addChild(panel, -1);
	auto panelTint = CCLayerColor::create({a.r, a.g, a.b, 18}, kPageW, kPageH);
	panelTint->setPosition({kPageX, kPageY});
	m_mainLayer->addChild(panelTint, -1);

	// selected-tab highlight
	m_marker = CCLayerColor::create({a.r, a.g, a.b, 110}, 92, 32);
	m_mainLayer->addChild(m_marker, -1);

	auto tabMenu = CCMenu::create();
	tabMenu->setPosition({0, 0});
	m_mainLayer->addChild(tabMenu);

	for (int i = 0; i < 6; i++) {
		auto btn = CCMenuItemExt::createSpriteExtra(ui::makeButton(kTabNames[i], 84, 26, 70, 0.38f),
			[this, i](CCMenuItemSpriteExtra*) { selectTab(i); });
		btn->setPosition({54, m_size.height - 58 - i * 36.f});
		tabMenu->addChild(btn);
		m_tabs.push_back(btn);
	}

	selectTab(std::clamp(s_tab, 0, 5));
	return true;
}

void TuffMenu::selectTab(int index) {
	s_tab = index;
	if (m_currentPage) {
		m_currentPage->removeFromParent();
		m_currentPage = nullptr;
	}
	m_rIn = m_gIn = m_bIn = nullptr;
	m_swatch = nullptr;

	auto pos = m_tabs[index]->getPosition();
	m_marker->setPosition({pos.x - 46, pos.y - 16});

	switch (index) {
		case 0: buildLevel(); break;
		case 1: buildPlayer(); break;
		case 2: buildCosmetics(); break;
		case 3: buildLabels(); break;
		case 4: buildBypass(); break;
		case 5: buildInterface(); break;
	}
}

TuffMenu::Page TuffMenu::makePage(int rows) {
	Page p;
	p.width = kPageW;
	p.contentH = std::max(kPageH, rows * p.rowH);
	p.scroll = geode::ScrollLayer::create({kPageW, kPageH});
	p.scroll->setPosition({kPageX, kPageY});
	p.scroll->m_contentLayer->setContentSize({kPageW, p.contentH});

	p.menu = CCMenu::create();
	p.menu->setPosition({0, 0});
	p.menu->setContentSize({kPageW, p.contentH});
	p.scroll->m_contentLayer->addChild(p.menu, 2);

	m_mainLayer->addChild(p.scroll);
	m_currentPage = p.scroll;
	return p;
}

float TuffMenu::rowY(Page const& p, int row) const {
	return p.contentH - (row + 0.5f) * p.rowH;
}

void TuffMenu::addRowLabel(Page& p, int row, char const* text) {
	auto label = CCLabelBMFont::create(text, "bigFont.fnt");
	label->setScale(0.4f);
	label->limitLabelWidth(150.f, 0.4f, 0.15f);
	label->setAnchorPoint({0, 0.5f});
	label->setPosition({12, rowY(p, row)});
	p.scroll->m_contentLayer->addChild(label, 1);
}

void TuffMenu::rowSwitch(Page& p, int row, char const* name, bool& value, std::function<void()> gear) {
	addRowLabel(p, row, name);
	auto sw = ui::makeSwitch(value);
	sw->setPosition({p.width - 40, rowY(p, row)});
	p.menu->addChild(sw);

	if (gear) {
		auto node = ui::makeButton("", 26, 22, 60);
		auto icon = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
		icon->setScale(0.3f);
		icon->setColor(accent());
		icon->setPosition({13, 11});
		node->addChild(icon);
		auto btn = CCMenuItemExt::createSpriteExtra(node, [gear](CCMenuItemSpriteExtra*) { gear(); });
		btn->setPosition({p.width - 40 - 27 - 8 - 13, rowY(p, row)});
		p.menu->addChild(btn);
	}
}

void TuffMenu::rowInput(Page& p, int row, char const* name, bool& value, std::string const& initial,
						std::string const& filter, int maxChars, std::function<void(std::string const&)> cb) {
	addRowLabel(p, row, name);
	auto sw = ui::makeSwitch(value);
	sw->setPosition({p.width - 40, rowY(p, row)});
	p.menu->addChild(sw);

	auto input = geode::TextInput::create(90.f, "0", "bigFont.fnt");
	input->setScale(0.7f);
	input->setFilter(filter);
	input->setMaxCharCount(maxChars);
	input->setString(initial, false);
	input->setCallback(cb);
	input->setPosition({p.width - 40 - 27 - 6 - 32, rowY(p, row)});
	p.scroll->m_contentLayer->addChild(input, 3);
}

// ---------------------------- pages ---------------------------------------

void TuffMenu::buildLevel() {
	auto p = makePage(3);
	auto& c = cfg();

	rowInput(p, 0, "SPEEDHACK", c.speedhack, fmt::format("{}", c.speed), "0123456789.", 6,
		[](std::string const& s) {
			float v = std::strtof(s.c_str(), nullptr);
			if (v > 0.f) {
				cfg().speed = std::clamp(v, 0.01f, 20.f);
				saveCfg();
			}
		});
	rowInput(p, 1, "AUTO CLICKER", c.autoClicker, fmt::format("{}", c.cps), "0123456789", 3,
		[](std::string const& s) {
			int v = std::atoi(s.c_str());
			if (v > 0) {
				cfg().cps = std::clamp(v, 1, 100);
				saveCfg();
			}
		});
	rowSwitch(p, 2, "HITBOXES", c.hitboxes);
}

void TuffMenu::buildPlayer() {
	auto p = makePage(2);
	auto& c = cfg();

	rowSwitch(p, 0, "NOCLIP", c.noclip, [] { NoclipOptions::create()->show(); });
	rowSwitch(p, 1, "JUMP HACK", c.jumpHack);
}

void TuffMenu::buildCosmetics() {
	auto p = makePage(6);
	auto& c = cfg();

	rowSwitch(p, 0, "NO DEATH EFFECT", c.noDeathEffect);
	rowSwitch(p, 1, "NO SPIDER DASH", c.noSpiderDash);
	rowSwitch(p, 2, "NO WAVE TRAIL", c.noWaveTrail);
	rowSwitch(p, 3, "NO TRAIL", c.noTrail);
	rowSwitch(p, 4, "NO CAMERA SHAKE", c.noShake);
	rowSwitch(p, 5, "HIDE ATTEMPTS", c.hideAttempts);
}

void TuffMenu::buildLabels() {
	auto p = makePage(7);
	auto& c = cfg();

	rowSwitch(p, 0, "FPS", c.labelFps);
	rowSwitch(p, 1, "CPS", c.labelCps);
	rowSwitch(p, 2, "TPS", c.labelTps);
	rowSwitch(p, 3, "RUN FROM", c.labelRunFrom);
	rowSwitch(p, 4, "ATTEMPT", c.labelAttempt);
	rowSwitch(p, 5, "PERCENT", c.labelPercent);
	rowSwitch(p, 6, "CLOCK", c.labelClock);
}

void TuffMenu::buildBypass() {
	auto p = makePage(2);
	auto& c = cfg();

	rowSwitch(p, 0, "UNLOCK ALL ICONS", c.unlockIcons);

	auto note = CCLabelBMFont::create("LOCAL ONLY - ICONS, COLORS & TRAILS", "chatFont.fnt");
	note->setScale(0.6f);
	note->setColor({150, 160, 180});
	note->setAnchorPoint({0, 0.5f});
	note->setPosition({12, rowY(p, 1)});
	p.scroll->m_contentLayer->addChild(note, 1);
}

void TuffMenu::refreshSwatch() {
	if (m_swatch) m_swatch->setColor(accent());
}

void TuffMenu::buildInterface() {
	auto p = makePage(5);
	auto& c = cfg();

	auto makeChannel = [&](int row, char const* name, int value, geode::TextInput*& slot, int Config::*member) {
		addRowLabel(p, row, name);
		auto input = geode::TextInput::create(90.f, "0", "bigFont.fnt");
		input->setScale(0.7f);
		input->setFilter("0123456789");
		input->setMaxCharCount(3);
		input->setString(fmt::format("{}", value), false);
		input->setCallback([this, member](std::string const& s) {
			if (s.empty()) return;
			cfg().*member = std::clamp(std::atoi(s.c_str()), 0, 255);
			saveCfg();
			refreshSwatch();
		});
		input->setPosition({p.width - 60, rowY(p, row)});
		p.scroll->m_contentLayer->addChild(input, 3);
		slot = input;
	};
	makeChannel(0, "RED (0-255)", c.r, m_rIn, &Config::r);
	makeChannel(1, "GREEN (0-255)", c.g, m_gIn, &Config::g);
	makeChannel(2, "BLUE (0-255)", c.b, m_bIn, &Config::b);

	// presets
	struct Preset { char const* name; int r, g, b; };
	static Preset const presets[5] = {
		{"BLUE", 40, 120, 255}, {"GREEN", 40, 220, 100}, {"RED", 255, 60, 60},
		{"PURPLE", 160, 80, 255}, {"PINK", 255, 90, 190},
	};
	for (int i = 0; i < 5; i++) {
		auto pr = presets[i];
		auto btn = CCMenuItemExt::createSpriteExtra(ui::makeButton(pr.name, 56, 22, 80, 0.3f),
			[this, pr](CCMenuItemSpriteExtra*) {
				auto& cc = cfg();
				cc.r = pr.r; cc.g = pr.g; cc.b = pr.b;
				saveCfg();
				if (m_rIn) m_rIn->setString(fmt::format("{}", pr.r), false);
				if (m_gIn) m_gIn->setString(fmt::format("{}", pr.g), false);
				if (m_bIn) m_bIn->setString(fmt::format("{}", pr.b), false);
				refreshSwatch();
			});
		btn->setPosition({34.f + i * 63.f, rowY(p, 3)});
		p.menu->addChild(btn);
	}

	// preview + apply
	m_swatch = CCLayerColor::create({accent().r, accent().g, accent().b, 255}, 70, 22);
	m_swatch->setPosition({190, rowY(p, 4) - 11});
	p.scroll->m_contentLayer->addChild(m_swatch, 1);

	auto apply = CCMenuItemExt::createSpriteExtra(ui::makeButton("APPLY", 100, 26, 110, 0.4f),
		[this](CCMenuItemSpriteExtra*) {
			// rebuild the menu so every button picks up the new colour
			auto fresh = TuffMenu::create();
			this->keyBackClicked();
			if (fresh) fresh->show();
		});
	apply->setPosition({92, rowY(p, 4)});
	p.menu->addChild(apply);
}
