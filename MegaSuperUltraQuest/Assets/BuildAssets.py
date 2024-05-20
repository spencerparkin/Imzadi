# BuildAssets.py

import os
import json
import subprocess

class OBJ_Model(object):
    def __init__(self):
        self.vertex_list = []
        self.texcoord_list = []
        self.normal_list = []
        self.triangle_list = []

    def print_stats(self):
        print('Num vertices: %d' % len(self.vertex_list))
        print('Num texcoords: %d' % len(self.texcoord_list))
        print('Num normals: %d' % len(self.normal_list))
        print('Num triangles: %d' % len(self.triangle_list))

    def parse_vertex(self, vertex_str):
        token_list = vertex_str.split('/')
        if len(token_list) == 3:
            vertex_index = int(token_list[0]) - 1
            texcoord_index = int(token_list[1]) - 1
            normal_index = int(token_list[2]) - 1
            return {
                'vertex': self.vertex_list[vertex_index],
                'texcoord': self.texcoord_list[texcoord_index],
                'normal': self.normal_list[normal_index]
            }
        else:
            raise Exception('Bad vertex!')

    def load_from_file(self, obj_file):
        with open(obj_file, 'r') as handle:
            while True:
                line = handle.readline()
                if not line:
                    break
                token_list = line.split()
                if len(token_list) == 0 or token_list[0] == '#':
                    continue
                if token_list[0] == 'v' and len(token_list) == 4:
                    self.vertex_list.append((
                        float(token_list[1]),
                        float(token_list[2]),
                        float(token_list[3])
                    ))
                elif token_list[0] == 'vt' and len(token_list) >= 3:
                    self.texcoord_list.append((
                        float(token_list[1]),
                        float(token_list[2])
                    ))
                elif token_list[0] == 'vn' and len(token_list) == 4:
                    self.normal_list.append((
                        float(token_list[1]),
                        float(token_list[2]),
                        float(token_list[3])
                    ))
                elif token_list[0] == 'f':
                    if len(token_list) != 4:
                        raise Exception('Only triangles are supported.')
                    self.triangle_list.append((
                        self.parse_vertex(token_list[1]),
                        self.parse_vertex(token_list[2]),
                        self.parse_vertex(token_list[3])
                    ))

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

def process_obj_file(obj_file, assets_base_dir):
    model = OBJ_Model()
    model.load_from_file(obj_file)
    model.print_stats()
    dir, name = os.path.split(obj_file)
    name, ext = os.path.splitext(name)
    vertex_buffer_file = os.path.join(dir, name + 'Vertices.buffer')
    index_buffer_file = os.path.join(dir, name + 'Indices.buffer')
    render_mesh_file = os.path.join(dir, name + ".render_mesh")
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

    min_x = min([vertex[0] for vertex in model.vertex_list])
    min_y = min([vertex[1] for vertex in model.vertex_list])
    min_z = min([vertex[2] for vertex in model.vertex_list])

    max_x = max([vertex[0] for vertex in model.vertex_list])
    max_y = max([vertex[1] for vertex in model.vertex_list])
    max_z = max([vertex[2] for vertex in model.vertex_list])

    if os.path.exists(vertex_buffer_file):
        os.remove(vertex_buffer_file)
    if os.path.exists(index_buffer_file):
        os.remove(index_buffer_file)
    if os.path.exists(render_mesh_file):
        os.remove(render_mesh_file)

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
        'primitive_type': 'TRIANGLE_LIST',
        'bounding_box': {
            'min': {'x': min_x, 'y': min_y, 'z': min_z},
            'max': {'x': max_x, 'y': max_y, 'z': max_z}
        }
    }

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

def process_shader_file(shader_file, assets_base_dir, config, shader_pdb_dir):
    with open(shader_file, 'r') as handle:
        json_text = handle.read()
        shader_data = json.loads(json_text)
        if 'vs_shader_object' in shader_data and 'ps_shader_object' in shader_data and 'shader_code' in shader_data:
            compile_shader(shader_data, config, 'vs', assets_base_dir, shader_pdb_dir)
            compile_shader(shader_data, config, 'ps', assets_base_dir, shader_pdb_dir)

if __name__ == '__main__':
    assets_base_dir = os.getcwd()

    print('Processing all OBJ files...')
    obj_file_list = []
    find_all_files(assets_base_dir, obj_file_list, '.obj')
    for obj_file in obj_file_list:
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