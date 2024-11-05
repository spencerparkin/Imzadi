# Utils.py

import os
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