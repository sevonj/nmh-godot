extends Node

@onready var _sidebar_cont := $hsplit/sidebar/sidebar_cont

@onready var _viewport_panel = $"hsplit/viewport_panel"
@onready var _viewport_cont = _viewport_panel.get_node("viewport_cont")
@onready var _viewport_ovl = _viewport_panel.get_node("viewport_ovl")

var _viewport_3d: Viewport3D
var _viewport_arc: Control

var _viewport_label: Label

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	get_tree().root.get_viewport().files_dropped.connect(on_files_dropped)

func on_files_dropped(files: Array[String]):
	_clear()

	var close_button := Button.new()
	close_button.text = "close"
	close_button.pressed.connect(_clear)
	_sidebar_cont.add_child(close_button)

	for file in files:
		var magic := GHMFile.get_file_type(file)
		if magic.is_empty():
			print("magic: unrecognized")
		else:
			print("magic: ", magic)
		match magic:
			"FLCG":
				var flcg := FLCG.new()
				flcg.open(file)
				_create_viewport_3d()
				_viewport_3d.add_model(flcg)
			"GAN2":
				pass
			"GCT0":
				pass
			"GMF2":
				var gmf2 := GMF2.new()
				gmf2.open(file)
				_create_viewport_3d()
				_viewport_3d.add_model(gmf2)
			"RMHG":
				var rmhg := RMHG.new()
				rmhg.open(file)
				_create_viewport_arc()

				# Create title
				#var title_cont := ScrollContainer.new()
				#title_cont.vertical_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
				#title_cont.horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_SHOW_NEVER
				#_sidebar_cont.add_child(title_cont)
				#var title := Label.new()
				#title.text = file# file.split("/")[-1]
				#title_cont.add_child(title)

				# Create tree
				var archive_tree := RMHGTree.new()
				archive_tree.load_rmhg(rmhg)
				_sidebar_cont.add_child(archive_tree)
				archive_tree.sig_res_selected.connect(_on_archive_file_selected)
			"RSAR":
				pass
			"RSTM":
				pass
			"SEST":
				pass
			"STMD":
				pass
			"STSD":
				pass
			"STRIMAG2":
				pass
			"STRIMAGE":
				pass
			"THP":
				pass
			"":
				pass

func _clear() -> void:
	# Clear Viewport
	_viewport_3d = null
	_viewport_arc = null
	if is_instance_valid(_viewport_label):
		_viewport_label.free()
	for child in _viewport_cont.get_children():
		child.queue_free()

	# Clear Sidebar
	for child in _sidebar_cont.get_children():
		child.queue_free()

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(_delta: float) -> void:
	if Input.is_key_pressed(KEY_P):
		get_tree().change_scene_to_file("res://scenes/world.tscn")

func _on_archive_file_selected(res: RMHGFileDescriptor) -> void:
	_set_viewport_message(res.to_string())

func _set_viewport_message(msg: String) -> void:
	if is_instance_valid(_viewport_label):
		_viewport_label.queue_free()
	_viewport_label = Label.new()
	_viewport_ovl.add_child(_viewport_label)
	_viewport_label.text = msg

# Create 3d viewport *if it doesn't exist*
func _create_viewport_3d() -> void:
	if is_instance_valid(_viewport_3d):
		return
	_viewport_3d = Viewport3D.new()
	_viewport_cont.add_child(_viewport_3d)

# Create viewport *if it doesn't exist*
func _create_viewport_arc() -> void:
	if is_instance_valid(_viewport_arc):
		return
	_viewport_arc = load("res://scenes/viewer/viewport_archive.tscn").instantiate()
	_viewport_cont.add_child(_viewport_arc)
