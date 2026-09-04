# PlatformIO extra script: pack www/ into data/ for LittleFS.
# index.html is produced as ONE self-contained file (style.css, script.js and logo.png inlined):
# on a weak WiFi link the ESP32 stalls when a browser opens several connections at once, and the
# page looked frozen while script.js waited. One request, one gzip stream, done.
# Text assets are gzipped (AsyncFileResponse serves name.gz with Content-Encoding: gzip when the
# plain file is absent); binaries are copied as-is.
Import("env")
import base64, gzip, os, re, shutil
src = os.path.join(env["PROJECT_DIR"], "www")
dst = env.subst("$PROJECT_DATA_DIR")
TEXT = (".html", ".js", ".css", ".json", ".svg", ".txt")

def read(name, mode="r"):
    with open(os.path.join(src, name), mode, **({} if "b" in mode else {"encoding": "utf-8"})) as f:
        return f.read()

def inline(html, js_name):
    """Inline style.css, the page's script and the logo so the page is one request."""
    css = read("style.css"); js = read(js_name)
    logo = "data:image/png;base64," + base64.b64encode(read("logo.png", "rb")).decode()
    link = '<link rel="stylesheet" href="style.css">'; tag = '<script src="%s"></script>' % js_name
    for needle in (link, tag):
        if needle not in html:
            raise SystemExit("build_www: expected %s in page (found: %s)" % (needle, re.findall(r'<(?:script|link)[^>]*>', html)))
    html = html.replace(link, "<style>\n" + css + "\n</style>", 1)
    html = html.replace(tag, "<script>\n" + js.replace("</script", "<\\/script") + "\n</script>", 1)
    return html.replace('src="logo.png"', 'src="' + logo + '"', 1)

def gz(data, out):
    with gzip.open(out, "wb", compresslevel=9) as o:
        o.write(data if isinstance(data, bytes) else data.encode("utf-8"))

if os.path.isdir(dst):
    shutil.rmtree(dst)
os.makedirs(dst)
for name in sorted(os.listdir(src)):
    p = os.path.join(src, name)
    if not os.path.isfile(p):
        continue
    if name == "index.html":
        gz(inline(read(name), "script.js"), os.path.join(dst, "index.html.gz"))
    elif name == "settings.html":
        gz(inline(read(name), "settings.js"), os.path.join(dst, "settings.html.gz"))
    elif name.endswith(TEXT):
        gz(read(name, "rb"), os.path.join(dst, name + ".gz"))
    else:
        shutil.copy2(p, dst)
print("www/ -> data/: " + ", ".join(f"{n} {os.path.getsize(os.path.join(dst, n))}B" for n in sorted(os.listdir(dst))))
