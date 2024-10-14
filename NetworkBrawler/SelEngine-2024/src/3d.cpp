#include <Sel/Core.hpp>
#include <Sel/Window.hpp>
#include <SDL_syswm.h>
#include <iostream>
#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu/webgpu.hpp>
#include <webgpu/webgpu-raii.hpp>
#include <windows.h>

int main()
{
	Sel::Core core;
	Sel::Window window("Sel3D", 1280, 720);

	SDL_SysWMinfo wmInfo;
	if (!window.GetWindowManagerInfo(wmInfo))
	{
		std::cerr << "failed to get window window manager info" << std::endl;
		return EXIT_FAILURE;
	}
	
	wgpu::raii::Instance instance = wgpu::createInstance({});

	wgpu::SurfaceDescriptorFromWindowsHWND windowsDescriptor(wgpu::Default);
	windowsDescriptor.chain.sType = WGPUSType_SurfaceDescriptorFromWindowsHWND;
	windowsDescriptor.chain.next = nullptr;
	windowsDescriptor.hinstance = wmInfo.info.win.hinstance;
	windowsDescriptor.hwnd = wmInfo.info.win.window;

	wgpu::SurfaceDescriptor surfaceDescriptor(wgpu::Default);
	surfaceDescriptor.nextInChain = &windowsDescriptor.chain;
	surfaceDescriptor.label = "Surface";

	wgpu::raii::Surface surface = instance->createSurface(surfaceDescriptor);
	if (!surface)
	{
		std::cerr << "failed to create surface" << std::endl;
		return EXIT_FAILURE;
	}

	wgpu::RequestAdapterOptions adapterOptions;
	adapterOptions.compatibleSurface = *surface;

	wgpu::raii::Adapter adapter = instance->requestAdapter(adapterOptions);
	if (!adapter)
	{
		std::cerr << "failed to create adapter" << std::endl;
		return EXIT_FAILURE;
	}

	wgpu::AdapterProperties properties;
	adapter->getProperties(&properties);

	std::cout << "Created adapter on " << properties.name << std::endl;

	wgpu::DeviceDescriptor deviceDescriptor(wgpu::Default);
	deviceDescriptor.label = "GPU";
	deviceDescriptor.deviceLostCallback = [](WGPUDeviceLostReason reason, char const* message, void* userdata)
	{
		std::cout << "device lost: " << message << std::endl;
		std::abort();
	};
	deviceDescriptor.defaultQueue.label = "Default queue";

	wgpu::raii::Device device = adapter->requestDevice(deviceDescriptor);
	if (!device)
	{
		std::cerr << "failed to create device" << std::endl;
		return EXIT_FAILURE;
	}

	wgpu::SurfaceCapabilities surfaceCaps;
	surface->getCapabilities(*adapter, &surfaceCaps);

	wgpu::SurfaceConfiguration surfaceConfig;
	surfaceConfig.width = window.GetSize().x;
	surfaceConfig.height = window.GetSize().y;
	surfaceConfig.device = *device;
	surfaceConfig.presentMode = surfaceCaps.presentModes[0];
	surfaceConfig.alphaMode = surfaceCaps.alphaModes[0];
	surfaceConfig.viewFormatCount = 0;
	surfaceConfig.usage = wgpu::TextureUsage::RenderAttachment;
	surfaceConfig.format = surfaceCaps.formats[0];

	surface->configure(surfaceConfig);

	auto h = device->setUncapturedErrorCallback([](wgpu::ErrorType type, char const* message)
	{
		std::cout << "Device error: type " << type;
		if (message) 
			std::cout << " (message: " << message << ")";
		std::cout << std::endl;
	});

	wgpu::raii::Queue queue = device->getQueue();

	std::cout << "Creating shader module..." << std::endl;
	
	const char* shaderSource = R"(
@vertex
fn vs_main(@builtin(vertex_index) in_vertex_index: u32) -> @builtin(position) vec4<f32> {
	var p = vec2f(0.0, 0.0);
	if (in_vertex_index == 0u) {
		p = vec2f(-0.5, -0.5);
	} else if (in_vertex_index == 1u) {
		p = vec2f(0.5, -0.5);
	} else {
		p = vec2f(0.0, 0.5);
	}
	return vec4f(p, 0.0, 1.0);
}

@fragment
fn fs_main() -> @location(0) vec4f {
    return vec4f(0.0, 0.4, 1.0, 1.0);
}
)";

	wgpu::ShaderModuleDescriptor shaderDesc;

	// Use the extension mechanism to load a WGSL shader source code
	wgpu::ShaderModuleWGSLDescriptor shaderCodeDesc;
	// Set the chained struct's header
	shaderCodeDesc.chain.next = nullptr;
	shaderCodeDesc.chain.sType = wgpu::SType::ShaderModuleWGSLDescriptor;
	// Connect the chain
	shaderDesc.nextInChain = &shaderCodeDesc.chain;

	// Setup the actual payload of the shader code descriptor
	shaderCodeDesc.code = shaderSource;

	wgpu::raii::ShaderModule shaderModule = device->createShaderModule(shaderDesc);
	std::cout << "Shader module: " << shaderModule << std::endl;

	std::cout << "Creating render pipeline..." << std::endl;
	wgpu::RenderPipelineDescriptor pipelineDesc(wgpu::Default);

	// Vertex fetch
	// (We don't use any input buffer so far)
	pipelineDesc.vertex.bufferCount = 0;
	pipelineDesc.vertex.buffers = nullptr;

	// Vertex shader
	pipelineDesc.vertex.module = *shaderModule;
	pipelineDesc.vertex.entryPoint = "vs_main";
	pipelineDesc.vertex.constantCount = 0;
	pipelineDesc.vertex.constants = nullptr;

	// Primitive assembly and rasterization
	// Each sequence of 3 vertices is considered as a triangle
	pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
	// We'll see later how to specify the order in which vertices should be
	// connected. When not specified, vertices are considered sequentially.
	pipelineDesc.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined;
	// The face orientation is defined by assuming that when looking
	// from the front of the face, its corner vertices are enumerated
	// in the counter-clockwise (CCW) order.
	pipelineDesc.primitive.frontFace = wgpu::FrontFace::CCW;
	// But the face orientation does not matter much because we do not
	// cull (i.e. "hide") the faces pointing away from us (which is often 
	// used for optimization).
	pipelineDesc.primitive.cullMode = wgpu::CullMode::None;

	// Fragment shader
	wgpu::FragmentState fragmentState;
	pipelineDesc.fragment = &fragmentState;
	fragmentState.module = *shaderModule;
	fragmentState.entryPoint = "fs_main";
	fragmentState.constantCount = 0;
	fragmentState.constants = nullptr;

	// Configure blend state
	wgpu::BlendState blendState;
	// Usual alpha blending for the color:
	blendState.color.srcFactor = wgpu::BlendFactor::SrcAlpha;
	blendState.color.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
	blendState.color.operation = wgpu::BlendOperation::Add;
	// We leave the target alpha untouched:
	blendState.alpha.srcFactor = wgpu::BlendFactor::Zero;
	blendState.alpha.dstFactor = wgpu::BlendFactor::One;
	blendState.alpha.operation = wgpu::BlendOperation::Add;

	wgpu::ColorTargetState colorTarget;
	colorTarget.format = surface->getPreferredFormat(*adapter);
	colorTarget.blend = &blendState;
	colorTarget.writeMask = wgpu::ColorWriteMask::All; // We could write to only some of the color channels.

	// We have only one target because our render pass has only one output color
	// attachment.
	fragmentState.targetCount = 1;
	fragmentState.targets = &colorTarget;

	// Depth and stencil tests are not used here
	pipelineDesc.depthStencil = nullptr;

	// Pipeline layout
	pipelineDesc.layout = nullptr;

	wgpu::raii::RenderPipeline pipeline = device->createRenderPipeline(pipelineDesc);
	std::cout << "Render pipeline: " << pipeline << std::endl;

	bool shouldExit = false;
	while (!shouldExit)
	{
		SDL_Event event;
		while (Sel::Core::PollEvent(event))
		{
			switch (event.type)
			{
				case SDL_QUIT:
					shouldExit = true;
					break;
			}
		}

		wgpu::SurfaceTexture nextTexture;
		surface->getCurrentTexture(&nextTexture);

		if (nextTexture.status != wgpu::SurfaceGetCurrentTextureStatus::Success) {
			std::cerr << "Cannot acquire next swap chain texture" << std::endl;
			return 1;
		}

		wgpu::raii::TextureView outputView(wgpu::TextureView(wgpuTextureCreateView(nextTexture.texture, nullptr)));

		wgpu::CommandEncoderDescriptor commandEncoderDesc;
		commandEncoderDesc.label = "Command Encoder";
		wgpu::raii::CommandEncoder encoder = device->createCommandEncoder(commandEncoderDesc);

		wgpu::RenderPassColorAttachment renderPassColorAttachment{};
		renderPassColorAttachment.view = *outputView;
		renderPassColorAttachment.resolveTarget = nullptr;
		renderPassColorAttachment.loadOp = wgpu::LoadOp::Clear;
		renderPassColorAttachment.storeOp = wgpu::StoreOp::Store;
		renderPassColorAttachment.clearValue = wgpu::Color{ 0.9, 0.1, 0.2, 1.0 };

		wgpu::RenderPassDescriptor renderPassDesc {};
		renderPassDesc.colorAttachmentCount = 1;
		renderPassDesc.colorAttachments = &renderPassColorAttachment;

		renderPassDesc.depthStencilAttachment = nullptr;
		renderPassDesc.timestampWrites = nullptr;

		wgpu::raii::RenderPassEncoder renderPass = encoder->beginRenderPass(renderPassDesc);

		// In its overall outline, drawing a triangle is as simple as this:
		// Select which render pipeline to use
		renderPass->setPipeline(*pipeline);
		// Draw 1 instance of a 3-vertices shape
		renderPass->draw(3, 1, 0, 0);

		renderPass->end();
		
		wgpu::CommandBufferDescriptor cmdBufferDescriptor;
		cmdBufferDescriptor.label = "Command buffer";
		wgpu::raii::CommandBuffer command = encoder->finish(cmdBufferDescriptor);

		queue->submit(*command);

		surface->present();
	}
}