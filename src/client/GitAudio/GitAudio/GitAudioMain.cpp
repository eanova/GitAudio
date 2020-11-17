#include "pch.h"
#include "GitAudioMain.h"
#include "Common\DirectXHelper.h"
#include "Audio.h"
using namespace GitAudio;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;
using namespace DirectX;
// Loads and initializes application assets when the application is loaded.
GitAudioMain::GitAudioMain(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_deviceResources(deviceResources), m_pointerLocationX(0.0f)
{
	// Register to be notified if the Device is lost or recreated
	m_deviceResources->RegisterDeviceNotify(this);

	// TODO: Replace this with your app's content initialization.
	m_sceneRenderer = std::unique_ptr<Sample3DSceneRenderer>(new Sample3DSceneRenderer(m_deviceResources));

	m_fpsTextRenderer = std::unique_ptr<SampleFpsTextRenderer>(new SampleFpsTextRenderer(m_deviceResources));

	// TODO: Change the timer settings if you want something other than the default variable timestep mode.
	// e.g. for 60 FPS fixed timestep update logic, call:
	/*
	m_timer.SetFixedTimeStep(true);
	m_timer.SetTargetElapsedSeconds(1.0 / 60);
	*/

	AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
	eflags |= AudioEngine_Debug;
#endif
	m_audEngine = std::make_unique<AudioEngine>(eflags);

	m_explode = std::make_unique<SoundEffect>(m_audEngine.get(), L"explo1.wav");
	m_ambient = std::make_unique<SoundEffect>(m_audEngine.get(),
		L"NightAmbienceSimple_02.wav");

	std::random_device rd;
	m_random = std::make_unique<std::mt19937>(rd());

	explodeDelay = 2.f;
}

GitAudioMain::~GitAudioMain()
{

	// Deregister device notification
	m_deviceResources->RegisterDeviceNotify(nullptr);
}

// Updates application state when the window size changes (e.g. device orientation change)
void GitAudioMain::CreateWindowSizeDependentResources()
{
	// TODO: Replace this with the size-dependent initialization of your app's content.
	m_sceneRenderer->CreateWindowSizeDependentResources();
}

void GitAudioMain::StartRenderLoop()
{
	// If the animation render loop is already running then do not start another thread.
	if (m_renderLoopWorker != nullptr && m_renderLoopWorker->Status == AsyncStatus::Started)
	{
		return;
	}

	// Create a task that will be run on a background thread.
	auto workItemHandler = ref new WorkItemHandler([this](IAsyncAction^ action)
		{
			// Calculate the updated frame and render once per vertical blanking interval.
			while (action->Status == AsyncStatus::Started)
			{
				critical_section::scoped_lock lock(m_criticalSection);
				Update();
				if (Render())
				{
					m_deviceResources->Present();
				}
			}
		});

	// Run task on a dedicated high priority background thread.
	m_renderLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
	m_audEngine->Resume();
	explodeDelay = 2.f;
}

void GitAudioMain::StopRenderLoop()
{
	m_renderLoopWorker->Cancel();
	m_audEngine->Suspend();
}

// Updates the application state once per frame.
void GitAudioMain::Update()
{
	ProcessInput();

	// Update scene objects.
	m_timer.Tick([&]()
		{
			float elapsedTime = float(m_timer.GetElapsedSeconds());

			// TODO: Replace this with your app's content update functions.
			m_sceneRenderer->Update(m_timer);
			m_fpsTextRenderer->Update(m_timer);
			if (!m_audEngine->Update())
			{
			


			}

			explodeDelay -= elapsedTime;
			if (explodeDelay < 0.f)
			{
				m_explode->Play();

				std::uniform_real_distribution<float> dist(1.f, 10.f);
				explodeDelay = dist(*m_random);
			}
		});
}

// Process all input from the user before updating game state
void GitAudioMain::ProcessInput()
{
	// TODO: Add per frame input handling here.
	m_sceneRenderer->TrackingUpdate(m_pointerLocationX);
}

// Renders the current frame according to the current application state.
// Returns true if the frame was rendered and is ready to be displayed.
bool GitAudioMain::Render()
{
	// Don't try to render anything before the first Update.
	if (m_timer.GetFrameCount() == 0)
	{
		return false;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	// Reset the viewport to target the whole screen.
	auto viewport = m_deviceResources->GetScreenViewport();
	context->RSSetViewports(1, &viewport);

	// Reset render targets to the screen.
	ID3D11RenderTargetView* const targets[1] = { m_deviceResources->GetBackBufferRenderTargetView() };
	context->OMSetRenderTargets(1, targets, m_deviceResources->GetDepthStencilView());

	// Clear the back buffer and depth stencil view.
	context->ClearRenderTargetView(m_deviceResources->GetBackBufferRenderTargetView(), DirectX::Colors::CornflowerBlue);
	context->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Render the scene objects.
	// TODO: Replace this with your app's content rendering functions.
	m_sceneRenderer->Render();
	m_fpsTextRenderer->Render();

	return true;
}

// Notifies renderers that device resources need to be released.
void GitAudioMain::OnDeviceLost()
{
	m_sceneRenderer->ReleaseDeviceDependentResources();
	m_fpsTextRenderer->ReleaseDeviceDependentResources();
}

// Notifies renderers that device resources may now be recreated.
void GitAudioMain::OnDeviceRestored()
{
	m_sceneRenderer->CreateDeviceDependentResources();
	m_fpsTextRenderer->CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}
