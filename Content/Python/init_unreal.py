import unreal

def _apply_fleet_commander_game_mode():
    try:
        subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
        world = subsystem.get_editor_world() if subsystem else None
        if not world:
            return
        settings = world.world_settings
        gm_class = unreal.load_class(None, "/Script/FleetCommander.FCGameMode")
        if gm_class and settings:
            settings.set_editor_property("default_game_mode", gm_class)
            unreal.log("Fleet Commander: World Settings GameMode -> FCGameMode")
    except Exception as err:
        unreal.log_warning("Fleet Commander startup: {}".format(err))

_apply_fleet_commander_game_mode()
