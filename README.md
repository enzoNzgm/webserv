# webserv

Serveur HTTP/1.1 écrit en C++98, inspiré de nginx, projet de l'école 42 Paris réalisé en groupe.

## Fonctionnalités

- Fichier de configuration façon nginx (`server`, `location`, `listen`, `root`, `error_page`, `limit_except`…)
- Multiplexage des connexions non bloquant avec `epoll`
- Méthodes GET, POST (upload de fichiers) et DELETE
- Exécution de scripts CGI (Python)
- Pages d'erreur personnalisées, autoindex

## Utilisation

```bash
make
./webserv webserv.conf
```

Puis ouvrir http://127.0.0.1:8080.

## Structure

- `srcs/`, `incs/` : le serveur
- `cgi-bin/` : scripts CGI de démo
- `website/` : site de test
- `ma_partie/` : ma partie du projet (parsing des requêtes, réponses, GET/POST, CGI) avant l'intégration

## Auteurs

Zakaria Meliani, Enzo Nzuguem Tiko
