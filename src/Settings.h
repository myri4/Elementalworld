#pragma once
#include <wc/Utils/YAML.h>

namespace wc {
	namespace Settings {
		bool InvertMouse = false;
		int i1 = 0;
		float MouseSensitivity = 5.f;
		float ZoomMouseSensitivity = 18.f;
		bool bloomEnable = true;
		bool ColorBlindMode = false;
		int item_current_idx = 0; // Here we store our selection data as an index.
		int WindowMode = 0; // Here we store our selection data as an index.
		int ResolutionIndex = 2; // Here we store our selection data as an index.
		//volume %
		int v1 = 100;
		int v2 = 50;
		int v3 = 35;
		int v4 = 20;
		// Bloom Settings
		float BloomThreshold = 1.f;
		float BloomKnee = 0.1f;

		void Save() {
			YAML::Node settings;
			settings["RenderDistance"] = i1;
			settings["MouseSensitivity"] = MouseSensitivity;
			settings["InvertMouse"] = InvertMouse;
			settings["Bloom"] = bloomEnable;
			settings["BloomThreshold"] = BloomThreshold;
			settings["BloomKnee"] = BloomKnee;
			settings["ColorBlindMode"] = ColorBlindMode;
			settings["FPSCap"] = item_current_idx;
			settings["WindowMode"] = WindowMode;
			settings["WindowResolution"] = ResolutionIndex;
			YAMLUtils::saveFile("settings.yaml", settings);
		}

		void Load() {
			YAML::Node settings = YAML::LoadFile("settings.yaml");
			if (settings["RenderDistance"])   i1 = settings["RenderDistance"].as<int>();
			if (settings["MouseSensitivity"]) MouseSensitivity = settings["MouseSensitivity"].as<float>();
			if (settings["InvertMouse"])    InvertMouse = settings["InvertMouse"].as<bool>();
			if (settings["Bloom"])      bloomEnable = settings["Bloom"].as<bool>();
			if (settings["BloomThreshold"]) BloomThreshold = settings["BloomThreshold"].as<float>();
			if (settings["BloomKnee"])      BloomKnee = settings["BloomKnee"].as<float>();
			if (settings["ColorBlindMode"]) ColorBlindMode = settings["ColorBlindMode"].as<bool>();
			if (settings["FPSCap"])           item_current_idx = settings["FPSCap"].as<int>();
			if (settings["WindowMode"])       WindowMode = settings["WindowMode"].as<int>();
			if (settings["WindowResolution"]) ResolutionIndex = settings["WindowResolution"].as<int>();
		}

		void Reset() {
			InvertMouse = false;
			i1 = 0;
			MouseSensitivity = 5.f;
			ZoomMouseSensitivity = 18.f;
			bloomEnable = true;
			ColorBlindMode = false;
			item_current_idx = 0; // Here we store our selection data as an index.
			WindowMode = 0; // Here we store our selection data as an index.
			ResolutionIndex = 2; // Here we store our selection data as an index.
			//volume %
			v1 = 100;
			v2 = 50;
			v3 = 35;
			v4 = 20;
			BloomThreshold = 1.f;
			BloomKnee = 0.1f;
		}
	}
}