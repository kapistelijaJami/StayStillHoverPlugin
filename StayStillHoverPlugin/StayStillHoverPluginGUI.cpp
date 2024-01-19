#include "pch.h"
#include "StayStillHoverPlugin.h"

// Plugin Settings Window code here
std::string StayStillHoverPlugin::GetPluginName() {
	return "StayStillHoverPlugin";
}

void StayStillHoverPlugin::SetImGuiContext(uintptr_t ctx) {
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

// Render the plugin settings here
// This will show up in bakkesmod when the plugin is loaded at
//  f2 -> plugins -> StayStillHoverPlugin
void StayStillHoverPlugin::RenderSettings() {
	ImGui::TextUnformatted("This plugin allows you to spin in place any direction without any extra movement.");
	ImGui::TextUnformatted("If target orientation is enabled, you will try to match the orientation of the target, and keep it there for a bit before time runs out.");
	ImGui::TextUnformatted("The target will match your roll if enabled. You can roll constantly, not roll at all, or roll when you feel comfortable. (The first two will be more challenging)");

	ImGui::Separator();

	ImGui::TextUnformatted("StayStillHoverPlugin plugin settings");

	CVarWrapper enableCvar = cvarManager->getCvar("stay_still_hover_enabled");
	if (!enableCvar) { return; }

	bool enabled = enableCvar.getBoolValue();
	if (ImGui::Checkbox("Enable plugin", &enabled)) {
		enableCvar.setValue(enabled);
		if (!enabled) {
			removeBotsBool = true;
		}
	}
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("Toggle Stay Still Hover Plugin");
	}

	CVarWrapper targetCvar = cvarManager->getCvar("stay_still_hover_target_orientation");
	if (!targetCvar) { return; }

	bool targetBool = targetCvar.getBoolValue();
	if (ImGui::Checkbox("Enable target orientation", &targetBool)) {
		targetCvar.setValue(targetBool);
	}
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("Toggle Target Orientation");
	}

	CVarWrapper targetRollsCvar = cvarManager->getCvar("stay_still_hover_target_rolls");
	if (!targetRollsCvar) { return; }

	bool targetRollsBool = targetRollsCvar.getBoolValue();
	if (ImGui::Checkbox("Enable target rolling", &targetRollsBool)) {
		targetRollsCvar.setValue(targetRollsBool);
	}
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("Toggle Target Rolling");
	}
}


/*
// Do ImGui rendering here
void StayStillHoverPlugin::Render()
{
	if (!ImGui::Begin(menuTitle_.c_str(), &isWindowOpen_, ImGuiWindowFlags_None))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
		return;
	}

	ImGui::End();

	if (!isWindowOpen_)
	{
		cvarManager->executeCommand("togglemenu " + GetMenuName());
	}
}

// Name of the menu that is used to toggle the window.
std::string StayStillHoverPlugin::GetMenuName()
{
	return "StayStillHoverPlugin";
}

// Title to give the menu
std::string StayStillHoverPlugin::GetMenuTitle()
{
	return menuTitle_;
}

// Don't call this yourself, BM will call this function with a pointer to the current ImGui context
void StayStillHoverPlugin::SetImGuiContext(uintptr_t ctx)
{
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

// Should events such as mouse clicks/key inputs be blocked so they won't reach the game
bool StayStillHoverPlugin::ShouldBlockInput()
{
	return ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
}

// Return true if window should be interactive
bool StayStillHoverPlugin::IsActiveOverlay()
{
	return true;
}

// Called when window is opened
void StayStillHoverPlugin::OnOpen()
{
	isWindowOpen_ = true;
}

// Called when window is closed
void StayStillHoverPlugin::OnClose()
{
	isWindowOpen_ = false;
}
*/
