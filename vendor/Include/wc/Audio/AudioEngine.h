#pragma once
#include <miniaudio.h>
#include <wc/Utils/Log.h>

namespace wc {

	namespace {
		ma_engine m_Engine;
	
	}
	
	void CreateAudioEngine() {
		ma_result result;
		result = ma_engine_init(NULL, &m_Engine);
		if (result != MA_SUCCESS) {
			WC_ERROR("Failed to initialize audio engine.");
			return;
		}
		
	}
	
	void DestroyAudioEngine() {
		ma_engine_uninit(&m_Engine);
	}
	

	class Sound {
	public:

		void Create(const std::string& location, const uint32_t& flags = 0) {
			ma_result result;

			result = ma_sound_init_from_file(&m_Engine, location.c_str(), flags, NULL, NULL, &m_Instance);
			if (result != MA_SUCCESS) {
				WC_ERROR("Could not find sound at location {}", location.c_str());
				return;
			}

		}

		void Start() {
			ma_sound_start(&m_Instance);
		}

		void Stop() {
			ma_sound_stop(&m_Instance);
		}

		void SetVolume(const float& volume) {
			ma_sound_set_volume(&m_Instance, volume);
		}

		void SetPan(const float& pan) {
			ma_sound_set_pan(&m_Instance, pan);
		}

		void SetPitch(const float& pitch) {
			ma_sound_set_pitch(&m_Instance, pitch);
		}
	private:
		ma_sound m_Instance;
	};
}