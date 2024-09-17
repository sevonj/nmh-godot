class_name CameraRigOrbital
extends Node3D

var _camera := Camera3D.new()
var _distance := 5.
var _mouse_lock := false:
	set(value):
		_mouse_lock = value
		if _mouse_lock:
			Input.mouse_mode = Input.MOUSE_MODE_CAPTURED
		else:
			Input.mouse_mode = Input.MOUSE_MODE_VISIBLE

var mouse_delta := Vector2.ZERO

func _ready() -> void:
	add_child(_camera)


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(_delta: float) -> void:
	if Input.is_action_just_pressed("ui_cancel"):
		_mouse_lock = false

	if not Input.is_mouse_button_pressed(MOUSE_BUTTON_LEFT):
		_mouse_lock = false
		mouse_delta *= 0.

	_update_rig()

func _input(event: InputEvent) -> void:

	# -- Look
	if event is InputEventMouseMotion:
		mouse_delta += event.relative
		return

	if event is InputEventMouseButton:
		# -- Drag
		if event.button_index == MOUSE_BUTTON_LEFT and event.is_pressed():
			_mouse_lock = true

		# -- Zoom in
		if event.button_index == MOUSE_BUTTON_WHEEL_UP and event.is_pressed():
			_distance *= .8
			return

		# -- Zoom out
		if event.button_index == MOUSE_BUTTON_WHEEL_DOWN and event.is_pressed():
			_distance *= 1. / .8
			return



func _update_rig() -> void:
	rotation.x -= mouse_delta.y * .01
	rotation.x = clamp(rotation.x, -PI/2., PI/2.)
	rotate_y(-mouse_delta.x * .01)
	mouse_delta *= 0

	_camera.position = Vector3.BACK * _distance
