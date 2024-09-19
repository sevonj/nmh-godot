extends Node3D

var _mouse_lock := false:
	set(value):
		_mouse_lock = value
		if _mouse_lock:
			Input.mouse_mode = Input.MOUSE_MODE_CAPTURED
		else:
			Input.mouse_mode = Input.MOUSE_MODE_VISIBLE

var _chunklist: Array[String] = [
	"AREA000",
	"AREA001",
	"AREA002",
	"AREA003",
	"AREA004",
	"AREA005",
	"AREA006",
	"AREA007",
	"AREA008",
	"AREA009",
	"AREA010",
	"AREA011",
	"AREA012",
	"AREA013",
	"AREA014",
	"AREA015",
	"AREA016",
	"AREA017",
	"AREA018",
	"AREA019",
	"AREA020",
	"AREA021",
	"AREA022",
	"AREA023",
	"AREA024",
	"AREA025",
	"AREA026",
	"AREA027",
	"AREA028",
	"AREA029",
	"AREA030",
	"AREA031",
	"AREA032",
	"AREA033",
	"AREA034",
	"AREA035",
	"AREA036",
	"AREA037",
	"AREA038",
	"AREA039",
	"AREA040",
	"AREA041",
	"AREA042",
	"AREA043"
]

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	_load_world()
	$ui/err_cant_find_files/MarginContainer/vbox/CenterContainer/retry.pressed.connect(get_tree().reload_current_scene)

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	if Input.is_action_just_pressed("ui_cancel"):
		_mouse_lock = !_mouse_lock


func _load_world() -> void:
	var basepath := OS.get_executable_path().get_base_dir() + "/"

	for chunkname in _chunklist:
		var filepath := "gamedata/STG_HI/" + chunkname
		var globalpath := basepath + filepath
		# globalpath = basepath + "nmh-godot/" + filepath

		if not FileAccess.file_exists(globalpath + ".GM2"):
			_on_load_fail(globalpath + ".GM2")
			break
		if not FileAccess.file_exists(globalpath + "COL.GCL"):
			_on_load_fail(globalpath + "COL.GCL")
			break

		var gmf2 := GMF2.new()
		var flcg := FLCG.new()
		gmf2.open(globalpath + ".GM2")
		flcg.open(globalpath + "COL.GCL")
		$static.add_child(gmf2)
		$static.add_child(flcg)

		var flcg_children = flcg.get_children()
		while not flcg_children.is_empty():
			var new_children = []
			for child in flcg_children:
				if child is MeshInstance3D:
					child.material_override = preload("res://assets/materials/mat_wireframe.tres")
				new_children.append_array(child.get_children())
			flcg_children = new_children

		var camera := CameraRigTP.new()
		var player := $entities/spawn/Player
		$entities.add_child(camera)
		camera.target = player.camera_target
		camera.enable_input = true
		player.camerarig = camera
		_mouse_lock = true



func _on_load_fail(file: String) -> void:
	$ui/err_cant_find_files.show()
	$ui/err_cant_find_files/MarginContainer/vbox/msg.text += "\nThis particular file failed:\n"+file
