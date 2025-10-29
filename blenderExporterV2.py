import bpy
import json
import os 
import re
from mathutils import Vector, Matrix


def camera_info(cam_obj, scene):
    cam = cam_obj.data
    loc = list(cam_obj.matrix_world.translation)
    gaze_world = (cam_obj.matrix_world.to_3x3() @ Vector((0, 0, -1))).normalized()
    up_world   = (cam_obj.matrix_world.to_3x3() @ Vector((0.0, 1.0,  0.0))).normalized()
    focal_mm = float(cam.lens)
    sensor_w_mm = float(cam.sensor_width)
    sensor_h_mm = float(cam.sensor_height)
    res_x = int(round(scene.render.resolution_x * scene.render.resolution_percentage / 100))
    res_y = int(round(scene.render.resolution_y * scene.render.resolution_percentage / 100))
    return {"name": cam_obj.name,
            "location": loc,
            "gaze": list(gaze_world),
            "up": list(up_world),
            "focal_length_mm": focal_mm,
            "sensor_width_mm": sensor_w_mm,
            "sensor_height_mm": sensor_h_mm,
            "film_resolution": {"x": res_x, "y": res_y},}

def point_light_info(light_obj):
    light = light_obj.data
    if light.type != 'POINT':return None
    loc = list(light_obj.matrix_world.translation)
    intensity = float(getattr(light, "energy", 0.0))
    return {"name": light_obj.name,
            "location": loc,
            "radiant_intensity": intensity}

def sphere_info(mesh_obj):
    # Belows are calculating the radius of the sphere
    mw = mesh_obj.matrix_world
    verts = getattr(mesh_obj.data, "vertices", [])
    if not verts: return max(mw.to_scale())
    origin = mw.translation
    dists = [(mw @ v.co - origin).length for v in verts]
    radius = sum(dists) / len(dists)
    # End of calculating the radius
    return {"name": mesh_obj.name,
            "location": list(mesh_obj.matrix_world.translation),
            "radius": float(radius)}

def cube_info(obj, note_list=None):
    if note_list is None: note_list = []
    loc = list(obj.matrix_world.translation)
    rot = list(obj.matrix_world.to_euler('XYZ'))
    sx, sy, sz = obj.matrix_world.to_scale()
    if abs(sx - sy) < 1e-6 and abs(sx - sz) < 1e-6:
        s1d = float(sx)
    else:
        s1d = float((sx + sy + sz) / 3.0)
        note_list.append(f"Cube '{obj.name}' non-uniform scale ({sx:.3g},{sy:.3g},{sz:.3g}); exported average.")
    return {"name": obj.name,
            "translation": loc,
            "rotation_euler_xyz_radians": rot,
            "scale_1d": s1d,}

def _ring_sort_world(points_world):
    """给定同一平面上的 4 个 world-space 点，返回按环绕顺序排序后的 4 点。"""
    assert len(points_world) >= 4
    P = [Vector(p) for p in points_world[:4]]
    # 质心
    c = sum(P, Vector((0,0,0))) / 4.0

    # 法向（尽量用非退化三点）
    def try_normal(a,b,d):
        n = (b - a).cross(d - a)
        if n.length > 1e-12: return n
        return None
    n = try_normal(P[0], P[1], P[2]) or try_normal(P[0], P[1], P[3]) \
        or try_normal(P[0], P[2], P[3]) or Vector((0,0,1))  # 最后兜底 Z+

    n.normalize()

    # 在该平面构造局部基 (u,v)
    u = (P[0] - c)
    if u.length <= 1e-12:
        u = (P[1] - c)
    if u.length <= 1e-12:
        # 如果实在重合，随便给个与 n 不平行的方向
        u = Vector((1,0,0))
        if abs(u.dot(n)) > 0.99:
            u = Vector((0,1,0))
    u = (u - u.project(n)).normalized()
    v = n.cross(u).normalized()

    # 极角排序
    nodes = []
    for Q in P:
        x = (Q - c).dot(u)
        y = (Q - c).dot(v)
        ang = math.atan2(y, x)
        nodes.append((ang, Q))
    nodes.sort(key=lambda t: t[0])

    return [list(q) for _, q in nodes]

def plane_info(obj, texture_out_dir, apply_srgb=False):
    mw = obj.matrix_world
    me = obj.data

    # -- 四角（保持你原来的优先级） --
    corners = []
    if hasattr(me, "polygons") and me.polygons:
        for poly in me.polygons:
            if poly.loop_total == 4:
                for vid in poly.vertices:
                    corners.append(list(mw @ me.vertices[vid].co))
                break
    if not corners and hasattr(me, "vertices") and len(me.vertices) >= 4:
        pts = [list(mw @ v.co) for v in me.vertices[:4]]
        corners = _ring_sort_world(pts)
    if not corners and hasattr(obj, "bound_box") and obj.bound_box:
        bb = [mw @ Vector(c) for c in obj.bound_box]
        pts = [list(bb[i]) for i in (0,1,2,3)]
        corners = _ring_sort_world(pts)

    info = {"name": obj.name, "corners": corners}

    # -- 材质/贴图（仅改 plane；其余形状保持你原实现） --
    mat = obj.active_material if obj.material_slots else None
    img, base_col = _find_basecolor_image_and_color(mat) if mat else (None, (0.8,0.8,0.8))

    if img:
        fname = _ensure_image_saved_ppm(img, texture_out_dir, apply_srgb=apply_srgb)
        info["texture"] = f"textures/{fname}"
    else:
        info["base_color"] = [float(base_col[0]), float(base_col[1]), float(base_col[2])]

    # 可选 UV 控制（先给默认，后面你要从 Mapping 节点读也容易加）
    info["uv_scale"]  = [1.0, 1.0]
    info["uv_offset"] = [0.0, 0.0]
    return info

# ----------------------------------------------------------------------
# --- 线性->sRGB（可选；保持线性就设 apply_srgb=False） ---
def _linear_to_srgb(x: float) -> float:
    if x <= 0.0031308: return 12.92 * x
    return 1.055 * (x ** (1.0/2.4)) - 0.055

# --- 把 Blender Image 写成 ASCII PPM(P3, 8-bit, RGB) ---
def _save_image_as_ppm(img: bpy.types.Image, dst_path: str, apply_srgb: bool = False):
    w, h = img.size
    px = list(img.pixels)  # RGBA float, len = w*h*4
    os.makedirs(os.path.dirname(dst_path), exist_ok=True)
    with open(dst_path, "w", encoding="utf-8") as f:
        f.write(f"P3\n{w} {h}\n255\n")
        # 逐行写；把 Blender 的行序翻转成“上到下”，便于你侧的读取直观
        for y in range(h-1, -1, -1):
            row = []
            base = y * w * 4
            for x in range(w):
                i = base + x*4
                r, g, b = px[i], px[i+1], px[i+2]
                if apply_srgb:
                    r, g, b = _linear_to_srgb(r), _linear_to_srgb(g), _linear_to_srgb(b)
                R = max(0, min(255, int(round(r*255))))
                G = max(0, min(255, int(round(g*255))))
                B = max(0, min(255, int(round(b*255))))
                row.append(f"{R} {G} {B}")
            f.write(" ".join(row) + "\n")

def _ensure_image_saved_ppm(img: bpy.types.Image, save_dir: str, apply_srgb: bool=False) -> str:
    os.makedirs(save_dir, exist_ok=True)
    base_name = ""
    if img.filepath:
        base_name = os.path.basename(bpy.path.abspath(img.filepath))
    if not base_name:
        base_name = img.name or "image"

    fname = safe_filename(base_name, ext=".ppm")   # ← 关键：清洗文件名
    dst   = os.path.join(save_dir, fname)
    _save_image_as_ppm(img, dst, apply_srgb=apply_srgb)
    return fname

def _find_basecolor_image_and_color(mat: bpy.types.Material):
    """从材质节点树里找连到 Principled BSDF Base Color 的图片；返回 (image, base_color0..1)"""
    if not mat or not mat.use_nodes or not mat.node_tree:
        return (None, (0.8, 0.8, 0.8))
    nt = mat.node_tree
    bsdf = next((n for n in nt.nodes if n.type == 'BSDF_PRINCIPLED'), None)
    base_col = tuple(bsdf.inputs["Base Color"].default_value[:3]) if bsdf else (0.8, 0.8, 0.8)

    if bsdf:
        stack = [l.from_node for l in nt.links if l.to_node == bsdf and l.to_socket.name == "Base Color"]
        visited = set()
        while stack:
            n = stack.pop()
            if n in visited: continue
            visited.add(n)
            if n.type == 'TEX_IMAGE' and n.image:
                return (n.image, base_col)
            stack.extend([l.from_node for l in nt.links if l.to_node == n])

    # 退化：找第一个 Image Texture
    for n in nt.nodes:
        if n.type == 'TEX_IMAGE' and n.image:
            return (n.image, base_col)
    return (None, base_col)

def _ring_sort_world(pts):
    cx = sum(p[0] for p in pts)/len(pts)
    cy = sum(p[1] for p in pts)/len(pts)
    def ang(p): return math.atan2(p[1]-cy, p[0]-cx)
    return sorted(pts, key=ang)

def safe_filename(name: str, ext: str = ".ppm") -> str:
    if not name:
        name = "image"
    name = os.path.basename(name)                 # 取最后一段
    name = name.replace("<UDIM>", "")             # 去掉 UDIM 占位
    name = os.path.splitext(name)[0]              # 去掉原扩展
    name = re.sub(r"\.\d{3}$", "", name)          # 去掉 .001/.002 等
    name = re.sub(r'[^A-Za-z0-9._-]+', '_', name) # 非法字符 -> _
    if not name:
        name = "image"
    return name + ext

#------------------Main-----------------------
def export_scene(export_dir, apply_srgb=False):
    """把场景导出到 export_dir；仅 plane 导出包含 texture/base_color(PPM)。"""
    export_dir = bpy.path.abspath(export_dir)
    tex_dir = os.path.join(export_dir, "textures")
    os.makedirs(export_dir, exist_ok=True)
    os.makedirs(tex_dir, exist_ok=True)
    scene = bpy.context.scene
    planes, spheres, cubes, lights, cameras = [], [], [], [], []

    for obj in bpy.context.scene.objects:
        if obj.type == 'MESH' and obj.data:
            n = obj.name.lower()
            if "plane" in n:
                planes.append(plane_info(obj, tex_dir, apply_srgb=apply_srgb))
            elif "sphere" in n:
                spheres.append(sphere_info(obj))   # ← 用你的原函数
            elif "cube" in n:
                cubes.append(cube_info(obj))       # ← 用你的原函数
            else:
                # 你也许还有 mesh/triangle 网格：这里继续用你现有导出
                pass
        elif obj.type == 'LIGHT':
            lights.append(point_light_info(obj))         # ← 用你的原函数
        elif obj.type == 'CAMERA':
            cameras.append(camera_info(obj, scene))       # ← 用你的原函数

    scene = {
        "planes": planes,
        "spheres": spheres,
        "cubes": cubes,
        "point_lights": lights,
        "cameras": cameras,
        # 还有别的就继续塞
    }

    out_json = os.path.join(export_dir, "scene_export.json")
    with open(out_json, "w", encoding="utf-8") as f:
        json.dump(scene, f, ensure_ascii=False, indent=2)

    print(f"[OK] Exported to: {out_json}")
    print(f"[OK] Textures dir: {tex_dir}")
export_scene("//scene_export2", apply_srgb=False)
#export_ascii_scene_json(bpy.path.abspath("//scene_export.json"), only_selected=False)

