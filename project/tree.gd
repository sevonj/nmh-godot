class_name RMHGTree
extends Tree

func _init() -> void:
	size_flags_horizontal = SIZE_EXPAND_FILL
	size_flags_vertical = SIZE_EXPAND_FILL

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	hide_root = true


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	pass

# --- Public --- #

func load_rmhg(rmhg: RMHG):
	if !is_instance_valid(rmhg):
		push_error("Invalid RMHG!")
		return

	var root := create_item()
	for string in rmhg.get_strings():
		var child := create_item(root)
		child.set_text(0, "[RMHG] " + string)
