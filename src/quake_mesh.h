#ifndef GDQUAKE_QUAKE_MESH_H
#define GDQUAKE_QUAKE_MESH_H

#include <cstdint>
#include <cstdio>

#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/array_mesh.hpp>

// Avoid "using namespace" in headers!
namespace godot::gdquake {
	namespace internal {
		typedef float MdlFVec2_t[2];
		typedef float MdlFVec3_t[3];

		typedef unsigned char MdlBVec3_t[3];
		typedef uint32_t MdlUVec3_t[3];

		typedef char MdlFrameName_t[16];

		typedef struct MdlTexCoord {
			uint32_t on_seam;
			uint32_t u;
			uint32_t v;
		} MdlTexCoord_t;

		typedef struct MdlTriangle {
			uint32_t facing_front;
			MdlUVec3_t indices;
		} MdlTriangle_t;

		typedef struct MdlVertex {
		   MdlBVec3_t coords;
		   unsigned char index_normal;
		} MdlVertex_t;

		typedef struct MdlHeader {
			char ident[4];
			uint32_t version;

			MdlFVec3_t scale;
   			MdlFVec3_t translation;
   			float radius;
   			MdlFVec3_t eyepos;

   			uint32_t num_skins;
   			uint32_t skinwidth;
   			uint32_t skinheight;

   			uint32_t num_verts;
   			uint32_t num_tris;
   			uint32_t num_frames;

   			uint32_t synctype;
   			uint32_t flags;
   			float size;
		} MdlHeader_t;

		typedef struct MdlFrameData { 
			MdlVertex_t boxmin;
   			MdlVertex_t boxmax;
   			MdlFrameName_t name;
		} MdlFrameData_t;

		typedef struct MdlSimpleFrame {
		   MdlFrameData_t data;
		} MdlSimpleFrame_t;
	}

    class QuakePakFile;

    // An extension of Godot's normal mesh data, must be loaded from a QuakePakFile instance
	// This stores frames internally as meshes
    // Uses SurfaceTool to create the data properly, stores skins under it as an array of "QuakeSkinTexture"s
    class QuakeMesh : public Mesh {
		GDCLASS(QuakeMesh, Mesh);

	protected:
		static void _bind_methods();

		void _notification(int p_what);
		bool _set(const StringName &p_name, const Variant &p_value);
		bool _get(const StringName &p_name, Variant &r_ret) const;
		void _get_property_list(List<PropertyInfo> *p_list) const;
		bool _property_can_revert(const StringName &p_name) const;
		bool _property_get_revert(const StringName &p_name, Variant &r_property) const;

		String _to_string() const;

    public:
        QuakeMesh();
        ~QuakeMesh();

		int64_t _get_surface_count() const override;
		int64_t _get_blend_shape_count() const override;
		
		int vert_count;
		int tri_count;
		int active_frame = 0;
		TypedArray<Mesh> frames {};

		bool load_from_file(QuakePakFile* file);
		Mesh* get_current_frame();
    };
}

#endif // GDQUAKE_QUAKE_MESH_H
