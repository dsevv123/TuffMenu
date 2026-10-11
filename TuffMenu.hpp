#pragma once
#include "Config.hpp"
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <functional>
#include <vector>

namespace tuff::ui {

// Dark button background with a blue (accent) overlay, used for EVERY button.
CCNode* makeButton(char const* text, float w, float h, int overlayOpacity, float fontScale = 0.4f);

// Dark panel with accent border (used as popup background).
void skinPopup(CCNode* mainLayer, CCSize size);

CCMenuItemToggler* makeSwitch(bool& ref, std::function<void(bool)> onChange = nullptr);

}

class NoclipOptions : public geode::Popup {
protected:
	bool init();
	CCLabelBMFont* m_deathLabel = nullptr;

public:
	static NoclipOptions* create();
};

class TuffMenu : public geode::Popup {
protected:
	struct Page {
		geode::ScrollLayer* scroll = nullptr;
		CCMenu* menu = nullptr;
		float contentH = 0.f;
		float width = 0.f;
		float rowH = 34.f;
	};

	CCNode* m_currentPage = nullptr;
	std::vector<CCMenuItemSpriteExtra*> m_tabs;
	CCLayerColor* m_marker = nullptr;

	// interface page widgets
	geode::TextInput* m_rIn = nullptr;
	geode::TextInput* m_gIn = nullptr;
	geode::TextInput* m_bIn = nullptr;
	CCLayerColor* m_swatch = nullptr;

	bool init();
	Page makePage(int rows);
	float rowY(Page const& p, int row) const;
	void addRowLabel(Page& p, int row, char const* text);
	void rowSwitch(Page& p, int row, char const* name, bool& value, std::function<void()> gear = nullptr);
	void rowInput(Page& p, int row, char const* name, bool& value, std::string const& initial,
				  std::string const& filter, int maxChars, std::function<void(std::string const&)> cb);
	void buildLevel();
	void buildPlayer();
	void buildCosmetics();
	void buildLabels();
	void buildBypass();
	void buildInterface();
	void selectTab(int index);
	void refreshSwatch();

public:
	static TuffMenu* create();
	static inline TuffMenu* s_open = nullptr;
	static inline int s_tab = 0;
	~TuffMenu() override;
};
