#pragma once
#include <wc/Utils/YAML.h>

namespace wc {
	namespace Settings {
		enum class GraphicsLevel {
			Low,
			Performance,
			Medium,
			High,
			Extreme
		};

		enum class TonemapFunction {
			ACES, 
			Filmic, 
			Reinhard, 
			Uncharted2, 
			Uchimura, 
			Lottes, 
			Unreal
		};

		bool InvertMouse = false;
		int RenderDistance = 0;
		float MouseSensitivity = 5.f;
		float ZoomMouseSensitivity = 18.f;
		bool bloomEnable = true;
		bool ColorBlindMode = false;
		int FPSCap = 0; // Here we store our selection data as an index.
		int toneMapFunctionID = 0;
		int WindowMode = 0; // Here we store our selection data as an index.
		int ResolutionIndex = 2; // Here we store our selection data as an index.
		int32_t maxBounceCount = 1;
		int32_t raysPerPixel = 5;
		//volume %
		int v1 = 100;
		int v2 = 50;
		int v3 = 35;
		int v4 = 20;
		// Bloom Settings
		float BloomThreshold = 0.8f;
		float BloomKnee = 0.6f;

		void Save() {
			YAML::Node settings;
			settings["RenderDistance"] = RenderDistance;
			settings["MouseSensitivity"] = MouseSensitivity;
			settings["InvertMouse"] = InvertMouse;
			settings["Bloom"] = bloomEnable;
			settings["BloomThreshold"] = BloomThreshold;
			settings["BloomKnee"] = BloomKnee;
			settings["ColorBlindMode"] = ColorBlindMode;
			settings["FPSCap"] = FPSCap;
			settings["WindowMode"] = WindowMode;
			settings["WindowResolution"] = ResolutionIndex;
			settings["ToneMapFunction"] = toneMapFunctionID;
			settings["maxBounceCount"] = maxBounceCount;
			settings["raysPerPixel"] = raysPerPixel;
			YAMLUtils::saveFile("settings.yaml", settings);
		}

		void Load() {
			YAML::Node settings = YAML::LoadFile("settings.yaml");
			if (settings["RenderDistance"])   RenderDistance = settings["RenderDistance"].as<int>();
			if (settings["MouseSensitivity"]) MouseSensitivity = settings["MouseSensitivity"].as<float>();
			if (settings["InvertMouse"])    InvertMouse = settings["InvertMouse"].as<bool>();
			if (settings["Bloom"])      bloomEnable = settings["Bloom"].as<bool>();
			if (settings["BloomThreshold"]) BloomThreshold = settings["BloomThreshold"].as<float>();
			if (settings["BloomKnee"])      BloomKnee = settings["BloomKnee"].as<float>();
			if (settings["ColorBlindMode"]) ColorBlindMode = settings["ColorBlindMode"].as<bool>();
			if (settings["FPSCap"])           FPSCap = settings["FPSCap"].as<int>();
			if (settings["WindowMode"])       WindowMode = settings["WindowMode"].as<int>();
			if (settings["WindowResolution"]) ResolutionIndex = settings["WindowResolution"].as<int>();
			if (settings["ToneMapFunction"]) toneMapFunctionID = settings["ToneMapFunction"].as<int>();
			if (settings["maxBounceCount"]) maxBounceCount = settings["maxBounceCount"].as<int>();
			if (settings["raysPerPixel"]) raysPerPixel = settings["raysPerPixel"].as<int>();
		}

		void Reset() {
			InvertMouse = false;
			RenderDistance = 0;
			MouseSensitivity = 5.f;
			ZoomMouseSensitivity = 18.f;
			bloomEnable = true;
			ColorBlindMode = false;
			FPSCap = 0; // Here we store our selection data as an index.
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