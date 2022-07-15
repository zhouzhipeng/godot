/*************************************************************************/
/*  texture_storage.h                                                    */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef TEXTURE_STORAGE_RD_H
#define TEXTURE_STORAGE_RD_H

#include "core/templates/rid_owner.h"
#include "servers/rendering/renderer_rd/shaders/canvas_sdf.glsl.gen.h"
#include "servers/rendering/storage/texture_storage.h"
#include "servers/rendering/storage/utilities.h"

namespace RendererRD {

enum DefaultRDTexture {
	DEFAULT_RD_TEXTURE_WHITE,
	DEFAULT_RD_TEXTURE_BLACK,
	DEFAULT_RD_TEXTURE_NORMAL,
	DEFAULT_RD_TEXTURE_ANISO,
	DEFAULT_RD_TEXTURE_DEPTH,
	DEFAULT_RD_TEXTURE_MULTIMESH_BUFFER,
	DEFAULT_RD_TEXTURE_CUBEMAP_BLACK,
	DEFAULT_RD_TEXTURE_CUBEMAP_ARRAY_BLACK,
	DEFAULT_RD_TEXTURE_CUBEMAP_WHITE,
	DEFAULT_RD_TEXTURE_3D_WHITE,
	DEFAULT_RD_TEXTURE_3D_BLACK,
	DEFAULT_RD_TEXTURE_2D_ARRAY_WHITE,
	DEFAULT_RD_TEXTURE_2D_UINT,
	DEFAULT_RD_TEXTURE_MAX
};

class CanvasTexture {
public:
	RID diffuse;
	RID normal_map;
	RID specular;
	Color specular_color = Color(1, 1, 1, 1);
	float shininess = 1.0;

	RS::CanvasItemTextureFilter texture_filter = RS::CANVAS_ITEM_TEXTURE_FILTER_DEFAULT;
	RS::CanvasItemTextureRepeat texture_repeat = RS::CANVAS_ITEM_TEXTURE_REPEAT_DEFAULT;
	RID uniform_sets[RS::CANVAS_ITEM_TEXTURE_FILTER_MAX][RS::CANVAS_ITEM_TEXTURE_REPEAT_MAX];

	Size2i size_cache = Size2i(1, 1);
	bool use_normal_cache = false;
	bool use_specular_cache = false;
	bool cleared_cache = true;

	void clear_sets();
	~CanvasTexture();
};

class Texture {
public:
	enum Type {
		TYPE_2D,
		TYPE_LAYERED,
		TYPE_3D
	};

	Type type;
	RS::TextureLayeredType layered_type = RS::TEXTURE_LAYERED_2D_ARRAY;

	RenderingDevice::TextureType rd_type;
	RID rd_texture;
	RID rd_texture_srgb;
	RenderingDevice::DataFormat rd_format;
	RenderingDevice::DataFormat rd_format_srgb;

	RD::TextureView rd_view;

	Image::Format format;
	Image::Format validated_format;

	int width;
	int height;
	int depth;
	int layers;
	int mipmaps;

	int height_2d;
	int width_2d;

	struct BufferSlice3D {
		Size2i size;
		uint32_t offset = 0;
		uint32_t buffer_size = 0;
	};
	Vector<BufferSlice3D> buffer_slices_3d;
	uint32_t buffer_size_3d = 0;

	bool is_render_target;
	bool is_proxy;

	Ref<Image> image_cache_2d;
	String path;

	RID proxy_to;
	Vector<RID> proxies;

	HashSet<RID> lightmap_users;

	RS::TextureDetectCallback detect_3d_callback = nullptr;
	void *detect_3d_callback_ud = nullptr;

	RS::TextureDetectCallback detect_normal_callback = nullptr;
	void *detect_normal_callback_ud = nullptr;

	RS::TextureDetectRoughnessCallback detect_roughness_callback = nullptr;
	void *detect_roughness_callback_ud = nullptr;

	CanvasTexture *canvas_texture = nullptr;

	void cleanup();
};

struct DecalAtlas {
	struct Texture {
		int panorama_to_dp_users;
		int users;
		Rect2 uv_rect;
	};

	struct SortItem {
		RID texture;
		Size2i pixel_size;
		Size2i size;
		Point2i pos;

		bool operator<(const SortItem &p_item) const {
			//sort larger to smaller
			if (size.height == p_item.size.height) {
				return size.width > p_item.size.width;
			} else {
				return size.height > p_item.size.height;
			}
		}
	};

	HashMap<RID, Texture> textures;
	bool dirty = true;
	int mipmaps = 5;

	RID texture;
	RID texture_srgb;
	struct MipMap {
		RID fb;
		RID texture;
		Size2i size;
	};
	Vector<MipMap> texture_mipmaps;

	Size2i size;
};

struct Decal {
	Vector3 extents = Vector3(1, 1, 1);
	RID textures[RS::DECAL_TEXTURE_MAX];
	float emission_energy = 1.0;
	float albedo_mix = 1.0;
	Color modulate = Color(1, 1, 1, 1);
	uint32_t cull_mask = (1 << 20) - 1;
	float upper_fade = 0.3;
	float lower_fade = 0.3;
	bool distance_fade = false;
	float distance_fade_begin = 10;
	float distance_fade_length = 1;
	float normal_fade = 0.0;

	Dependency dependency;
};

struct RenderTarget {
	Size2i size;
	uint32_t view_count;
	RID framebuffer;
	RID color;

	//used for retrieving from CPU
	RD::DataFormat color_format = RD::DATA_FORMAT_R4G4_UNORM_PACK8;
	RD::DataFormat color_format_srgb = RD::DATA_FORMAT_R4G4_UNORM_PACK8;
	Image::Format image_format = Image::FORMAT_L8;

	bool is_transparent = false;

	bool sdf_enabled = false;

	RID backbuffer; //used for effects
	RID backbuffer_fb;
	RID backbuffer_mipmap0;

	Vector<RID> backbuffer_mipmaps;

	RID framebuffer_uniform_set;
	RID backbuffer_uniform_set;

	RID sdf_buffer_write;
	RID sdf_buffer_write_fb;
	RID sdf_buffer_process[2];
	RID sdf_buffer_read;
	RID sdf_buffer_process_uniform_sets[2];
	RS::ViewportSDFOversize sdf_oversize = RS::VIEWPORT_SDF_OVERSIZE_120_PERCENT;
	RS::ViewportSDFScale sdf_scale = RS::VIEWPORT_SDF_SCALE_50_PERCENT;
	Size2i process_size;

	//texture generated for this owner (nor RD).
	RID texture;
	bool was_used;

	//clear request
	bool clear_requested;
	Color clear_color;
};

struct RenderTargetSDF {
	enum {
		SHADER_LOAD,
		SHADER_LOAD_SHRINK,
		SHADER_PROCESS,
		SHADER_PROCESS_OPTIMIZED,
		SHADER_STORE,
		SHADER_STORE_SHRINK,
		SHADER_MAX
	};

	struct PushConstant {
		int32_t size[2];
		int32_t stride;
		int32_t shift;
		int32_t base_size[2];
		int32_t pad[2];
	};

	CanvasSdfShaderRD shader;
	RID shader_version;
	RID pipelines[SHADER_MAX];
};

class TextureStorage : public RendererTextureStorage {
private:
	static TextureStorage *singleton;

	/* Canvas Texture API */

	RID_Owner<RendererRD::CanvasTexture, true> canvas_texture_owner;

	/* Texture API */

	//textures can be created from threads, so this RID_Owner is thread safe
	mutable RID_Owner<Texture, true> texture_owner;

	struct TextureToRDFormat {
		RD::DataFormat format;
		RD::DataFormat format_srgb;
		RD::TextureSwizzle swizzle_r;
		RD::TextureSwizzle swizzle_g;
		RD::TextureSwizzle swizzle_b;
		RD::TextureSwizzle swizzle_a;
		TextureToRDFormat() {
			format = RD::DATA_FORMAT_MAX;
			format_srgb = RD::DATA_FORMAT_MAX;
			swizzle_r = RD::TEXTURE_SWIZZLE_R;
			swizzle_g = RD::TEXTURE_SWIZZLE_G;
			swizzle_b = RD::TEXTURE_SWIZZLE_B;
			swizzle_a = RD::TEXTURE_SWIZZLE_A;
		}
	};

	Ref<Image> _validate_texture_format(const Ref<Image> &p_image, TextureToRDFormat &r_format);
	void _texture_2d_update(RID p_texture, const Ref<Image> &p_image, int p_layer = 0, bool p_immediate = false);

	/* DECAL API */

	DecalAtlas decal_atlas;

	mutable RID_Owner<Decal, true> decal_owner;

	/* RENDER TARGET API */

	mutable RID_Owner<RenderTarget> render_target_owner;

	void _clear_render_target(RenderTarget *rt);
	void _update_render_target(RenderTarget *rt);
	void _create_render_target_backbuffer(RenderTarget *rt);
	void _render_target_allocate_sdf(RenderTarget *rt);
	void _render_target_clear_sdf(RenderTarget *rt);
	Rect2i _render_target_get_sdf_rect(const RenderTarget *rt) const;

	RenderTargetSDF rt_sdf;

public:
	static TextureStorage *get_singleton();

	RID default_rd_textures[DEFAULT_RD_TEXTURE_MAX];

	_FORCE_INLINE_ RID texture_rd_get_default(DefaultRDTexture p_texture) {
		return default_rd_textures[p_texture];
	}

	TextureStorage();
	virtual ~TextureStorage();

	/* Canvas Texture API */

	CanvasTexture *get_canvas_texture(RID p_rid) { return canvas_texture_owner.get_or_null(p_rid); };
	bool owns_canvas_texture(RID p_rid) { return canvas_texture_owner.owns(p_rid); };

	virtual RID canvas_texture_allocate() override;
	virtual void canvas_texture_initialize(RID p_rid) override;
	virtual void canvas_texture_free(RID p_rid) override;

	virtual void canvas_texture_set_channel(RID p_canvas_texture, RS::CanvasTextureChannel p_channel, RID p_texture) override;
	virtual void canvas_texture_set_shading_parameters(RID p_canvas_texture, const Color &p_base_color, float p_shininess) override;

	virtual void canvas_texture_set_texture_filter(RID p_item, RS::CanvasItemTextureFilter p_filter) override;
	virtual void canvas_texture_set_texture_repeat(RID p_item, RS::CanvasItemTextureRepeat p_repeat) override;

	bool canvas_texture_get_uniform_set(RID p_texture, RS::CanvasItemTextureFilter p_base_filter, RS::CanvasItemTextureRepeat p_base_repeat, RID p_base_shader, int p_base_set, RID &r_uniform_set, Size2i &r_size, Color &r_specular_shininess, bool &r_use_normal, bool &r_use_specular);

	/* Texture API */

	Texture *get_texture(RID p_rid) { return texture_owner.get_or_null(p_rid); };
	bool owns_texture(RID p_rid) { return texture_owner.owns(p_rid); };

	virtual bool can_create_resources_async() const override;

	virtual RID texture_allocate() override;
	virtual void texture_free(RID p_rid) override;

	virtual void texture_2d_initialize(RID p_texture, const Ref<Image> &p_image) override;
	virtual void texture_2d_layered_initialize(RID p_texture, const Vector<Ref<Image>> &p_layers, RS::TextureLayeredType p_layered_type) override;
	virtual void texture_3d_initialize(RID p_texture, Image::Format, int p_width, int p_height, int p_depth, bool p_mipmaps, const Vector<Ref<Image>> &p_data) override;
	virtual void texture_proxy_initialize(RID p_texture, RID p_base) override; //all slices, then all the mipmaps, must be coherent

	virtual void texture_2d_update(RID p_texture, const Ref<Image> &p_image, int p_layer = 0) override;
	virtual void texture_3d_update(RID p_texture, const Vector<Ref<Image>> &p_data) override;
	virtual void texture_proxy_update(RID p_proxy, RID p_base) override;

	//these two APIs can be used together or in combination with the others.
	virtual void texture_2d_placeholder_initialize(RID p_texture) override;
	virtual void texture_2d_layered_placeholder_initialize(RID p_texture, RenderingServer::TextureLayeredType p_layered_type) override;
	virtual void texture_3d_placeholder_initialize(RID p_texture) override;

	virtual Ref<Image> texture_2d_get(RID p_texture) const override;
	virtual Ref<Image> texture_2d_layer_get(RID p_texture, int p_layer) const override;
	virtual Vector<Ref<Image>> texture_3d_get(RID p_texture) const override;

	virtual void texture_replace(RID p_texture, RID p_by_texture) override;
	virtual void texture_set_size_override(RID p_texture, int p_width, int p_height) override;

	virtual void texture_set_path(RID p_texture, const String &p_path) override;
	virtual String texture_get_path(RID p_texture) const override;

	virtual void texture_set_detect_3d_callback(RID p_texture, RS::TextureDetectCallback p_callback, void *p_userdata) override;
	virtual void texture_set_detect_normal_callback(RID p_texture, RS::TextureDetectCallback p_callback, void *p_userdata) override;
	virtual void texture_set_detect_roughness_callback(RID p_texture, RS::TextureDetectRoughnessCallback p_callback, void *p_userdata) override;

	virtual void texture_debug_usage(List<RS::TextureInfo> *r_info) override;

	virtual void texture_set_force_redraw_if_visible(RID p_texture, bool p_enable) override;

	virtual Size2 texture_size_with_proxy(RID p_proxy) override;

	//internal usage

	_FORCE_INLINE_ RID texture_get_rd_texture(RID p_texture, bool p_srgb = false) {
		if (p_texture.is_null()) {
			return RID();
		}
		RendererRD::Texture *tex = texture_owner.get_or_null(p_texture);

		if (!tex) {
			return RID();
		}
		return (p_srgb && tex->rd_texture_srgb.is_valid()) ? tex->rd_texture_srgb : tex->rd_texture;
	}

	_FORCE_INLINE_ Size2i texture_2d_get_size(RID p_texture) {
		if (p_texture.is_null()) {
			return Size2i();
		}
		RendererRD::Texture *tex = texture_owner.get_or_null(p_texture);

		if (!tex) {
			return Size2i();
		}
		return Size2i(tex->width_2d, tex->height_2d);
	}

	/* DECAL API */

	void update_decal_atlas();

	Decal *get_decal(RID p_rid) { return decal_owner.get_or_null(p_rid); };
	bool owns_decal(RID p_rid) { return decal_owner.owns(p_rid); };

	RID decal_atlas_get_texture() const;
	RID decal_atlas_get_texture_srgb() const;
	_FORCE_INLINE_ Rect2 decal_atlas_get_texture_rect(RID p_texture) {
		DecalAtlas::Texture *t = decal_atlas.textures.getptr(p_texture);
		if (!t) {
			return Rect2();
		}

		return t->uv_rect;
	}

	virtual RID decal_allocate() override;
	virtual void decal_initialize(RID p_decal) override;
	virtual void decal_free(RID p_rid) override;

	virtual void decal_set_extents(RID p_decal, const Vector3 &p_extents) override;
	virtual void decal_set_texture(RID p_decal, RS::DecalTexture p_type, RID p_texture) override;
	virtual void decal_set_emission_energy(RID p_decal, float p_energy) override;
	virtual void decal_set_albedo_mix(RID p_decal, float p_mix) override;
	virtual void decal_set_modulate(RID p_decal, const Color &p_modulate) override;
	virtual void decal_set_cull_mask(RID p_decal, uint32_t p_layers) override;
	virtual void decal_set_distance_fade(RID p_decal, bool p_enabled, float p_begin, float p_length) override;
	virtual void decal_set_fade(RID p_decal, float p_above, float p_below) override;
	virtual void decal_set_normal_fade(RID p_decal, float p_fade) override;

	void decal_atlas_mark_dirty_on_texture(RID p_texture);
	void decal_atlas_remove_texture(RID p_texture);

	virtual void texture_add_to_decal_atlas(RID p_texture, bool p_panorama_to_dp = false) override;
	virtual void texture_remove_from_decal_atlas(RID p_texture, bool p_panorama_to_dp = false) override;

	_FORCE_INLINE_ Vector3 decal_get_extents(RID p_decal) {
		const Decal *decal = decal_owner.get_or_null(p_decal);
		return decal->extents;
	}

	_FORCE_INLINE_ RID decal_get_texture(RID p_decal, RS::DecalTexture p_texture) {
		const Decal *decal = decal_owner.get_or_null(p_decal);
		return decal->textures[p_texture];
	}

	_FORCE_INLINE_ Color decal_get_modulate(RID p_decal) {
		const Decal *decal = decal_owner.get_or_null(p_decal);
		return decal->modulate;
	}

	_FORCE_INLINE_ float decal_get_emission_energy(RID p_decal) {
		const Decal *decal = decal_owner.get_or_null(p_decal);
		return decal->emission_energy;
	}

	_FORCE_INLINE_ float decal_get_albedo_mix(RID p_decal) {
		const Decal *decal = decal_owner.get_or_null(p_decal);
		return decal->albedo_mix;
	}

	_FORCE_INLINE_ uint32_t decal_get_cull_mask(RID p_decal) {
		const Decal *decal = decal_owner.get_or_null(p_decal);
		return decal->cull_mask;
	}

	_FORCE_INLINE_ float decal_get_upper_fade(RID p_decal) {
		const Decal *decal = decal_owner.get_or_null(p_decal);
		return decal->upper_fade;
	}

	_FORCE_INLINE_ float decal_get_lower_fade(RID p_decal) {
		const Decal *decal = decal_owner.get_or_null(p_decal);
		return decal->lower_fade;
	}

	_FORCE_INLINE_ float decal_get_normal_fade(RID p_decal) {
		const Decal *decal = decal_owner.get_or_null(p_decal);
		return decal->normal_fade;
	}

	_FORCE_INLINE_ bool decal_is_distance_fade_enabled(RID p_decal) {
		const Decal *decal = decal_owner.get_or_null(p_decal);
		return decal->distance_fade;
	}

	_FORCE_INLINE_ float decal_get_distance_fade_begin(RID p_decal) {
		const Decal *decal = decal_owner.get_or_null(p_decal);
		return decal->distance_fade_begin;
	}

	_FORCE_INLINE_ float decal_get_distance_fade_length(RID p_decal) {
		const Decal *decal = decal_owner.get_or_null(p_decal);
		return decal->distance_fade_length;
	}

	virtual AABB decal_get_aabb(RID p_decal) const override;

	/* RENDER TARGET API */

	RenderTarget *get_render_target(RID p_rid) { return render_target_owner.get_or_null(p_rid); };
	bool owns_render_target(RID p_rid) { return render_target_owner.owns(p_rid); };

	virtual RID render_target_create() override;
	virtual void render_target_free(RID p_rid) override;

	virtual void render_target_set_position(RID p_render_target, int p_x, int p_y) override;
	virtual void render_target_set_size(RID p_render_target, int p_width, int p_height, uint32_t p_view_count) override;
	virtual RID render_target_get_texture(RID p_render_target) override;
	virtual void render_target_set_external_texture(RID p_render_target, unsigned int p_texture_id) override;
	virtual void render_target_set_transparent(RID p_render_target, bool p_is_transparent) override;
	virtual void render_target_set_direct_to_screen(RID p_render_target, bool p_direct_to_screen) override;
	virtual bool render_target_was_used(RID p_render_target) override;
	virtual void render_target_set_as_unused(RID p_render_target) override;

	void render_target_copy_to_back_buffer(RID p_render_target, const Rect2i &p_region, bool p_gen_mipmaps);
	void render_target_clear_back_buffer(RID p_render_target, const Rect2i &p_region, const Color &p_color);
	void render_target_gen_back_buffer_mipmaps(RID p_render_target, const Rect2i &p_region);
	RID render_target_get_back_buffer_uniform_set(RID p_render_target, RID p_base_shader);

	virtual void render_target_request_clear(RID p_render_target, const Color &p_clear_color) override;
	virtual bool render_target_is_clear_requested(RID p_render_target) override;
	virtual Color render_target_get_clear_request_color(RID p_render_target) override;
	virtual void render_target_disable_clear_request(RID p_render_target) override;
	virtual void render_target_do_clear_request(RID p_render_target) override;

	virtual void render_target_set_sdf_size_and_scale(RID p_render_target, RS::ViewportSDFOversize p_size, RS::ViewportSDFScale p_scale) override;
	RID render_target_get_sdf_texture(RID p_render_target);
	RID render_target_get_sdf_framebuffer(RID p_render_target);
	void render_target_sdf_process(RID p_render_target);
	virtual Rect2i render_target_get_sdf_rect(RID p_render_target) const override;
	virtual void render_target_mark_sdf_enabled(RID p_render_target, bool p_enabled) override;
	bool render_target_is_sdf_enabled(RID p_render_target) const;

	Size2 render_target_get_size(RID p_render_target);
	RID render_target_get_rd_framebuffer(RID p_render_target);
	RID render_target_get_rd_texture(RID p_render_target);
	RID render_target_get_rd_backbuffer(RID p_render_target);
	RID render_target_get_rd_backbuffer_framebuffer(RID p_render_target);

	RID render_target_get_framebuffer_uniform_set(RID p_render_target);
	RID render_target_get_backbuffer_uniform_set(RID p_render_target);

	void render_target_set_framebuffer_uniform_set(RID p_render_target, RID p_uniform_set);
	void render_target_set_backbuffer_uniform_set(RID p_render_target, RID p_uniform_set);
};

} // namespace RendererRD

#endif // !_TEXTURE_STORAGE_RD_H
