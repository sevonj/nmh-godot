extends Node3D
var _chunklist: Array[String] = [
	"AREA000",
	"AREA001",
	"AREA002",
	"AREA003",
	"AREA004",
	"AREA005",
	"AREA006",
	"AREA007",
	"AREA008",
	"AREA009",
	"AREA010",
	"AREA011",
	"AREA012",
	"AREA013",
	"AREA014",
	"AREA015",
	"AREA016",
	"AREA017",
	"AREA018",
	"AREA019",
	"AREA020",
	"AREA021",
	"AREA022",
	"AREA023",
	"AREA024",
	"AREA025",
	"AREA026",
	"AREA027",
	"AREA028",
	"AREA029",
	"AREA030",
	"AREA031",
	"AREA032",
	"AREA033",
	"AREA034",
	"AREA035",
	"AREA036",
	"AREA037",
	"AREA038",
	"AREA039",
	"AREA040",
	"AREA041",
	"AREA042",
	"AREA043"
]

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	_load_world()

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	pass


func _load_world() -> void:
	var basepath := OS.get_executable_path().get_base_dir() + "/"

	for chunkname in _chunklist:
		var filepath := "gamedata/STG_HI/" + chunkname
		var globalpath := basepath + filepath
		if not OS.has_feature("standalone"):
			globalpath = basepath + "nmh/" + filepath
		if not FileAccess.file_exists(globalpath + ".GM2"):
			_on_load_fail(globalpath + ".GM2")
			break
		if not FileAccess.file_exists(globalpath + "COL.GCL"):
			_on_load_fail(globalpath + "COL.GCL")
			break
		var gmf2 := GMF2.new()
		var flcg := FLCG.new()
		gmf2.open(globalpath + ".GM2")
		flcg.open(globalpath + "COL.GCL")
		add_child(gmf2)
		add_child(flcg)

func _on_load_fail(file: String) -> void:
	$ui/err_cant_find_files.show()
	$ui/err_cant_find_files/center/errormsg.text += "\nThis particular file failed:\n"+file
