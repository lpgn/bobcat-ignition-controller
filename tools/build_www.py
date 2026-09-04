# PlatformIO extra script: pack www/ into data/ for LittleFS.
# Text assets are gzipped (the firmware's AsyncFileResponse serves name.gz with
# Content-Encoding: gzip when the plain file is absent); binaries are copied.
Import("env")
import gzip, os, shutil
src = os.path.join(env["PROJECT_DIR"], "www")
dst = env.subst("$PROJECT_DATA_DIR")
TEXT = (".html", ".js", ".css", ".json", ".svg", ".txt")
if os.path.isdir(dst):
    shutil.rmtree(dst)
os.makedirs(dst)
for name in sorted(os.listdir(src)):
    p = os.path.join(src, name)
    if not os.path.isfile(p):
        continue
    if name.endswith(TEXT):
        with open(p, "rb") as i, gzip.open(os.path.join(dst, name + ".gz"), "wb", compresslevel=9) as o:
            o.write(i.read())
    else:
        shutil.copy2(p, dst)
print("www/ -> data/: " + ", ".join(sorted(os.listdir(dst))))
