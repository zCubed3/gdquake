#ifndef GDQUAKE_QUAKE_PAKFILE_H
#define GDQUAKE_QUAKE_PAKFILE_H

#include <cstdint>
#include <cstdio>

#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/object.hpp>

// Avoid "using namespace" in headers!
namespace godot::gdquake {
	namespace internal {
		// C-style structs to make reading Quake data easier, this is not passed to Godot as-is, we make it safer!
		// TODO: Big endian?
		typedef struct PakHeader {
			char ident[4];
			uint32_t offset;
			uint32_t size;
		} PakHeader_t;

		typedef struct PakEntry {
			char path[56];
			uint32_t offset;
			uint32_t size;
		} PakEntry_t;
	}

	class QuakeMesh;

	class QuakePakEntry : public Object {
		GDCLASS(QuakePakEntry, Object);

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
		String path = "";
		uint32_t offset = 0;
		uint32_t size = 0;

		QuakePakEntry();
		~QuakePakEntry();

		String get_path() const;
	};

	// The C <-> Godot glue for loading Quake PAK files
	class QuakePakFile : public Object {
		GDCLASS(QuakePakFile, Object);

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
		QuakePakFile();
		~QuakePakFile();

		void load_file(const String& path);
		
		TypedArray<QuakePakEntry> get_entry_array() const;
		
		QuakePakEntry* get_entry(const String& path);

		QuakeMesh* load_mdl(const String& path);

	public:
		// For safety reasons the C style data-types are not persistent outside of their respective functions
		// Minus the file stream, that is necessary for streaming assets in a runtime without loading all ahead of time

		TypedArray<QuakePakEntry> entry_array {};
		FILE* file = nullptr;
	};
}

#endif // GDQUAKE_QUAKE_PAKFILE_H
