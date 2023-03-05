#pragma once
#include <wc/Utils/YAML.h>

namespace wc {
	namespace Biomes {

		float TaigaTempMin = 0.f;
		float TaigaTempMax = 0.25f;
		float TaigaMoistMin = 0.f;
		float TaigaMoistMax = 1.f;
		
		float PlainsTempMin = 0.25f;
		float PlainsTempMax = 0.625f;
		float PlainsMoistMin = 0.f;
		float PlainsMoistMax = 0.25f;

		float DesertTempMin = 0.625f;
		float DesertTempMax = 1.f;
		float DesertMoistMin = 0.f;
		float DesertMoistMax = 0.25f;

		float TundraTempMin = 0.25f;
		float TundraTempMax = 0.5f;
		float TundraMoistMin = 0.25f;
		float TundraMoistMax = 1.f;

		float ShrublandTempMin = 0.5f;
		float ShrublandTempMax = 0.75f;
		float ShrublandMoistMin = 0.25;
		float ShrublandMoistMax = 0.5f;

		float SavannaTempMin = 0.75f;
		float SavannaTempMax = 1.f;
		float SavannaMoistMin = 0.25f;
		float SavannaMoistMax = 0.5f;

		float ForestTempMin = 0.5f;
		float ForestTempMax = 0.75f;
		float ForestMoistMin = 0.5f;
		float ForestMoistMax = 0.75f;

		float SeasonalForestTempMin = 0.75f;
		float SeasonalForestTempMax = 1.f;
		float SeasonalForestMoistMin = 0.5f;
		float SeasonalForestMoistMax = 0.75f;

		float RainForestTempMin = 0.5f;
		float RainForestTempMax = 0.75f;
		float RainForestMoistMin = 0.75f;
		float RainForestMoistMax = 1.f;

		float SwampTempMin = 0.75f;
		float SwampTempMax = 1.f;
		float SwampMoistMin = 0.75f;
		float SwampMoistMax = 1.f;

		void Save() {
			YAML::Node biomes;			
			biomes["TaigaTempMin"] = TaigaTempMin;
			biomes["TaigaTempMax"] = TaigaTempMax;
			biomes["TaigaMoistMin"] = TaigaMoistMin;
			biomes["TaigaMoistMax"] = TaigaMoistMax;

			biomes["PlainsTempMin"] = PlainsTempMin;
			biomes["PlainsTempMax"] = PlainsTempMax;
			biomes["PlainsMoistMin"] = PlainsMoistMin;
			biomes["PlainsMoistMax"] = PlainsMoistMax;

			biomes["DesertTempMin"] = DesertTempMin;
			biomes["DesertTempMax"] = DesertTempMax;
			biomes["DesertMoistMin"] = DesertMoistMin;
			biomes["DesertMoistMax"] = DesertMoistMax;
			
			biomes["TundraTempMin"] = TundraTempMin;
			biomes["TundraTempMax"] = TundraTempMax;
			biomes["TundraMoistMin"] = TundraMoistMin;
			biomes["TundraMoistMax"] = TundraMoistMax;
			
			biomes["ShrublandTempMin"] = ShrublandTempMin;
			biomes["ShrublandTempMax"] = ShrublandTempMax;
			biomes["ShrublandMoistMin"] = ShrublandMoistMin;
			biomes["ShrublandMoistMax"] = ShrublandMoistMax;
			
			biomes["SavannaTempMin"] = SavannaTempMin;
			biomes["SavannaTempMax"] = SavannaTempMax;
			biomes["SavannaMoistMin"] = SavannaMoistMin;
			biomes["SavannaMoistMax"] = SavannaMoistMax;
			
			biomes["ForestTempMin"] = ForestTempMin;
			biomes["ForestTempMax"] = ForestTempMax;
			biomes["ForestMoistMin"] = ForestMoistMin;
			biomes["ForestMoistMax"] = ForestMoistMax;
			 
			biomes["SeasonalForestTempMin"] = SeasonalForestTempMin;
			biomes["SeasonalForestTempMax"] = SeasonalForestTempMax;
			biomes["SeasonalForestMoistMin"] = SeasonalForestMoistMin;
			biomes["SeasonalForestMoistMax"] = SeasonalForestMoistMax;
			 
			biomes["RainForestTempMin"] = RainForestTempMin;
			biomes["RainForestTempMax"] = RainForestTempMax;
			biomes["RainForestMoistMin"] = RainForestMoistMin;
			biomes["RainForestMoistMax"] = RainForestMoistMax;
			 
			biomes["SwampTempMin"] = SwampTempMin;
			biomes["SwampTempMax"] = SwampTempMax;
			biomes["SwampMoistMin"] = SwampMoistMin;
			biomes["SwampMoistMax"] = SwampMoistMax;
			YAMLUtils::saveFile("biomes.yaml", biomes);
		}

		void Load() {
			YAML::Node biomes = YAML::LoadFile("biomes.yaml");
			if(biomes["TaigaTempMin"]) TaigaTempMin = biomes["TaigaTempMin"].as<float>();
			if(biomes["TaigaTempMax"]) TaigaTempMax = biomes["TaigaTempMin"].as<float>();
			if(biomes["TaigaMoistMin"]) TaigaMoistMin = biomes["TaigaMoistMin"].as<float>();
			if(biomes["TaigaMoistMax"]) TaigaMoistMax = biomes["TaigaMoistMax"].as<float>();
			
			if(biomes["PlainsTempMin"]) PlainsTempMin = biomes["PlainsTempMin"].as<float>();
			if(biomes["PlainsTempMax"]) PlainsTempMax = biomes["PlainsTempMax"].as<float>();
			if(biomes["PlainsMoistMin"]) PlainsMoistMin = biomes["PlainsMoistMin"].as<float>();
			if(biomes["PlainsMoistMax"]) PlainsMoistMax = biomes["PlainsMoistMax"].as<float>();
			
			if(biomes["DesertTempMin"]) DesertTempMin = biomes["DesertTempMin"].as<float>();
			if(biomes["DesertTempMax"]) DesertTempMax = biomes["DesertTempMax"].as<float>();
			if(biomes["DesertMoistMin"]) DesertMoistMin = biomes["DesertMoistMin"].as<float>();
			if(biomes["DesertMoistMax"]) DesertMoistMax = biomes["DesertMoistMax"].as<float>();
			
			if(biomes["TundraTempMin"]) TundraTempMin = biomes["TundraTempMin"].as<float>();
			if(biomes["TundraTempMax"]) TundraTempMax = biomes["TundraTempMax"].as<float>();
			if(biomes["TundraMoistMin"]) TundraMoistMin = biomes["TundraMoistMin"].as<float>();
			if(biomes["TundraMoistMax"]) TundraMoistMax = biomes["TundraMoistMax"].as<float>();
			
			if(biomes["ShrublandTempMin"]) ShrublandTempMin = biomes["ShrublandTempMin"].as<float>();
			if(biomes["ShrublandTempMax"]) ShrublandTempMax = biomes["ShrublandTempMax"].as<float>();
			if(biomes["ShrublandMoistMin"]) ShrublandMoistMin = biomes["ShrublandMoistMin"].as<float>();
			if(biomes["ShrublandMoistMax"]) ShrublandMoistMax = biomes["ShrublandMoistMax"].as<float>();
			
			if(biomes["SavannaTempMin"]) SavannaTempMin = biomes["SavannaTempMin"].as<float>();
			if(biomes["SavannaTempMax"]) SavannaTempMax = biomes["SavannaTempMax"].as<float>();
			if(biomes["SavannaMoistMin"]) SavannaMoistMin = biomes["SavannaMoistMin"].as<float>();
			if(biomes["SavannaMoistMax"]) SavannaMoistMax = biomes["SavannaMoistMax"].as<float>();
			
			if(biomes["ForestTempMin"]) ForestTempMin = biomes["ForestTempMin"].as<float>();
			if(biomes["ForestTempMax"]) ForestTempMax = biomes["ForestTempMax"].as<float>();
			if(biomes["ForestMoistMin"]) ForestMoistMin = biomes["ForestMoistMin"].as<float>();
			if(biomes["ForestMoistMax"]) ForestMoistMax = biomes["ForestMoistMax"].as<float>();
			
			if(biomes["SeasonalForestTempMin"]) SeasonalForestTempMin = biomes["SeasonalForestTempMin"].as<float>();
			if(biomes["SeasonalForestTempMax"]) SeasonalForestTempMax = biomes["SeasonalForestTempMax"].as<float>();
			if(biomes["SeasonalForestMoistMin"]) SeasonalForestMoistMin = biomes["SeasonalForestMoistMin"].as<float>();
			if(biomes["SeasonalForestMoistMax"]) SeasonalForestMoistMax = biomes["SeasonalForestMoistMax"].as<float>();
			
			if(biomes["RainForestTempMin"]) RainForestTempMin = biomes["RainForestTempMin"].as<float>();
			if(biomes["RainForestTempMax"]) RainForestTempMax = biomes["RainForestTempMax"].as<float>();
			if(biomes["RainForestMoistMin"]) RainForestMoistMin = biomes["RainForestMoistMin"].as<float>();
			if(biomes["RainForestMoistMax"]) RainForestMoistMax = biomes["RainForestMoistMax"].as<float>();
			
			if(biomes["SwampTempMin"]) SwampTempMin = biomes["SwampTempMin"].as<float>();
			if(biomes["SwampTempMax"]) SwampTempMax = biomes["SwampTempMax"].as<float>();
			if(biomes["SwampMoistMin"]) SwampMoistMin = biomes["SwampMoistMin"].as<float>();
			if(biomes["SwampMoistMax"]) SwampMoistMax = biomes["SwampMoistMax"].as<float>();
		}

		void Reset() {
			TaigaTempMin = 0.f;
			TaigaTempMax = 0.25f;
			TaigaMoistMin = 0.f;
			TaigaMoistMax = 1.f;

			PlainsTempMin = 0.25f;
			PlainsTempMax = 0.625f;
			PlainsMoistMin = 0.f;
			PlainsMoistMax = 0.25f;

			DesertTempMin = 0.625f;
			DesertTempMax = 1.f;
			DesertMoistMin = 0.f;
			DesertMoistMax = 0.25f;

			TundraTempMin = 0.25f;
			TundraTempMax = 0.5f;
			TundraMoistMin = 0.25f;
			TundraMoistMax = 1.f;

			ShrublandTempMin = 0.5f;
			ShrublandTempMax = 0.75f;
			ShrublandMoistMin = 0.25;
			ShrublandMoistMax = 0.5f;

			SavannaTempMin = 0.75f;
			SavannaTempMax = 1.f;
			SavannaMoistMin = 0.25f;
			SavannaMoistMax = 0.5f;

			ForestTempMin = 0.5f;
			ForestTempMax = 0.75f;
			ForestMoistMin = 0.5f;
			ForestMoistMax = 0.75f;

			SeasonalForestTempMin = 0.75f;
			SeasonalForestTempMax = 1.f;
			SeasonalForestMoistMin = 0.5f;
			SeasonalForestMoistMax = 0.75f;

			RainForestTempMin = 0.5f;
			RainForestTempMax = 0.75f;
			RainForestMoistMin = 0.75f;
			RainForestMoistMax = 1.f;

			SwampTempMin = 0.75f;
			SwampTempMax = 1.f;
			SwampMoistMin = 0.75f;
			SwampMoistMax = 1.f;
		}
	}
}