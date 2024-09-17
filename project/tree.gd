class_name RMHGTree
extends HSplitContainer

var _tree: Tree # Tree node for this archive
var _preview: Control # The main view for when anything is opened

var rmhg: RMHG

func _init() -> void:
	size_flags_horizontal = SIZE_EXPAND_FILL
	size_flags_vertical = SIZE_EXPAND_FILL
	_tree = Tree.new()

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	add_child(_tree)
	_tree.hide_root = true
	_tree.item_selected.connect(_on_tree_item_selected)

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	pass

# --- Public --- #

func load_rmhg(input: RMHG):
	rmhg = input
	if !is_instance_valid(rmhg):
		push_error("Invalid RMHG!")
		return

	var root := _tree.create_item()
	var hierarchy = rmhg.get_hierarchy()
	for key in hierarchy:
		var child := _tree.create_item(root)
		var entry: Dictionary = hierarchy[key]
		var str := "[%s] %s" % [entry.get("type"), entry.get("name")]
		child.set_text(0, entry.get("name"))

	print(rmhg.get_hierarchy())

# --- Private --- #

func _on_tree_item_selected():
	if is_instance_valid(_preview):
		_preview.queue_free()

	var hierarchy = rmhg.get_hierarchy()
	var key = _tree.get_selected().get_text(0)
	print("hierarchy:", hierarchy)
	print("key: ", key)
	var res: Dictionary = hierarchy.get(key)

	if res.get("type") == "RMHG":
		var inner_rmhg := RMHG.new()
		inner_rmhg.open_at_offset(rmhg.get_file_path(), res.get("offset"))
		#_preview = RMHGTree.new()
		#_preview.load_rmhg(inner_rmhg)
