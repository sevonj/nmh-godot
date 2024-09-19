class_name Viewport3D
extends SubViewportContainer

@onready var _3d_root := $"subviewport/3d_root"
@onready var _3d_content_cont := $"subviewport/3d_root/loaded_content_container"

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	pass # Replace with function body.


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	pass

func clear() -> void:
	for child in _3d_content_cont.get_children():
		child.queue_free()

func add_model(node: Node3D) -> void:
	_3d_content_cont.add_child(node)
