class_name Viewport3D
extends SubViewportContainer

@onready var _subviewport := SubViewport.new()
@onready var _camerarig := CameraRigOrbital.new()
@onready var _container := Node3D.new()
@onready var _env_cont := Node3D.new()
@onready var _env_light := DirectionalLight3D.new()
@onready var _draw_ovl := UIOverlayDebugdraw.new() # Overlay for drawing lines

const _env_light_rot := Vector3(-44.9, -122.3, 0)

# --- Virtual --- #
func _init() -> void:
	stretch = true

func _ready() -> void:
	_subviewport.name = "subviewport"
	_camerarig.name = "camerarig"
	_container.name = "container"
	_env_cont.name = "env"
	_env_light.name = "sun"
	_env_light.rotation_degrees = _env_light_rot
	_env_light.shadow_enabled = true
	add_child(_subviewport)
	_subviewport.add_child(_camerarig)
	_subviewport.add_child(_container)
	_subviewport.add_child(_env_cont)
	_env_cont.add_child(_env_light)
	_subviewport.add_child(_draw_ovl)

func _process(_delta: float) -> void:
	# Draw a gizmo to the overlay
	_draw_gizmo_line(Vector3.ZERO, Vector3.UP, Color.GREEN)
	_draw_gizmo_line(Vector3.ZERO, Vector3.LEFT, Color.RED)
	_draw_gizmo_line(Vector3.ZERO, Vector3.FORWARD, Color.BLUE)

func _draw_gizmo_line(start3d: Vector3, end3d: Vector3, color := Color.OLIVE):
	var camera = _camerarig.get_camera()
	var dist: float = abs(camera.global_position - start3d).length()
	end3d *= dist * .2
	if camera.is_position_behind(start3d) or camera.is_position_behind(end3d):
		return
	var line_start := camera.unproject_position(start3d)
	var line_end := camera.unproject_position(end3d)
	if is_nan(line_start.x) or is_nan(line_start.y) or is_nan(line_end.x) or is_nan(line_end.y):
		return
	_draw_ovl.add_line(line_start, line_end, 0., color)

# --- Public --- #

func clear() -> void:
	for child in _container.get_children():
		child.queue_free()

func add_model(node: Node3D) -> void:
	_container.add_child(node)
