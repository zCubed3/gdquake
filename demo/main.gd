extends Node

func _ready():
	var pak := QuakePakFile.new()
	
	print(pak)
	
	pak.open("res://id1/pak0.pak")
	var arr := pak.get_entry_array()

	var shift = 0.0
	for ent in arr:
		var path = ent.get_path()
		#print(path)
	
		if ".mdl" in path:
			var node := MeshInstance3D.new()
			add_child(node)
			
			var mdl = pak.load_mdl(path)

			if mdl != null:
				node.mesh = mdl.get_current_frame()
				node.translate(Vector3.RIGHT * shift)
				shift += node.mesh.get_aabb().size.x * 2
