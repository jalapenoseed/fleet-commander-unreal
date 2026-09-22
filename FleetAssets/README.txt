Unity 1.3.0 native assets from jalapenoseed/fleet-commander
branch `unity`  commit ccac89c
Unity 6000.6.2f1  scene FleetCommander.unity

DroneModels/*.fcm1  — decompressed FCM1 (gzip .fleetmesh) Scout/Relay/Cargo/Utility LOD + Far
ScenePacks/*.fcp1   — decompressed FCP1 (gzip .fleetprop) GRIDRUNNER houses / camp / field ops
DroneTextures/      — original PBR maps from the Unity project
PropTextures/       — scenery albedos

Keep this folder next to FleetCommander.uproject. The C++ loader reads it at Play.
Do not put it inside Content/ (Unreal would try to import unknown types).
