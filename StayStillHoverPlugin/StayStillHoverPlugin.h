#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"

#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);


class StayStillHoverPlugin: public BakkesMod::Plugin::BakkesModPlugin, public BakkesMod::Plugin::PluginSettingsWindow/*, public BakkesMod::Plugin::PluginWindow*/
{
private:
	int timer = 0;
	int closeEnoughCounter = 0;
	bool insideTarget = false;
	int resultTimer = 0;
	bool result = false;
	int score = 0;
	bool removeBotsBool = false;

	int targetAngleThreshold = 30;
	float timeToHitTargetS = 4;
	float timeToHoldTargetS = 1;

	//std::shared_ptr<bool> enabled;

	//Boilerplate
	virtual void onLoad();
	virtual void onUnload();

	bool isEnabled();
	bool isTargetOrientationEnabled();
	bool isTargetRollingEnabled();
	void tick();
	void spawnCar();
	CarWrapper getEmptyNamedCar(ServerWrapper server);
	void updateTargetOrientation(ServerWrapper server, CarWrapper car);
	void orientTarget(CarWrapper car);
	void removeBots();
	Rotator getRandomOrientation();
	bool isOrientationCloseEnough(Rotator orientation, Rotator target);
	float angleBetweenVectors(Vector first, Vector second);
	float getPercentDone();
	void render(CanvasWrapper canvas);
	void dampenNoInputAngularMomentum(CarWrapper car);
	bool noAngularMomentumInputActive(ControllerInput input);
	void freeplayReset();

	// Inherited via PluginSettingsWindow
	
	void RenderSettings() override;
	std::string GetPluginName() override;
	void SetImGuiContext(uintptr_t ctx) override;
	

	// Inherited via PluginWindow
	/*

	bool isWindowOpen_ = false;
	bool isMinimized_ = false;
	std::string menuTitle_ = "StayStillHoverPlugin";

	virtual void Render() override;
	virtual std::string GetMenuName() override;
	virtual std::string GetMenuTitle() override;
	virtual void SetImGuiContext(uintptr_t ctx) override;
	virtual bool ShouldBlockInput() override;
	virtual bool IsActiveOverlay() override;
	virtual void OnOpen() override;
	virtual void OnClose() override;
	
	*/
};

