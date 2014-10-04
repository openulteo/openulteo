Installation Mac OS X
=====================

Différentes méthodes existent pour installer Mac OS X dans un environnement de
virtualisation ou sur machine physique.

Officiellement le contrat de licence de Mac OS X interdit de  virtualiser une
version standard de Mac OS X sur tout type d'ordinateur. La seule autorisation
concédée est la virtualisation de Mac OS X Server sur un Mac uniquement.

Les éditeurs de logiciels de virtualisation supportent  Mac OS X Server de
manière officielle uniquement sur un Mac. Mais grâce à l'implantation de l'EFI
(le successeur du BIOS utilisé pour démarrer Mac OS X sur tous les Mac) il est
maintenant possible de démarrer facilement une VM sans émuler l'EFI de manière
externe.

La future version de Mac OS X (Lion) ne fera pas de distinction entre la version
serveur et la version client, il y aura peut être des changements dans le
contrat de licence.


DVD officiel
------------

Le DVD officiel de Mac OS X Server doit pouvoir s'installer dans une VM qui
fonctionne à l'aide d'un hyperviseur tournant sous Mac OS X.

Les hyperviseurs actuels supportent l'émulation de l'EFI sur tous les systèmes
hôtes, il est donc possible d'installer un Mac OS X Server sans d'autre
modifications.

Il faut paramétrer la VM de la manière suivante :

- utiliser l'EFI à la place du BIOS
- avoir un espace disque de 20 Gio
- affecter au minimum 1 Gio de mémoire vive
- affecter au minimum 64 Mio de mémoire graphique


Image VMWare/VirtualBox
-----------------------

Il existe sur le web des images de VM pour VMWare et VirtualBox prêtes à être
utilisées. Généralement des patchs et des modifications sont appliqués pour
permettre le démarrage malgré l'absence d'EFI et le support d'un maximum de
périphérique.

Les éditeurs d'hyperviseurs ne fournissent pas de support des invités Mac OS X
autrement que pour la version Mac OS X hôte. Ils n'ont pas de raisons d'en
fournir pour d'autre hôte vu le contrat de licence de Mac OS X.

Il est important de ne pas mettre à jour sans lire les instructions sur les
sites web informés. Généralement des modifications au niveau du noyau et de ses
extensions empêchent le prochain redémarrage.

Hackintosh
----------

À la manière des image VMWare/VirtualBox, il existe des images de DVD ou de
partitions complètes qui sont spécialement modifiée pour permettre à un PC de
démarrer Mac OS X.

Les mêmes restrictions s'appliquent, il est difficile de mettre à jour le
système sans casse, il faut bien prévoir le déroulement des opérations.
