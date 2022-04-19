/*************************************************************************/
/*  renderer_storage_rd.h                                                */
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

#ifndef RENDERING_SERVER_STORAGE_RD_H
#define RENDERING_SERVER_STORAGE_RD_H

#include "core/templates/list.h"
#include "core/templates/local_vector.h"
#include "core/templates/rid_owner.h"
#include "servers/rendering/renderer_compositor.h"
#include "servers/rendering/renderer_rd/effects_rd.h"
#include "servers/rendering/renderer_rd/shaders/voxel_gi_sdf.glsl.gen.h"
#include "servers/rendering/renderer_rd/storage_rd/material_storage.h"
#include "servers/rendering/renderer_scene_render.h"
#include "servers/rendering/rendering_device.h"
#include "servers/rendering/shader_compiler.h"

class RendererStorageRD : public RendererStorage {
public:
	static _FORCE_INLINE_ void store_transform(const Transform3D &p_mtx, float *p_array) {
		p_array[0] = p_mtx.basis.elements[0][0];
		p_array[1] = p_mtx.basis.elements[1][0];
		p_array[2] = p_mtx.basis.elements[2][0];
		p_array[3] = 0;
		p_array[4] = p_mtx.basis.elements[0][1];
		p_array[5] = p_mtx.basis.elements[1][1];
		p_array[6] = p_mtx.basis.elements[2][1];
		p_array[7] = 0;
		p_array[8] = p_mtx.basis.elements[0][2];
		p_array[9] = p_mtx.basis.elements[1][2];
		p_array[10] = p_mtx.basis.elements[2][2];
		p_array[11] = 0;
		p_array[12] = p_mtx.origin.x;
		p_array[13] = p_mtx.origin.y;
		p_array[14] = p_mtx.origin.z;
		p_array[15] = 1;
	}

	static _FORCE_INLINE_ void store_basis_3x4(const Basis &p_mtx, float *p_array) {
		p_array[0] = p_mtx.elements[0][0];
		p_array[1] = p_mtx.elements[1][0];
		p_array[2] = p_mtx.elements[2][0];
		p_array[3] = 0;
		p_array[4] = p_mtx.elements[0][1];
		p_array[5] = p_mtx.elements[1][1];
		p_array[6] = p_mtx.elements[2][1];
		p_array[7] = 0;
		p_array[8] = p_mtx.elements[0][2];
		p_array[9] = p_mtx.elements[1][2];
		p_array[10] = p_mtx.elements[2][2];
		p_array[11] = 0;
	}

	static _FORCE_INLINE_ void store_transform_3x3(const Basis &p_mtx, float *p_array) {
		p_array[0] = p_mtx.elements[0][0];
		p_array[1] = p_mtx.elements[1][0];
		p_array[2] = p_mtx.elements[2][0];
		p_array[3] = 0;
		p_array[4] = p_mtx.elements[0][1];
		p_array[5] = p_mtx.elements[1][1];
		p_array[6] = p_mtx.elements[2][1];
		p_array[7] = 0;
		p_array[8] = p_mtx.elements[0][2];
		p_array[9] = p_mtx.elements[1][2];
		p_array[10] = p_mtx.elements[2][2];
		p_array[11] = 0;
	}

	static _FORCE_INLINE_ void store_transform_transposed_3x4(const Transform3D &p_mtx, float *p_array) {
		p_array[0] = p_mtx.basis.elements[0][0];
		p_array[1] = p_mtx.basis.elements[0][1];
		p_array[2] = p_mtx.basis.elements[0][2];
		p_array[3] = p_mtx.origin.x;
		p_array[4] = p_mtx.basis.elements[1][0];
		p_array[5] = p_mtx.basis.elements[1][1];
		p_array[6] = p_mtx.basis.elements[1][2];
		p_array[7] = p_mtx.origin.y;
		p_array[8] = p_mtx.basis.elements[2][0];
		p_array[9] = p_mtx.basis.elements[2][1];
		p_array[10] = p_mtx.basis.elements[2][2];
		p_array[11] = p_mtx.origin.z;
	}

	static _FORCE_INLINE_ void store_camera(const CameraMatrix &p_mtx, float *p_array) {
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				p_array[i * 4 + j] = p_mtx.matrix[i][j];
			}
		}
	}

	static _FORCE_INLINE_ void store_soft_shadow_kernel(const float *p_kernel, float *p_array) {
		for (int i = 0; i < 128; i++) {
			p_array[i] = p_kernel[i];
		}
	}

private:
	/* FOG VOLUMES */

	struct FogVolume {
		RID material;
		Vector3 extents = Vector3(1, 1, 1);

		RS::FogVolumeShape shape = RS::FOG_VOLUME_SHAPE_BOX;

		Dependency dependency;
	};

	mutable RID_Owner<FogVolume, true> fog_volume_owner;

	/* visibility_notifier */

	struct VisibilityNotifier {
		AABB aabb;
		Callable enter_callback;
		Callable exit_callback;
		Dependency dependency;
	};

	mutable RID_Owner<VisibilityNotifier> visibility_notifier_owner;

	/* VOXEL GI */

	struct VoxelGI {
		RID octree_buffer;
		RID data_buffer;
		RID sdf_texture;

		uint32_t octree_buffer_size = 0;
		uint32_t data_buffer_size = 0;

		Vector<int> level_counts;

		int cell_count = 0;

		Transform3D to_cell_xform;
		AABB bounds;
		Vector3i octree_size;

		float dynamic_range = 2.0;
		float energy = 1.0;
		float bias = 1.4;
		float normal_bias = 0.0;
		float propagation = 0.7;
		bool interior = false;
		bool use_two_bounces = false;

		float anisotropy_strength = 0.5;

		uint32_t version = 1;
		uint32_t data_version = 1;

		Dependency dependency;
	};

	mutable RID_Owner<VoxelGI, true> voxel_gi_owner;

	/* EFFECTS */

	EffectsRD *effects = nullptr;

public:
	//internal usage

	void base_update_dependency(RID p_base, DependencyTracker *p_instance);

	/* VOXEL GI API */

	RID voxel_gi_allocate();
	void voxel_gi_initialize(RID p_voxel_gi);

	void voxel_gi_allocate_data(RID p_voxel_gi, const Transform3D &p_to_cell_xform, const AABB &p_aabb, const Vector3i &p_octree_size, const Vector<uint8_t> &p_octree_cells, const Vector<uint8_t> &p_data_cells, const Vector<uint8_t> &p_distance_field, const Vector<int> &p_level_counts);

	AABB voxel_gi_get_bounds(RID p_voxel_gi) const;
	Vector3i voxel_gi_get_octree_size(RID p_voxel_gi) const;
	Vector<uint8_t> voxel_gi_get_octree_cells(RID p_voxel_gi) const;
	Vector<uint8_t> voxel_gi_get_data_cells(RID p_voxel_gi) const;
	Vector<uint8_t> voxel_gi_get_distance_field(RID p_voxel_gi) const;

	Vector<int> voxel_gi_get_level_counts(RID p_voxel_gi) const;
	Transform3D voxel_gi_get_to_cell_xform(RID p_voxel_gi) const;

	void voxel_gi_set_dynamic_range(RID p_voxel_gi, float p_range);
	float voxel_gi_get_dynamic_range(RID p_voxel_gi) const;

	void voxel_gi_set_propagation(RID p_voxel_gi, float p_range);
	float voxel_gi_get_propagation(RID p_voxel_gi) const;

	void voxel_gi_set_energy(RID p_voxel_gi, float p_energy);
	float voxel_gi_get_energy(RID p_voxel_gi) const;

	void voxel_gi_set_bias(RID p_voxel_gi, float p_bias);
	float voxel_gi_get_bias(RID p_voxel_gi) const;

	void voxel_gi_set_normal_bias(RID p_voxel_gi, float p_range);
	float voxel_gi_get_normal_bias(RID p_voxel_gi) const;

	void voxel_gi_set_interior(RID p_voxel_gi, bool p_enable);
	bool voxel_gi_is_interior(RID p_voxel_gi) const;

	void voxel_gi_set_use_two_bounces(RID p_voxel_gi, bool p_enable);
	bool voxel_gi_is_using_two_bounces(RID p_voxel_gi) const;

	void voxel_gi_set_anisotropy_strength(RID p_voxel_gi, float p_strength);
	float voxel_gi_get_anisotropy_strength(RID p_voxel_gi) const;

	uint32_t voxel_gi_get_version(RID p_probe);
	uint32_t voxel_gi_get_data_version(RID p_probe);

	RID voxel_gi_get_octree_buffer(RID p_voxel_gi) const;
	RID voxel_gi_get_data_buffer(RID p_voxel_gi) const;

	RID voxel_gi_get_sdf_texture(RID p_voxel_gi);

	/* FOG VOLUMES */

	virtual RID fog_volume_allocate();
	virtual void fog_volume_initialize(RID p_rid);

	virtual void fog_volume_set_shape(RID p_fog_volume, RS::FogVolumeShape p_shape);
	virtual void fog_volume_set_extents(RID p_fog_volume, const Vector3 &p_extents);
	virtual void fog_volume_set_material(RID p_fog_volume, RID p_material);
	virtual RS::FogVolumeShape fog_volume_get_shape(RID p_fog_volume) const;
	virtual RID fog_volume_get_material(RID p_fog_volume) const;
	virtual AABB fog_volume_get_aabb(RID p_fog_volume) const;
	virtual Vector3 fog_volume_get_extents(RID p_fog_volume) const;

	/* VISIBILITY NOTIFIER */

	virtual RID visibility_notifier_allocate();
	virtual void visibility_notifier_initialize(RID p_notifier);
	virtual void visibility_notifier_set_aabb(RID p_notifier, const AABB &p_aabb);
	virtual void visibility_notifier_set_callbacks(RID p_notifier, const Callable &p_enter_callbable, const Callable &p_exit_callable);

	virtual AABB visibility_notifier_get_aabb(RID p_notifier) const;
	virtual void visibility_notifier_call(RID p_notifier, bool p_enter, bool p_deferred);

	RS::InstanceType get_base_type(RID p_rid) const;

	bool free(RID p_rid);

	bool has_os_feature(const String &p_feature) const;

	void update_dirty_resources();

	void set_debug_generate_wireframes(bool p_generate) {}

	//keep cached since it can be called form any thread
	uint64_t texture_mem_cache = 0;
	uint64_t buffer_mem_cache = 0;
	uint64_t total_mem_cache = 0;

	virtual void update_memory_info();
	virtual uint64_t get_rendering_info(RS::RenderingInfo p_info);

	String get_video_adapter_name() const;
	String get_video_adapter_vendor() const;
	RenderingDevice::DeviceType get_video_adapter_type() const;

	virtual void capture_timestamps_begin();
	virtual void capture_timestamp(const String &p_name);
	virtual uint32_t get_captured_timestamps_count() const;
	virtual uint64_t get_captured_timestamps_frame() const;
	virtual uint64_t get_captured_timestamp_gpu_time(uint32_t p_index) const;
	virtual uint64_t get_captured_timestamp_cpu_time(uint32_t p_index) const;
	virtual String get_captured_timestamp_name(uint32_t p_index) const;

	static RendererStorageRD *base_singleton;

	void init_effects(bool p_prefer_raster_effects);
	EffectsRD *get_effects();

	RendererStorageRD();
	~RendererStorageRD();
};

#endif // RASTERIZER_STORAGE_RD_H
