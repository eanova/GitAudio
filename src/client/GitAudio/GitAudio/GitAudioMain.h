#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
#include "Content\Sample3DSceneRenderer.h"
#include "Content\SampleFpsTextRenderer.h"
using namespace std;
// Renders Direct2D and 3D content on the screen.


namespace GitAudio
{
	class GitAudioMain : public DX::IDeviceNotify
	{
	public:
		GitAudioMain(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		~GitAudioMain();
		void CreateWindowSizeDependentResources();
		void StartTracking() { m_sceneRenderer->StartTracking(); }
		void TrackingUpdate(float positionX) { m_pointerLocationX = positionX; }
		void StopTracking() { m_sceneRenderer->StopTracking(); }
		bool IsTracking() { return m_sceneRenderer->IsTracking(); }
		void StartRenderLoop();
		void StopRenderLoop();
		Concurrency::critical_section& GetCriticalSection() { return m_criticalSection; }

		// IDeviceNotify
		virtual void OnDeviceLost();
		virtual void OnDeviceRestored();

	private:
		void ProcessInput();
		void Update();
		bool Render();

		// pointer to the audio engine
		std::unique_ptr<DirectX::AudioEngine> m_audEngine;
		std::unique_ptr<DirectX::SoundEffect> m_explode;
		std::unique_ptr<DirectX::SoundEffect> m_ambient;
		
		std::unique_ptr<mt19937> m_random;
		float explodeDelay;

		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;
		
		// TODO: Replace with your own content renderers.
		std::unique_ptr<Sample3DSceneRenderer> m_sceneRenderer;
		std::unique_ptr<SampleFpsTextRenderer> m_fpsTextRenderer;

		Windows::Foundation::IAsyncAction^ m_renderLoopWorker;
		Concurrency::critical_section m_criticalSection;

		// Rendering loop timer.
		DX::StepTimer m_timer;

		// Track current input pointer position.
		float m_pointerLocationX;
	};
}