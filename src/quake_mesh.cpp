#include "quake_mesh.h"

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/surface_tool.hpp>

#include "quake_pakfile.h"

#include <vector>

namespace godot::gdquake {
    QuakeMesh::QuakeMesh() {

    }

    QuakeMesh::~QuakeMesh() {
    	UtilityFunctions::print("QuakeMesh destroyed.");
    }

    void QuakeMesh::_notification(int p_what) {

    }

    bool QuakeMesh::_set(const StringName &p_name, const Variant &p_value) {
    	return false;
    }

    bool QuakeMesh::_get(const StringName &p_name, Variant &r_ret) const {
    	return false;
    }

    String QuakeMesh::_to_string() const {
    	return "<QuakeMesh:" + itos(get_instance_id()) + ">";
    }

    void QuakeMesh::_get_property_list(List<PropertyInfo> *p_list) const {

    }

    bool QuakeMesh::_property_can_revert(const StringName &p_name) const {
        return false;
    };

    bool QuakeMesh::_property_get_revert(const StringName &p_name, Variant &r_property) const {
    	return false;
    };

    void QuakeMesh::_bind_methods() {
        ClassDB::bind_method(D_METHOD("get_current_frame"), &QuakeMesh::get_current_frame);
    }

    // We treat all the frames as 1 because only 1 can show at a time
    int64_t QuakeMesh::_get_surface_count() const {
        return 1;
    }

    // Quake meshes have no blendshapes, vertex animation doesn't count here!
    int64_t QuakeMesh::_get_blend_shape_count() const {
        return 0;
    }

    bool QuakeMesh::load_from_file(QuakePakFile* file) {
        if (file->file == nullptr) {
            UtilityFunctions::printerr("File stream ptr was null! Did you call open?");
            return false;
        }

        // We assume the file has already seeked to the entry location
        internal::MdlHeader_t header;
        fread(&header, sizeof(internal::MdlHeader_t), 1, file->file);

        // We actually have 2x the number of verts because of backface UVs
        vert_count = header.num_verts * 2; 

        // But this number can change based on how many backfaces there are
        tri_count = header.num_tris;

        // frame and skin count is based on the array lengths

        // Skins
        for (int s = 0; s < header.num_skins; s++) {
            uint32_t is_group;
            fread(&is_group, sizeof(uint32_t), 1, file->file);

            if (is_group) {
                UtilityFunctions::printerr("MEM LEAK!!! TODO: Read complex skins!");
                return false;
            } else {
                UtilityFunctions::print("TODO: Read simple skins!");
                fseek(file->file, header.skinwidth * header.skinheight, SEEK_CUR);
            }
        }

        // UVs
        internal::MdlTexCoord_t* quake_coords = static_cast<internal::MdlTexCoord_t*>(calloc(vert_count, sizeof(internal::MdlTexCoord_t)));
        fread(quake_coords, sizeof(internal::MdlTexCoord_t), header.num_verts, file->file);

        internal::MdlFVec2_t* tex_coords = static_cast<internal::MdlFVec2_t*>(calloc(vert_count, sizeof(internal::MdlFVec2_t)));

        for (int t = 0; t < header.num_verts; t++) {
            internal::MdlTexCoord_t fix_coord = quake_coords[t];
            internal::MdlFVec2_t precise = { (float)fix_coord.u, (float)fix_coord.v };

            tex_coords[t][0] = precise[0] / header.skinwidth;
            tex_coords[t][1] = precise[1] / header.skinheight;

            precise[0] += header.skinwidth / 2.0F;

            tex_coords[t + header.num_verts][0] = precise[0] / header.skinwidth;
            tex_coords[t + header.num_verts][1] = precise[1] / header.skinheight;
        }

        // Triangles
        internal::MdlTriangle_t* triangles = static_cast<internal::MdlTriangle_t*>(calloc(tri_count, sizeof(internal::MdlTriangle_t)));
        fread(triangles, sizeof(internal::MdlTriangle_t), tri_count, file->file);

        for (int t = 0; t < tri_count; t++)
            for (int v = 0; v < 3; v++)
                if (quake_coords[triangles[t].indices[v]].on_seam && !triangles->facing_front)
                    triangles[t].indices[v] += header.num_verts;

        // Frames
        for (int f = 0; f < header.num_frames; f++) {
            uint32_t is_complex;
            fread(&is_complex, sizeof(uint32_t), 1, file->file);

            SurfaceTool* tool = memnew(SurfaceTool);
            tool->begin(Mesh::PRIMITIVE_TRIANGLES);

            if (is_complex) {
                UtilityFunctions::printerr("MEM LEAK!!! TODO: Read complex frames!");
                return false;
            } else {
                internal::MdlFrameData_t frame_data;
                fread(&frame_data, sizeof(internal::MdlFrameData_t), 1, file->file);

                internal::MdlVertex_t* positions = static_cast<internal::MdlVertex_t*>(calloc(header.num_verts, sizeof(internal::MdlVertex_t)));
                fread(positions, sizeof(internal::MdlVertex_t), header.num_verts, file->file);

                for (int p = 0; p < vert_count; p++) {
                    int v = p;
                    if (v >= header.num_verts)
                        v -= header.num_verts;

                    internal::MdlBVec3_t b_vec = { positions[v].coords[0], positions[v].coords[1], positions[v].coords[2] };
                    Vector3 pos = Vector3((float)b_vec[1], (float)b_vec[2], (float)b_vec[0]) * Vector3(header.scale[1], header.scale[2], header.scale[0]);
                    pos += Vector3(header.translation[1], header.translation[2], header.translation[0]);
                    pos /= 100;

                    tool->set_uv(Vector2(tex_coords[p][0], tex_coords[p][1]));
                    tool->add_vertex(pos);
                }

                std::free(positions);
            }

            for (int t = 0; t < tri_count; t++)
                for (int v = 0; v < 3; v++)
                    tool->add_index(triangles[t].indices[v]);

            tool->generate_normals();

            Ref<ArrayMesh> mesh_ref = tool->commit();
            ArrayMesh* mesh = *mesh_ref;
            frames.push_back(mesh);

            memdelete(tool);
        }

        // Our active frame is 0 by default!

        // Cleanup
        std::free(triangles);
        std::free(quake_coords);
        std::free(tex_coords);

        return true;
    }

    Mesh* QuakeMesh::get_current_frame() {
        if (active_frame >= frames.size())
            active_frame = 0;

        return cast_to<ArrayMesh>(frames[active_frame]);
    }
}
