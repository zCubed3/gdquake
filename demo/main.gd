extends Node

func _ready():
	var pak = QuakePakFile.new()
	
	print(pak)
	
	pak.load("res://id1/pak0.pak")
	var arr : Array[QuakePakEntry] = pak.entries

	for ent in arr:
		print(ent.get_path())
