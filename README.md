# dns.c — Proxy DNS en C

Un serveur DNS proxy écrit from scratch en C, sans librairie externe. Résout les domaines depuis une table statique chargée depuis un fichier, bloque les domaines indésirables, et forward les requêtes inconnues vers Google DNS (`8.8.8.8`).

## Fonctionnalités

- Écoute les requêtes DNS en UDP sur le port 8053
- Parse le format binaire DNS (header + labels)
- Charge la table de résolution depuis un fichier `blocklist.txt`
- Bloque les domaines indésirables en répondant `0.0.0.0`
- Forward les domaines inconnus vers `8.8.8.8` et retransmet la réponse au client

## Compilation

```bash
gcc dns.c -o dns
```

## Utilisation

```bash
./dns
```

Le serveur charge `blocklist.txt` au démarrage puis écoute sur le port `8053`.

```bash
# Tester avec dig
dig @127.0.0.1 -p 8053 google.com    # résolu depuis la table
dig @127.0.0.1 -p 8053 facebook.com  # bloqué → 0.0.0.0
dig @127.0.0.1 -p 8053 twitch.tv     # forwarded vers 8.8.8.8
```

## Format de blocklist.txt

Un enregistrement par ligne : `domaine ip`

```
google.com 142.250.74.46
facebook.com 0.0.0.0
youtube.com 172.217.20.174
```

- IP normale → résout vers cette adresse
- `0.0.0.0` → domaine bloqué
- Domaine absent du fichier → forwarded vers Google DNS

Le fichier supporte jusqu'à 50 enregistrements (`#define MAX 50`).

## Architecture

```
main()
├── load_table()       — charge blocklist.txt au démarrage
├── init_sockfd()      — crée le socket UDP
├── init_addr()        — configure l'adresse d'écoute
└── boucle principale
    ├── recvfrom()     — reçoit la requête DNS
    ├── save_head()    — parse le header (12 octets)
    └── send_resp()
        ├── parse_name()   — extrait le nom de domaine (labels)
        ├── save_quest()   — extrait qtype et qclass
        ├── build_header() — construit le header de réponse
        ├── build_resp()
        │   ├── resolve()  — cherche dans la table
        │   └── forward()  — forward vers 8.8.8.8 si inconnu
        └── sendto()       — envoie la réponse au client
```

## Format DNS implémenté

```
[ Header 12 octets  ]  — ID, FLAGS, QDCOUNT, ANCOUNT...
[ Question          ]  — labels encodés \x06google\x03com\x00
[ Réponse A record  ]  — pointeur 0xC00C + TTL 300 + IPv4 4 octets
```

## Ce qui n'est pas implémenté

- AAAA records (IPv6)
- Cache des réponses forwardées
- Support de plusieurs questions par paquet
- Timeout sur le forwarding

## Contexte

Projet d'apprentissage de la programmation réseau en C — sockets UDP, parsing binaire big-endian, format du protocole DNS (RFC 1035), proxy DNS avec forwarding.
