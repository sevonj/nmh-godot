class_name RMHGTree
extends Tree

var rmhg: RMHG
signal sig_res_selected(res: RMHGFileDescriptor)

enum meta{
	RES_DESC = 0
}

func _init() -> void:
	size_flags_horizontal = SIZE_EXPAND_FILL
	size_flags_vertical = SIZE_EXPAND_FILL

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	#hide_root = true
	item_selected.connect(_on_tree_item_selected)

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(_delta: float) -> void:
	pass

# --- Public --- #

func load_rmhg(input: RMHG):
	rmhg = input
	if !is_instance_valid(rmhg):
		push_error("Invalid RMHG!")
		return

	var tree_root := create_item()
	var archive_root := rmhg.get_root()
	tree_root.set_text(0, archive_root.get_name())
	tree_root.set_metadata(meta.RES_DESC, archive_root)
	_load_dir(archive_root, tree_root)

func _load_dir(dir: RMHGDirDescriptor, tree_parent: TreeItem) -> void:
	for resource in dir.get_contents():
		var tree_child := create_item(tree_parent)
		tree_child.set_metadata(meta.RES_DESC, resource)
		if resource is RMHGDirDescriptor:
			tree_child.set_text(0, resource.get_name() + " (dir)")
			_load_dir(resource, tree_child)
		else:
			tree_child.set_text(0, resource.get_name())

# --- Private --- #

func _on_tree_item_selected():
	var selected := get_selected()
	var resource: RMHGFileDescriptor = selected.get_metadata(meta.RES_DESC)
	sig_res_selected.emit(resource)
