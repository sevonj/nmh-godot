extends Node3D

var models: Array[Node] = []

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	get_viewport().files_dropped.connect(on_files_dropped)

func on_files_dropped(files: Array[String]):
	_clear()

	for file in files:
		if file.to_lower().ends_with(".gm2"):
			var gmf2 := GMF2.new()
			gmf2.open(file)
			add_child(gmf2)
			models.append(gmf2)
			continue
		if file.to_lower().ends_with(".gcl"):
			var flcg := FLCG.new()
			flcg.open(file)
			add_child(flcg)
			models.append(flcg)
			continue
		if file.to_lower().ends_with(".rsl"):
			var rmhg := RMHG.new()
			rmhg.open(file)
			var strings := rmhg.get_strings()
			print(strings)
			for string in strings:
				print("S: ", string)
			continue

func _clear() -> void:
	for model in models:
		model.free()
	models.clear()

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(_delta: float) -> void:
	if Input.is_key_pressed(KEY_P):
		get_tree().change_scene_to_file("res://scenes/world.tscn")
