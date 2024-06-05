# BuildAssets.py

import os
import json
import subprocess
import re
import math

class OBJ_Model(object):
    def __init__(self):
        self.name = ''
        self.triangle_list = []

    def print_stats(self):
        print('---------------------------')
        print('Name: %s' % self.name)
        print('Num triangles: %d' % len(self.triangle_list))

def run_shell_proc(shell_command, working_dir=None, captured_output=None):
    if working_dir is None:
        working_dir = os.getcwd()
    if type(shell_command) is list:
        shell_command = ' '.join(shell_command)
    print('Executing command: ' + shell_command)
    print('CWD: ' + working_dir)
    with subprocess.Popen(shell_command, cwd=working_dir, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True) as proc:
        while True:
            while True:
                command_output = proc.stdout.readline()
                if command_output is None or len(command_output) == 0:
                    break
                command_output = command_output.rstrip('\r\n')
                if type(captured_output) == list:
                    captured_output.append(command_output)
                print(command_output)
            if proc.poll() is not None:
                break
        if proc.returncode != 0:
            raise Exception('Command exited with non-zero return code!  (Return code: %d)' % proc.returncode)

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

def compile_shader(shader_data, config, prefix, assets_base_dir, shader_pdb_dir):
    sdk_bin_dir = r'C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64'
    compiler_exe = os.path.join(sdk_bin_dir, 'fxc.exe')     # Note that dxc.exe works here, but then the game crashes.  I have no idea what's going on.
    if not os.path.exists(compiler_exe):
        raise Exception('Could not locate: ' + compiler_exe)

    shader_obj_file = os.path.join(assets_base_dir, shader_data[prefix + '_shader_object'])

    name, ext = os.path.splitext(shader_obj_file)
    if ext != '.dxbc':
        raise Exception('Must use .dxbc file extension for the shader object files.')

    if os.path.exists(shader_obj_file):
        os.remove(shader_obj_file)

    shader_code_file = os.path.join(assets_base_dir, shader_data['shader_code'])

    if not os.path.exists(shader_code_file):
        raise Exception('Could not locate: ' + shader_code_file)

    cmd_line = [
        '"' + compiler_exe + '"',
        shader_code_file,
    ]

    if config == 'debug':
        cmd_line.append('/Zi')
        cmd_line.append('/Zss')
        cmd_line.append('/Fd %s\\' % shader_pdb_dir)    # Must have trailing backslash!
    elif config == 'release':
        cmd_line.append('/O4')

    if prefix + '_entry_point' in shader_data:
        cmd_line.append('/E ' + shader_data[prefix + '_entry_point'])

    if prefix + '_model' in shader_data:
        cmd_line.append('/T %s' % shader_data[prefix + '_model'])

    cmd_line.append('/Fo ' + shader_obj_file)

    run_shell_proc(cmd_line)

def align(offset, multiple, direction):
    if direction == 'down':
        while offset % multiple != 0:
            offset -= 1
    elif direction == 'up':
        while offset % multiple != 0:
            offset += 1
    return offset

def calculate_constant_buffer_layout(shader_data, assets_base_dir):
    if 'shader_code' not in shader_data:
        return False

    hlsl_file = os.path.join(assets_base_dir, shader_data['shader_code'])
    if not os.path.exists(hlsl_file):
        raise Exception('File does not exists!  (%s)' % hlsl_file)

    with open(hlsl_file) as handle:
        hlsl_code = handle.read()
        hlsl_lines = hlsl_code.split('\n')

    size_map = {
        'float': 4,
        'float2': 8,
        'float3': 12,
        'float4': 16,
        'float2x2': 16,
        'float3x3': 36,
        'float4x4': 64
    }

    current_offset = 0

    constants_map = {}
    inside_constants_buffer = False
    for line in hlsl_lines:
        if not inside_constants_buffer:
            if line.find('cbuffer constants') >= 0:
                inside_constants_buffer = True
        elif line.find('};') >= 0:
            break
        else:
            match = re.search(r'(\w+)\s+(\w+);', line)
            if match:
                type_name = match.group(1)
                var_name = match.group(2)

                component_type = ''
                if any([type_name == known_type for known_type in ['float', 'float2', 'float3', 'float4', 'float2x2', 'float3x3', 'float4x4']]):
                    component_type = 'float'
                if len(component_type) == 0:
                    raise Exception('Could not decyper component type!')

                size = size_map.get(type_name, -1)
                if size == -1:
                    raise Exception('Size of type not known!')

                alignment_a = align(current_offset, 16, 'down')
                alignment_b = align(current_offset + size, 16, 'down')
                if alignment_a != alignment_b and alignment_b < current_offset + size:
                    current_offset = align(current_offset, 16, 'up')

                constant_data = {
                    'offset': current_offset,
                    'size': size,
                    'type': component_type
                }
                constants_map[var_name] = constant_data
                current_offset += size

    shader_data['constants'] = constants_map
    return True

def process_shader_file(shader_file, assets_base_dir, config, shader_pdb_dir):
    with open(shader_file, 'r') as handle:
        json_text = handle.read()
        shader_data = json.loads(json_text)
        if 'vs_shader_object' in shader_data and 'ps_shader_object' in shader_data and 'shader_code' in shader_data:
            compile_shader(shader_data, config, 'vs', assets_base_dir, shader_pdb_dir)
            compile_shader(shader_data, config, 'ps', assets_base_dir, shader_pdb_dir)
    if calculate_constant_buffer_layout(shader_data, assets_base_dir):
        with open(shader_file, 'w') as handle:
            json_text = json.dumps(shader_data, sort_keys=True, indent=4)
            handle.write(json_text)

if __name__ == '__main__':
    assets_base_dir = os.getcwd()

    print('Processing all OBJ files...')
    obj_file_list = []
    find_all_files(assets_base_dir, obj_file_list, '.obj')
    for obj_file in obj_file_list:
        print('=================================================================================')
        print('Processing %s...' % obj_file)
        process_obj_file(obj_file, assets_base_dir)

    print('Deleting PDB files...')
    shader_pdb_dir = os.path.join(assets_base_dir, 'ShaderPDBs')
    os.makedirs(shader_pdb_dir, exist_ok=True)
    pdb_file_list = []
    find_all_files(shader_pdb_dir, pdb_file_list, '.pdb')
    for pdb_file in pdb_file_list:
        os.remove(pdb_file)
        print('Deleted: ' + pdb_file)

    print('Processing shader files...')
    shader_file_list = []
    find_all_files(assets_base_dir, shader_file_list, '.shader')
    for shader_file in shader_file_list:
        print('Processing %s...' % shader_file)
        process_shader_file(shader_file, assets_base_dir, 'debug', shader_pdb_dir)

    print('Asset build complete!')