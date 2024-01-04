Import("env")
import os, zipfile, shutil
from pathlib import Path

# Get the version number from the build environment.
firmware_version = os.environ.get('VERSION', "")

# Get the ZIP filename from the build environment.
community_project = env.GetProjectOption('community_project', "")

# Get the custom folder from the build environment.
community_path = env.GetProjectOption('custom_device_folder', "")


firmware_version = firmware_version.lstrip("v")
firmware_version = firmware_version.strip(".")


def copy_fw_files (source, target, env):
    if firmware_version == "0.0.1":
        return

    fw_file_name=str(target[0])

    if os.path.exists(community_path + "/Community/firmware") == False:
        os.mkdir(community_path + "/Community/firmware")
    
    if fw_file_name[-3:] == "bin":
        fw_file_name=fw_file_name[0:-3] + "uf2"

    shutil.copy(fw_file_name, community_path + "/Community/firmware")

    original_folder_path = community_path + "/Community"
    zip_file_path = Path('./zip_files/' + community_project + '_' + firmware_version + '.zip')
    new_folder_in_zip = community_project
    createZIP(original_folder_path, zip_file_path, new_folder_in_zip)


def createZIP(original_folder_path, zip_file_path, new_folder_name):
    if os.path.exists("./zip_files") == False:
        os.mkdir("./zip_files")
    with zipfile.ZipFile(zip_file_path, 'w') as zipf:
        for root, dirs, files in os.walk(original_folder_path):
            for file in files:
                # Create a new path in the ZIP file
                new_path = os.path.join(new_folder_name, os.path.relpath(os.path.join(root, file), original_folder_path))
                # Add the file to the ZIP file
                zipf.write(os.path.join(root, file), new_path)

def createZIP_old():
    if os.path.exists("./zip_files") == False:
        os.mkdir("./zip_files")
    
    complete_ZIP_Filename = Path('./zip_files/' + community_project + '_' + firmware_version + '.zip')
    path_to_archive = Path("./" + community_path + "/Community")
    with zipfile.ZipFile(complete_ZIP_Filename, "w", zipfile.ZIP_DEFLATED) as zipf:
        for fp in path_to_archive.glob("**/*"):
            zipf.write(fp, arcname=fp.relative_to(path_to_archive))


env.AddPostAction("$BUILD_DIR/${PROGNAME}.hex", copy_fw_files)
env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", copy_fw_files)
