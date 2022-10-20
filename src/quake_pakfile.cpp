#include "quake_pakfile.h"

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/node3d.hpp>

#include <cstdio>
#include <string>

#include "quake_mesh.h"

// QuakePakEntry
namespace godot::gdquake {
    //
    // Godot functions
    //
    QuakePakEntry::QuakePakEntry() {

    }

    QuakePakEntry::~QuakePakEntry() {
    	UtilityFunctions::print("QuakePakEntry destroyed.");
    }

    void QuakePakEntry::_notification(int p_what) {

    }

    bool QuakePakEntry::_set(const StringName &p_name, const Variant &p_value) {
    	return false;
    }

    bool QuakePakEntry::_get(const StringName &p_name, Variant &r_ret) const {
    	return false;
    }

    String QuakePakEntry::_to_string() const {
    	return "<QuakePakEntry:" + itos(get_instance_id()) + ">";
    }

    void QuakePakEntry::_get_property_list(List<PropertyInfo> *p_list) const {

    }

    bool QuakePakEntry::_property_can_revert(const StringName &p_name) const {
        return false;
    };

    bool QuakePakEntry::_property_get_revert(const StringName &p_name, Variant &r_property) const {
    	return false;
    };

    void QuakePakEntry::_bind_methods() {
        // TODO: Is _get_property_list bugged?
        ClassDB::bind_method(D_METHOD("get_path"), &QuakePakEntry::get_path);
    }

    String QuakePakEntry::get_path() const {
        return path;
    }
}

// QuakePakFile
namespace godot::gdquake {
    //
    // Godot functions
    //
    QuakePakFile::QuakePakFile() {
    	UtilityFunctions::print("PakFile created.");
    }

    QuakePakFile::~QuakePakFile() {
        if (file != nullptr)
            fclose(file);

    	UtilityFunctions::print("PakFile destroyed.");
    }

    void QuakePakFile::_notification(int p_what) {

    }

    bool QuakePakFile::_set(const StringName &p_name, const Variant &p_value) {
    	return false;
    }

    bool QuakePakFile::_get(const StringName &p_name, Variant &r_ret) const {
    	return false;
    }

    String QuakePakFile::_to_string() const {
    	return "<QuakePakFile:#" + itos(get_instance_id()) + ">";
    }

    void QuakePakFile::_get_property_list(List<PropertyInfo> *p_list) const {

    }

    bool QuakePakFile::_property_can_revert(const StringName &p_name) const {
        return false;
    };

    bool QuakePakFile::_property_get_revert(const StringName &p_name, Variant &r_property) const {
    	return false;
    };

    void QuakePakFile::_bind_methods() {
        ClassDB::bind_method(D_METHOD("open", "path"), &QuakePakFile::load_file);

	    ClassDB::bind_method(D_METHOD("get_entry_array"), &QuakePakFile::get_entry_array);
        ClassDB::bind_method(D_METHOD("get_entry"), &QuakePakFile::get_entry);
        ClassDB::bind_method(D_METHOD("load_mdl"), &QuakePakFile::load_mdl);
	    //ClassDB::bind_method(D_METHOD("set_entries", "replacement"), &QuakePakFile::set_entries);
        //ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "entries"), "set_entries", "get_entries");
    }

    //
    // Pak loading
    //
    void QuakePakFile::load_file(const String& string) {
        ProjectSettings* settings = ProjectSettings::get_singleton();
        String absolute_path = settings->globalize_path(string);

        // We need to go around godot's normal way of loading
        // Therefore this is probably not threadsafe!
        // TODO: Thread safety?
        if (file != nullptr)
            fclose(file);

        file = fopen(absolute_path.ascii().get_data(), "rb");

        if (file == nullptr) {
            UtilityFunctions::printerr("Failed to open file at '" + absolute_path + "'!");
            return;
        }

        // TODO: Not crash the entire editor if a malformed PAK is loaded
        internal::PakHeader_t header;
        fread(&header, sizeof(internal::PakHeader_t), 1, file);

        size_t elem_size = sizeof(internal::PakEntry_t);
        size_t entry_count = header.size / elem_size;

        internal::PakEntry_t* entries = static_cast<internal::PakEntry_t*>(calloc(entry_count, elem_size));

        fseek(file, header.offset, SEEK_SET);
        fread(entries, elem_size, entry_count, file);

        // We need to convert the entries to godot safe types
        // This is because Godot tends to convert data internally to safer types, this just helps make that process easier
        // Plus this lets us access entries in GDScript :)
        entry_array.clear();
        entry_array.resize(entry_count);
        for (int e = 0; e < entry_count; e++) {
            QuakePakEntry* entry = memnew(QuakePakEntry);

            entry->path = entries[e].path;
            entry->offset = entries[e].offset;
            entry->size = entries[e].size;

            entry_array[e] = entry;
        }

        std::free(entries);
    }

    TypedArray<QuakePakEntry> QuakePakFile::get_entry_array() const {
        return entry_array;
    }

    QuakePakEntry* QuakePakFile::get_entry(const String& path) {
        for (int e = 0; e < entry_array.size(); e++) {
            QuakePakEntry* entry = cast_to<QuakePakEntry>(entry_array[e]);

            if (entry == nullptr)
                continue;

            if (entry->path == path)
                return entry;
        }

        return nullptr;
    }

    QuakeMesh* QuakePakFile::load_mdl(const String& path) {
        QuakePakEntry* entry = get_entry(path);

        if (entry == nullptr)
            return nullptr;

        fseek(file, entry->offset, SEEK_SET);
        
        QuakeMesh* mesh = memnew(QuakeMesh);
        if (!mesh->load_from_file(this)) {
            memdelete(mesh);
            return nullptr;
        }
        else
            return mesh;
    }
}