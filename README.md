# dns.c — Serveur DNS minimal en C

Un serveur DNS écrit from scratch en C, sans librairie externe. Répond aux requêtes A record via UDP, avec une table de résolution statique et blocage de domaines.

## Fonctionnalités

- Écoute les requêtes DNS en UDP
- Parse le format binaire DNS (header + labels)
- Résout les domaines depuis une table statique
- Bloque les domaines indésirables en répondant `0.0.0.0`
- Répond `0.0.0.0` pour tout domaine inconnu

## Compilation

```bash
gcc dns.c -o dns
```

## Utilisation

```bash
./dns
```

Le serveur écoute sur le port `8053`. Pour tester :

```bash
dig @127.0.0.1 -p 8053 google.com
```

## Table de résolution

Modifiable directement dans `main()` :

```c
Record table[] = {
    {"google.com",   {142, 250, 74, 46}},
    {"facebook.com", {0, 0, 0, 0}},       // bloqué
    {"youtube.com",  {172, 217, 20, 174}},
    {NULL, {0}}                             // sentinelle de fin
};
```

## Format DNS implémenté

```
[ Header 12 octets ]
[ Question         ]  — parsing des labels (ex: \x06google\x03com\x00)
[ Réponse A record ]  — pointeur 0xC00C + TTL 300 + IPv4
```

## Ce qui n'est pas implémenté

- Récursion (forwarding vers un vrai serveur DNS)
- AAAA records (IPv6)
- Chargement de la table depuis un fichier
- Support de plusieurs questions par paquet

## Contexte

Projet d'apprentissage de la programmation réseau en C — sockets UDP, parsing binaire big-endian, format du protocole DNS (RFC 1035).
