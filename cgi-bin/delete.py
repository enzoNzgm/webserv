#!/usr/bin/env python3
import os
import sys
import urllib.parse
import re

UPLOAD_DIR = "./website/uploads"

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

def is_path_safe(filepath, base_dir):
    """Vérifie que le chemin reste dans le répertoire de base"""
    abs_base = os.path.abspath(base_dir)
    abs_file = os.path.abspath(filepath)
    return abs_file.startswith(abs_base)

print("Content-Type: text/html\r\n\r\n")

try:
    # Lire body depuis stdin (Content-Length est géré par le serveur)
    content_length = os.environ.get('CONTENT_LENGTH', '0')

    try:
        length = int(content_length)
    except ValueError:
        length = 0

    if length > 0:
        body = sys.stdin.read(length)
    else:
        body = sys.stdin.read()

    params = urllib.parse.parse_qs(body)

    if "filename" not in params:
        print("<html><body><h2>Erreur: aucun nom de fichier spécifié</h2></body></html>")
        sys.exit(0)

    filename = params["filename"][0].strip()

    # Vérifie que le nom de fichier n'est pas vide
    if not filename:
        print("<html><body><h2>Erreur: nom de fichier vide</h2></body></html>")
        sys.exit(0)

    # Extrait uniquement le nom de base (sécurité contre path traversal)
    filename = os.path.basename(filename)

    # Vérifie la sécurité du nom de fichier
    if not is_safe_filename(filename):
        print("<html><body><h2>Erreur: nom de fichier invalide ou dangereux</h2></body></html>")
        sys.exit(0)

    filepath = os.path.join(UPLOAD_DIR, filename)

    # Double vérification du chemin (protection contre path traversal)
    if not is_path_safe(filepath, UPLOAD_DIR):
        print("<html><body><h2>Erreur: accès refusé</h2></body></html>")
        sys.exit(0)

    # Vérifie que c'est bien un fichier et pas un répertoire
    if os.path.exists(filepath):
        if not os.path.isfile(filepath):
            print(f"<html><body><h2>Erreur: '{filename}' n'est pas un fichier</h2></body></html>")
            sys.exit(0)

        # Supprime le fichier
        os.remove(filepath)
        print(f"""
        <html>
        <head><title>Suppression réussie</title></head>
        <body>
            <h2>Fichier supprimé: {filename}</h2>
            <a href="/">Retour à l'accueil</a>
        </body>
        </html>
        """)
    else:
        print(f"""
        <html>
        <body>
            <h2>Erreur: fichier '{filename}' introuvable</h2>
            <a href="/">Retour à l'accueil</a>
        </body>
        </html>
        """)

except Exception as e:
    print(f"<html><body><h2>Erreur interne: {str(e)}</h2></body></html>")
    sys.exit(1)
