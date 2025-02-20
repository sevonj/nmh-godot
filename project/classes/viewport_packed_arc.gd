class_name ViewportPackedArc
extends HSplitContainer

var _viewport: Control
var _arc_tree := RMHGTree.new()

func _ready() -> void:
	add_child(_arc_tree)
	add_child(load("res://scenes/viewer/viewport_archive.tscn").instantiate())

func load_rmhg(file: RMHG) -> void:
	_arc_tree.load_rmhg(file)
