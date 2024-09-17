extends Node

@onready var _sidebar_cont := $hsplit/sidebar/sidebar_cont

@onready var _3d_root := $"hsplit/3d_panel/subviewport_cont/subviewport/3d_root"
@onready var _3d_cont := $"hsplit/3d_panel/subviewport_cont/subviewport/3d_root/loaded_content_container"

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	get_tree().root.get_viewport().files_dropped.connect(on_files_dropped)

func on_files_dropped(files: Array[String]):
	_clear()

	for file in files:
		if file.to_lower().ends_with(".gm2"):
			var gmf2 := GMF2.new()
			gmf2.open(file)
			_3d_cont.add_child(gmf2)
			continue

		if file.to_lower().ends_with(".gcl"):
			var flcg := FLCG.new()
			flcg.open(file)
			_3d_cont.add_child(flcg)
			continue

		if file.to_lower().ends_with(".rsl"):
			var rmhg := RMHG.new()
			rmhg.open(file)
			var tree := RMHGTree.new()
			tree.load_rmhg(rmhg)
			_sidebar_cont.add_child(tree)
			#var strings := rmhg.get_strings()
			#print(strings)
			#for string in strings:
			#	print("S: ", string)
			continue

func _clear() -> void:
	# Clear 3D
	for child in _3d_cont.get_children():
		child.queue_free()

	# Clear Sidebar
	for child in _sidebar_cont.get_children():
		child.queue_free()


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(_delta: float) -> void:
	if Input.is_key_pressed(KEY_P):
		get_tree().change_scene_to_file("res://scenes/world.tscn")
