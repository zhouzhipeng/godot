/*************************************************************************/
/*  rasterizer_storage_gles3.cpp                                         */
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

#include "rasterizer_storage_gles3.h"

#ifdef GLES3_ENABLED

#include "core/config/project_settings.h"
#include "core/math/transform_3d.h"
// #include "rasterizer_canvas_gles3.h"
#include "rasterizer_scene_gles3.h"
#include "servers/rendering/shader_language.h"

void RasterizerStorageGLES3::bind_quad_array() const {
	//glBindBuffer(GL_ARRAY_BUFFER, resources.quadie);
	//glVertexAttribPointer(RS::ARRAY_VERTEX, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, 0);
	//glVertexAttribPointer(RS::ARRAY_TEX_UV, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, CAST_INT_TO_UCHAR_PTR(8));

	//glEnableVertexAttribArray(RS::ARRAY_VERTEX);
	//glEnableVertexAttribArray(RS::ARRAY_TEX_UV);
}

RID RasterizerStorageGLES3::sky_create() {
	Sky *sky = memnew(Sky);
	sky->radiance = 0;
	return sky_owner.make_rid(sky);
}

void RasterizerStorageGLES3::sky_set_texture(RID p_sky, RID p_panorama, int p_radiance_size) {
}

void RasterizerStorageGLES3::base_update_dependency(RID p_base, DependencyTracker *p_instance) {
}

/* VOXEL GI API */

RID RasterizerStorageGLES3::voxel_gi_allocate() {
	return RID();
}

void RasterizerStorageGLES3::voxel_gi_initialize(RID p_rid) {
}

void RasterizerStorageGLES3::voxel_gi_allocate_data(RID p_voxel_gi, const Transform3D &p_to_cell_xform, const AABB &p_aabb, const Vector3i &p_octree_size, const Vector<uint8_t> &p_octree_cells, const Vector<uint8_t> &p_data_cells, const Vector<uint8_t> &p_distance_field, const Vector<int> &p_level_counts) {
}

AABB RasterizerStorageGLES3::voxel_gi_get_bounds(RID p_voxel_gi) const {
	return AABB();
}

Vector3i RasterizerStorageGLES3::voxel_gi_get_octree_size(RID p_voxel_gi) const {
	return Vector3i();
}

Vector<uint8_t> RasterizerStorageGLES3::voxel_gi_get_octree_cells(RID p_voxel_gi) const {
	return Vector<uint8_t>();
}

Vector<uint8_t> RasterizerStorageGLES3::voxel_gi_get_data_cells(RID p_voxel_gi) const {
	return Vector<uint8_t>();
}

Vector<uint8_t> RasterizerStorageGLES3::voxel_gi_get_distance_field(RID p_voxel_gi) const {
	return Vector<uint8_t>();
}

Vector<int> RasterizerStorageGLES3::voxel_gi_get_level_counts(RID p_voxel_gi) const {
	return Vector<int>();
}

Transform3D RasterizerStorageGLES3::voxel_gi_get_to_cell_xform(RID p_voxel_gi) const {
	return Transform3D();
}

void RasterizerStorageGLES3::voxel_gi_set_dynamic_range(RID p_voxel_gi, float p_range) {
}

float RasterizerStorageGLES3::voxel_gi_get_dynamic_range(RID p_voxel_gi) const {
	return 0;
}

void RasterizerStorageGLES3::voxel_gi_set_propagation(RID p_voxel_gi, float p_range) {
}

float RasterizerStorageGLES3::voxel_gi_get_propagation(RID p_voxel_gi) const {
	return 0;
}

void RasterizerStorageGLES3::voxel_gi_set_energy(RID p_voxel_gi, float p_range) {
}

float RasterizerStorageGLES3::voxel_gi_get_energy(RID p_voxel_gi) const {
	return 0.0;
}

void RasterizerStorageGLES3::voxel_gi_set_bias(RID p_voxel_gi, float p_range) {
}

float RasterizerStorageGLES3::voxel_gi_get_bias(RID p_voxel_gi) const {
	return 0.0;
}

void RasterizerStorageGLES3::voxel_gi_set_normal_bias(RID p_voxel_gi, float p_range) {
}

float RasterizerStorageGLES3::voxel_gi_get_normal_bias(RID p_voxel_gi) const {
	return 0.0;
}

void RasterizerStorageGLES3::voxel_gi_set_interior(RID p_voxel_gi, bool p_enable) {
}

bool RasterizerStorageGLES3::voxel_gi_is_interior(RID p_voxel_gi) const {
	return false;
}

void RasterizerStorageGLES3::voxel_gi_set_use_two_bounces(RID p_voxel_gi, bool p_enable) {
}

bool RasterizerStorageGLES3::voxel_gi_is_using_two_bounces(RID p_voxel_gi) const {
	return false;
}

void RasterizerStorageGLES3::voxel_gi_set_anisotropy_strength(RID p_voxel_gi, float p_strength) {
}

float RasterizerStorageGLES3::voxel_gi_get_anisotropy_strength(RID p_voxel_gi) const {
	return 0;
}

uint32_t RasterizerStorageGLES3::voxel_gi_get_version(RID p_voxel_gi) {
	return 0;
}

/* OCCLUDER */

void RasterizerStorageGLES3::occluder_set_mesh(RID p_occluder, const PackedVector3Array &p_vertices, const PackedInt32Array &p_indices) {
}

/* FOG */

RID RasterizerStorageGLES3::fog_volume_allocate() {
	return RID();
}

void RasterizerStorageGLES3::fog_volume_initialize(RID p_rid) {
}

void RasterizerStorageGLES3::fog_volume_set_shape(RID p_fog_volume, RS::FogVolumeShape p_shape) {
}

void RasterizerStorageGLES3::fog_volume_set_extents(RID p_fog_volume, const Vector3 &p_extents) {
}

void RasterizerStorageGLES3::fog_volume_set_material(RID p_fog_volume, RID p_material) {
}

AABB RasterizerStorageGLES3::fog_volume_get_aabb(RID p_fog_volume) const {
	return AABB();
}

RS::FogVolumeShape RasterizerStorageGLES3::fog_volume_get_shape(RID p_fog_volume) const {
	return RS::FOG_VOLUME_SHAPE_BOX;
}

/* VISIBILITY NOTIFIER */
RID RasterizerStorageGLES3::visibility_notifier_allocate() {
	return RID();
}

void RasterizerStorageGLES3::visibility_notifier_initialize(RID p_notifier) {
}

void RasterizerStorageGLES3::visibility_notifier_set_aabb(RID p_notifier, const AABB &p_aabb) {
}

void RasterizerStorageGLES3::visibility_notifier_set_callbacks(RID p_notifier, const Callable &p_enter_callbable, const Callable &p_exit_callable) {
}

AABB RasterizerStorageGLES3::visibility_notifier_get_aabb(RID p_notifier) const {
	return AABB();
}

void RasterizerStorageGLES3::visibility_notifier_call(RID p_notifier, bool p_enter, bool p_deferred) {
}

/* CANVAS SHADOW */

RID RasterizerStorageGLES3::canvas_light_shadow_buffer_create(int p_width) {
	CanvasLightShadow *cls = memnew(CanvasLightShadow);

	if (p_width > config->max_texture_size) {
		p_width = config->max_texture_size;
	}

	cls->size = p_width;
	cls->height = 16;

	glActiveTexture(GL_TEXTURE0);

	glGenFramebuffers(1, &cls->fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, cls->fbo);

	glGenRenderbuffers(1, &cls->depth);
	glBindRenderbuffer(GL_RENDERBUFFER, cls->depth);
	glRenderbufferStorage(GL_RENDERBUFFER, config->depth_buffer_internalformat, cls->size, cls->height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, cls->depth);

	glGenTextures(1, &cls->distance);
	glBindTexture(GL_TEXTURE_2D, cls->distance);
	if (config->use_rgba_2d_shadows) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, cls->size, cls->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	} else {
#ifdef GLES_OVER_GL
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, cls->size, cls->height, 0, _RED_OES, GL_FLOAT, nullptr);
#else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_FLOAT, cls->size, cls->height, 0, _RED_OES, GL_FLOAT, NULL);
#endif
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, cls->distance, 0);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	//printf("errnum: %x\n",status);
	glBindFramebuffer(GL_FRAMEBUFFER, GLES3::TextureStorage::system_fbo);

	if (status != GL_FRAMEBUFFER_COMPLETE) {
		memdelete(cls);
		ERR_FAIL_COND_V(status != GL_FRAMEBUFFER_COMPLETE, RID());
	}

	return canvas_light_shadow_owner.make_rid(cls);
}

/* LIGHT SHADOW MAPPING */
/*

RID RasterizerStorageGLES3::canvas_light_occluder_create() {
	CanvasOccluder *co = memnew(CanvasOccluder);
	co->index_id = 0;
	co->vertex_id = 0;
	co->len = 0;

	return canvas_occluder_owner.make_rid(co);
}

void RasterizerStorageGLES3::canvas_light_occluder_set_polylines(RID p_occluder, const PoolVector<Vector2> &p_lines) {
	CanvasOccluder *co = canvas_occluder_owner.get(p_occluder);
	ERR_FAIL_COND(!co);

	co->lines = p_lines;

	if (p_lines.size() != co->len) {
		if (co->index_id) {
			glDeleteBuffers(1, &co->index_id);
		} if (co->vertex_id) {
			glDeleteBuffers(1, &co->vertex_id);
		}

		co->index_id = 0;
		co->vertex_id = 0;
		co->len = 0;
	}

	if (p_lines.size()) {
		PoolVector<float> geometry;
		PoolVector<uint16_t> indices;
		int lc = p_lines.size();

		geometry.resize(lc * 6);
		indices.resize(lc * 3);

		PoolVector<float>::Write vw = geometry.write();
		PoolVector<uint16_t>::Write iw = indices.write();

		PoolVector<Vector2>::Read lr = p_lines.read();

		const int POLY_HEIGHT = 16384;

		for (int i = 0; i < lc / 2; i++) {
			vw[i * 12 + 0] = lr[i * 2 + 0].x;
			vw[i * 12 + 1] = lr[i * 2 + 0].y;
			vw[i * 12 + 2] = POLY_HEIGHT;

			vw[i * 12 + 3] = lr[i * 2 + 1].x;
			vw[i * 12 + 4] = lr[i * 2 + 1].y;
			vw[i * 12 + 5] = POLY_HEIGHT;

			vw[i * 12 + 6] = lr[i * 2 + 1].x;
			vw[i * 12 + 7] = lr[i * 2 + 1].y;
			vw[i * 12 + 8] = -POLY_HEIGHT;

			vw[i * 12 + 9] = lr[i * 2 + 0].x;
			vw[i * 12 + 10] = lr[i * 2 + 0].y;
			vw[i * 12 + 11] = -POLY_HEIGHT;

			iw[i * 6 + 0] = i * 4 + 0;
			iw[i * 6 + 1] = i * 4 + 1;
			iw[i * 6 + 2] = i * 4 + 2;

			iw[i * 6 + 3] = i * 4 + 2;
			iw[i * 6 + 4] = i * 4 + 3;
			iw[i * 6 + 5] = i * 4 + 0;
		}

		//if same buffer len is being set, just use BufferSubData to avoid a pipeline flush

		if (!co->vertex_id) {
			glGenBuffers(1, &co->vertex_id);
			glBindBuffer(GL_ARRAY_BUFFER, co->vertex_id);
			glBufferData(GL_ARRAY_BUFFER, lc * 6 * sizeof(real_t), vw.ptr(), GL_STATIC_DRAW);
		} else {
			glBindBuffer(GL_ARRAY_BUFFER, co->vertex_id);
			glBufferSubData(GL_ARRAY_BUFFER, 0, lc * 6 * sizeof(real_t), vw.ptr());
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind

		if (!co->index_id) {
			glGenBuffers(1, &co->index_id);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, co->index_id);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, lc * 3 * sizeof(uint16_t), iw.ptr(), GL_DYNAMIC_DRAW);
		} else {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, co->index_id);
			glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, lc * 3 * sizeof(uint16_t), iw.ptr());
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); //unbind

		co->len = lc;
	}
}
*/

RS::InstanceType RasterizerStorageGLES3::get_base_type(RID p_rid) const {
	return RS::INSTANCE_NONE;

	/*
	if (mesh_owner.owns(p_rid)) {
		return RS::INSTANCE_MESH;
	} else if (light_owner.owns(p_rid)) {
		return RS::INSTANCE_LIGHT;
	} else if (multimesh_owner.owns(p_rid)) {
		return RS::INSTANCE_MULTIMESH;
	} else if (immediate_owner.owns(p_rid)) {
		return RS::INSTANCE_IMMEDIATE;
	} else if (reflection_probe_owner.owns(p_rid)) {
		return RS::INSTANCE_REFLECTION_PROBE;
	} else if (lightmap_capture_data_owner.owns(p_rid)) {
		return RS::INSTANCE_LIGHTMAP_CAPTURE;
	} else {
		return RS::INSTANCE_NONE;
	}
*/
}

bool RasterizerStorageGLES3::free(RID p_rid) {
	if (GLES3::TextureStorage::get_singleton()->owns_render_target(p_rid)) {
		GLES3::TextureStorage::get_singleton()->render_target_free(p_rid);
		return true;
	} else if (GLES3::TextureStorage::get_singleton()->owns_texture(p_rid)) {
		GLES3::TextureStorage::get_singleton()->texture_free(p_rid);
		return true;
	} else if (GLES3::TextureStorage::get_singleton()->owns_canvas_texture(p_rid)) {
		GLES3::TextureStorage::get_singleton()->canvas_texture_free(p_rid);
		return true;
	} else if (sky_owner.owns(p_rid)) {
		Sky *sky = sky_owner.get_or_null(p_rid);
		sky_set_texture(p_rid, RID(), 256);
		sky_owner.free(p_rid);
		memdelete(sky);

		return true;
	} else if (GLES3::MaterialStorage::get_singleton()->owns_shader(p_rid)) {
		GLES3::MaterialStorage::get_singleton()->shader_free(p_rid);
		return true;
	} else if (GLES3::MaterialStorage::get_singleton()->owns_material(p_rid)) {
		GLES3::MaterialStorage::get_singleton()->material_free(p_rid);
		return true;
	} else {
		return false;
	}
	/*
	} else if (skeleton_owner.owns(p_rid)) {
		Skeleton *s = skeleton_owner.get_or_null(p_rid);

		if (s->update_list.in_list()) {
			skeleton_update_list.remove(&s->update_list);
		}

		for (Set<InstanceBaseDependency *>::Element *E = s->instances.front(); E; E = E->next()) {
			E->get()->skeleton = RID();
		}

		skeleton_allocate(p_rid, 0, false);

		if (s->tex_id) {
			glDeleteTextures(1, &s->tex_id);
		}

		skeleton_owner.free(p_rid);
		memdelete(s);

		return true;
	} else if (mesh_owner.owns(p_rid)) {
		Mesh *mesh = mesh_owner.get_or_null(p_rid);

		mesh->instance_remove_deps();
		mesh_clear(p_rid);

		while (mesh->multimeshes.first()) {
			MultiMesh *multimesh = mesh->multimeshes.first()->self();
			multimesh->mesh = RID();
			multimesh->dirty_aabb = true;

			mesh->multimeshes.remove(mesh->multimeshes.first());

			if (!multimesh->update_list.in_list()) {
				multimesh_update_list.add(&multimesh->update_list);
			}
		}

		mesh_owner.free(p_rid);
		memdelete(mesh);

		return true;
	} else if (multimesh_owner.owns(p_rid)) {
		MultiMesh *multimesh = multimesh_owner.get_or_null(p_rid);
		multimesh->instance_remove_deps();

		if (multimesh->mesh.is_valid()) {
			Mesh *mesh = mesh_owner.get_or_null(multimesh->mesh);
			if (mesh) {
				mesh->multimeshes.remove(&multimesh->mesh_list);
			}
		}

		multimesh_allocate(p_rid, 0, RS::MULTIMESH_TRANSFORM_3D, RS::MULTIMESH_COLOR_NONE);

		update_dirty_multimeshes();

		multimesh_owner.free(p_rid);
		memdelete(multimesh);

		return true;
	} else if (immediate_owner.owns(p_rid)) {
		Immediate *im = immediate_owner.get_or_null(p_rid);
		im->instance_remove_deps();

		immediate_owner.free(p_rid);
		memdelete(im);

		return true;
	} else if (light_owner.owns(p_rid)) {
		Light *light = light_owner.get_or_null(p_rid);
		light->instance_remove_deps();

		light_owner.free(p_rid);
		memdelete(light);

		return true;
	} else if (reflection_probe_owner.owns(p_rid)) {
		// delete the texture
		ReflectionProbe *reflection_probe = reflection_probe_owner.get_or_null(p_rid);
		reflection_probe->instance_remove_deps();

		reflection_probe_owner.free(p_rid);
		memdelete(reflection_probe);

		return true;
	} else if (lightmap_capture_data_owner.owns(p_rid)) {
		// delete the texture
		LightmapCapture *lightmap_capture = lightmap_capture_data_owner.get_or_null(p_rid);
		lightmap_capture->instance_remove_deps();

		lightmap_capture_data_owner.free(p_rid);
		memdelete(lightmap_capture);
		return true;

	} else if (canvas_occluder_owner.owns(p_rid)) {
		CanvasOccluder *co = canvas_occluder_owner.get_or_null(p_rid);
		if (co->index_id) {
			glDeleteBuffers(1, &co->index_id);
		}
		if (co->vertex_id) {
			glDeleteBuffers(1, &co->vertex_id);
		}

		canvas_occluder_owner.free(p_rid);
		memdelete(co);

		return true;

	} else if (canvas_light_shadow_owner.owns(p_rid)) {
		CanvasLightShadow *cls = canvas_light_shadow_owner.get_or_null(p_rid);
		glDeleteFramebuffers(1, &cls->fbo);
		glDeleteRenderbuffers(1, &cls->depth);
		glDeleteTextures(1, &cls->distance);
		canvas_light_shadow_owner.free(p_rid);
		memdelete(cls);

		return true;
		*/
}

bool RasterizerStorageGLES3::has_os_feature(const String &p_feature) const {
	if (p_feature == "s3tc") {
		return config->s3tc_supported;
	}

	if (p_feature == "etc") {
		return config->etc_supported;
	}

	if (p_feature == "skinning_fallback") {
		return config->use_skeleton_software;
	}

	return false;
}

////////////////////////////////////////////

void RasterizerStorageGLES3::set_debug_generate_wireframes(bool p_generate) {
}

//void RasterizerStorageGLES3::render_info_begin_capture() {
//	info.snap = info.render;
//}

//void RasterizerStorageGLES3::render_info_end_capture() {
//	info.snap.object_count = info.render.object_count - info.snap.object_count;
//	info.snap.draw_call_count = info.render.draw_call_count - info.snap.draw_call_count;
//	info.snap.material_switch_count = info.render.material_switch_count - info.snap.material_switch_count;
//	info.snap.surface_switch_count = info.render.surface_switch_count - info.snap.surface_switch_count;
//	info.snap.shader_rebind_count = info.render.shader_rebind_count - info.snap.shader_rebind_count;
//	info.snap.vertices_count = info.render.vertices_count - info.snap.vertices_count;
//	info.snap._2d_item_count = info.render._2d_item_count - info.snap._2d_item_count;
//	info.snap._2d_draw_call_count = info.render._2d_draw_call_count - info.snap._2d_draw_call_count;
//}

//int RasterizerStorageGLES3::get_captured_render_info(RS::RenderInfo p_info) {
//	switch (p_info) {
//		case RS::INFO_OBJECTS_IN_FRAME: {
//			return info.snap.object_count;
//		} break;
//		case RS::INFO_VERTICES_IN_FRAME: {
//			return info.snap.vertices_count;
//		} break;
//		case RS::INFO_MATERIAL_CHANGES_IN_FRAME: {
//			return info.snap.material_switch_count;
//		} break;
//		case RS::INFO_SHADER_CHANGES_IN_FRAME: {
//			return info.snap.shader_rebind_count;
//		} break;
//		case RS::INFO_SURFACE_CHANGES_IN_FRAME: {
//			return info.snap.surface_switch_count;
//		} break;
//		case RS::INFO_DRAW_CALLS_IN_FRAME: {
//			return info.snap.draw_call_count;
//		} break;
//			/*
//		case RS::INFO_2D_ITEMS_IN_FRAME: {
//			return info.snap._2d_item_count;
//		} break;
//		case RS::INFO_2D_DRAW_CALLS_IN_FRAME: {
//			return info.snap._2d_draw_call_count;
//		} break;
//			*/
//		default: {
//			return get_render_info(p_info);
//		}
//	}
//}

//int RasterizerStorageGLES3::get_render_info(RS::RenderInfo p_info) {
//	switch (p_info) {
//		case RS::INFO_OBJECTS_IN_FRAME:
//			return info.render_final.object_count;
//		case RS::INFO_VERTICES_IN_FRAME:
//			return info.render_final.vertices_count;
//		case RS::INFO_MATERIAL_CHANGES_IN_FRAME:
//			return info.render_final.material_switch_count;
//		case RS::INFO_SHADER_CHANGES_IN_FRAME:
//			return info.render_final.shader_rebind_count;
//		case RS::INFO_SURFACE_CHANGES_IN_FRAME:
//			return info.render_final.surface_switch_count;
//		case RS::INFO_DRAW_CALLS_IN_FRAME:
//			return info.render_final.draw_call_count;
//			/*
//		case RS::INFO_2D_ITEMS_IN_FRAME:
//			return info.render_final._2d_item_count;
//		case RS::INFO_2D_DRAW_CALLS_IN_FRAME:
//			return info.render_final._2d_draw_call_count;
//*/
//		case RS::INFO_USAGE_VIDEO_MEM_TOTAL:
//			return 0; //no idea
//		case RS::INFO_VIDEO_MEM_USED:
//			return info.vertex_mem + info.texture_mem;
//		case RS::INFO_TEXTURE_MEM_USED:
//			return info.texture_mem;
//		case RS::INFO_VERTEX_MEM_USED:
//			return info.vertex_mem;
//		default:
//			return 0; //no idea either
//	}
//}

String RasterizerStorageGLES3::get_video_adapter_name() const {
	return (const char *)glGetString(GL_RENDERER);
}

String RasterizerStorageGLES3::get_video_adapter_vendor() const {
	return (const char *)glGetString(GL_VENDOR);
}

RenderingDevice::DeviceType RasterizerStorageGLES3::get_video_adapter_type() const {
	return RenderingDevice::DeviceType::DEVICE_TYPE_OTHER;
}

void RasterizerStorageGLES3::initialize() {
	config = GLES3::Config::get_singleton();

	//picky requirements for these
	config->support_shadow_cubemaps = config->support_depth_texture && config->support_write_depth && config->support_depth_cubemaps;

	// the use skeleton software path should be used if either float texture is not supported,
	// OR max_vertex_texture_image_units is zero
	config->use_skeleton_software = (config->float_texture_supported == false) || (config->max_vertex_texture_image_units == 0);

	{
		// quad for copying stuff

		glGenBuffers(1, &resources.quadie);
		glBindBuffer(GL_ARRAY_BUFFER, resources.quadie);
		{
			const float qv[16] = {
				-1,
				-1,
				0,
				0,
				-1,
				1,
				0,
				1,
				1,
				1,
				1,
				1,
				1,
				-1,
				1,
				0,
			};

			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 16, qv, GL_STATIC_DRAW);
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	{
		//default textures

		glGenTextures(1, &resources.white_tex);
		unsigned char whitetexdata[8 * 8 * 3];
		for (int i = 0; i < 8 * 8 * 3; i++) {
			whitetexdata[i] = 255;
		}

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, resources.white_tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 8, 8, 0, GL_RGB, GL_UNSIGNED_BYTE, whitetexdata);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenTextures(1, &resources.black_tex);
		unsigned char blacktexdata[8 * 8 * 3];
		for (int i = 0; i < 8 * 8 * 3; i++) {
			blacktexdata[i] = 0;
		}

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, resources.black_tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 8, 8, 0, GL_RGB, GL_UNSIGNED_BYTE, blacktexdata);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenTextures(1, &resources.normal_tex);
		unsigned char normaltexdata[8 * 8 * 3];
		for (int i = 0; i < 8 * 8 * 3; i += 3) {
			normaltexdata[i + 0] = 128;
			normaltexdata[i + 1] = 128;
			normaltexdata[i + 2] = 255;
		}

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, resources.normal_tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 8, 8, 0, GL_RGB, GL_UNSIGNED_BYTE, normaltexdata);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenTextures(1, &resources.aniso_tex);
		unsigned char anisotexdata[8 * 8 * 3];
		for (int i = 0; i < 8 * 8 * 3; i += 3) {
			anisotexdata[i + 0] = 255;
			anisotexdata[i + 1] = 128;
			anisotexdata[i + 2] = 0;
		}

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, resources.aniso_tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 8, 8, 0, GL_RGB, GL_UNSIGNED_BYTE, anisotexdata);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	// skeleton buffer
	{
		resources.skeleton_transform_buffer_size = 0;
		glGenBuffers(1, &resources.skeleton_transform_buffer);
	}

	// radical inverse vdc cache texture
	// used for cubemap filtering
	if (true /*||config->float_texture_supported*/) { //uint8 is similar and works everywhere
		glGenTextures(1, &resources.radical_inverse_vdc_cache_tex);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, resources.radical_inverse_vdc_cache_tex);

		uint8_t radical_inverse[512];

		for (uint32_t i = 0; i < 512; i++) {
			uint32_t bits = i;

			bits = (bits << 16) | (bits >> 16);
			bits = ((bits & 0x55555555) << 1) | ((bits & 0xAAAAAAAA) >> 1);
			bits = ((bits & 0x33333333) << 2) | ((bits & 0xCCCCCCCC) >> 2);
			bits = ((bits & 0x0F0F0F0F) << 4) | ((bits & 0xF0F0F0F0) >> 4);
			bits = ((bits & 0x00FF00FF) << 8) | ((bits & 0xFF00FF00) >> 8);

			float value = float(bits) * 2.3283064365386963e-10;
			radical_inverse[i] = uint8_t(CLAMP(value * 255.0, 0, 255));
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 512, 1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, radical_inverse);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //need this for proper sampling

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	{
		glGenFramebuffers(1, &resources.mipmap_blur_fbo);
		glGenTextures(1, &resources.mipmap_blur_color);
	}

#ifdef GLES_OVER_GL
	//this needs to be enabled manually in OpenGL 2.1

	if (config->extensions.has("GL_ARB_seamless_cube_map")) {
		glEnable(_EXT_TEXTURE_CUBE_MAP_SEAMLESS);
	}
	glEnable(GL_POINT_SPRITE);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
#endif
}

void RasterizerStorageGLES3::finalize() {
}

void RasterizerStorageGLES3::_copy_screen() {
	bind_quad_array();
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void RasterizerStorageGLES3::update_memory_info() {
}

uint64_t RasterizerStorageGLES3::get_rendering_info(RS::RenderingInfo p_info) {
	return 0;
}

void RasterizerStorageGLES3::update_dirty_resources() {
	GLES3::MaterialStorage::get_singleton()->update_dirty_shaders();
	GLES3::MaterialStorage::get_singleton()->update_dirty_materials();
	//	update_dirty_skeletons();
	//	update_dirty_multimeshes();
}

RasterizerStorageGLES3::RasterizerStorageGLES3() {
	initialize();
}

RasterizerStorageGLES3::~RasterizerStorageGLES3() {
}

#endif // GLES3_ENABLED
