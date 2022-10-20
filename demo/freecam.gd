extends Camera3D

var move_delta : Vector3 = Vector3.ZERO
var look_delta : Vector3 = Vector3.ZERO

func _process(delta):
	position += quaternion * move_delta * delta
	rotation -= look_delta * delta * 0.5
	
	look_delta = Vector3.ZERO

func _input(event):
	move_delta = Vector3.ZERO
	move_delta.x = Input.get_axis("Left", "Right")
	move_delta.z = Input.get_axis("Forward", "Back")
	
	if event is InputEventMouseMotion:
		look_delta = Vector3(event.relative.y, event.relative.x, 0)
