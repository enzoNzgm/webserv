#!/usr/bin/env python3
import os
import sys
from email.parser import BytesParser
from email.policy import HTTP
import re

UPLOAD_DIR = "./website/uploads"
MAX_FILE_SIZE = 10 * 1024 * 1024  # 10 MB
ALLOWED_EXTENSIONS = {'.txt', '.jpg', '.jpeg', '.png', '.gif', '.pdf', '.zip', '.html', '.css', '.js'}

def is_safe_filename(filename):
    """Vérifie que le nom de fichier est sûr"""
    # Refuse les chemins relatifs et absolus
    if filename != os.path.basename(filename):
        return False
    # Refuse les caractères dangereux
    if re.search(r'[<>:"|?*\x00-\x1f]', filename):
        return False
    # Refuse les noms spéciaux
    if filename in ('', '.', '..'):
        return False
    return True

def get_file_extension(filename):
    """Retourne l'extension du fichier"""
    return os.path.splitext(filename)[1].lower()

print("Content-Type: text/html\r\n\r\n")

try:
    # Crée le répertoire d'upload s'il n'existe pas
    if not os.path.exists(UPLOAD_DIR):
        os.makedirs(UPLOAD_DIR, mode=0o755)

    # Parse le MIME multipart depuis stdin
    content_type = os.environ.get("CONTENT_TYPE", "")
    body = sys.stdin.buffer.read()
    message = BytesParser(policy=HTTP).parsebytes(
        b"Content-Type: " + content_type.encode() + b"\r\n\r\n" + body)

    fileitem = None
    if message.is_multipart():
        for part in message.iter_parts():
            if part.get_param("name", header="content-disposition") == "file":
                fileitem = part
                break

    if fileitem is None or not fileitem.get_filename():
        print("<html><body><h1>Erreur: Aucun fichier envoyé</h1></body></html>")
        sys.exit(0)

    filename = os.path.basename(fileitem.get_filename())

    # Vérifie la sécurité du nom de fichier
    if not is_safe_filename(filename):
        print("<html><body><h1>Erreur: Nom de fichier invalide</h1></body></html>")
        sys.exit(0)

    # Vérifie l'extension
    ext = get_file_extension(filename)
    if ext not in ALLOWED_EXTENSIONS:
        print(f"<html><body><h1>Erreur: Extension '{ext}' non autorisée</h1>"
              f"<p>Extensions autorisées: {', '.join(ALLOWED_EXTENSIONS)}</p></body></html>")
        sys.exit(0)

    filepath = os.path.join(UPLOAD_DIR, filename)

    # Vérifie si le fichier existe déjà
    if os.path.exists(filepath):
        base, extension = os.path.splitext(filename)
        counter = 1
        while os.path.exists(filepath):
            filename = f"{base}_{counter}{extension}"
            filepath = os.path.join(UPLOAD_DIR, filename)
            counter += 1

    # Lit et écrit le fichier avec limite de taille
    file_data = fileitem.get_payload(decode=True) or b""

    if len(file_data) > MAX_FILE_SIZE:
        print(f"<html><body><h1>Erreur: Fichier trop volumineux</h1>"
              f"<p>Taille maximale: {MAX_FILE_SIZE / (1024*1024):.0f} MB</p></body></html>")
        sys.exit(0)

    with open(filepath, "wb") as f:
        f.write(file_data)

    # Définit les permissions du fichier
    os.chmod(filepath, 0o644)

    print(f"""
    <html>
    <head><title>Upload réussi</title></head>
    <body>
        <h1>Fichier <b>{filename}</b> uploadé avec succès !</h1>
        <p>Taille: {len(file_data)} octets</p>
        <p>Stocké dans : <code>{UPLOAD_DIR}</code></p>
        <a href="/">Retour à l'accueil</a>
    </body>
    </html>
    """)

except Exception as e:
    print(f"<html><body><h1>Erreur interne: {str(e)}</h1></body></html>")
    sys.exit(1)
