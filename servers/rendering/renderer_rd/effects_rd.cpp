/*************************************************************************/
/*  effects_rd.cpp                                                       */
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

#include "effects_rd.h"

#include "core/config/project_settings.h"
#include "core/math/math_defs.h"
#include "core/os/os.h"

#include "servers/rendering/renderer_rd/renderer_compositor_rd.h"
#include "thirdparty/misc/cubemap_coeffs.h"

bool EffectsRD::get_prefer_raster_effects() {
	return prefer_raster_effects;
}

static _FORCE_INLINE_ void store_camera(const CameraMatrix &p_mtx, float *p_array) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			p_array[i * 4 + j] = p_mtx.matrix[i][j];
		}
	}
}

RID EffectsRD::_get_uniform_set_from_image(RID p_image) {
	if (image_to_uniform_set_cache.has(p_image)) {
		RID uniform_set = image_to_uniform_set_cache[p_image];
		if (RD::get_singleton()->uniform_set_is_valid(uniform_set)) {
			return uniform_set;
		}
	}
	Vector<RD::Uniform> uniforms;
	RD::Uniform u;
	u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
	u.binding = 0;
	u.append_id(p_image);
	uniforms.push_back(u);
	//any thing with the same configuration (one texture in binding 0 for set 0), is good
	RID uniform_set = RD::get_singleton()->uniform_set_create(uniforms, luminance_reduce.shader.version_get_shader(luminance_reduce.shader_version, 0), 1);

	image_to_uniform_set_cache[p_image] = uniform_set;

	return uniform_set;
}

RID EffectsRD::_get_uniform_set_from_texture(RID p_texture, bool p_use_mipmaps) {
	if (texture_to_uniform_set_cache.has(p_texture)) {
		RID uniform_set = texture_to_uniform_set_cache[p_texture];
		if (RD::get_singleton()->uniform_set_is_valid(uniform_set)) {
			return uniform_set;
		}
	}

	Vector<RD::Uniform> uniforms;
	RD::Uniform u;
	u.uniform_type = RD::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE;
	u.binding = 0;
	u.append_id(p_use_mipmaps ? default_mipmap_sampler : default_sampler);
	u.append_id(p_texture);
	uniforms.push_back(u);
	// anything with the same configuration (one texture in binding 0 for set 0), is good
	RID uniform_set = RD::get_singleton()->uniform_set_create(uniforms, cube_to_dp.shader.version_get_shader(cube_to_dp.shader_version, 0), 0);

	texture_to_uniform_set_cache[p_texture] = uniform_set;

	return uniform_set;
}

RID EffectsRD::_get_compute_uniform_set_from_texture(RID p_texture, bool p_use_mipmaps) {
	if (texture_to_compute_uniform_set_cache.has(p_texture)) {
		RID uniform_set = texture_to_compute_uniform_set_cache[p_texture];
		if (RD::get_singleton()->uniform_set_is_valid(uniform_set)) {
			return uniform_set;
		}
	}

	Vector<RD::Uniform> uniforms;
	RD::Uniform u;
	u.uniform_type = RD::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE;
	u.binding = 0;
	u.append_id(p_use_mipmaps ? default_mipmap_sampler : default_sampler);
	u.append_id(p_texture);
	uniforms.push_back(u);
	//any thing with the same configuration (one texture in binding 0 for set 0), is good
	RID uniform_set = RD::get_singleton()->uniform_set_create(uniforms, luminance_reduce.shader.version_get_shader(luminance_reduce.shader_version, 0), 0);

	texture_to_compute_uniform_set_cache[p_texture] = uniform_set;

	return uniform_set;
}

RID EffectsRD::_get_compute_uniform_set_from_texture_and_sampler(RID p_texture, RID p_sampler) {
	TextureSamplerPair tsp;
	tsp.texture = p_texture;
	tsp.sampler = p_sampler;

	if (texture_sampler_to_compute_uniform_set_cache.has(tsp)) {
		RID uniform_set = texture_sampler_to_compute_uniform_set_cache[tsp];
		if (RD::get_singleton()->uniform_set_is_valid(uniform_set)) {
			return uniform_set;
		}
	}

	Vector<RD::Uniform> uniforms;
	RD::Uniform u;
	u.uniform_type = RD::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE;
	u.binding = 0;
	u.append_id(p_sampler);
	u.append_id(p_texture);
	uniforms.push_back(u);
	//any thing with the same configuration (one texture in binding 0 for set 0), is good
	RID uniform_set = RD::get_singleton()->uniform_set_create(uniforms, ssao.blur_shader.version_get_shader(ssao.blur_shader_version, 0), 0);

	texture_sampler_to_compute_uniform_set_cache[tsp] = uniform_set;

	return uniform_set;
}

RID EffectsRD::_get_compute_uniform_set_from_texture_pair(RID p_texture1, RID p_texture2, bool p_use_mipmaps) {
	TexturePair tp;
	tp.texture1 = p_texture1;
	tp.texture2 = p_texture2;

	if (texture_pair_to_compute_uniform_set_cache.has(tp)) {
		RID uniform_set = texture_pair_to_compute_uniform_set_cache[tp];
		if (RD::get_singleton()->uniform_set_is_valid(uniform_set)) {
			return uniform_set;
		}
	}

	Vector<RD::Uniform> uniforms;
	{
		RD::Uniform u;
		u.uniform_type = RD::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE;
		u.binding = 0;
		u.append_id(p_use_mipmaps ? default_mipmap_sampler : default_sampler);
		u.append_id(p_texture1);
		uniforms.push_back(u);
	}
	{
		RD::Uniform u;
		u.uniform_type = RD::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE;
		u.binding = 1;
		u.append_id(p_use_mipmaps ? default_mipmap_sampler : default_sampler);
		u.append_id(p_texture2);
		uniforms.push_back(u);
	}
	//any thing with the same configuration (one texture in binding 0 for set 0), is good
	RID uniform_set = RD::get_singleton()->uniform_set_create(uniforms, ssr_scale.shader.version_get_shader(ssr_scale.shader_version, 0), 1);

	texture_pair_to_compute_uniform_set_cache[tp] = uniform_set;

	return uniform_set;
}

RID EffectsRD::_get_compute_uniform_set_from_image_pair(RID p_texture1, RID p_texture2) {
	TexturePair tp;
	tp.texture1 = p_texture1;
	tp.texture2 = p_texture2;

	if (image_pair_to_compute_uniform_set_cache.has(tp)) {
		RID uniform_set = image_pair_to_compute_uniform_set_cache[tp];
		if (RD::get_singleton()->uniform_set_is_valid(uniform_set)) {
			return uniform_set;
		}
	}

	Vector<RD::Uniform> uniforms;
	{
		RD::Uniform u;
		u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
		u.binding = 0;
		u.append_id(p_texture1);
		uniforms.push_back(u);
	}
	{
		RD::Uniform u;
		u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
		u.binding = 1;
		u.append_id(p_texture2);
		uniforms.push_back(u);
	}
	//any thing with the same configuration (one texture in binding 0 for set 0), is good
	RID uniform_set = RD::get_singleton()->uniform_set_create(uniforms, ssr_scale.shader.version_get_shader(ssr_scale.shader_version, 0), 3);

	image_pair_to_compute_uniform_set_cache[tp] = uniform_set;

	return uniform_set;
}

void EffectsRD::fsr_upscale(RID p_source_rd_texture, RID p_secondary_texture, RID p_destination_texture, const Size2i &p_internal_size, const Size2i &p_size, float p_fsr_upscale_sharpness) {
	memset(&FSR_upscale.push_constant, 0, sizeof(FSRUpscalePushConstant));

	int dispatch_x = (p_size.x + 15) / 16;
	int dispatch_y = (p_size.y + 15) / 16;

	RD::ComputeListID compute_list = RD::get_singleton()->compute_list_begin();
	RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, FSR_upscale.pipeline);

	FSR_upscale.push_constant.resolution_width = p_internal_size.width;
	FSR_upscale.push_constant.resolution_height = p_internal_size.height;
	FSR_upscale.push_constant.upscaled_width = p_size.width;
	FSR_upscale.push_constant.upscaled_height = p_size.height;
	FSR_upscale.push_constant.sharpness = p_fsr_upscale_sharpness;

	//FSR Easc
	FSR_upscale.push_constant.pass = FSR_UPSCALE_PASS_EASU;
	RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_texture(p_source_rd_texture), 0);
	RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_uniform_set_from_image(p_secondary_texture), 1);

	RD::get_singleton()->compute_list_set_push_constant(compute_list, &FSR_upscale.push_constant, sizeof(FSRUpscalePushConstant));

	RD::get_singleton()->compute_list_dispatch(compute_list, dispatch_x, dispatch_y, 1);
	RD::get_singleton()->compute_list_add_barrier(compute_list);

	//FSR Rcas
	FSR_upscale.push_constant.pass = FSR_UPSCALE_PASS_RCAS;
	RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_texture(p_secondary_texture), 0);
	RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_uniform_set_from_image(p_destination_texture), 1);

	RD::get_singleton()->compute_list_set_push_constant(compute_list, &FSR_upscale.push_constant, sizeof(FSRUpscalePushConstant));

	RD::get_singleton()->compute_list_dispatch(compute_list, dispatch_x, dispatch_y, 1);

	RD::get_singleton()->compute_list_end(compute_list);
}

void EffectsRD::taa_resolve(RID p_frame, RID p_temp, RID p_depth, RID p_velocity, RID p_prev_velocity, RID p_history, Size2 p_resolution, float p_z_near, float p_z_far) {
	UniformSetCacheRD *uniform_set_cache = UniformSetCacheRD::get_singleton();
	ERR_FAIL_NULL(uniform_set_cache);

	RID shader = TAA_resolve.shader.version_get_shader(TAA_resolve.shader_version, 0);
	ERR_FAIL_COND(shader.is_null());

	memset(&TAA_resolve.push_constant, 0, sizeof(TAAResolvePushConstant));
	TAA_resolve.push_constant.resolution_width = p_resolution.width;
	TAA_resolve.push_constant.resolution_height = p_resolution.height;
	TAA_resolve.push_constant.disocclusion_threshold = 0.025f;
	TAA_resolve.push_constant.disocclusion_scale = 10.0f;

	RD::ComputeListID compute_list = RD::get_singleton()->compute_list_begin();
	RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, TAA_resolve.pipeline);

	RD::Uniform u_frame_source(RD::UNIFORM_TYPE_IMAGE, 0, { p_frame });
	RD::Uniform u_depth(RD::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE, 1, { default_sampler, p_depth });
	RD::Uniform u_velocity(RD::UNIFORM_TYPE_IMAGE, 2, { p_velocity });
	RD::Uniform u_prev_velocity(RD::UNIFORM_TYPE_IMAGE, 3, { p_prev_velocity });
	RD::Uniform u_history(RD::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE, 4, { default_sampler, p_history });
	RD::Uniform u_frame_dest(RD::UNIFORM_TYPE_IMAGE, 5, { p_temp });

	RD::get_singleton()->compute_list_bind_uniform_set(compute_list, uniform_set_cache->get_cache(shader, 0, u_frame_source, u_depth, u_velocity, u_prev_velocity, u_history, u_frame_dest), 0);
	RD::get_singleton()->compute_list_set_push_constant(compute_list, &TAA_resolve.push_constant, sizeof(TAAResolvePushConstant));
	RD::get_singleton()->compute_list_dispatch_threads(compute_list, p_resolution.width, p_resolution.height, 1);
	RD::get_singleton()->compute_list_end();
}

void EffectsRD::screen_space_reflection(RID p_diffuse, RID p_normal_roughness, RenderingServer::EnvironmentSSRRoughnessQuality p_roughness_quality, RID p_blur_radius, RID p_blur_radius2, RID p_metallic, const Color &p_metallic_mask, RID p_depth, RID p_scale_depth, RID p_scale_normal, RID p_output, RID p_output_blur, const Size2i &p_screen_size, int p_max_steps, float p_fade_in, float p_fade_out, float p_tolerance, const CameraMatrix &p_camera) {
	RD::ComputeListID compute_list = RD::get_singleton()->compute_list_begin();

	{ //scale color and depth to half
		ssr_scale.push_constant.camera_z_far = p_camera.get_z_far();
		ssr_scale.push_constant.camera_z_near = p_camera.get_z_near();
		ssr_scale.push_constant.orthogonal = p_camera.is_orthogonal();
		ssr_scale.push_constant.filter = false; //enabling causes arctifacts
		ssr_scale.push_constant.screen_size[0] = p_screen_size.x;
		ssr_scale.push_constant.screen_size[1] = p_screen_size.y;

		RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, ssr_scale.pipeline);

		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_texture(p_diffuse), 0);
		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_texture_pair(p_depth, p_normal_roughness), 1);
		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_uniform_set_from_image(p_output_blur), 2);
		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_image_pair(p_scale_depth, p_scale_normal), 3);

		RD::get_singleton()->compute_list_set_push_constant(compute_list, &ssr_scale.push_constant, sizeof(ScreenSpaceReflectionScalePushConstant));

		RD::get_singleton()->compute_list_dispatch_threads(compute_list, p_screen_size.width, p_screen_size.height, 1);

		RD::get_singleton()->compute_list_add_barrier(compute_list);
	}

	{
		ssr.push_constant.camera_z_far = p_camera.get_z_far();
		ssr.push_constant.camera_z_near = p_camera.get_z_near();
		ssr.push_constant.orthogonal = p_camera.is_orthogonal();
		ssr.push_constant.screen_size[0] = p_screen_size.x;
		ssr.push_constant.screen_size[1] = p_screen_size.y;
		ssr.push_constant.curve_fade_in = p_fade_in;
		ssr.push_constant.distance_fade = p_fade_out;
		ssr.push_constant.num_steps = p_max_steps;
		ssr.push_constant.depth_tolerance = p_tolerance;
		ssr.push_constant.use_half_res = true;
		ssr.push_constant.proj_info[0] = -2.0f / (p_screen_size.width * p_camera.matrix[0][0]);
		ssr.push_constant.proj_info[1] = -2.0f / (p_screen_size.height * p_camera.matrix[1][1]);
		ssr.push_constant.proj_info[2] = (1.0f - p_camera.matrix[0][2]) / p_camera.matrix[0][0];
		ssr.push_constant.proj_info[3] = (1.0f + p_camera.matrix[1][2]) / p_camera.matrix[1][1];
		ssr.push_constant.metallic_mask[0] = CLAMP(p_metallic_mask.r * 255.0, 0, 255);
		ssr.push_constant.metallic_mask[1] = CLAMP(p_metallic_mask.g * 255.0, 0, 255);
		ssr.push_constant.metallic_mask[2] = CLAMP(p_metallic_mask.b * 255.0, 0, 255);
		ssr.push_constant.metallic_mask[3] = CLAMP(p_metallic_mask.a * 255.0, 0, 255);
		store_camera(p_camera, ssr.push_constant.projection);

		RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, ssr.pipelines[(p_roughness_quality != RS::ENV_SSR_ROUGHNESS_QUALITY_DISABLED) ? SCREEN_SPACE_REFLECTION_ROUGH : SCREEN_SPACE_REFLECTION_NORMAL]);

		RD::get_singleton()->compute_list_set_push_constant(compute_list, &ssr.push_constant, sizeof(ScreenSpaceReflectionPushConstant));

		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_image_pair(p_output_blur, p_scale_depth), 0);

		if (p_roughness_quality != RS::ENV_SSR_ROUGHNESS_QUALITY_DISABLED) {
			RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_image_pair(p_output, p_blur_radius), 1);
		} else {
			RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_uniform_set_from_image(p_output), 1);
		}
		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_texture(p_metallic), 3);
		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_uniform_set_from_image(p_scale_normal), 2);

		RD::get_singleton()->compute_list_dispatch_threads(compute_list, p_screen_size.width, p_screen_size.height, 1);
	}

	if (p_roughness_quality != RS::ENV_SSR_ROUGHNESS_QUALITY_DISABLED) {
		//blur

		RD::get_singleton()->compute_list_add_barrier(compute_list);

		ssr_filter.push_constant.orthogonal = p_camera.is_orthogonal();
		ssr_filter.push_constant.edge_tolerance = Math::sin(Math::deg2rad(15.0));
		ssr_filter.push_constant.proj_info[0] = -2.0f / (p_screen_size.width * p_camera.matrix[0][0]);
		ssr_filter.push_constant.proj_info[1] = -2.0f / (p_screen_size.height * p_camera.matrix[1][1]);
		ssr_filter.push_constant.proj_info[2] = (1.0f - p_camera.matrix[0][2]) / p_camera.matrix[0][0];
		ssr_filter.push_constant.proj_info[3] = (1.0f + p_camera.matrix[1][2]) / p_camera.matrix[1][1];
		ssr_filter.push_constant.vertical = 0;
		if (p_roughness_quality == RS::ENV_SSR_ROUGHNESS_QUALITY_LOW) {
			ssr_filter.push_constant.steps = p_max_steps / 3;
			ssr_filter.push_constant.increment = 3;
		} else if (p_roughness_quality == RS::ENV_SSR_ROUGHNESS_QUALITY_MEDIUM) {
			ssr_filter.push_constant.steps = p_max_steps / 2;
			ssr_filter.push_constant.increment = 2;
		} else {
			ssr_filter.push_constant.steps = p_max_steps;
			ssr_filter.push_constant.increment = 1;
		}

		ssr_filter.push_constant.screen_size[0] = p_screen_size.width;
		ssr_filter.push_constant.screen_size[1] = p_screen_size.height;

		RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, ssr_filter.pipelines[SCREEN_SPACE_REFLECTION_FILTER_HORIZONTAL]);

		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_image_pair(p_output, p_blur_radius), 0);
		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_uniform_set_from_image(p_scale_normal), 1);
		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_image_pair(p_output_blur, p_blur_radius2), 2);
		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_uniform_set_from_image(p_scale_depth), 3);

		RD::get_singleton()->compute_list_set_push_constant(compute_list, &ssr_filter.push_constant, sizeof(ScreenSpaceReflectionFilterPushConstant));

		RD::get_singleton()->compute_list_dispatch_threads(compute_list, p_screen_size.width, p_screen_size.height, 1);

		RD::get_singleton()->compute_list_add_barrier(compute_list);

		RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, ssr_filter.pipelines[SCREEN_SPACE_REFLECTION_FILTER_VERTICAL]);

		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_image_pair(p_output_blur, p_blur_radius2), 0);
		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_uniform_set_from_image(p_scale_normal), 1);
		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_uniform_set_from_image(p_output), 2);
		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_uniform_set_from_image(p_scale_depth), 3);

		ssr_filter.push_constant.vertical = 1;

		RD::get_singleton()->compute_list_set_push_constant(compute_list, &ssr_filter.push_constant, sizeof(ScreenSpaceReflectionFilterPushConstant));

		RD::get_singleton()->compute_list_dispatch_threads(compute_list, p_screen_size.width, p_screen_size.height, 1);
	}

	RD::get_singleton()->compute_list_end();
}

void EffectsRD::sub_surface_scattering(RID p_diffuse, RID p_diffuse2, RID p_depth, const CameraMatrix &p_camera, const Size2i &p_screen_size, float p_scale, float p_depth_scale, RenderingServer::SubSurfaceScatteringQuality p_quality) {
	RD::ComputeListID compute_list = RD::get_singleton()->compute_list_begin();

	Plane p = p_camera.xform4(Plane(1, 0, -1, 1));
	p.normal /= p.d;
	float unit_size = p.normal.x;

	{ //scale color and depth to half
		sss.push_constant.camera_z_far = p_camera.get_z_far();
		sss.push_constant.camera_z_near = p_camera.get_z_near();
		sss.push_constant.orthogonal = p_camera.is_orthogonal();
		sss.push_constant.unit_size = unit_size;
		sss.push_constant.screen_size[0] = p_screen_size.x;
		sss.push_constant.screen_size[1] = p_screen_size.y;
		sss.push_constant.vertical = false;
		sss.push_constant.scale = p_scale;
		sss.push_constant.depth_scale = p_depth_scale;

		RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, sss.pipelines[p_quality - 1]);

		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_texture(p_diffuse), 0);
		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_uniform_set_from_image(p_diffuse2), 1);
		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_texture(p_depth), 2);

		RD::get_singleton()->compute_list_set_push_constant(compute_list, &sss.push_constant, sizeof(SubSurfaceScatteringPushConstant));

		RD::get_singleton()->compute_list_dispatch_threads(compute_list, p_screen_size.width, p_screen_size.height, 1);

		RD::get_singleton()->compute_list_add_barrier(compute_list);

		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_texture(p_diffuse2), 0);
		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_uniform_set_from_image(p_diffuse), 1);
		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_texture(p_depth), 2);

		sss.push_constant.vertical = true;
		RD::get_singleton()->compute_list_set_push_constant(compute_list, &sss.push_constant, sizeof(SubSurfaceScatteringPushConstant));

		RD::get_singleton()->compute_list_dispatch_threads(compute_list, p_screen_size.width, p_screen_size.height, 1);

		RD::get_singleton()->compute_list_end();
	}
}

void EffectsRD::merge_specular(RID p_dest_framebuffer, RID p_specular, RID p_base, RID p_reflection) {
	RD::DrawListID draw_list = RD::get_singleton()->draw_list_begin(p_dest_framebuffer, RD::INITIAL_ACTION_KEEP, RD::FINAL_ACTION_READ, RD::INITIAL_ACTION_KEEP, RD::FINAL_ACTION_READ, Vector<Color>());

	if (p_reflection.is_valid()) {
		if (p_base.is_valid()) {
			RD::get_singleton()->draw_list_bind_render_pipeline(draw_list, specular_merge.pipelines[SPECULAR_MERGE_SSR].get_render_pipeline(RD::INVALID_ID, RD::get_singleton()->framebuffer_get_format(p_dest_framebuffer)));
			RD::get_singleton()->draw_list_bind_uniform_set(draw_list, _get_uniform_set_from_texture(p_base), 2);
		} else {
			RD::get_singleton()->draw_list_bind_render_pipeline(draw_list, specular_merge.pipelines[SPECULAR_MERGE_ADDITIVE_SSR].get_render_pipeline(RD::INVALID_ID, RD::get_singleton()->framebuffer_get_format(p_dest_framebuffer)));
		}

		RD::get_singleton()->draw_list_bind_uniform_set(draw_list, _get_uniform_set_from_texture(p_specular), 0);
		RD::get_singleton()->draw_list_bind_uniform_set(draw_list, _get_uniform_set_from_texture(p_reflection), 1);

	} else {
		if (p_base.is_valid()) {
			RD::get_singleton()->draw_list_bind_render_pipeline(draw_list, specular_merge.pipelines[SPECULAR_MERGE_ADD].get_render_pipeline(RD::INVALID_ID, RD::get_singleton()->framebuffer_get_format(p_dest_framebuffer)));
			RD::get_singleton()->draw_list_bind_uniform_set(draw_list, _get_uniform_set_from_texture(p_base), 2);
		} else {
			RD::get_singleton()->draw_list_bind_render_pipeline(draw_list, specular_merge.pipelines[SPECULAR_MERGE_ADDITIVE_ADD].get_render_pipeline(RD::INVALID_ID, RD::get_singleton()->framebuffer_get_format(p_dest_framebuffer)));
		}

		RD::get_singleton()->draw_list_bind_uniform_set(draw_list, _get_uniform_set_from_texture(p_specular), 0);
	}

	RD::get_singleton()->draw_list_bind_index_array(draw_list, index_array);
	RD::get_singleton()->draw_list_draw(draw_list, true);
	RD::get_singleton()->draw_list_end();
}

void EffectsRD::copy_cubemap_to_dp(RID p_source_rd_texture, RID p_dst_framebuffer, const Rect2 &p_rect, const Vector2 &p_dst_size, float p_z_near, float p_z_far, bool p_dp_flip) {
	CopyToDPPushConstant push_constant;
	push_constant.screen_rect[0] = p_rect.position.x;
	push_constant.screen_rect[1] = p_rect.position.y;
	push_constant.screen_rect[2] = p_rect.size.width;
	push_constant.screen_rect[3] = p_rect.size.height;
	push_constant.z_far = p_z_far;
	push_constant.z_near = p_z_near;
	push_constant.texel_size[0] = 1.0f / p_dst_size.x;
	push_constant.texel_size[1] = 1.0f / p_dst_size.y;
	push_constant.texel_size[0] *= p_dp_flip ? -1.0f : 1.0f; // Encode dp flip as x size sign

	RD::DrawListID draw_list = RD::get_singleton()->draw_list_begin(p_dst_framebuffer, RD::INITIAL_ACTION_DROP, RD::FINAL_ACTION_DISCARD, RD::INITIAL_ACTION_KEEP, RD::FINAL_ACTION_READ);
	RD::get_singleton()->draw_list_bind_render_pipeline(draw_list, cube_to_dp.pipeline.get_render_pipeline(RD::INVALID_ID, RD::get_singleton()->framebuffer_get_format(p_dst_framebuffer)));
	RD::get_singleton()->draw_list_bind_uniform_set(draw_list, _get_uniform_set_from_texture(p_source_rd_texture), 0);
	RD::get_singleton()->draw_list_bind_index_array(draw_list, index_array);

	RD::get_singleton()->draw_list_set_push_constant(draw_list, &push_constant, sizeof(CopyToDPPushConstant));
	RD::get_singleton()->draw_list_draw(draw_list, true);
	RD::get_singleton()->draw_list_end(RD::BARRIER_MASK_RASTER | RD::BARRIER_MASK_TRANSFER);
}

void EffectsRD::luminance_reduction(RID p_source_texture, const Size2i p_source_size, const Vector<RID> p_reduce, RID p_prev_luminance, float p_min_luminance, float p_max_luminance, float p_adjust, bool p_set) {
	ERR_FAIL_COND_MSG(prefer_raster_effects, "Can't use compute version of luminance reduction with the mobile renderer.");

	luminance_reduce.push_constant.source_size[0] = p_source_size.x;
	luminance_reduce.push_constant.source_size[1] = p_source_size.y;
	luminance_reduce.push_constant.max_luminance = p_max_luminance;
	luminance_reduce.push_constant.min_luminance = p_min_luminance;
	luminance_reduce.push_constant.exposure_adjust = p_adjust;

	RD::ComputeListID compute_list = RD::get_singleton()->compute_list_begin();

	for (int i = 0; i < p_reduce.size(); i++) {
		if (i == 0) {
			RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, luminance_reduce.pipelines[LUMINANCE_REDUCE_READ]);
			RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_texture(p_source_texture), 0);
		} else {
			RD::get_singleton()->compute_list_add_barrier(compute_list); //needs barrier, wait until previous is done

			if (i == p_reduce.size() - 1 && !p_set) {
				RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, luminance_reduce.pipelines[LUMINANCE_REDUCE_WRITE]);
				RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_texture(p_prev_luminance), 2);
			} else {
				RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, luminance_reduce.pipelines[LUMINANCE_REDUCE]);
			}

			RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_uniform_set_from_image(p_reduce[i - 1]), 0);
		}

		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_uniform_set_from_image(p_reduce[i]), 1);

		RD::get_singleton()->compute_list_set_push_constant(compute_list, &luminance_reduce.push_constant, sizeof(LuminanceReducePushConstant));

		RD::get_singleton()->compute_list_dispatch_threads(compute_list, luminance_reduce.push_constant.source_size[0], luminance_reduce.push_constant.source_size[1], 1);

		luminance_reduce.push_constant.source_size[0] = MAX(luminance_reduce.push_constant.source_size[0] / 8, 1);
		luminance_reduce.push_constant.source_size[1] = MAX(luminance_reduce.push_constant.source_size[1] / 8, 1);
	}

	RD::get_singleton()->compute_list_end();
}

void EffectsRD::luminance_reduction_raster(RID p_source_texture, const Size2i p_source_size, const Vector<RID> p_reduce, Vector<RID> p_fb, RID p_prev_luminance, float p_min_luminance, float p_max_luminance, float p_adjust, bool p_set) {
	ERR_FAIL_COND_MSG(!prefer_raster_effects, "Can't use raster version of luminance reduction with the clustered renderer.");
	ERR_FAIL_COND_MSG(p_reduce.size() != p_fb.size(), "Incorrect frame buffer account for luminance reduction.");

	luminance_reduce_raster.push_constant.max_luminance = p_max_luminance;
	luminance_reduce_raster.push_constant.min_luminance = p_min_luminance;
	luminance_reduce_raster.push_constant.exposure_adjust = p_adjust;

	for (int i = 0; i < p_reduce.size(); i++) {
		luminance_reduce_raster.push_constant.source_size[0] = i == 0 ? p_source_size.x : luminance_reduce_raster.push_constant.dest_size[0];
		luminance_reduce_raster.push_constant.source_size[1] = i == 0 ? p_source_size.y : luminance_reduce_raster.push_constant.dest_size[1];
		luminance_reduce_raster.push_constant.dest_size[0] = MAX(luminance_reduce_raster.push_constant.source_size[0] / 8, 1);
		luminance_reduce_raster.push_constant.dest_size[1] = MAX(luminance_reduce_raster.push_constant.source_size[1] / 8, 1);

		bool final = !p_set && (luminance_reduce_raster.push_constant.dest_size[0] == 1) && (luminance_reduce_raster.push_constant.dest_size[1] == 1);
		LuminanceReduceRasterMode mode = final ? LUMINANCE_REDUCE_FRAGMENT_FINAL : (i == 0 ? LUMINANCE_REDUCE_FRAGMENT_FIRST : LUMINANCE_REDUCE_FRAGMENT);

		RD::DrawListID draw_list = RD::get_singleton()->draw_list_begin(p_fb[i], RD::INITIAL_ACTION_KEEP, RD::FINAL_ACTION_READ, RD::INITIAL_ACTION_KEEP, RD::FINAL_ACTION_DISCARD);
		RD::get_singleton()->draw_list_bind_render_pipeline(draw_list, luminance_reduce_raster.pipelines[mode].get_render_pipeline(RD::INVALID_ID, RD::get_singleton()->framebuffer_get_format(p_fb[i])));
		RD::get_singleton()->draw_list_bind_uniform_set(draw_list, _get_uniform_set_from_texture(i == 0 ? p_source_texture : p_reduce[i - 1]), 0);
		if (final) {
			RD::get_singleton()->draw_list_bind_uniform_set(draw_list, _get_uniform_set_from_texture(p_prev_luminance), 1);
		}
		RD::get_singleton()->draw_list_bind_index_array(draw_list, index_array);

		RD::get_singleton()->draw_list_set_push_constant(draw_list, &luminance_reduce_raster.push_constant, sizeof(LuminanceReduceRasterPushConstant));

		RD::get_singleton()->draw_list_draw(draw_list, true);
		RD::get_singleton()->draw_list_end();
	}
}

void EffectsRD::downsample_depth(RID p_depth_buffer, const Vector<RID> &p_depth_mipmaps, RS::EnvironmentSSAOQuality p_ssao_quality, RS::EnvironmentSSILQuality p_ssil_quality, bool p_invalidate_uniform_set, bool p_ssao_half_size, bool p_ssil_half_size, Size2i p_full_screen_size, const CameraMatrix &p_projection) {
	// Downsample and deinterleave the depth buffer for SSAO and SSIL
	RD::ComputeListID compute_list = RD::get_singleton()->compute_list_begin();

	int downsample_pipeline = SS_EFFECTS_DOWNSAMPLE;
	bool use_mips = p_ssao_quality > RS::ENV_SSAO_QUALITY_MEDIUM || p_ssil_quality > RS::ENV_SSIL_QUALITY_MEDIUM;

	if (p_ssao_quality == RS::ENV_SSAO_QUALITY_VERY_LOW && p_ssil_quality == RS::ENV_SSIL_QUALITY_VERY_LOW) {
		downsample_pipeline = SS_EFFECTS_DOWNSAMPLE_HALF;
	} else if (use_mips) {
		downsample_pipeline = SS_EFFECTS_DOWNSAMPLE_MIPMAP;
	}

	bool use_half_size = false;
	bool use_full_mips = false;

	if (p_ssao_half_size && p_ssil_half_size) {
		downsample_pipeline++;
		use_half_size = true;
	} else if (p_ssao_half_size != p_ssil_half_size) {
		if (use_mips) {
			downsample_pipeline = SS_EFFECTS_DOWNSAMPLE_FULL_MIPS;
			use_full_mips = true;
		} else {
			// Only need the first two mipmaps, but the cost to generate the next two is trivial
			// TODO investigate the benefit of a shader version to generate only 2 mips
			downsample_pipeline = SS_EFFECTS_DOWNSAMPLE_MIPMAP;
			use_mips = true;
		}
	}

	int depth_index = use_half_size ? 1 : 0;

	RD::get_singleton()->draw_command_begin_label("Downsample Depth");
	if (p_invalidate_uniform_set || use_full_mips != ss_effects.used_full_mips_last_frame || use_half_size != ss_effects.used_half_size_last_frame || use_mips != ss_effects.used_mips_last_frame) {
		Vector<RD::Uniform> uniforms;
		{
			RD::Uniform u;
			u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
			u.binding = 0;
			u.append_id(p_depth_mipmaps[depth_index + 1]);
			uniforms.push_back(u);
		}
		{
			RD::Uniform u;
			u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
			u.binding = 1;
			u.append_id(p_depth_mipmaps[depth_index + 2]);
			uniforms.push_back(u);
		}
		{
			RD::Uniform u;
			u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
			u.binding = 2;
			u.append_id(p_depth_mipmaps[depth_index + 3]);
			uniforms.push_back(u);
		}
		if (use_full_mips) {
			RD::Uniform u;
			u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
			u.binding = 3;
			u.append_id(p_depth_mipmaps[4]);
			uniforms.push_back(u);
		}
		ss_effects.downsample_uniform_set = RD::get_singleton()->uniform_set_create(uniforms, ss_effects.downsample_shader.version_get_shader(ss_effects.downsample_shader_version, use_full_mips ? 6 : 2), 2);
	}

	float depth_linearize_mul = -p_projection.matrix[3][2];
	float depth_linearize_add = p_projection.matrix[2][2];
	if (depth_linearize_mul * depth_linearize_add < 0) {
		depth_linearize_add = -depth_linearize_add;
	}

	ss_effects.downsample_push_constant.orthogonal = p_projection.is_orthogonal();
	ss_effects.downsample_push_constant.z_near = depth_linearize_mul;
	ss_effects.downsample_push_constant.z_far = depth_linearize_add;
	if (ss_effects.downsample_push_constant.orthogonal) {
		ss_effects.downsample_push_constant.z_near = p_projection.get_z_near();
		ss_effects.downsample_push_constant.z_far = p_projection.get_z_far();
	}
	ss_effects.downsample_push_constant.pixel_size[0] = 1.0 / p_full_screen_size.x;
	ss_effects.downsample_push_constant.pixel_size[1] = 1.0 / p_full_screen_size.y;
	ss_effects.downsample_push_constant.radius_sq = 1.0;

	RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, ss_effects.pipelines[downsample_pipeline]);
	RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_texture(p_depth_buffer), 0);
	RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_uniform_set_from_image(p_depth_mipmaps[depth_index + 0]), 1);
	if (use_mips) {
		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, ss_effects.downsample_uniform_set, 2);
	}
	RD::get_singleton()->compute_list_set_push_constant(compute_list, &ss_effects.downsample_push_constant, sizeof(SSEffectsDownsamplePushConstant));

	Size2i size(MAX(1, p_full_screen_size.x >> (use_half_size ? 2 : 1)), MAX(1, p_full_screen_size.y >> (use_half_size ? 2 : 1)));

	RD::get_singleton()->compute_list_dispatch_threads(compute_list, size.x, size.y, 1);
	RD::get_singleton()->compute_list_add_barrier(compute_list);
	RD::get_singleton()->draw_command_end_label();

	RD::get_singleton()->compute_list_end(RD::BARRIER_MASK_COMPUTE);

	ss_effects.used_full_mips_last_frame = use_full_mips;
	ss_effects.used_half_size_last_frame = use_half_size;
}

void EffectsRD::gather_ssao(RD::ComputeListID p_compute_list, const Vector<RID> p_ao_slices, const SSAOSettings &p_settings, bool p_adaptive_base_pass, RID p_gather_uniform_set, RID p_importance_map_uniform_set) {
	RD::get_singleton()->compute_list_bind_uniform_set(p_compute_list, p_gather_uniform_set, 0);
	if ((p_settings.quality == RS::ENV_SSAO_QUALITY_ULTRA) && !p_adaptive_base_pass) {
		RD::get_singleton()->compute_list_bind_uniform_set(p_compute_list, p_importance_map_uniform_set, 1);
	}

	for (int i = 0; i < 4; i++) {
		if ((p_settings.quality == RS::ENV_SSAO_QUALITY_VERY_LOW) && ((i == 1) || (i == 2))) {
			continue;
		}

		ssao.gather_push_constant.pass_coord_offset[0] = i % 2;
		ssao.gather_push_constant.pass_coord_offset[1] = i / 2;
		ssao.gather_push_constant.pass_uv_offset[0] = ((i % 2) - 0.0) / p_settings.full_screen_size.x;
		ssao.gather_push_constant.pass_uv_offset[1] = ((i / 2) - 0.0) / p_settings.full_screen_size.y;
		ssao.gather_push_constant.pass = i;
		RD::get_singleton()->compute_list_bind_uniform_set(p_compute_list, _get_uniform_set_from_image(p_ao_slices[i]), 2);
		RD::get_singleton()->compute_list_set_push_constant(p_compute_list, &ssao.gather_push_constant, sizeof(SSAOGatherPushConstant));

		Size2i size = Size2i(p_settings.full_screen_size.x >> (p_settings.half_size ? 2 : 1), p_settings.full_screen_size.y >> (p_settings.half_size ? 2 : 1));

		RD::get_singleton()->compute_list_dispatch_threads(p_compute_list, size.x, size.y, 1);
	}
	RD::get_singleton()->compute_list_add_barrier(p_compute_list);
}

void EffectsRD::generate_ssao(RID p_normal_buffer, RID p_depth_mipmaps_texture, RID p_ao, const Vector<RID> p_ao_slices, RID p_ao_pong, const Vector<RID> p_ao_pong_slices, RID p_upscale_buffer, RID p_importance_map, RID p_importance_map_pong, const CameraMatrix &p_projection, const SSAOSettings &p_settings, bool p_invalidate_uniform_sets, RID &r_gather_uniform_set, RID &r_importance_map_uniform_set) {
	RD::ComputeListID compute_list = RD::get_singleton()->compute_list_begin();
	memset(&ssao.gather_push_constant, 0, sizeof(SSAOGatherPushConstant));
	/* FIRST PASS */

	RD::get_singleton()->draw_command_begin_label("Process Screen Space Ambient Occlusion");
	/* SECOND PASS */
	// Sample SSAO
	{
		RD::get_singleton()->draw_command_begin_label("Gather Samples");
		ssao.gather_push_constant.screen_size[0] = p_settings.full_screen_size.x;
		ssao.gather_push_constant.screen_size[1] = p_settings.full_screen_size.y;

		ssao.gather_push_constant.half_screen_pixel_size[0] = 1.0 / p_settings.half_screen_size.x;
		ssao.gather_push_constant.half_screen_pixel_size[1] = 1.0 / p_settings.half_screen_size.y;
		float tan_half_fov_x = 1.0 / p_projection.matrix[0][0];
		float tan_half_fov_y = 1.0 / p_projection.matrix[1][1];
		ssao.gather_push_constant.NDC_to_view_mul[0] = tan_half_fov_x * 2.0;
		ssao.gather_push_constant.NDC_to_view_mul[1] = tan_half_fov_y * -2.0;
		ssao.gather_push_constant.NDC_to_view_add[0] = tan_half_fov_x * -1.0;
		ssao.gather_push_constant.NDC_to_view_add[1] = tan_half_fov_y;
		ssao.gather_push_constant.is_orthogonal = p_projection.is_orthogonal();

		ssao.gather_push_constant.half_screen_pixel_size_x025[0] = ssao.gather_push_constant.half_screen_pixel_size[0] * 0.25;
		ssao.gather_push_constant.half_screen_pixel_size_x025[1] = ssao.gather_push_constant.half_screen_pixel_size[1] * 0.25;

		ssao.gather_push_constant.radius = p_settings.radius;
		float radius_near_limit = (p_settings.radius * 1.2f);
		if (p_settings.quality <= RS::ENV_SSAO_QUALITY_LOW) {
			radius_near_limit *= 1.50f;

			if (p_settings.quality == RS::ENV_SSAO_QUALITY_VERY_LOW) {
				ssao.gather_push_constant.radius *= 0.8f;
			}
		}
		radius_near_limit /= tan_half_fov_y;
		ssao.gather_push_constant.intensity = p_settings.intensity;
		ssao.gather_push_constant.shadow_power = p_settings.power;
		ssao.gather_push_constant.shadow_clamp = 0.98;
		ssao.gather_push_constant.fade_out_mul = -1.0 / (p_settings.fadeout_to - p_settings.fadeout_from);
		ssao.gather_push_constant.fade_out_add = p_settings.fadeout_from / (p_settings.fadeout_to - p_settings.fadeout_from) + 1.0;
		ssao.gather_push_constant.horizon_angle_threshold = p_settings.horizon;
		ssao.gather_push_constant.inv_radius_near_limit = 1.0f / radius_near_limit;
		ssao.gather_push_constant.neg_inv_radius = -1.0 / ssao.gather_push_constant.radius;

		ssao.gather_push_constant.load_counter_avg_div = 9.0 / float((p_settings.quarter_screen_size.x) * (p_settings.quarter_screen_size.y) * 255);
		ssao.gather_push_constant.adaptive_sample_limit = p_settings.adaptive_target;

		ssao.gather_push_constant.detail_intensity = p_settings.detail;
		ssao.gather_push_constant.quality = MAX(0, p_settings.quality - 1);
		ssao.gather_push_constant.size_multiplier = p_settings.half_size ? 2 : 1;

		if (p_invalidate_uniform_sets) {
			Vector<RD::Uniform> uniforms;
			{
				RD::Uniform u;
				u.uniform_type = RD::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE;
				u.binding = 0;
				u.append_id(default_sampler);
				u.append_id(p_depth_mipmaps_texture);
				uniforms.push_back(u);
			}
			{
				RD::Uniform u;
				u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
				u.binding = 1;
				u.append_id(p_normal_buffer);
				uniforms.push_back(u);
			}
			{
				RD::Uniform u;
				u.uniform_type = RD::UNIFORM_TYPE_UNIFORM_BUFFER;
				u.binding = 2;
				u.append_id(ss_effects.gather_constants_buffer);
				uniforms.push_back(u);
			}
			r_gather_uniform_set = RD::get_singleton()->uniform_set_create(uniforms, ssao.gather_shader.version_get_shader(ssao.gather_shader_version, 0), 0);
		}

		if (p_invalidate_uniform_sets) {
			Vector<RD::Uniform> uniforms;
			{
				RD::Uniform u;
				u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
				u.binding = 0;
				u.append_id(p_ao_pong);
				uniforms.push_back(u);
			}
			{
				RD::Uniform u;
				u.uniform_type = RD::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE;
				u.binding = 1;
				u.append_id(default_sampler);
				u.append_id(p_importance_map);
				uniforms.push_back(u);
			}
			{
				RD::Uniform u;
				u.uniform_type = RD::UNIFORM_TYPE_STORAGE_BUFFER;
				u.binding = 2;
				u.append_id(ssao.importance_map_load_counter);
				uniforms.push_back(u);
			}
			r_importance_map_uniform_set = RD::get_singleton()->uniform_set_create(uniforms, ssao.gather_shader.version_get_shader(ssao.gather_shader_version, 2), 1);
		}

		if (p_settings.quality == RS::ENV_SSAO_QUALITY_ULTRA) {
			RD::get_singleton()->draw_command_begin_label("Generate Importance Map");
			ssao.importance_map_push_constant.half_screen_pixel_size[0] = 1.0 / p_settings.half_screen_size.x;
			ssao.importance_map_push_constant.half_screen_pixel_size[1] = 1.0 / p_settings.half_screen_size.y;
			ssao.importance_map_push_constant.intensity = p_settings.intensity;
			ssao.importance_map_push_constant.power = p_settings.power;
			//base pass
			RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, ssao.pipelines[SSAO_GATHER_BASE]);
			gather_ssao(compute_list, p_ao_pong_slices, p_settings, true, r_gather_uniform_set, RID());
			//generate importance map

			RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, ssao.pipelines[SSAO_GENERATE_IMPORTANCE_MAP]);
			RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_texture(p_ao_pong), 0);
			RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_uniform_set_from_image(p_importance_map), 1);
			RD::get_singleton()->compute_list_set_push_constant(compute_list, &ssao.importance_map_push_constant, sizeof(SSAOImportanceMapPushConstant));
			RD::get_singleton()->compute_list_dispatch_threads(compute_list, p_settings.quarter_screen_size.x, p_settings.quarter_screen_size.y, 1);
			RD::get_singleton()->compute_list_add_barrier(compute_list);
			//process importance map A
			RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, ssao.pipelines[SSAO_PROCESS_IMPORTANCE_MAPA]);
			RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_texture(p_importance_map), 0);
			RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_uniform_set_from_image(p_importance_map_pong), 1);
			RD::get_singleton()->compute_list_set_push_constant(compute_list, &ssao.importance_map_push_constant, sizeof(SSAOImportanceMapPushConstant));
			RD::get_singleton()->compute_list_dispatch_threads(compute_list, p_settings.quarter_screen_size.x, p_settings.quarter_screen_size.y, 1);
			RD::get_singleton()->compute_list_add_barrier(compute_list);
			//process Importance Map B
			RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, ssao.pipelines[SSAO_PROCESS_IMPORTANCE_MAPB]);
			RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_texture(p_importance_map_pong), 0);
			RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_uniform_set_from_image(p_importance_map), 1);
			RD::get_singleton()->compute_list_bind_uniform_set(compute_list, ssao.counter_uniform_set, 2);
			RD::get_singleton()->compute_list_set_push_constant(compute_list, &ssao.importance_map_push_constant, sizeof(SSAOImportanceMapPushConstant));
			RD::get_singleton()->compute_list_dispatch_threads(compute_list, p_settings.quarter_screen_size.x, p_settings.quarter_screen_size.y, 1);
			RD::get_singleton()->compute_list_add_barrier(compute_list);

			RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, ssao.pipelines[SSAO_GATHER_ADAPTIVE]);
			RD::get_singleton()->draw_command_end_label(); // Importance Map
		} else {
			RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, ssao.pipelines[SSAO_GATHER]);
		}

		gather_ssao(compute_list, p_ao_slices, p_settings, false, r_gather_uniform_set, r_importance_map_uniform_set);
		RD::get_singleton()->draw_command_end_label(); // Gather SSAO
	}

	//	/* THIRD PASS */
	//	// Blur
	//
	{
		RD::get_singleton()->draw_command_begin_label("Edge Aware Blur");
		ssao.blur_push_constant.edge_sharpness = 1.0 - p_settings.sharpness;
		ssao.blur_push_constant.half_screen_pixel_size[0] = 1.0 / p_settings.half_screen_size.x;
		ssao.blur_push_constant.half_screen_pixel_size[1] = 1.0 / p_settings.half_screen_size.y;

		int blur_passes = p_settings.quality > RS::ENV_SSAO_QUALITY_VERY_LOW ? p_settings.blur_passes : 1;

		for (int pass = 0; pass < blur_passes; pass++) {
			int blur_pipeline = SSAO_BLUR_PASS;
			if (p_settings.quality > RS::ENV_SSAO_QUALITY_VERY_LOW) {
				blur_pipeline = SSAO_BLUR_PASS_SMART;
				if (pass < blur_passes - 2) {
					blur_pipeline = SSAO_BLUR_PASS_WIDE;
				} else {
					blur_pipeline = SSAO_BLUR_PASS_SMART;
				}
			}

			for (int i = 0; i < 4; i++) {
				if ((p_settings.quality == RS::ENV_SSAO_QUALITY_VERY_LOW) && ((i == 1) || (i == 2))) {
					continue;
				}

				RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, ssao.pipelines[blur_pipeline]);
				if (pass % 2 == 0) {
					if (p_settings.quality == RS::ENV_SSAO_QUALITY_VERY_LOW) {
						RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_texture(p_ao_slices[i]), 0);
					} else {
						RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_texture_and_sampler(p_ao_slices[i], ss_effects.mirror_sampler), 0);
					}

					RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_uniform_set_from_image(p_ao_pong_slices[i]), 1);
				} else {
					if (p_settings.quality == RS::ENV_SSAO_QUALITY_VERY_LOW) {
						RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_texture(p_ao_pong_slices[i]), 0);
					} else {
						RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_texture_and_sampler(p_ao_pong_slices[i], ss_effects.mirror_sampler), 0);
					}
					RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_uniform_set_from_image(p_ao_slices[i]), 1);
				}
				RD::get_singleton()->compute_list_set_push_constant(compute_list, &ssao.blur_push_constant, sizeof(SSAOBlurPushConstant));

				Size2i size(p_settings.full_screen_size.x >> (p_settings.half_size ? 2 : 1), p_settings.full_screen_size.y >> (p_settings.half_size ? 2 : 1));
				RD::get_singleton()->compute_list_dispatch_threads(compute_list, size.x, size.y, 1);
			}

			if (p_settings.quality > RS::ENV_SSAO_QUALITY_VERY_LOW) {
				RD::get_singleton()->compute_list_add_barrier(compute_list);
			}
		}
		RD::get_singleton()->draw_command_end_label(); // Blur
	}

	/* FOURTH PASS */
	// Interleave buffers
	// back to full size
	{
		RD::get_singleton()->draw_command_begin_label("Interleave Buffers");
		ssao.interleave_push_constant.inv_sharpness = 1.0 - p_settings.sharpness;
		ssao.interleave_push_constant.pixel_size[0] = 1.0 / p_settings.full_screen_size.x;
		ssao.interleave_push_constant.pixel_size[1] = 1.0 / p_settings.full_screen_size.y;
		ssao.interleave_push_constant.size_modifier = uint32_t(p_settings.half_size ? 4 : 2);

		int interleave_pipeline = SSAO_INTERLEAVE_HALF;
		if (p_settings.quality == RS::ENV_SSAO_QUALITY_LOW) {
			interleave_pipeline = SSAO_INTERLEAVE;
		} else if (p_settings.quality >= RS::ENV_SSAO_QUALITY_MEDIUM) {
			interleave_pipeline = SSAO_INTERLEAVE_SMART;
		}

		RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, ssao.pipelines[interleave_pipeline]);

		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_uniform_set_from_image(p_upscale_buffer), 0);
		if (p_settings.quality > RS::ENV_SSAO_QUALITY_VERY_LOW && p_settings.blur_passes % 2 == 0) {
			RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_texture(p_ao), 1);
		} else {
			RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_texture(p_ao_pong), 1);
		}

		RD::get_singleton()->compute_list_set_push_constant(compute_list, &ssao.interleave_push_constant, sizeof(SSAOInterleavePushConstant));

		RD::get_singleton()->compute_list_dispatch_threads(compute_list, p_settings.full_screen_size.x, p_settings.full_screen_size.y, 1);
		RD::get_singleton()->compute_list_add_barrier(compute_list);
		RD::get_singleton()->draw_command_end_label(); // Interleave
	}
	RD::get_singleton()->draw_command_end_label(); //SSAO
	RD::get_singleton()->compute_list_end(RD::BARRIER_MASK_NO_BARRIER); //wait for upcoming transfer

	int zero[1] = { 0 };
	RD::get_singleton()->buffer_update(ssao.importance_map_load_counter, 0, sizeof(uint32_t), &zero, 0); //no barrier
}

void EffectsRD::gather_ssil(RD::ComputeListID p_compute_list, const Vector<RID> p_ssil_slices, const Vector<RID> p_edges_slices, const SSILSettings &p_settings, bool p_adaptive_base_pass, RID p_gather_uniform_set, RID p_importance_map_uniform_set, RID p_projection_uniform_set) {
	RD::get_singleton()->compute_list_bind_uniform_set(p_compute_list, p_gather_uniform_set, 0);
	if ((p_settings.quality == RS::ENV_SSIL_QUALITY_ULTRA) && !p_adaptive_base_pass) {
		RD::get_singleton()->compute_list_bind_uniform_set(p_compute_list, p_importance_map_uniform_set, 1);
	}
	RD::get_singleton()->compute_list_bind_uniform_set(p_compute_list, p_projection_uniform_set, 3);

	for (int i = 0; i < 4; i++) {
		if ((p_settings.quality == RS::ENV_SSIL_QUALITY_VERY_LOW) && ((i == 1) || (i == 2))) {
			continue;
		}

		ssil.gather_push_constant.pass_coord_offset[0] = i % 2;
		ssil.gather_push_constant.pass_coord_offset[1] = i / 2;
		ssil.gather_push_constant.pass_uv_offset[0] = ((i % 2) - 0.0) / p_settings.full_screen_size.x;
		ssil.gather_push_constant.pass_uv_offset[1] = ((i / 2) - 0.0) / p_settings.full_screen_size.y;
		ssil.gather_push_constant.pass = i;
		RD::get_singleton()->compute_list_bind_uniform_set(p_compute_list, _get_compute_uniform_set_from_image_pair(p_ssil_slices[i], p_edges_slices[i]), 2);
		RD::get_singleton()->compute_list_set_push_constant(p_compute_list, &ssil.gather_push_constant, sizeof(SSILGatherPushConstant));

		Size2i size = Size2i(p_settings.full_screen_size.x >> (p_settings.half_size ? 2 : 1), p_settings.full_screen_size.y >> (p_settings.half_size ? 2 : 1));

		RD::get_singleton()->compute_list_dispatch_threads(p_compute_list, size.x, size.y, 1);
	}
	RD::get_singleton()->compute_list_add_barrier(p_compute_list);
}

void EffectsRD::screen_space_indirect_lighting(RID p_diffuse, RID p_destination, RID p_normal_buffer, RID p_depth_mipmaps_texture, RID p_ssil, const Vector<RID> p_ssil_slices, RID p_ssil_pong, const Vector<RID> p_ssil_pong_slices, RID p_importance_map, RID p_importance_map_pong, RID p_edges, const Vector<RID> p_edges_slices, const CameraMatrix &p_projection, const CameraMatrix &p_last_projection, const SSILSettings &p_settings, bool p_invalidate_uniform_sets, RID &r_gather_uniform_set, RID &r_importance_map_uniform_set, RID &r_projection_uniform_set) {
	RD::get_singleton()->draw_command_begin_label("Process Screen Space Indirect Lighting");
	//Store projection info before starting the compute list
	SSILProjectionUniforms projection_uniforms;
	store_camera(p_last_projection, projection_uniforms.inv_last_frame_projection_matrix);

	RD::get_singleton()->buffer_update(ssil.projection_uniform_buffer, 0, sizeof(SSILProjectionUniforms), &projection_uniforms);

	memset(&ssil.gather_push_constant, 0, sizeof(SSILGatherPushConstant));

	RD::ComputeListID compute_list = RD::get_singleton()->compute_list_begin();
	{
		RD::get_singleton()->draw_command_begin_label("Gather Samples");
		ssil.gather_push_constant.screen_size[0] = p_settings.full_screen_size.x;
		ssil.gather_push_constant.screen_size[1] = p_settings.full_screen_size.y;

		ssil.gather_push_constant.half_screen_pixel_size[0] = 1.0 / p_settings.half_screen_size.x;
		ssil.gather_push_constant.half_screen_pixel_size[1] = 1.0 / p_settings.half_screen_size.y;
		float tan_half_fov_x = 1.0 / p_projection.matrix[0][0];
		float tan_half_fov_y = 1.0 / p_projection.matrix[1][1];
		ssil.gather_push_constant.NDC_to_view_mul[0] = tan_half_fov_x * 2.0;
		ssil.gather_push_constant.NDC_to_view_mul[1] = tan_half_fov_y * -2.0;
		ssil.gather_push_constant.NDC_to_view_add[0] = tan_half_fov_x * -1.0;
		ssil.gather_push_constant.NDC_to_view_add[1] = tan_half_fov_y;
		ssil.gather_push_constant.z_near = p_projection.get_z_near();
		ssil.gather_push_constant.z_far = p_projection.get_z_far();
		ssil.gather_push_constant.is_orthogonal = p_projection.is_orthogonal();

		ssil.gather_push_constant.half_screen_pixel_size_x025[0] = ssil.gather_push_constant.half_screen_pixel_size[0] * 0.25;
		ssil.gather_push_constant.half_screen_pixel_size_x025[1] = ssil.gather_push_constant.half_screen_pixel_size[1] * 0.25;

		ssil.gather_push_constant.radius = p_settings.radius;
		float radius_near_limit = (p_settings.radius * 1.2f);
		if (p_settings.quality <= RS::ENV_SSIL_QUALITY_LOW) {
			radius_near_limit *= 1.50f;

			if (p_settings.quality == RS::ENV_SSIL_QUALITY_VERY_LOW) {
				ssil.gather_push_constant.radius *= 0.8f;
			}
		}
		radius_near_limit /= tan_half_fov_y;
		ssil.gather_push_constant.intensity = p_settings.intensity * Math_PI;
		ssil.gather_push_constant.fade_out_mul = -1.0 / (p_settings.fadeout_to - p_settings.fadeout_from);
		ssil.gather_push_constant.fade_out_add = p_settings.fadeout_from / (p_settings.fadeout_to - p_settings.fadeout_from) + 1.0;
		ssil.gather_push_constant.inv_radius_near_limit = 1.0f / radius_near_limit;
		ssil.gather_push_constant.neg_inv_radius = -1.0 / ssil.gather_push_constant.radius;
		ssil.gather_push_constant.normal_rejection_amount = p_settings.normal_rejection;

		ssil.gather_push_constant.load_counter_avg_div = 9.0 / float((p_settings.quarter_screen_size.x) * (p_settings.quarter_screen_size.y) * 255);
		ssil.gather_push_constant.adaptive_sample_limit = p_settings.adaptive_target;

		ssil.gather_push_constant.quality = MAX(0, p_settings.quality - 1);
		ssil.gather_push_constant.size_multiplier = p_settings.half_size ? 2 : 1;

		if (p_invalidate_uniform_sets) {
			Vector<RD::Uniform> uniforms;
			{
				RD::Uniform u;
				u.uniform_type = RD::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE;
				u.binding = 0;
				u.append_id(default_mipmap_sampler);
				u.append_id(p_diffuse);
				uniforms.push_back(u);
			}
			{
				RD::Uniform u;
				u.uniform_type = RD::UNIFORM_TYPE_UNIFORM_BUFFER;
				u.binding = 1;
				u.append_id(ssil.projection_uniform_buffer);
				uniforms.push_back(u);
			}
			r_projection_uniform_set = RD::get_singleton()->uniform_set_create(uniforms, ssil.gather_shader.version_get_shader(ssil.gather_shader_version, 0), 3);
		}

		if (p_invalidate_uniform_sets) {
			Vector<RD::Uniform> uniforms;
			{
				RD::Uniform u;
				u.uniform_type = RD::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE;
				u.binding = 0;
				u.append_id(default_sampler);
				u.append_id(p_depth_mipmaps_texture);
				uniforms.push_back(u);
			}
			{
				RD::Uniform u;
				u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
				u.binding = 1;
				u.append_id(p_normal_buffer);
				uniforms.push_back(u);
			}
			{
				RD::Uniform u;
				u.uniform_type = RD::UNIFORM_TYPE_UNIFORM_BUFFER;
				u.binding = 2;
				u.append_id(ss_effects.gather_constants_buffer);
				uniforms.push_back(u);
			}
			r_gather_uniform_set = RD::get_singleton()->uniform_set_create(uniforms, ssil.gather_shader.version_get_shader(ssil.gather_shader_version, 0), 0);
		}

		if (p_invalidate_uniform_sets) {
			Vector<RD::Uniform> uniforms;
			{
				RD::Uniform u;
				u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
				u.binding = 0;
				u.append_id(p_ssil_pong);
				uniforms.push_back(u);
			}
			{
				RD::Uniform u;
				u.uniform_type = RD::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE;
				u.binding = 1;
				u.append_id(default_sampler);
				u.append_id(p_importance_map);
				uniforms.push_back(u);
			}
			{
				RD::Uniform u;
				u.uniform_type = RD::UNIFORM_TYPE_STORAGE_BUFFER;
				u.binding = 2;
				u.append_id(ssil.importance_map_load_counter);
				uniforms.push_back(u);
			}
			r_importance_map_uniform_set = RD::get_singleton()->uniform_set_create(uniforms, ssil.gather_shader.version_get_shader(ssil.gather_shader_version, 2), 1);
		}

		if (p_settings.quality == RS::ENV_SSIL_QUALITY_ULTRA) {
			RD::get_singleton()->draw_command_begin_label("Generate Importance Map");
			ssil.importance_map_push_constant.half_screen_pixel_size[0] = 1.0 / p_settings.half_screen_size.x;
			ssil.importance_map_push_constant.half_screen_pixel_size[1] = 1.0 / p_settings.half_screen_size.y;
			ssil.importance_map_push_constant.intensity = p_settings.intensity * Math_PI;
			//base pass
			RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, ssil.pipelines[SSIL_GATHER_BASE]);
			gather_ssil(compute_list, p_ssil_pong_slices, p_edges_slices, p_settings, true, r_gather_uniform_set, r_importance_map_uniform_set, r_projection_uniform_set);
			//generate importance map

			RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, ssil.pipelines[SSIL_GENERATE_IMPORTANCE_MAP]);
			RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_texture(p_ssil_pong), 0);
			RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_uniform_set_from_image(p_importance_map), 1);
			RD::get_singleton()->compute_list_set_push_constant(compute_list, &ssil.importance_map_push_constant, sizeof(SSILImportanceMapPushConstant));
			RD::get_singleton()->compute_list_dispatch_threads(compute_list, p_settings.quarter_screen_size.x, p_settings.quarter_screen_size.y, 1);
			RD::get_singleton()->compute_list_add_barrier(compute_list);
			// process Importance Map A
			RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, ssil.pipelines[SSIL_PROCESS_IMPORTANCE_MAPA]);
			RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_texture(p_importance_map), 0);
			RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_uniform_set_from_image(p_importance_map_pong), 1);
			RD::get_singleton()->compute_list_set_push_constant(compute_list, &ssil.importance_map_push_constant, sizeof(SSILImportanceMapPushConstant));
			RD::get_singleton()->compute_list_dispatch_threads(compute_list, p_settings.quarter_screen_size.x, p_settings.quarter_screen_size.y, 1);
			RD::get_singleton()->compute_list_add_barrier(compute_list);
			// process Importance Map B
			RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, ssil.pipelines[SSIL_PROCESS_IMPORTANCE_MAPB]);
			RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_texture(p_importance_map_pong), 0);
			RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_uniform_set_from_image(p_importance_map), 1);
			RD::get_singleton()->compute_list_bind_uniform_set(compute_list, ssil.counter_uniform_set, 2);
			RD::get_singleton()->compute_list_set_push_constant(compute_list, &ssil.importance_map_push_constant, sizeof(SSILImportanceMapPushConstant));
			RD::get_singleton()->compute_list_dispatch_threads(compute_list, p_settings.quarter_screen_size.x, p_settings.quarter_screen_size.y, 1);
			RD::get_singleton()->compute_list_add_barrier(compute_list);

			RD::get_singleton()->draw_command_end_label(); // Importance Map

			RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, ssil.pipelines[SSIL_GATHER_ADAPTIVE]);
		} else {
			RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, ssil.pipelines[SSIL_GATHER]);
		}

		gather_ssil(compute_list, p_ssil_slices, p_edges_slices, p_settings, false, r_gather_uniform_set, r_importance_map_uniform_set, r_projection_uniform_set);
		RD::get_singleton()->draw_command_end_label(); //Gather
	}

	{
		RD::get_singleton()->draw_command_begin_label("Edge Aware Blur");
		ssil.blur_push_constant.edge_sharpness = 1.0 - p_settings.sharpness;
		ssil.blur_push_constant.half_screen_pixel_size[0] = 1.0 / p_settings.half_screen_size.x;
		ssil.blur_push_constant.half_screen_pixel_size[1] = 1.0 / p_settings.half_screen_size.y;

		int blur_passes = p_settings.quality > RS::ENV_SSIL_QUALITY_VERY_LOW ? p_settings.blur_passes : 1;

		for (int pass = 0; pass < blur_passes; pass++) {
			int blur_pipeline = SSIL_BLUR_PASS;
			if (p_settings.quality > RS::ENV_SSIL_QUALITY_VERY_LOW) {
				blur_pipeline = SSIL_BLUR_PASS_SMART;
				if (pass < blur_passes - 2) {
					blur_pipeline = SSIL_BLUR_PASS_WIDE;
				}
			}

			for (int i = 0; i < 4; i++) {
				if ((p_settings.quality == RS::ENV_SSIL_QUALITY_VERY_LOW) && ((i == 1) || (i == 2))) {
					continue;
				}

				RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, ssil.pipelines[blur_pipeline]);
				if (pass % 2 == 0) {
					if (p_settings.quality == RS::ENV_SSIL_QUALITY_VERY_LOW) {
						RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_texture(p_ssil_slices[i]), 0);
					} else {
						RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_texture_and_sampler(p_ssil_slices[i], ss_effects.mirror_sampler), 0);
					}
					RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_uniform_set_from_image(p_ssil_pong_slices[i]), 1);
				} else {
					if (p_settings.quality == RS::ENV_SSIL_QUALITY_VERY_LOW) {
						RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_texture(p_ssil_pong_slices[i]), 0);
					} else {
						RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_texture_and_sampler(p_ssil_pong_slices[i], ss_effects.mirror_sampler), 0);
					}
					RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_uniform_set_from_image(p_ssil_slices[i]), 1);
				}
				RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_uniform_set_from_image(p_edges_slices[i]), 2);

				RD::get_singleton()->compute_list_set_push_constant(compute_list, &ssil.blur_push_constant, sizeof(SSILBlurPushConstant));

				int x_groups = (p_settings.full_screen_size.x >> (p_settings.half_size ? 2 : 1));
				int y_groups = (p_settings.full_screen_size.y >> (p_settings.half_size ? 2 : 1));

				RD::get_singleton()->compute_list_dispatch_threads(compute_list, x_groups, y_groups, 1);
				if (p_settings.quality > RS::ENV_SSIL_QUALITY_VERY_LOW) {
					RD::get_singleton()->compute_list_add_barrier(compute_list);
				}
			}
		}

		RD::get_singleton()->draw_command_end_label(); // Blur
	}

	{
		RD::get_singleton()->draw_command_begin_label("Interleave Buffers");
		ssil.interleave_push_constant.inv_sharpness = 1.0 - p_settings.sharpness;
		ssil.interleave_push_constant.pixel_size[0] = 1.0 / p_settings.full_screen_size.x;
		ssil.interleave_push_constant.pixel_size[1] = 1.0 / p_settings.full_screen_size.y;
		ssil.interleave_push_constant.size_modifier = uint32_t(p_settings.half_size ? 4 : 2);

		int interleave_pipeline = SSIL_INTERLEAVE_HALF;
		if (p_settings.quality == RS::ENV_SSIL_QUALITY_LOW) {
			interleave_pipeline = SSIL_INTERLEAVE;
		} else if (p_settings.quality >= RS::ENV_SSIL_QUALITY_MEDIUM) {
			interleave_pipeline = SSIL_INTERLEAVE_SMART;
		}

		RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, ssil.pipelines[interleave_pipeline]);

		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_uniform_set_from_image(p_destination), 0);

		if (p_settings.quality > RS::ENV_SSIL_QUALITY_VERY_LOW && p_settings.blur_passes % 2 == 0) {
			RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_texture(p_ssil), 1);
		} else {
			RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_texture(p_ssil_pong), 1);
		}
		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_uniform_set_from_image(p_edges), 2);

		RD::get_singleton()->compute_list_set_push_constant(compute_list, &ssil.interleave_push_constant, sizeof(SSILInterleavePushConstant));

		RD::get_singleton()->compute_list_dispatch_threads(compute_list, p_settings.full_screen_size.x, p_settings.full_screen_size.y, 1);
		RD::get_singleton()->compute_list_add_barrier(compute_list);
		RD::get_singleton()->draw_command_end_label(); // Interleave
	}

	RD::get_singleton()->draw_command_end_label(); // SSIL

	RD::get_singleton()->compute_list_end(RD::BARRIER_MASK_NO_BARRIER);

	int zero[1] = { 0 };
	RD::get_singleton()->buffer_update(ssil.importance_map_load_counter, 0, sizeof(uint32_t), &zero, 0); //no barrier
}

void EffectsRD::roughness_limit(RID p_source_normal, RID p_roughness, const Size2i &p_size, float p_curve) {
	roughness_limiter.push_constant.screen_size[0] = p_size.x;
	roughness_limiter.push_constant.screen_size[1] = p_size.y;
	roughness_limiter.push_constant.curve = p_curve;

	RD::ComputeListID compute_list = RD::get_singleton()->compute_list_begin();
	RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, roughness_limiter.pipeline);
	RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_texture(p_source_normal), 0);
	RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_uniform_set_from_image(p_roughness), 1);

	RD::get_singleton()->compute_list_set_push_constant(compute_list, &roughness_limiter.push_constant, sizeof(RoughnessLimiterPushConstant)); //not used but set anyway

	RD::get_singleton()->compute_list_dispatch_threads(compute_list, p_size.x, p_size.y, 1);

	RD::get_singleton()->compute_list_end();
}

void EffectsRD::cubemap_roughness(RID p_source_rd_texture, RID p_dest_texture, uint32_t p_face_id, uint32_t p_sample_count, float p_roughness, float p_size) {
	ERR_FAIL_COND_MSG(prefer_raster_effects, "Can't use compute based cubemap roughness with the mobile renderer.");

	memset(&roughness.push_constant, 0, sizeof(CubemapRoughnessPushConstant));

	roughness.push_constant.face_id = p_face_id > 9 ? 0 : p_face_id;
	roughness.push_constant.roughness = p_roughness * p_roughness; // Shader expects roughness, not perceptual roughness, so multiply before passing in.
	roughness.push_constant.sample_count = p_sample_count;
	roughness.push_constant.use_direct_write = p_roughness == 0.0;
	roughness.push_constant.face_size = p_size;

	RD::ComputeListID compute_list = RD::get_singleton()->compute_list_begin();
	RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, roughness.compute_pipeline);

	RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_texture(p_source_rd_texture, true), 0);
	RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_uniform_set_from_image(p_dest_texture), 1);

	RD::get_singleton()->compute_list_set_push_constant(compute_list, &roughness.push_constant, sizeof(CubemapRoughnessPushConstant));

	int x_groups = (p_size - 1) / 8 + 1;
	int y_groups = (p_size - 1) / 8 + 1;

	RD::get_singleton()->compute_list_dispatch(compute_list, x_groups, y_groups, p_face_id > 9 ? 6 : 1);

	RD::get_singleton()->compute_list_end();
}

void EffectsRD::cubemap_roughness_raster(RID p_source_rd_texture, RID p_dest_framebuffer, uint32_t p_face_id, uint32_t p_sample_count, float p_roughness, float p_size) {
	ERR_FAIL_COND_MSG(!prefer_raster_effects, "Can't use raster based cubemap roughness with the clustered renderer.");
	ERR_FAIL_COND_MSG(p_face_id >= 6, "Raster implementation of cubemap roughness must process one side at a time.");

	memset(&roughness.push_constant, 0, sizeof(CubemapRoughnessPushConstant));

	roughness.push_constant.face_id = p_face_id;
	roughness.push_constant.roughness = p_roughness * p_roughness; // Shader expects roughness, not perceptual roughness, so multiply before passing in.
	roughness.push_constant.sample_count = p_sample_count;
	roughness.push_constant.use_direct_write = p_roughness == 0.0;
	roughness.push_constant.face_size = p_size;

	RD::DrawListID draw_list = RD::get_singleton()->draw_list_begin(p_dest_framebuffer, RD::INITIAL_ACTION_KEEP, RD::FINAL_ACTION_READ, RD::INITIAL_ACTION_KEEP, RD::FINAL_ACTION_DISCARD);
	RD::get_singleton()->draw_list_bind_render_pipeline(draw_list, roughness.raster_pipeline.get_render_pipeline(RD::INVALID_ID, RD::get_singleton()->framebuffer_get_format(p_dest_framebuffer)));
	RD::get_singleton()->draw_list_bind_uniform_set(draw_list, _get_uniform_set_from_texture(p_source_rd_texture), 0);
	RD::get_singleton()->draw_list_bind_index_array(draw_list, index_array);

	RD::get_singleton()->draw_list_set_push_constant(draw_list, &roughness.push_constant, sizeof(CubemapRoughnessPushConstant));

	RD::get_singleton()->draw_list_draw(draw_list, true);
	RD::get_singleton()->draw_list_end();
}

void EffectsRD::cubemap_downsample(RID p_source_cubemap, RID p_dest_cubemap, const Size2i &p_size) {
	ERR_FAIL_COND_MSG(prefer_raster_effects, "Can't use compute based cubemap downsample with the mobile renderer.");

	cubemap_downsampler.push_constant.face_size = p_size.x;
	cubemap_downsampler.push_constant.face_id = 0; // we render all 6 sides to each layer in one call

	RD::ComputeListID compute_list = RD::get_singleton()->compute_list_begin();
	RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, cubemap_downsampler.compute_pipeline);
	RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_texture(p_source_cubemap), 0);
	RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_uniform_set_from_image(p_dest_cubemap), 1);

	int x_groups = (p_size.x - 1) / 8 + 1;
	int y_groups = (p_size.y - 1) / 8 + 1;

	RD::get_singleton()->compute_list_set_push_constant(compute_list, &cubemap_downsampler.push_constant, sizeof(CubemapDownsamplerPushConstant));

	RD::get_singleton()->compute_list_dispatch(compute_list, x_groups, y_groups, 6); // one z_group for each face

	RD::get_singleton()->compute_list_end();
}

void EffectsRD::cubemap_downsample_raster(RID p_source_cubemap, RID p_dest_framebuffer, uint32_t p_face_id, const Size2i &p_size) {
	ERR_FAIL_COND_MSG(!prefer_raster_effects, "Can't use raster based cubemap downsample with the clustered renderer.");
	ERR_FAIL_COND_MSG(p_face_id >= 6, "Raster implementation of cubemap downsample must process one side at a time.");

	cubemap_downsampler.push_constant.face_size = p_size.x;
	cubemap_downsampler.push_constant.face_id = p_face_id;

	RD::DrawListID draw_list = RD::get_singleton()->draw_list_begin(p_dest_framebuffer, RD::INITIAL_ACTION_KEEP, RD::FINAL_ACTION_READ, RD::INITIAL_ACTION_KEEP, RD::FINAL_ACTION_DISCARD);
	RD::get_singleton()->draw_list_bind_render_pipeline(draw_list, cubemap_downsampler.raster_pipeline.get_render_pipeline(RD::INVALID_ID, RD::get_singleton()->framebuffer_get_format(p_dest_framebuffer)));
	RD::get_singleton()->draw_list_bind_uniform_set(draw_list, _get_uniform_set_from_texture(p_source_cubemap), 0);
	RD::get_singleton()->draw_list_bind_index_array(draw_list, index_array);

	RD::get_singleton()->draw_list_set_push_constant(draw_list, &cubemap_downsampler.push_constant, sizeof(CubemapDownsamplerPushConstant));

	RD::get_singleton()->draw_list_draw(draw_list, true);
	RD::get_singleton()->draw_list_end();
}

void EffectsRD::cubemap_filter(RID p_source_cubemap, Vector<RID> p_dest_cubemap, bool p_use_array) {
	ERR_FAIL_COND_MSG(prefer_raster_effects, "Can't use compute based cubemap filter with the mobile renderer.");

	Vector<RD::Uniform> uniforms;
	for (int i = 0; i < p_dest_cubemap.size(); i++) {
		RD::Uniform u;
		u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
		u.binding = i;
		u.append_id(p_dest_cubemap[i]);
		uniforms.push_back(u);
	}
	if (RD::get_singleton()->uniform_set_is_valid(filter.image_uniform_set)) {
		RD::get_singleton()->free(filter.image_uniform_set);
	}
	filter.image_uniform_set = RD::get_singleton()->uniform_set_create(uniforms, filter.compute_shader.version_get_shader(filter.shader_version, 0), 2);

	int pipeline = p_use_array ? FILTER_MODE_HIGH_QUALITY_ARRAY : FILTER_MODE_HIGH_QUALITY;
	pipeline = filter.use_high_quality ? pipeline : pipeline + 1;
	RD::ComputeListID compute_list = RD::get_singleton()->compute_list_begin();
	RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, filter.compute_pipelines[pipeline]);
	RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_texture(p_source_cubemap, true), 0);
	RD::get_singleton()->compute_list_bind_uniform_set(compute_list, filter.uniform_set, 1);
	RD::get_singleton()->compute_list_bind_uniform_set(compute_list, filter.image_uniform_set, 2);

	int x_groups = p_use_array ? 1792 : 342; // (128 * 128 * 7) / 64 : (128*128 + 64*64 + 32*32 + 16*16 + 8*8 + 4*4 + 2*2) / 64

	RD::get_singleton()->compute_list_dispatch(compute_list, x_groups, 6, 1); // one y_group for each face

	RD::get_singleton()->compute_list_end();
}

void EffectsRD::cubemap_filter_raster(RID p_source_cubemap, RID p_dest_framebuffer, uint32_t p_face_id, uint32_t p_mip_level) {
	ERR_FAIL_COND_MSG(!prefer_raster_effects, "Can't use raster based cubemap filter with the clustered renderer.");
	ERR_FAIL_COND_MSG(p_face_id >= 6, "Raster implementation of cubemap filter must process one side at a time.");

	// TODO implement!
	CubemapFilterRasterPushConstant push_constant;
	push_constant.mip_level = p_mip_level;
	push_constant.face_id = p_face_id;

	CubemapFilterMode mode = filter.use_high_quality ? FILTER_MODE_HIGH_QUALITY : FILTER_MODE_LOW_QUALITY;

	RD::DrawListID draw_list = RD::get_singleton()->draw_list_begin(p_dest_framebuffer, RD::INITIAL_ACTION_KEEP, RD::FINAL_ACTION_READ, RD::INITIAL_ACTION_KEEP, RD::FINAL_ACTION_DISCARD);
	RD::get_singleton()->draw_list_bind_render_pipeline(draw_list, filter.raster_pipelines[mode].get_render_pipeline(RD::INVALID_ID, RD::get_singleton()->framebuffer_get_format(p_dest_framebuffer)));
	RD::get_singleton()->draw_list_bind_uniform_set(draw_list, _get_uniform_set_from_texture(p_source_cubemap), 0);
	RD::get_singleton()->draw_list_bind_uniform_set(draw_list, filter.uniform_set, 1);
	RD::get_singleton()->draw_list_bind_index_array(draw_list, index_array);

	RD::get_singleton()->draw_list_set_push_constant(draw_list, &push_constant, sizeof(CubemapFilterRasterPushConstant));

	RD::get_singleton()->draw_list_draw(draw_list, true);
	RD::get_singleton()->draw_list_end();
}

void EffectsRD::resolve_gi(RID p_source_depth, RID p_source_normal_roughness, RID p_source_voxel_gi, RID p_dest_depth, RID p_dest_normal_roughness, RID p_dest_voxel_gi, Vector2i p_screen_size, int p_samples, uint32_t p_barrier) {
	ResolvePushConstant push_constant;
	push_constant.screen_size[0] = p_screen_size.x;
	push_constant.screen_size[1] = p_screen_size.y;
	push_constant.samples = p_samples;

	RD::ComputeListID compute_list = RD::get_singleton()->compute_list_begin();
	RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, resolve.pipelines[p_source_voxel_gi.is_valid() ? RESOLVE_MODE_GI_VOXEL_GI : RESOLVE_MODE_GI]);
	RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_texture_pair(p_source_depth, p_source_normal_roughness), 0);
	RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_image_pair(p_dest_depth, p_dest_normal_roughness), 1);
	if (p_source_voxel_gi.is_valid()) {
		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_texture(p_source_voxel_gi), 2);
		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_uniform_set_from_image(p_dest_voxel_gi), 3);
	}

	RD::get_singleton()->compute_list_set_push_constant(compute_list, &push_constant, sizeof(ResolvePushConstant));

	RD::get_singleton()->compute_list_dispatch_threads(compute_list, p_screen_size.x, p_screen_size.y, 1);

	RD::get_singleton()->compute_list_end(p_barrier);
}

void EffectsRD::resolve_depth(RID p_source_depth, RID p_dest_depth, Vector2i p_screen_size, int p_samples, uint32_t p_barrier) {
	ResolvePushConstant push_constant;
	push_constant.screen_size[0] = p_screen_size.x;
	push_constant.screen_size[1] = p_screen_size.y;
	push_constant.samples = p_samples;

	RD::ComputeListID compute_list = RD::get_singleton()->compute_list_begin();
	RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, resolve.pipelines[RESOLVE_MODE_DEPTH]);
	RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_compute_uniform_set_from_texture(p_source_depth), 0);
	RD::get_singleton()->compute_list_bind_uniform_set(compute_list, _get_uniform_set_from_image(p_dest_depth), 1);

	RD::get_singleton()->compute_list_set_push_constant(compute_list, &push_constant, sizeof(ResolvePushConstant));

	RD::get_singleton()->compute_list_dispatch_threads(compute_list, p_screen_size.x, p_screen_size.y, 1);

	RD::get_singleton()->compute_list_end(p_barrier);
}

void EffectsRD::sort_buffer(RID p_uniform_set, int p_size) {
	Sort::PushConstant push_constant;
	push_constant.total_elements = p_size;

	bool done = true;

	int numThreadGroups = ((p_size - 1) >> 9) + 1;

	if (numThreadGroups > 1) {
		done = false;
	}

	RD::ComputeListID compute_list = RD::get_singleton()->compute_list_begin();

	RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, sort.pipelines[SORT_MODE_BLOCK]);
	RD::get_singleton()->compute_list_bind_uniform_set(compute_list, p_uniform_set, 1);
	RD::get_singleton()->compute_list_set_push_constant(compute_list, &push_constant, sizeof(Sort::PushConstant));
	RD::get_singleton()->compute_list_dispatch(compute_list, numThreadGroups, 1, 1);

	int presorted = 512;

	while (!done) {
		RD::get_singleton()->compute_list_add_barrier(compute_list);

		done = true;
		RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, sort.pipelines[SORT_MODE_STEP]);

		numThreadGroups = 0;

		if (p_size > presorted) {
			if (p_size > presorted * 2) {
				done = false;
			}

			int pow2 = presorted;
			while (pow2 < p_size) {
				pow2 *= 2;
			}
			numThreadGroups = pow2 >> 9;
		}

		unsigned int nMergeSize = presorted * 2;

		for (unsigned int nMergeSubSize = nMergeSize >> 1; nMergeSubSize > 256; nMergeSubSize = nMergeSubSize >> 1) {
			push_constant.job_params[0] = nMergeSubSize;
			if (nMergeSubSize == nMergeSize >> 1) {
				push_constant.job_params[1] = (2 * nMergeSubSize - 1);
				push_constant.job_params[2] = -1;
			} else {
				push_constant.job_params[1] = nMergeSubSize;
				push_constant.job_params[2] = 1;
			}
			push_constant.job_params[3] = 0;

			RD::get_singleton()->compute_list_set_push_constant(compute_list, &push_constant, sizeof(Sort::PushConstant));
			RD::get_singleton()->compute_list_dispatch(compute_list, numThreadGroups, 1, 1);
			RD::get_singleton()->compute_list_add_barrier(compute_list);
		}

		RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, sort.pipelines[SORT_MODE_INNER]);
		RD::get_singleton()->compute_list_set_push_constant(compute_list, &push_constant, sizeof(Sort::PushConstant));
		RD::get_singleton()->compute_list_dispatch(compute_list, numThreadGroups, 1, 1);

		presorted *= 2;
	}

	RD::get_singleton()->compute_list_end();
}

EffectsRD::EffectsRD(bool p_prefer_raster_effects) {
	{
		Vector<String> FSR_upscale_modes;

#if defined(OSX_ENABLED) || defined(IPHONE_ENABLED)
		// MoltenVK does not support some of the operations used by the normal mode of FSR. Fallback works just fine though.
		FSR_upscale_modes.push_back("\n#define MODE_FSR_UPSCALE_FALLBACK\n");
#else
		// Everyone else can use normal mode when available.
		if (RD::get_singleton()->get_device_capabilities()->supports_fsr_half_float) {
			FSR_upscale_modes.push_back("\n#define MODE_FSR_UPSCALE_NORMAL\n");
		} else {
			FSR_upscale_modes.push_back("\n#define MODE_FSR_UPSCALE_FALLBACK\n");
		}
#endif

		FSR_upscale.shader.initialize(FSR_upscale_modes);

		FSR_upscale.shader_version = FSR_upscale.shader.version_create();
		FSR_upscale.pipeline = RD::get_singleton()->compute_pipeline_create(FSR_upscale.shader.version_get_shader(FSR_upscale.shader_version, 0));
	}

	prefer_raster_effects = p_prefer_raster_effects;

	{
		// Initialize roughness
		Vector<String> cubemap_roughness_modes;
		cubemap_roughness_modes.push_back("");

		if (prefer_raster_effects) {
			roughness.raster_shader.initialize(cubemap_roughness_modes);

			roughness.shader_version = roughness.raster_shader.version_create();

			roughness.raster_pipeline.setup(roughness.raster_shader.version_get_shader(roughness.shader_version, 0), RD::RENDER_PRIMITIVE_TRIANGLES, RD::PipelineRasterizationState(), RD::PipelineMultisampleState(), RD::PipelineDepthStencilState(), RD::PipelineColorBlendState::create_disabled(), 0);

		} else {
			roughness.compute_shader.initialize(cubemap_roughness_modes);

			roughness.shader_version = roughness.compute_shader.version_create();

			roughness.compute_pipeline = RD::get_singleton()->compute_pipeline_create(roughness.compute_shader.version_get_shader(roughness.shader_version, 0));
			roughness.raster_pipeline.clear();
		}
	}

	if (prefer_raster_effects) {
		Vector<String> luminance_reduce_modes;
		luminance_reduce_modes.push_back("\n#define FIRST_PASS\n"); // LUMINANCE_REDUCE_FRAGMENT_FIRST
		luminance_reduce_modes.push_back("\n"); // LUMINANCE_REDUCE_FRAGMENT
		luminance_reduce_modes.push_back("\n#define FINAL_PASS\n"); // LUMINANCE_REDUCE_FRAGMENT_FINAL

		luminance_reduce_raster.shader.initialize(luminance_reduce_modes);
		memset(&luminance_reduce_raster.push_constant, 0, sizeof(LuminanceReduceRasterPushConstant));
		luminance_reduce_raster.shader_version = luminance_reduce_raster.shader.version_create();

		for (int i = 0; i < LUMINANCE_REDUCE_FRAGMENT_MAX; i++) {
			luminance_reduce_raster.pipelines[i].setup(luminance_reduce_raster.shader.version_get_shader(luminance_reduce_raster.shader_version, i), RD::RENDER_PRIMITIVE_TRIANGLES, RD::PipelineRasterizationState(), RD::PipelineMultisampleState(), RD::PipelineDepthStencilState(), RD::PipelineColorBlendState::create_disabled(), 0);
		}
	} else {
		// Initialize luminance_reduce
		Vector<String> luminance_reduce_modes;
		luminance_reduce_modes.push_back("\n#define READ_TEXTURE\n");
		luminance_reduce_modes.push_back("\n");
		luminance_reduce_modes.push_back("\n#define WRITE_LUMINANCE\n");

		luminance_reduce.shader.initialize(luminance_reduce_modes);

		luminance_reduce.shader_version = luminance_reduce.shader.version_create();

		for (int i = 0; i < LUMINANCE_REDUCE_MAX; i++) {
			luminance_reduce.pipelines[i] = RD::get_singleton()->compute_pipeline_create(luminance_reduce.shader.version_get_shader(luminance_reduce.shader_version, i));
		}

		for (int i = 0; i < LUMINANCE_REDUCE_FRAGMENT_MAX; i++) {
			luminance_reduce_raster.pipelines[i].clear();
		}
	}

	{
		// Initialize copier
		Vector<String> copy_modes;
		copy_modes.push_back("\n");

		cube_to_dp.shader.initialize(copy_modes);

		cube_to_dp.shader_version = cube_to_dp.shader.version_create();
		RID shader = cube_to_dp.shader.version_get_shader(cube_to_dp.shader_version, 0);
		RD::PipelineDepthStencilState dss;
		dss.enable_depth_test = true;
		dss.depth_compare_operator = RD::COMPARE_OP_ALWAYS;
		dss.enable_depth_write = true;
		cube_to_dp.pipeline.setup(shader, RD::RENDER_PRIMITIVE_TRIANGLES, RD::PipelineRasterizationState(), RD::PipelineMultisampleState(), dss, RD::PipelineColorBlendState(), 0);
	}

	if (!prefer_raster_effects) {
		{
			// Initialize depth buffer for screen space effects
			Vector<String> downsampler_modes;
			downsampler_modes.push_back("\n");
			downsampler_modes.push_back("\n#define USE_HALF_SIZE\n");
			downsampler_modes.push_back("\n#define GENERATE_MIPS\n");
			downsampler_modes.push_back("\n#define GENERATE_MIPS\n#define USE_HALF_SIZE\n");
			downsampler_modes.push_back("\n#define USE_HALF_BUFFERS\n");
			downsampler_modes.push_back("\n#define USE_HALF_BUFFERS\n#define USE_HALF_SIZE\n");
			downsampler_modes.push_back("\n#define GENERATE_MIPS\n#define GENERATE_FULL_MIPS");

			ss_effects.downsample_shader.initialize(downsampler_modes);

			ss_effects.downsample_shader_version = ss_effects.downsample_shader.version_create();

			for (int i = 0; i < SS_EFFECTS_MAX; i++) {
				ss_effects.pipelines[i] = RD::get_singleton()->compute_pipeline_create(ss_effects.downsample_shader.version_get_shader(ss_effects.downsample_shader_version, i));
			}

			ss_effects.gather_constants_buffer = RD::get_singleton()->uniform_buffer_create(sizeof(SSEffectsGatherConstants));
			SSEffectsGatherConstants gather_constants;

			const int sub_pass_count = 5;
			for (int pass = 0; pass < 4; pass++) {
				for (int subPass = 0; subPass < sub_pass_count; subPass++) {
					int a = pass;
					int b = subPass;

					int spmap[5]{ 0, 1, 4, 3, 2 };
					b = spmap[subPass];

					float ca, sa;
					float angle0 = (float(a) + float(b) / float(sub_pass_count)) * Math_PI * 0.5f;

					ca = Math::cos(angle0);
					sa = Math::sin(angle0);

					float scale = 1.0f + (a - 1.5f + (b - (sub_pass_count - 1.0f) * 0.5f) / float(sub_pass_count)) * 0.07f;

					gather_constants.rotation_matrices[pass * 20 + subPass * 4 + 0] = scale * ca;
					gather_constants.rotation_matrices[pass * 20 + subPass * 4 + 1] = scale * -sa;
					gather_constants.rotation_matrices[pass * 20 + subPass * 4 + 2] = -scale * sa;
					gather_constants.rotation_matrices[pass * 20 + subPass * 4 + 3] = -scale * ca;
				}
			}

			RD::get_singleton()->buffer_update(ss_effects.gather_constants_buffer, 0, sizeof(SSEffectsGatherConstants), &gather_constants);
		}

		{
			// Initialize ssao

			RD::SamplerState sampler;
			sampler.mag_filter = RD::SAMPLER_FILTER_NEAREST;
			sampler.min_filter = RD::SAMPLER_FILTER_NEAREST;
			sampler.mip_filter = RD::SAMPLER_FILTER_NEAREST;
			sampler.repeat_u = RD::SAMPLER_REPEAT_MODE_MIRRORED_REPEAT;
			sampler.repeat_v = RD::SAMPLER_REPEAT_MODE_MIRRORED_REPEAT;
			sampler.repeat_w = RD::SAMPLER_REPEAT_MODE_MIRRORED_REPEAT;
			sampler.max_lod = 4;

			ss_effects.mirror_sampler = RD::get_singleton()->sampler_create(sampler);

			uint32_t pipeline = 0;
			{
				Vector<String> ssao_modes;

				ssao_modes.push_back("\n");
				ssao_modes.push_back("\n#define SSAO_BASE\n");
				ssao_modes.push_back("\n#define ADAPTIVE\n");

				ssao.gather_shader.initialize(ssao_modes);

				ssao.gather_shader_version = ssao.gather_shader.version_create();

				for (int i = 0; i <= SSAO_GATHER_ADAPTIVE; i++) {
					ssao.pipelines[pipeline] = RD::get_singleton()->compute_pipeline_create(ssao.gather_shader.version_get_shader(ssao.gather_shader_version, i));
					pipeline++;
				}
			}
			{
				Vector<String> ssao_modes;
				ssao_modes.push_back("\n#define GENERATE_MAP\n");
				ssao_modes.push_back("\n#define PROCESS_MAPA\n");
				ssao_modes.push_back("\n#define PROCESS_MAPB\n");

				ssao.importance_map_shader.initialize(ssao_modes);

				ssao.importance_map_shader_version = ssao.importance_map_shader.version_create();

				for (int i = SSAO_GENERATE_IMPORTANCE_MAP; i <= SSAO_PROCESS_IMPORTANCE_MAPB; i++) {
					ssao.pipelines[pipeline] = RD::get_singleton()->compute_pipeline_create(ssao.importance_map_shader.version_get_shader(ssao.importance_map_shader_version, i - SSAO_GENERATE_IMPORTANCE_MAP));

					pipeline++;
				}
				ssao.importance_map_load_counter = RD::get_singleton()->storage_buffer_create(sizeof(uint32_t));
				int zero[1] = { 0 };
				RD::get_singleton()->buffer_update(ssao.importance_map_load_counter, 0, sizeof(uint32_t), &zero);
				RD::get_singleton()->set_resource_name(ssao.importance_map_load_counter, "Importance Map Load Counter");

				Vector<RD::Uniform> uniforms;
				{
					RD::Uniform u;
					u.uniform_type = RD::UNIFORM_TYPE_STORAGE_BUFFER;
					u.binding = 0;
					u.append_id(ssao.importance_map_load_counter);
					uniforms.push_back(u);
				}
				ssao.counter_uniform_set = RD::get_singleton()->uniform_set_create(uniforms, ssao.importance_map_shader.version_get_shader(ssao.importance_map_shader_version, 2), 2);
				RD::get_singleton()->set_resource_name(ssao.counter_uniform_set, "Load Counter Uniform Set");
			}
			{
				Vector<String> ssao_modes;
				ssao_modes.push_back("\n#define MODE_NON_SMART\n");
				ssao_modes.push_back("\n#define MODE_SMART\n");
				ssao_modes.push_back("\n#define MODE_WIDE\n");

				ssao.blur_shader.initialize(ssao_modes);

				ssao.blur_shader_version = ssao.blur_shader.version_create();

				for (int i = SSAO_BLUR_PASS; i <= SSAO_BLUR_PASS_WIDE; i++) {
					ssao.pipelines[pipeline] = RD::get_singleton()->compute_pipeline_create(ssao.blur_shader.version_get_shader(ssao.blur_shader_version, i - SSAO_BLUR_PASS));

					pipeline++;
				}
			}
			{
				Vector<String> ssao_modes;
				ssao_modes.push_back("\n#define MODE_NON_SMART\n");
				ssao_modes.push_back("\n#define MODE_SMART\n");
				ssao_modes.push_back("\n#define MODE_HALF\n");

				ssao.interleave_shader.initialize(ssao_modes);

				ssao.interleave_shader_version = ssao.interleave_shader.version_create();
				for (int i = SSAO_INTERLEAVE; i <= SSAO_INTERLEAVE_HALF; i++) {
					ssao.pipelines[pipeline] = RD::get_singleton()->compute_pipeline_create(ssao.interleave_shader.version_get_shader(ssao.interleave_shader_version, i - SSAO_INTERLEAVE));
					RD::get_singleton()->set_resource_name(ssao.pipelines[pipeline], "Interleave Pipeline " + itos(i));
					pipeline++;
				}
			}

			ERR_FAIL_COND(pipeline != SSAO_MAX);
		}
	}

	if (!prefer_raster_effects) {
		// Initialize roughness limiter
		Vector<String> shader_modes;
		shader_modes.push_back("");

		roughness_limiter.shader.initialize(shader_modes);

		roughness_limiter.shader_version = roughness_limiter.shader.version_create();

		roughness_limiter.pipeline = RD::get_singleton()->compute_pipeline_create(roughness_limiter.shader.version_get_shader(roughness_limiter.shader_version, 0));
	}

	{
		//Initialize cubemap downsampler
		Vector<String> cubemap_downsampler_modes;
		cubemap_downsampler_modes.push_back("");

		if (prefer_raster_effects) {
			cubemap_downsampler.raster_shader.initialize(cubemap_downsampler_modes);

			cubemap_downsampler.shader_version = cubemap_downsampler.raster_shader.version_create();

			cubemap_downsampler.raster_pipeline.setup(cubemap_downsampler.raster_shader.version_get_shader(cubemap_downsampler.shader_version, 0), RD::RENDER_PRIMITIVE_TRIANGLES, RD::PipelineRasterizationState(), RD::PipelineMultisampleState(), RD::PipelineDepthStencilState(), RD::PipelineColorBlendState::create_disabled(), 0);
		} else {
			cubemap_downsampler.compute_shader.initialize(cubemap_downsampler_modes);

			cubemap_downsampler.shader_version = cubemap_downsampler.compute_shader.version_create();

			cubemap_downsampler.compute_pipeline = RD::get_singleton()->compute_pipeline_create(cubemap_downsampler.compute_shader.version_get_shader(cubemap_downsampler.shader_version, 0));
			cubemap_downsampler.raster_pipeline.clear();
		}
	}

	{
		// Initialize cubemap filter
		filter.use_high_quality = GLOBAL_GET("rendering/reflections/sky_reflections/fast_filter_high_quality");

		Vector<String> cubemap_filter_modes;
		cubemap_filter_modes.push_back("\n#define USE_HIGH_QUALITY\n");
		cubemap_filter_modes.push_back("\n#define USE_LOW_QUALITY\n");
		cubemap_filter_modes.push_back("\n#define USE_HIGH_QUALITY\n#define USE_TEXTURE_ARRAY\n");
		cubemap_filter_modes.push_back("\n#define USE_LOW_QUALITY\n#define USE_TEXTURE_ARRAY\n");

		if (filter.use_high_quality) {
			filter.coefficient_buffer = RD::get_singleton()->storage_buffer_create(sizeof(high_quality_coeffs));
			RD::get_singleton()->buffer_update(filter.coefficient_buffer, 0, sizeof(high_quality_coeffs), &high_quality_coeffs[0]);
		} else {
			filter.coefficient_buffer = RD::get_singleton()->storage_buffer_create(sizeof(low_quality_coeffs));
			RD::get_singleton()->buffer_update(filter.coefficient_buffer, 0, sizeof(low_quality_coeffs), &low_quality_coeffs[0]);
		}

		if (prefer_raster_effects) {
			filter.raster_shader.initialize(cubemap_filter_modes);

			// array variants are not supported in raster
			filter.raster_shader.set_variant_enabled(FILTER_MODE_HIGH_QUALITY_ARRAY, false);
			filter.raster_shader.set_variant_enabled(FILTER_MODE_LOW_QUALITY_ARRAY, false);

			filter.shader_version = filter.raster_shader.version_create();

			for (int i = 0; i < FILTER_MODE_MAX; i++) {
				if (filter.raster_shader.is_variant_enabled(i)) {
					filter.raster_pipelines[i].setup(filter.raster_shader.version_get_shader(filter.shader_version, i), RD::RENDER_PRIMITIVE_TRIANGLES, RD::PipelineRasterizationState(), RD::PipelineMultisampleState(), RD::PipelineDepthStencilState(), RD::PipelineColorBlendState::create_disabled(), 0);
				} else {
					filter.raster_pipelines[i].clear();
				}
			}

			Vector<RD::Uniform> uniforms;
			{
				RD::Uniform u;
				u.uniform_type = RD::UNIFORM_TYPE_STORAGE_BUFFER;
				u.binding = 0;
				u.append_id(filter.coefficient_buffer);
				uniforms.push_back(u);
			}
			filter.uniform_set = RD::get_singleton()->uniform_set_create(uniforms, filter.raster_shader.version_get_shader(filter.shader_version, filter.use_high_quality ? 0 : 1), 1);
		} else {
			filter.compute_shader.initialize(cubemap_filter_modes);
			filter.shader_version = filter.compute_shader.version_create();

			for (int i = 0; i < FILTER_MODE_MAX; i++) {
				filter.compute_pipelines[i] = RD::get_singleton()->compute_pipeline_create(filter.compute_shader.version_get_shader(filter.shader_version, i));
				filter.raster_pipelines[i].clear();
			}

			Vector<RD::Uniform> uniforms;
			{
				RD::Uniform u;
				u.uniform_type = RD::UNIFORM_TYPE_STORAGE_BUFFER;
				u.binding = 0;
				u.append_id(filter.coefficient_buffer);
				uniforms.push_back(u);
			}
			filter.uniform_set = RD::get_singleton()->uniform_set_create(uniforms, filter.compute_shader.version_get_shader(filter.shader_version, filter.use_high_quality ? 0 : 1), 1);
		}
	}

	if (!prefer_raster_effects) {
		Vector<String> specular_modes;
		specular_modes.push_back("\n#define MODE_MERGE\n");
		specular_modes.push_back("\n#define MODE_MERGE\n#define MODE_SSR\n");
		specular_modes.push_back("\n");
		specular_modes.push_back("\n#define MODE_SSR\n");

		specular_merge.shader.initialize(specular_modes);

		specular_merge.shader_version = specular_merge.shader.version_create();

		//use additive

		RD::PipelineColorBlendState::Attachment ba;
		ba.enable_blend = true;
		ba.src_color_blend_factor = RD::BLEND_FACTOR_ONE;
		ba.dst_color_blend_factor = RD::BLEND_FACTOR_ONE;
		ba.src_alpha_blend_factor = RD::BLEND_FACTOR_ONE;
		ba.dst_alpha_blend_factor = RD::BLEND_FACTOR_ONE;
		ba.color_blend_op = RD::BLEND_OP_ADD;
		ba.alpha_blend_op = RD::BLEND_OP_ADD;

		RD::PipelineColorBlendState blend_additive;
		blend_additive.attachments.push_back(ba);

		for (int i = 0; i < SPECULAR_MERGE_MAX; i++) {
			RD::PipelineColorBlendState blend_state;
			if (i == SPECULAR_MERGE_ADDITIVE_ADD || i == SPECULAR_MERGE_ADDITIVE_SSR) {
				blend_state = blend_additive;
			} else {
				blend_state = RD::PipelineColorBlendState::create_disabled();
			}
			specular_merge.pipelines[i].setup(specular_merge.shader.version_get_shader(specular_merge.shader_version, i), RD::RENDER_PRIMITIVE_TRIANGLES, RD::PipelineRasterizationState(), RD::PipelineMultisampleState(), RD::PipelineDepthStencilState(), blend_state, 0);
		}
	}

	if (!prefer_raster_effects) {
		{
			Vector<String> ssr_modes;
			ssr_modes.push_back("\n");
			ssr_modes.push_back("\n#define MODE_ROUGH\n");

			ssr.shader.initialize(ssr_modes);

			ssr.shader_version = ssr.shader.version_create();

			for (int i = 0; i < SCREEN_SPACE_REFLECTION_MAX; i++) {
				ssr.pipelines[i] = RD::get_singleton()->compute_pipeline_create(ssr.shader.version_get_shader(ssr.shader_version, i));
			}
		}

		{
			Vector<String> ssr_filter_modes;
			ssr_filter_modes.push_back("\n");
			ssr_filter_modes.push_back("\n#define VERTICAL_PASS\n");

			ssr_filter.shader.initialize(ssr_filter_modes);

			ssr_filter.shader_version = ssr_filter.shader.version_create();

			for (int i = 0; i < SCREEN_SPACE_REFLECTION_FILTER_MAX; i++) {
				ssr_filter.pipelines[i] = RD::get_singleton()->compute_pipeline_create(ssr_filter.shader.version_get_shader(ssr_filter.shader_version, i));
			}
		}

		{
			Vector<String> ssr_scale_modes;
			ssr_scale_modes.push_back("\n");

			ssr_scale.shader.initialize(ssr_scale_modes);

			ssr_scale.shader_version = ssr_scale.shader.version_create();

			ssr_scale.pipeline = RD::get_singleton()->compute_pipeline_create(ssr_scale.shader.version_get_shader(ssr_scale.shader_version, 0));
		}

		{
			Vector<String> sss_modes;
			sss_modes.push_back("\n#define USE_11_SAMPLES\n");
			sss_modes.push_back("\n#define USE_17_SAMPLES\n");
			sss_modes.push_back("\n#define USE_25_SAMPLES\n");

			sss.shader.initialize(sss_modes);

			sss.shader_version = sss.shader.version_create();

			for (int i = 0; i < sss_modes.size(); i++) {
				sss.pipelines[i] = RD::get_singleton()->compute_pipeline_create(sss.shader.version_get_shader(sss.shader_version, i));
			}
		}

		{
			Vector<String> ssil_modes;
			ssil_modes.push_back("\n");
			ssil_modes.push_back("\n#define SSIL_BASE\n");
			ssil_modes.push_back("\n#define ADAPTIVE\n");

			ssil.gather_shader.initialize(ssil_modes);

			ssil.gather_shader_version = ssil.gather_shader.version_create();

			for (int i = SSIL_GATHER; i <= SSIL_GATHER_ADAPTIVE; i++) {
				ssil.pipelines[i] = RD::get_singleton()->compute_pipeline_create(ssil.gather_shader.version_get_shader(ssil.gather_shader_version, i));
			}
			ssil.projection_uniform_buffer = RD::get_singleton()->uniform_buffer_create(sizeof(SSILProjectionUniforms));
		}

		{
			Vector<String> ssil_modes;
			ssil_modes.push_back("\n#define GENERATE_MAP\n");
			ssil_modes.push_back("\n#define PROCESS_MAPA\n");
			ssil_modes.push_back("\n#define PROCESS_MAPB\n");

			ssil.importance_map_shader.initialize(ssil_modes);

			ssil.importance_map_shader_version = ssil.importance_map_shader.version_create();

			for (int i = SSIL_GENERATE_IMPORTANCE_MAP; i <= SSIL_PROCESS_IMPORTANCE_MAPB; i++) {
				ssil.pipelines[i] = RD::get_singleton()->compute_pipeline_create(ssil.importance_map_shader.version_get_shader(ssil.importance_map_shader_version, i - SSIL_GENERATE_IMPORTANCE_MAP));
			}
			ssil.importance_map_load_counter = RD::get_singleton()->storage_buffer_create(sizeof(uint32_t));
			int zero[1] = { 0 };
			RD::get_singleton()->buffer_update(ssil.importance_map_load_counter, 0, sizeof(uint32_t), &zero);
			RD::get_singleton()->set_resource_name(ssil.importance_map_load_counter, "Importance Map Load Counter");

			Vector<RD::Uniform> uniforms;
			{
				RD::Uniform u;
				u.uniform_type = RD::UNIFORM_TYPE_STORAGE_BUFFER;
				u.binding = 0;
				u.append_id(ssil.importance_map_load_counter);
				uniforms.push_back(u);
			}
			ssil.counter_uniform_set = RD::get_singleton()->uniform_set_create(uniforms, ssil.importance_map_shader.version_get_shader(ssil.importance_map_shader_version, 2), 2);
			RD::get_singleton()->set_resource_name(ssil.counter_uniform_set, "Load Counter Uniform Set");
		}
		{
			Vector<String> ssil_modes;
			ssil_modes.push_back("\n#define MODE_NON_SMART\n");
			ssil_modes.push_back("\n#define MODE_SMART\n");
			ssil_modes.push_back("\n#define MODE_WIDE\n");

			ssil.blur_shader.initialize(ssil_modes);

			ssil.blur_shader_version = ssil.blur_shader.version_create();
			for (int i = SSIL_BLUR_PASS; i <= SSIL_BLUR_PASS_WIDE; i++) {
				ssil.pipelines[i] = RD::get_singleton()->compute_pipeline_create(ssil.blur_shader.version_get_shader(ssil.blur_shader_version, i - SSIL_BLUR_PASS));
			}
		}

		{
			Vector<String> ssil_modes;
			ssil_modes.push_back("\n#define MODE_NON_SMART\n");
			ssil_modes.push_back("\n#define MODE_SMART\n");
			ssil_modes.push_back("\n#define MODE_HALF\n");

			ssil.interleave_shader.initialize(ssil_modes);

			ssil.interleave_shader_version = ssil.interleave_shader.version_create();
			for (int i = SSIL_INTERLEAVE; i <= SSIL_INTERLEAVE_HALF; i++) {
				ssil.pipelines[i] = RD::get_singleton()->compute_pipeline_create(ssil.interleave_shader.version_get_shader(ssil.interleave_shader_version, i - SSIL_INTERLEAVE));
			}
		}

		{
			Vector<String> resolve_modes;
			resolve_modes.push_back("\n#define MODE_RESOLVE_GI\n");
			resolve_modes.push_back("\n#define MODE_RESOLVE_GI\n#define VOXEL_GI_RESOLVE\n");
			resolve_modes.push_back("\n#define MODE_RESOLVE_DEPTH\n");

			resolve.shader.initialize(resolve_modes);

			resolve.shader_version = resolve.shader.version_create();

			for (int i = 0; i < RESOLVE_MODE_MAX; i++) {
				resolve.pipelines[i] = RD::get_singleton()->compute_pipeline_create(resolve.shader.version_get_shader(resolve.shader_version, i));
			}
		}
	}

	{
		Vector<String> sort_modes;
		sort_modes.push_back("\n#define MODE_SORT_BLOCK\n");
		sort_modes.push_back("\n#define MODE_SORT_STEP\n");
		sort_modes.push_back("\n#define MODE_SORT_INNER\n");

		sort.shader.initialize(sort_modes);

		sort.shader_version = sort.shader.version_create();

		for (int i = 0; i < SORT_MODE_MAX; i++) {
			sort.pipelines[i] = RD::get_singleton()->compute_pipeline_create(sort.shader.version_get_shader(sort.shader_version, i));
		}
	}

	{
		Vector<String> taa_modes;
		taa_modes.push_back("\n#define MODE_TAA_RESOLVE");
		TAA_resolve.shader.initialize(taa_modes);
		TAA_resolve.shader_version = TAA_resolve.shader.version_create();
		TAA_resolve.pipeline = RD::get_singleton()->compute_pipeline_create(TAA_resolve.shader.version_get_shader(TAA_resolve.shader_version, 0));
	}

	RD::SamplerState sampler;
	sampler.mag_filter = RD::SAMPLER_FILTER_LINEAR;
	sampler.min_filter = RD::SAMPLER_FILTER_LINEAR;
	sampler.max_lod = 0;

	default_sampler = RD::get_singleton()->sampler_create(sampler);
	RD::get_singleton()->set_resource_name(default_sampler, "Default Linear Sampler");

	sampler.min_filter = RD::SAMPLER_FILTER_LINEAR;
	sampler.mip_filter = RD::SAMPLER_FILTER_LINEAR;
	sampler.max_lod = 1e20;

	default_mipmap_sampler = RD::get_singleton()->sampler_create(sampler);
	RD::get_singleton()->set_resource_name(default_mipmap_sampler, "Default MipMap Sampler");

	{ //create index array for copy shaders
		Vector<uint8_t> pv;
		pv.resize(6 * 4);
		{
			uint8_t *w = pv.ptrw();
			int *p32 = (int *)w;
			p32[0] = 0;
			p32[1] = 1;
			p32[2] = 2;
			p32[3] = 0;
			p32[4] = 2;
			p32[5] = 3;
		}
		index_buffer = RD::get_singleton()->index_buffer_create(6, RenderingDevice::INDEX_BUFFER_FORMAT_UINT32, pv);
		index_array = RD::get_singleton()->index_array_create(index_buffer, 0, 6);
	}
}

EffectsRD::~EffectsRD() {
	if (RD::get_singleton()->uniform_set_is_valid(filter.image_uniform_set)) {
		RD::get_singleton()->free(filter.image_uniform_set);
	}

	if (RD::get_singleton()->uniform_set_is_valid(filter.uniform_set)) {
		RD::get_singleton()->free(filter.uniform_set);
	}

	RD::get_singleton()->free(default_sampler);
	RD::get_singleton()->free(default_mipmap_sampler);
	RD::get_singleton()->free(index_buffer); //array gets freed as dependency
	RD::get_singleton()->free(filter.coefficient_buffer);

	FSR_upscale.shader.version_free(FSR_upscale.shader_version);
	TAA_resolve.shader.version_free(TAA_resolve.shader_version);
	if (prefer_raster_effects) {
		luminance_reduce_raster.shader.version_free(luminance_reduce_raster.shader_version);
		roughness.raster_shader.version_free(roughness.shader_version);
		cubemap_downsampler.raster_shader.version_free(cubemap_downsampler.shader_version);
		filter.raster_shader.version_free(filter.shader_version);
	} else {
		luminance_reduce.shader.version_free(luminance_reduce.shader_version);
		roughness.compute_shader.version_free(roughness.shader_version);
		cubemap_downsampler.compute_shader.version_free(cubemap_downsampler.shader_version);
		filter.compute_shader.version_free(filter.shader_version);
	}
	if (!prefer_raster_effects) {
		resolve.shader.version_free(resolve.shader_version);
		specular_merge.shader.version_free(specular_merge.shader_version);
		ss_effects.downsample_shader.version_free(ss_effects.downsample_shader_version);
		ssao.blur_shader.version_free(ssao.blur_shader_version);
		ssao.gather_shader.version_free(ssao.gather_shader_version);
		ssao.interleave_shader.version_free(ssao.interleave_shader_version);
		ssao.importance_map_shader.version_free(ssao.importance_map_shader_version);
		ssil.blur_shader.version_free(ssil.blur_shader_version);
		ssil.gather_shader.version_free(ssil.gather_shader_version);
		ssil.interleave_shader.version_free(ssil.interleave_shader_version);
		ssil.importance_map_shader.version_free(ssil.importance_map_shader_version);
		roughness_limiter.shader.version_free(roughness_limiter.shader_version);
		ssr.shader.version_free(ssr.shader_version);
		ssr_filter.shader.version_free(ssr_filter.shader_version);
		ssr_scale.shader.version_free(ssr_scale.shader_version);
		sss.shader.version_free(sss.shader_version);

		RD::get_singleton()->free(ss_effects.mirror_sampler);
		RD::get_singleton()->free(ss_effects.gather_constants_buffer);
		RD::get_singleton()->free(ssao.importance_map_load_counter);
		RD::get_singleton()->free(ssil.importance_map_load_counter);
		RD::get_singleton()->free(ssil.projection_uniform_buffer);
	}
	cube_to_dp.shader.version_free(cube_to_dp.shader_version);
	sort.shader.version_free(sort.shader_version);
}
