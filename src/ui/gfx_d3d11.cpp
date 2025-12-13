#include <ui/gfx.h>

#include <ui/window.h>

#include <ui/os.h>

#include <base/free_list.h>

static IDXGIFactory* dxgi_factory;
static IDXGISwapChain* swap_chain;

static ID3D11DeviceContext* context;
static ID3D11Device* device;

static ID3D11RenderTargetView* rtv;

typedef struct Texture Texture;

struct Texture
{
	ID3D11ShaderResourceView* srv;
};

static FreeList <Texture, 512> textures;

#define VERIFY_HR(hr) if (FAILED(hr)) { __builtin_debugtrap(); }

namespace gfx
{

static inline void init()
{
	VERIFY_HR(CreateDXGIFactory(__uuidof(dxgi_factory), reinterpret_cast<void**>(&dxgi_factory)));

	// device & context
	{
		uint32_t flags = 0;
		
		#if DEBUG
			flags |= D3D11_CREATE_DEVICE_DEBUG;
		#endif
	
		constexpr D3D_FEATURE_LEVEL feature_level[] = { D3D_FEATURE_LEVEL_11_0 };
	
		HRESULT hr;
	
		// create hardware device, if it fails, try software
		hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, feature_level, 1, D3D11_SDK_VERSION, &device, nullptr, &context);
		
		if (FAILED(hr))
		{
			hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_SOFTWARE, nullptr, flags, feature_level, 1, D3D11_SDK_VERSION, &device, nullptr, &context);
		}
	
		VERIFY_HR(hr);
	}

	// swap chain
	{
		DXGI_SWAP_CHAIN_DESC desc =
		{
			.BufferDesc =
			{
				.Width = window::size.x,
				.Height = window::size.y,
				.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
			},
			.SampleDesc = 
			{
				.Count = 1,
				.Quality = 0
			},
			.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
			.BufferCount = 2,
			.OutputWindow = static_cast<HWND>(window::handle),
			.Windowed = true,
			.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL,
			.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
		};

		VERIFY_HR(dxgi_factory->CreateSwapChain(device, &desc, &swap_chain));
	}

	// render target
	{
		ID3D11Texture2D* back;

		VERIFY_HR(swap_chain->GetBuffer(0, __uuidof(back), reinterpret_cast<void**>(&back)));

		VERIFY_HR(device->CreateRenderTargetView((ID3D11Resource*)back, nullptr, &rtv));

		back->Release();
	}

	textures.init();
}

static inline bool is_ready()
{
	return device && context && swap_chain; 
}

static inline void clear(float r, float g, float b, float a)
{
	context->ClearRenderTargetView(rtv, (FLOAT[]){ r, g, b, a });
}

static inline void begin()
{
	context->OMSetRenderTargets(0, (ID3D11RenderTargetView*[]){ rtv }, nullptr);
}

static inline void present()
{
	VERIFY_HR(swap_chain->Present(0, 0));
}

static inline void resize_viewport()
{
	rtv->Release();

	ID3D11Texture2D* back;

	VERIFY_HR(swap_chain->GetBuffer(0, __uuidof(back), (void**)&back));

	VERIFY_HR(device->CreateRenderTargetView(static_cast<ID3D11Resource*>(back), nullptr, &rtv));

	back->Release();
}

static inline TextureId create_texture(const uint8_t* data, uint16_t w, uint16_t h)
{
	TextureId id = static_cast<TextureId>(textures.push());

	const D3D11_TEXTURE2D_DESC desc =
	{
		.Width = w,
		.Height = h,
		.MipLevels = 1,
		.ArraySize = 1,
		.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
		.SampleDesc = 
		{
			.Count = 1,
			.Quality = 0
		},
		.Usage = D3D11_USAGE_DEFAULT,
		.BindFlags = D3D11_BIND_SHADER_RESOURCE,
	};

	const D3D11_SUBRESOURCE_DATA subresource_data = 
	{
		.pSysMem = data,
		.SysMemPitch = static_cast<uint32_t>(w * sizeof(uint8_t) * 4),
		.SysMemSlicePitch = static_cast<uint32_t>(w * h * sizeof(uint8_t) * 4)
	};

	ID3D11Texture2D* texture;

	VERIFY_HR(device->CreateTexture2D(&desc, &subresource_data, &texture));
	VERIFY_HR(device->CreateShaderResourceView(static_cast<ID3D11Resource*>(texture), nullptr, &textures[id].srv));

	return id;
}

}