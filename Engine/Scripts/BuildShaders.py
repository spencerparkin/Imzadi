# BuildShaders.py

import os
import re
import json
import subprocess

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

def compile_shader(shader_data, config, prefix, assets_base_dir, shader_pdb_dir):
    sdk_bin_dir = r'C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64'
    compiler_exe = os.path.join(sdk_bin_dir, 'dxc.exe')
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
    # TODO: Don't recompile everything all the time.  Only recompile what needs to be recompiled.  Compile incrementally.

    assets_base_dir = os.getcwd()

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