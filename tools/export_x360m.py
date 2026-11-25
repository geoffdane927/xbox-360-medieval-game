# Blender 2.79 FBX -> X360M v1 exporter.
# Import the FBX first, then run this script in Blender's Text Editor.
import bpy
import struct
from mathutils import Matrix

OUTPUT_PATH = bpy.path.abspath("//adventurer.x360m")
SAMPLE_RATE = 24.0
MAX_BONES = 128

def pack_string(value, length=32):
    data = value.encode("utf-8")[:length-1]
    return data + b"\0" * (length-len(data))

def write_matrix(f, matrix):
    # Runtime matrices are column-major.
    vals = [matrix[r][c] for c in range(4) for r in range(4)]
    f.write(struct.pack("<16f", *vals))

def mesh_vertices(obj, armature, bone_index):
    # Do not apply the Armature modifier here: the runtime performs skinning.
    mesh = obj.to_mesh(bpy.context.scene, False, "PREVIEW")
    mesh.calc_tessface()
    uv_layer = mesh.tessface_uv_textures.active
    arm_inv = armature.matrix_world.inverted()
    obj_to_arm = arm_inv * obj.matrix_world
    normal_to_arm = obj_to_arm.to_3x3()
    vertices, indices = [], []
    for face in mesh.tessfaces:
        face_indices = list(face.vertices)
        if len(face_indices) != 3:
            continue
        base = len(vertices)
        for corner, source_index in enumerate(face_indices):
            v = mesh.vertices[source_index]
            uv = (0.0, 0.0)
            if uv_layer:
                uv = uv_layer.data[face.index].uv[corner]
            influences = []
            for group in v.groups:
                if group.group >= len(obj.vertex_groups):
                    continue
                name = obj.vertex_groups[group.group].name
                if name in bone_index:
                    influences.append((group.weight, bone_index[name]))
            influences.sort(reverse=True)
            influences = influences[:4]
            total = sum(x[0] for x in influences)
            if total <= 0.00001:
                influences, total = [(1.0, 0)], 1.0
            ids = [x[1] for x in influences] + [0] * (4-len(influences))
            weights = [x[0]/total for x in influences] + [0.0] * (4-len(influences))
            co = obj_to_arm * v.co
            no = (normal_to_arm * v.normal).normalized()
            vertices.append((co.x, co.y, co.z, no.x, no.y, no.z,
                             uv[0], uv[1], ids, weights))
        indices.extend([base, base+1, base+2])
    bpy.data.meshes.remove(mesh)
    return vertices, indices

def export():
    armatures = [o for o in bpy.context.scene.objects if o.type == "ARMATURE"]
    if not armatures:
        raise RuntimeError("No armature found in scene.")
    arm = armatures[0]
    bones = list(arm.data.bones)
    if not bones or len(bones) > MAX_BONES:
        raise RuntimeError("Skeleton must contain 1..%d bones." % MAX_BONES)
    bone_index = {b.name: i for i, b in enumerate(bones)}
    meshes = [o for o in bpy.context.scene.objects if o.type == "MESH"]
    if not meshes:
        raise RuntimeError("No mesh objects found.")
    actions = list(bpy.data.actions)
    if not actions:
        raise RuntimeError("No Blender actions found. Re-import FBX with animation enabled.")

    with open(OUTPUT_PATH, "wb") as f:
        f.write(struct.pack("<8s4I", b"X360M01\0", 1, len(meshes), len(bones), len(actions)))
        for bone in bones:
            parent = bone_index[bone.parent.name] if bone.parent else -1
            f.write(pack_string(bone.name))
            f.write(struct.pack("<i", parent))
            write_matrix(f, bone.matrix_local.inverted())

        for obj in meshes:
            verts, inds = mesh_vertices(obj, arm, bone_index)
            if len(verts) > 65535:
                raise RuntimeError("A mesh exceeds 65535 vertices; split it before export.")
            f.write(struct.pack("<2I", len(verts), len(inds)))
            for v in verts:
                f.write(struct.pack("<8f4H4f", *(v[:8] + tuple(v[8]) + tuple(v[9]))))
            if inds:
                f.write(struct.pack("<%dH" % len(inds), *inds))

        scene = bpy.context.scene
        old_action = arm.animation_data.action if arm.animation_data else None
        if not arm.animation_data:
            arm.animation_data_create()
        for action in actions:
            arm.animation_data.action = action
            start, end = int(action.frame_range[0]), int(action.frame_range[1])
            count = max(1, int((end-start) * SAMPLE_RATE / scene.render.fps) + 1)
            duration = max(1.0/SAMPLE_RATE, (end-start)/float(scene.render.fps))
            f.write(pack_string(action.name))
            f.write(struct.pack("<2fI", duration, SAMPLE_RATE, count))
            for sample in range(count):
                frame = start + sample * scene.render.fps / SAMPLE_RATE
                scene.frame_set(int(round(frame)))
                for bone in bones:
                    write_matrix(f, arm.pose.bones[bone.name].matrix)
        if arm.animation_data:
            arm.animation_data.action = old_action
    print("Wrote X360M:", OUTPUT_PATH)

export()
