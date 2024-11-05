# BuildInstaller.py

import os
import argparse

from Utils import run_shell_proc

# TODO: Can't write progress file to install directory using MSIX.  Change the game so that it uses local app data store.

manifest_xml_format = """<?xml version="1.0" encoding="utf-8"?>
<Package
  xmlns="http://schemas.microsoft.com/appx/manifest/foundation/windows10"
  xmlns:uap="http://schemas.microsoft.com/appx/manifest/uap/windows10"
  xmlns:uap10="http://schemas.microsoft.com/appx/manifest/uap/windows10/10"
  xmlns:rescap="http://schemas.microsoft.com/appx/manifest/foundation/windows10/restrictedcapabilities">
  <Identity Name="SpencerSoft.%s" Version="1.0.0.0" Publisher="CN=SpencerSoft, O=SpencerSoft, L=Bountiful, S=Utah, C=UnitedStates" ProcessorArchitecture="x64" />
  <Properties>
    <DisplayName>%s</DisplayName>
    <PublisherDisplayName>SpencerSoft</PublisherDisplayName>
    <Description>%s</Description>
    <Logo>%s</Logo>
  </Properties>
  <Resources>
    <Resource Language="en-us" />
  </Resources>
  <Dependencies>
    <TargetDeviceFamily Name="Windows.Desktop" MinVersion="%s" MaxVersionTested="%s" />
  </Dependencies>
  <Capabilities>
    <rescap:Capability Name="runFullTrust"/>
  </Capabilities>
  <Applications>
    <Application Id="%s" Executable="%s"
      uap10:RuntimeBehavior="packagedClassicApp"
      uap10:TrustLevel="mediumIL">
      <uap:VisualElements DisplayName="%s" Description="%s"	Square150x150Logo="%s"
        Square44x44Logo="%s" BackgroundColor="snow" />
    </Application>
  </Applications>
</Package>
"""

def dump_file_maps(map_file_handle, source_folder, destination_folder):
    for root_dir, dir_list, file_list in os.walk(source_folder):
        for file in file_list:
            source_file = os.path.join(root_dir, file)
            destination_file = os.path.relpath(source_file, source_folder)
            destination_file = os.path.join(destination_folder, destination_file)
            map_file_handle.write('"%s"\t"%s"\n' % (source_file, destination_file))

if __name__ == '__main__':
    # Parse command-line arguments.
    parser = argparse.ArgumentParser(description='Build a MSIX file that can be used to install a game.')
    parser.add_argument('--game', help='Specify the game you want to package as an installer.')
    parser.add_argument('--win_version', help='Specify the windows version to use to locate the makeappx.exe tool.')
    parser.add_argument('--display_name', help='This is the name of the game, potentially with spaces.')
    parser.add_argument('--description', help='This is a description of the game in one sentence.')
    args = parser.parse_args()

    # Locate the tool that's used to make the MSIX files.
    win_version = args.win_version if args.win_version else '10.0.22621.0'
    make_appx_exe = r'C:\Program Files (x86)\Windows Kits\10\bin\%s\x64\makeappx.exe' % win_version
    if not os.path.exists(make_appx_exe):
        raise Exception('Could not find makeappx.exe utility at: ' + make_appx_exe)

    # This should be the engine folder.
    engine_folder = os.path.normpath(os.path.join(os.getcwd(), '..'))
    print('Assuming engine is at: ' + engine_folder)

    # This should be the root folder.
    root_folder = os.path.normpath(os.path.join(os.getcwd(), '..\\..'))
    print('Assuming root folder is at: ' + root_folder)

    # Locate the game that the user wants to package.
    game_folder = os.path.join(root_folder, ('Games\\%s' % args.game))
    print('Assuming game folder is at: ' + game_folder)
    if not os.path.exists(game_folder) or not os.path.isdir(game_folder):
        raise Exception('Game folder invalid.')

    # TODO: Compile the game here for a release build?  For now, just assume it's already built for release.
    targets_folder = os.path.normpath(os.path.join(root_folder, 'out/build/x64-Release/Bin'))
    game_exe_path = os.path.join(targets_folder, '%s.exe' % args.game)
    if not os.path.exists(game_exe_path):
        raise Exception('Could not find game executable at: ' + game_exe_path)

    # Generate a manifest file for the tool to use while making the package.
    description = args.description if args.description else 'This is a game developed by Spencer T. Parkin.'
    if not args.display_name:
        raise Exception('Must give display name argument.')
    logo_icon_path = os.path.relpath(os.path.join(game_folder, 'Assets\\Icons\\Icon.png'), root_folder)
    logo_icon_150x150_path = os.path.relpath(os.path.join(game_folder, 'Assets\\Icons\\Icon_150x150.png'), root_folder)
    logo_icon_44x44_path = os.path.relpath(os.path.join(game_folder, 'Assets\\Icons\\Icon_44x44.png'), root_folder)
    manifest_xml_text = manifest_xml_format % (
        args.game,
        args.game,
        description,
        logo_icon_path,
        win_version,
        win_version,
        args.game,
        '%s.exe' % args.game,
        args.display_name,
        description,
        logo_icon_150x150_path,
        logo_icon_44x44_path
    )
    manifest_file = os.path.join(root_folder, 'manifest.xml')
    if os.path.exists(manifest_file):
        os.remove(manifest_file)
    with open(manifest_file, 'w') as manifest_file_handle:
        manifest_file_handle.write(manifest_xml_text)
    print('Wrote file: ' + manifest_file)

    # Generate a mapping file for the tool to use while making the package.
    mapping_file = os.path.join(root_folder, 'mapping.txt')
    if os.path.exists(mapping_file):
        os.remove(mapping_file)
    with open(mapping_file, 'w') as map_file_handle:
        map_file_handle.write('[Files]\n')
        map_file_handle.write('"%s"\t"%s"\n' % (game_exe_path, ('%s.exe' % args.game)))
        dump_file_maps(map_file_handle, os.path.join(engine_folder, 'Assets\\Fonts'), 'Engine\\Assets\\Fonts')
        dump_file_maps(map_file_handle, os.path.join(engine_folder, 'Assets\\Shaders'), 'Engine\\Assets\\Shaders')
        dump_file_maps(map_file_handle, os.path.join(game_folder, 'Assets\\Animations'), 'Game\\Assets\\Animations')
        dump_file_maps(map_file_handle, os.path.join(game_folder, 'Assets\\Audio'), 'Game\\Assets\\Audio')
        dump_file_maps(map_file_handle, os.path.join(game_folder, 'Assets\\Dialog'), 'Game\\Assets\\Dialog')
        dump_file_maps(map_file_handle, os.path.join(game_folder, 'Assets\\Icons'), 'Game\\Assets\\Icons')
        dump_file_maps(map_file_handle, os.path.join(game_folder, 'Assets\\Levels'), 'Game\\Assets\\Levels')
        dump_file_maps(map_file_handle, os.path.join(game_folder, 'Assets\\Models'), 'Game\\Assets\\Models')
        dump_file_maps(map_file_handle, os.path.join(game_folder, 'Assets\\Textures'), 'Game\\Assets\\Textures')
    print('Wrote file: ' + mapping_file)

    # Call the MSIX maker tool to build the package.
    package_file_name = os.path.join(root_folder, args.game + '.msix')
    package_cmd_args = [
        'pack',
        '/v',
        '/o',
        '/m',
        manifest_file,
        '/f',
        mapping_file,
        '/p',
        package_file_name
    ]
    package_cmd = ('"%s"' % make_appx_exe) + ' ' + ' '.join(package_cmd_args)
    run_shell_proc(package_cmd, os.getcwd())

    # Do some clean-up.
    os.remove(mapping_file)
    os.remove(manifest_file)