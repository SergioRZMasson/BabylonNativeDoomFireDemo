#include <filesystem>
#include <iostream>
#include <stdio.h>
#include <random>
#include <vector>
#include <array>

#include <Babylon/AppRuntime.h>
#include <Babylon/Graphics/Device.h>
#include <Babylon/ScriptLoader.h>
#include <Babylon/Plugins/NativeEngine.h>
#include <Babylon/Plugins/NativeOptimizations.h>
#include <Babylon/Plugins/NativeInput.h>
#include <Babylon/Polyfills/Console.h>
#include <Babylon/Polyfills/Window.h>
#include <Babylon/Polyfills/XMLHttpRequest.h>
#include <Babylon/Polyfills/Canvas.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#if GLFW_VERSION_MINOR < 2
#error "GLFW 3.2 or later is required"
#endif // GLFW_VERSION_MINOR < 2

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_babylon.h"

#if TARGET_PLATFORM_LINUX
#define GLFW_EXPOSE_NATIVE_X11
#elif TARGET_PLATFORM_OSX
#define GLFW_EXPOSE_NATIVE_COCOA
#elif TARGET_PLATFORM_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#endif //
#include <GLFW/glfw3native.h>

#include "Random.h"
#include "FireSimulator.h"

std::unique_ptr<Babylon::AppRuntime> runtime{};
std::unique_ptr<Babylon::Graphics::Device> device{};
std::unique_ptr<Babylon::Graphics::DeviceUpdate> update{};
Babylon::Plugins::NativeInput* nativeInput{};
std::unique_ptr<Babylon::Polyfills::Canvas> nativeCanvas{};
bool minimized = false;
FireCubeSimulator simulator;

#define INITIAL_WIDTH 1920
#define INITIAL_HEIGHT 1080

static bool s_showImgui = false;

static void* glfwNativeWindowHandle( GLFWwindow* _window )
{
#if TARGET_PLATFORM_LINUX
	return (void*) (uintptr_t) glfwGetX11Window( _window );
#elif TARGET_PLATFORM_OSX
	return ((NSWindow*) glfwGetCocoaWindow( _window )).contentView;
#elif TARGET_PLATFORM_WINDOWS
	return glfwGetWin32Window( _window );
#endif // TARGET_PLATFORM_
}

void Uninitialize()
{
	if( device )
	{
		update->Finish();
		device->FinishRenderingCurrentFrame();
		ImGui_ImplBabylon_Shutdown();
	}

	nativeInput = {};
	runtime.reset();
	nativeCanvas.reset();
	update.reset();
	device.reset();
}

void RefreshBabylon( GLFWwindow* window )
{
	Uninitialize();

	int width, height;
	glfwGetWindowSize( window, &width, &height );

	Babylon::Graphics::Configuration graphicsConfig{};
	graphicsConfig.Window = (Babylon::Graphics::WindowT) glfwNativeWindowHandle( window );
	graphicsConfig.Width = width;
	graphicsConfig.Height = height;
	graphicsConfig.MSAASamples = 4;

	device = std::make_unique<Babylon::Graphics::Device>( graphicsConfig );
	update = std::make_unique<Babylon::Graphics::DeviceUpdate>( device->GetUpdate( "update" ) );
	device->StartRenderingCurrentFrame();
	update->Start();

	runtime = std::make_unique<Babylon::AppRuntime>();

	runtime->Dispatch( []( Napi::Env env )
	{
		env.Global().Set( "nativeUpdate", Napi::Function::New( env, []( const Napi::CallbackInfo& info )
		{
			auto colorArray = info[0].As<Napi::Float32Array>();
			int numPerSide = info[1].As<Napi::Number>();
			simulator.ResizeBuffers(numPerSide);
			simulator.UpdateFire(colorArray);
		} ) );

		device->AddToJavaScript( env );

		Babylon::Polyfills::Console::Initialize( env, []( const char* message, auto )
		{
			std::cout << message << std::endl;
		} );

		Babylon::Polyfills::Window::Initialize( env );
		Babylon::Polyfills::XMLHttpRequest::Initialize( env );

		nativeCanvas = std::make_unique <Babylon::Polyfills::Canvas>( Babylon::Polyfills::Canvas::Initialize( env ) );

		Babylon::Plugins::NativeEngine::Initialize( env );
		Babylon::Plugins::NativeOptimizations::Initialize( env );

		nativeInput = &Babylon::Plugins::NativeInput::CreateForJavaScript( env );
		auto context = &Babylon::Graphics::DeviceContext::GetFromJavaScript( env );

		ImGui_ImplBabylon_SetContext( context );
	} );

	Babylon::ScriptLoader loader{ *runtime };

	// Load Babylon.js infrastructure.
	loader.Eval( "document = {}", "" );
	loader.LoadScript( "app:///Scripts/babylon.max.js" );
	loader.LoadScript( "app:///Scripts/babylonjs.loaders.js" );
	loader.LoadScript( "app:///Scripts/babylonjs.materials.js" );
	loader.LoadScript( "app:///Scripts/babylon.gui.js" );

	// Load our application script.
	loader.LoadScript( "app:///Scripts/DoomFireNative.js" );

	ImGui_ImplBabylon_Init( width, height );
}

static void key_callback( GLFWwindow* window, int key, int scancode, int action, int mods )
{
	if( key == GLFW_KEY_R && action == GLFW_PRESS )
	{
		RefreshBabylon( window );
	}
	else if( key == GLFW_KEY_D && action == GLFW_PRESS )
	{
		s_showImgui = !s_showImgui;
	}
}

void mouse_button_callback( GLFWwindow* window, int button, int action, int mods )
{
	if( s_showImgui )
		return;

	double xpos, ypos;
	glfwGetCursorPos( window, &xpos, &ypos );
	int32_t x = static_cast<int32_t>(xpos);
	int32_t y = static_cast<int32_t>(ypos);

	if( button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS )
		nativeInput->MouseDown( Babylon::Plugins::NativeInput::LEFT_MOUSE_BUTTON_ID, x, y );
	else if( button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE )
		nativeInput->MouseUp( Babylon::Plugins::NativeInput::LEFT_MOUSE_BUTTON_ID, x, y );
	else if( button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS )
		nativeInput->MouseDown( Babylon::Plugins::NativeInput::RIGHT_MOUSE_BUTTON_ID, x, y );
	else if( button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE )
		nativeInput->MouseUp( Babylon::Plugins::NativeInput::RIGHT_MOUSE_BUTTON_ID, x, y );
	else if( button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS )
		nativeInput->MouseDown( Babylon::Plugins::NativeInput::MIDDLE_MOUSE_BUTTON_ID, x, y );
	else if( button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE )
		nativeInput->MouseUp( Babylon::Plugins::NativeInput::MIDDLE_MOUSE_BUTTON_ID, x, y );
}

static void cursor_position_callback( GLFWwindow* window, double xpos, double ypos )
{
	int32_t x = static_cast<int32_t>(xpos);
	int32_t y = static_cast<int32_t>(ypos);

	nativeInput->MouseMove( x, y );
}

void scroll_callback( GLFWwindow* window, double xoffset, double yoffset )
{
	nativeInput->MouseWheel( Babylon::Plugins::NativeInput::MOUSEWHEEL_Y_ID, static_cast<int>(-yoffset * 100.0) );
}

static void window_resize_callback( GLFWwindow* window, int width, int height )
{
	device->UpdateSize( width, height );
}

static void change_instances_count(int count)
{
	runtime->Dispatch( [count]( Napi::Env env )
	{
		auto function = env.Global().Get( "resetInstancesPerSide" ).As<Napi::Function>();
		function.Call( { Napi::Number::New( env, count ) } );
	} );
}

int main()
{
	if( !glfwInit() )
		exit( EXIT_FAILURE );

	glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
	glfwWindowHint( GLFW_MAXIMIZED, GLFW_TRUE );

	auto window = glfwCreateWindow( INITIAL_WIDTH, INITIAL_HEIGHT, "DOOM Fire Native Example", NULL, NULL );

	if( !window )
	{
		glfwTerminate();
		exit( EXIT_FAILURE );
	}

	glfwSetKeyCallback( window, key_callback );
	glfwSetWindowSizeCallback( window, window_resize_callback );
	glfwSetCursorPosCallback( window, cursor_position_callback );
	glfwSetMouseButtonCallback( window, mouse_button_callback );
	glfwSetScrollCallback( window, scroll_callback );

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void) io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOther( window, true );

	// Our state
	bool show_ball = true;
	bool show_floor = true;
	ImVec4 ballColor = ImVec4( 0.55f, 0.55f, 0.55f, 1.00f );

	RefreshBabylon( window );

	while( !glfwWindowShouldClose( window ) )
	{
		if( device )
		{
			update->Finish();
			device->FinishRenderingCurrentFrame();
			device->StartRenderingCurrentFrame();
			update->Start();
		}
		glfwPollEvents();

		// Start the Dear ImGui frame
		ImGui_ImplBabylon_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();

		static float ballSize = 1.0f;
		ImGui::Begin( "DOOM Fire Native" );
		ImGui::SliderInt( "Wind", &simulator.WindStrength, 0, 10 );
		ImGui::SliderInt( "Fire decay", &simulator.FireDecay, 1, 10 );
		
		int sizeCount = simulator.GetSizeCount();

		if( ImGui::SliderInt( "Instances per side", &sizeCount, 0, 100 ) )
		{
			change_instances_count(sizeCount);
		}

		ImGui::Text( "Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate );
		ImGui::End();

		ImGui::Render();
		ImGui_ImplBabylon_RenderDrawData( ImGui::GetDrawData() );
	}

	Uninitialize();

	// Cleanup
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow( window );
	glfwTerminate();
	exit( EXIT_SUCCESS );
}