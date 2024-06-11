# BuildAssets.py

# TODO: I want this script to entirely go away and be superceeded by the ImzadiGameEditor, based on the engine and AssImp.

import os
import json
import math

class OBJ_Model(object):
    def __init__(self):
        self.name = ''
        self.triangle_list = []

    def print_stats(self):
        print('---------------------------')
        print('Name: %s' % self.name)
        print('Num triangles: %d' % len(self.triangle_list))

def find_all_files(search_dir, found_file_list, desired_ext):
    for root_dir, dir_list, file_list in os.walk(search_dir):
        for file in file_list:
            name, ext = os.path.splitext(file)
            if ext.lower() == desired_ext:
                found_file_list.append(os.path.join(root_dir, file))

def make_vertex_key(vertex):
    key = '%f-%f-%f/%f-%f/%f-%f-%f' % (
        vertex['vertex'][0],
        vertex['vertex'][1],
        vertex['vertex'][2],
        vertex['texcoord'][0],
        vertex['texcoord'][1],
        vertex['normal'][0],
        vertex['normal'][1],
        vertex['normal'][2])
    return key

def append_vertex(vertex, vertex_array):
    vertex_array.append(vertex['vertex'][0])
    vertex_array.append(vertex['vertex'][1])
    vertex_array.append(vertex['vertex'][2])
    vertex_array.append(vertex['texcoord'][0])
    vertex_array.append(vertex['texcoord'][1])
    vertex_array.append(vertex['normal'][0])
    vertex_array.append(vertex['normal'][1])
    vertex_array.append(vertex['normal'][2])

def parse_vertex(vertex_str, vertex_list, texcoord_list, normal_list):
    token_list = vertex_str.split('/')
    if len(token_list) == 3:
        vertex_index = int(token_list[0]) - 1
        texcoord_index = int(token_list[1]) - 1
        normal_index = int(token_list[2]) - 1
        return {
            'vertex': vertex_list[vertex_index],
            'texcoord': texcoord_list[texcoord_index],
            'normal': normal_list[normal_index]
        }
    else:
        raise Exception('Bad vertex!')

def load_obj_file(obj_file):
    vertex_list = []
    texcoord_list = []
    normal_list = []
    model_array = []
    model = OBJ_Model()
    with open(obj_file, 'r') as handle:
        while True:
            line = handle.readline()
            if not line:
                break
            token_list = line.split()
            if len(token_list) == 0 or token_list[0] == '#':
                continue
            if token_list[0] == 'v' and len(token_list) == 4:
                if len(model.name) > 0:
                    model_array.append(model)
                    model = OBJ_Model()
                vertex_list.append((
                    float(token_list[1]),
                    float(token_list[2]),
                    float(token_list[3])
                ))
            elif token_list[0] == 'vt' and len(token_list) >= 3:
                texcoord_list.append((
                    float(token_list[1]),
                    float(token_list[2])
                ))
            elif token_list[0] == 'vn' and len(token_list) == 4:
                x = float(token_list[1])
                y = float(token_list[2])
                z = float(token_list[3])
                length = math.sqrt(x*x + y*y + z*z)
                x /= length
                y /= length
                z /= length
                normal_list.append((x, y, z))
            elif token_list[0] == 'f':
                if len(token_list) != 4:
                    raise Exception('Only triangles are supported.')
                model.triangle_list.append((
                    parse_vertex(token_list[1], vertex_list, texcoord_list, normal_list),
                    parse_vertex(token_list[2], vertex_list, texcoord_list, normal_list),
                    parse_vertex(token_list[3], vertex_list, texcoord_list, normal_list)
                ))
            elif token_list[0] == 'o' and len(token_list) == 2:
                model.name = token_list[1]

    if len(model.name) > 0:
        model_array.append(model)

    return model_array

def process_obj_file(obj_file, assets_base_dir):
    model_dir, name = os.path.split(obj_file)
    base_name, ext = os.path.splitext(name)
    model_array = load_obj_file(obj_file)
    for model in model_array:
        process_model(model, base_name, model_dir, assets_base_dir)

def process_model(model, base_name, model_dir, assets_base_dir):
    model.print_stats()

    vertex_buffer_file = os.path.join(model_dir, base_name + '_' + model.name + '_Vertices.buffer')
    index_buffer_file = os.path.join(model_dir, base_name + '_' + model.name + '_Indices.buffer')
    render_mesh_file = os.path.join(model_dir, base_name + '_' + model.name + '.render_mesh')
    texture_file = os.path.join(model_dir, base_name + '_' + model.name + ".texture")
    collision_file = os.path.join(model_dir, base_name + '_' + model.name + ".collision")
    image_file = os.path.join(model_dir, base_name + '_' + model.name + '.png')

    shape_set = []
    for triangle in model.triangle_list:
        vertex_array = [
            triangle[0]['vertex'],
            triangle[1]['vertex'],
            triangle[2]['vertex']
        ]
        shape_info = {
            'type': 'polygon',
            'vertex_array': vertex_array
        }
        shape_set.append(shape_info)

    collision_file_data = {
        'shape_set': shape_set
    }

    if os.path.exists(collision_file):
        os.remove(collision_file)

    with open(collision_file, 'w') as handle:
        json_text = json.dumps(collision_file_data, indent=4, sort_keys=True)
        handle.write(json_text)
    print('Wrote file: %s!' % collision_file)

    vertex_map = {}
    vertex_array = []
    index_array = []
    for triangle in model.triangle_list:
        for i in range(0, 3):
            vertex = triangle[i]
            vertex_key = make_vertex_key(vertex)
            if vertex_key not in vertex_map:
                index = int(len(vertex_array) / 8)
                index_array.append(index)
                append_vertex(vertex, vertex_array)
                vertex_map[vertex_key] = index
            else:
                index = vertex_map[vertex_key]
                index_array.append(index)

    min_x = min([min([vertex['vertex'][0] for vertex in triangle]) for triangle in model.triangle_list])
    min_y = min([min([vertex['vertex'][1] for vertex in triangle]) for triangle in model.triangle_list])
    min_z = min([min([vertex['vertex'][2] for vertex in triangle]) for triangle in model.triangle_list])

    max_x = max([max([vertex['vertex'][0] for vertex in triangle]) for triangle in model.triangle_list])
    max_y = max([max([vertex['vertex'][1] for vertex in triangle]) for triangle in model.triangle_list])
    max_z = max([max([vertex['vertex'][2] for vertex in triangle]) for triangle in model.triangle_list])

    if os.path.exists(vertex_buffer_file):
        os.remove(vertex_buffer_file)
    if os.path.exists(index_buffer_file):
        os.remove(index_buffer_file)
    if os.path.exists(render_mesh_file):
        os.remove(render_mesh_file)
    if os.path.exists(texture_file):
        os.remove(texture_file)

    vertex_file_data = {
        'buffer': vertex_array,
        'type': 'float',
        'bind': 'vertex',
        'stride': 8
    }

    index_file_data = {
        'buffer': index_array,
        'type': 'ushort',
        'bind': 'index',
        'stride': 1
    }

    render_mesh_file_data = {
        'vertex_buffer': os.path.relpath(vertex_buffer_file, assets_base_dir),
        'index_buffer': os.path.relpath(index_buffer_file, assets_base_dir),
        'shader': 'Shaders/Standard.shader',
        'shadow_shader': 'Shaders/StandardShadow.shader',
        'primitive_type': 'TRIANGLE_LIST',
        'bounding_box': {
            'min': {'x': min_x, 'y': min_y, 'z': min_z},
            'max': {'x': max_x, 'y': max_y, 'z': max_z}
        }
    }

    if os.path.exists(image_file):
        with open(texture_file, 'w') as handle:
            texture_data = {
                'image_file': os.path.relpath(image_file, assets_base_dir),
                'flip_vertical': True
            }
            json_text = json.dumps(texture_data, sort_keys=True, indent=4)
            handle.write(json_text)
        render_mesh_file_data['texture'] = os.path.relpath(texture_file, assets_base_dir)

    with open(vertex_buffer_file, 'w') as handle:
        json_text = json.dumps(vertex_file_data, sort_keys=True, indent=4)
        handle.write(json_text)
    print('Wrote file: %s!' % vertex_buffer_file)

    with open(index_buffer_file, 'w') as handle:
        json_text = json.dumps(index_file_data, sort_keys=True, indent=4)
        handle.write(json_text)
    print('Wrote file: %s!' % index_buffer_file)

    with open(render_mesh_file, 'w') as handle:
        json_text = json.dumps(render_mesh_file_data, sort_keys=True, indent=4)
        handle.write(json_text)
    print('Wrote file: %s!' % render_mesh_file)

if __name__ == '__main__':
    assets_base_dir = os.getcwd()

    print('Processing all OBJ files...')
    obj_file_list = []
    find_all_files(assets_base_dir, obj_file_list, '.obj')
    for obj_file in obj_file_list:
        print('=================================================================================')
        print('Processing %s...' % obj_file)
        process_obj_file(obj_file, assets_base_dir)

    print('Asset build complete!')