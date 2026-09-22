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


def _ensure_fleet_pbr():
    path = "/Game/Fleet/M_FleetPBR"
    try:
        if unreal.EditorAssetLibrary.does_asset_exist(path):
            return
        asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        factory = unreal.MaterialFactoryNew()
        mat = asset_tools.create_asset("M_FleetPBR", "/Game/Fleet", unreal.Material, factory)
        if not mat:
            return
        lib = unreal.MaterialEditingLibrary
        mat.set_editor_property("used_with_instanced_static_meshes", True)
        mat.set_editor_property("used_with_static_mesh", True)
        mat.set_editor_property("used_with_skeletal_mesh", False)

        albedo = lib.create_material_expression(mat, unreal.MaterialExpressionTextureSampleParameter2D, -640, -180)
        albedo.set_editor_property("parameter_name", "Albedo")

        color = lib.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, -640, 80)
        color.set_editor_property("parameter_name", "Color")
        color.set_editor_property("default_value", unreal.LinearColor(1.0, 1.0, 1.0, 1.0))

        mul = lib.create_material_expression(mat, unreal.MaterialExpressionMultiply, -280, -80)
        lib.connect_material_expressions(albedo, "RGB", mul, "A")
        lib.connect_material_expressions(color, "", mul, "B")
        lib.connect_material_property(mul, "", unreal.MaterialProperty.MP_BASE_COLOR)

        nrm = lib.create_material_expression(mat, unreal.MaterialExpressionTextureSampleParameter2D, -640, 260)
        nrm.set_editor_property("parameter_name", "Normal")
        nrm.set_editor_property("sampler_type", unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL)
        lib.connect_material_property(nrm, "RGB", unreal.MaterialProperty.MP_NORMAL)

        rough_tex = lib.create_material_expression(mat, unreal.MaterialExpressionTextureSampleParameter2D, -640, 520)
        rough_tex.set_editor_property("parameter_name", "Roughness")
        rough_tex.set_editor_property("sampler_type", unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR)
        rough_mul = lib.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -640, 720)
        rough_mul.set_editor_property("parameter_name", "RoughnessMul")
        rough_mul.set_editor_property("default_value", 0.45)
        rough = lib.create_material_expression(mat, unreal.MaterialExpressionMultiply, -280, 560)
        lib.connect_material_expressions(rough_tex, "R", rough, "A")
        lib.connect_material_expressions(rough_mul, "", rough, "B")
        lib.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)

        metal_tex = lib.create_material_expression(mat, unreal.MaterialExpressionTextureSampleParameter2D, -80, 520)
        metal_tex.set_editor_property("parameter_name", "Metallic")
        metal_tex.set_editor_property("sampler_type", unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR)
        metal_mul = lib.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -80, 720)
        metal_mul.set_editor_property("parameter_name", "MetallicMul")
        metal_mul.set_editor_property("default_value", 0.12)
        metal = lib.create_material_expression(mat, unreal.MaterialExpressionMultiply, 180, 560)
        lib.connect_material_expressions(metal_tex, "R", metal, "A")
        lib.connect_material_expressions(metal_mul, "", metal, "B")
        lib.connect_material_property(metal, "", unreal.MaterialProperty.MP_METALLIC)

        ao = lib.create_material_expression(mat, unreal.MaterialExpressionTextureSampleParameter2D, -80, 260)
        ao.set_editor_property("parameter_name", "AO")
        ao.set_editor_property("sampler_type", unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR)
        lib.connect_material_property(ao, "R", unreal.MaterialProperty.MP_AMBIENT_OCCLUSION)

        glow = lib.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -80, 80)
        glow.set_editor_property("parameter_name", "Emissive")
        glow.set_editor_property("default_value", 0.0)
        emul = lib.create_material_expression(mat, unreal.MaterialExpressionMultiply, 180, 80)
        lib.connect_material_expressions(color, "", emul, "A")
        lib.connect_material_expressions(glow, "", emul, "B")
        lib.connect_material_property(emul, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)

        lib.recompile_material(mat)
        unreal.EditorAssetLibrary.save_asset(path)
        unreal.log("Fleet Commander: created /Game/Fleet/M_FleetPBR")
    except Exception as err:
        unreal.log_warning("Fleet Commander material: {}".format(err))


_apply_fleet_commander_game_mode()
_ensure_fleet_pbr()
