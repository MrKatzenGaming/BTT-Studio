import shutil
import os
import zipfile

projpath = os.path.abspath("/home/kassu/SMO-Projekte/BTT-Studio")
buildpath = os.path.join(projpath, "build")
exefspath = os.path.join(projpath, "atmosphere", "contents", "0100000000010000", "exefs")
romfspath = os.path.join(projpath, "atmosphere", "contents", "0100000000010000", "romfs")


def pack():
    os.makedirs(exefspath, exist_ok=True)
    os.makedirs(romfspath, exist_ok=True)
    shutil.copy(os.path.join(buildpath, "BTT-Studio.nso"), os.path.join(exefspath, "subsdk4"))
    shutil.copy(os.path.join(buildpath, "main.npdm"), os.path.join(exefspath, "main.npdm"))
    shutil.copytree(os.path.join(projpath, "romfs"), romfspath, dirs_exist_ok=True)
    
    
    with zipfile.ZipFile(os.path.join(projpath,"make-release", "Btt-Studio.zip"), 'w', zipfile.ZIP_DEFLATED) as smm_zip:
        for root, dirs, files in os.walk(os.path.join(projpath, "atmosphere")):
            for file in files:
                file_path = os.path.join(root, file)
                smm_zip.write(file_path, os.path.relpath(file_path, projpath))
    print("BTT-Studio.zip created successfully in make-release directory.")
    
    os.renames(os.path.join(projpath, "atmosphere"), os.path.join(projpath, "BTT-Studio"))

    with zipfile.ZipFile(os.path.join(projpath,"make-release", "Btt-Studio-SimpleModManager.zip"), 'w', zipfile.ZIP_DEFLATED) as smm_zip:
        for root, dirs, files in os.walk(os.path.join(projpath, "BTT-Studio")):
            for file in files:
                file_path = os.path.join(root, file)
                smm_zip.write(file_path, os.path.relpath(file_path, projpath))
    print("Btt-Studio-SimpleModManager.zip created successfully in make-release directory.")
    
    shutil.rmtree(projpath+"/BTT-Studio")
    
    shutil.copy(os.path.join(buildpath, "BTT-Studio.nss"), os.path.join(projpath, "make-release", "BTT-Studio.nss"))
    

    
if __name__ == "__main__":
    pack()