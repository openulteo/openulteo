Environnement de compilation
============================

Introduction
------------

L'environnement de compilation nécessaire au développement d'application pour
iOS fonctionne uniquement à l'aide de Mac OS X. 

Préalable
---------

- Mac OS X Snow Leopard (10.6.6)
- Xcode

Mise en place de l'environnement
--------------------------------

Sources
~~~~~~~

On considéra pour la suite du document que les sources sont disponible dans le
répertoire ``~/code``.

Xcode
~~~~~
L'installation d'Xcode_ fournit l'environent de développement (IDE) (Xcode), les
programmes nécessaires à la compilation (gcc, clang, llvm) et les SDKs de Mac OS
X et d'iOS.

Il est nécessaire de télécharger Xcode par l'intermédiaire de l'App Store (~
4€), ou de le récupérer sur le site developer.apple.com après avoir souscrit à
un contrat développeur.

L'installation d'Xcode commence par l'ouverture de l'image disque (ex:
Xcode-401.dmg) et se poursuit en exécutant l'assistant d'installation (Xcode and
iOS SDK.mpkg).

Les choix par défaut conviennent à une installation standard, il n'est pas
nécessaire des les modifier. L'installation occupe approximativement 10 Gio d'espace
disque et nécessite environ 20 minutes pour se terminer.

    L'environnement de compilation est fonctionnel pour des tâches courantes de
    création d'applications aussi bien pour bureau que mobiles. L'OVD iOS
    nécessite l'utilisation d'outils annexes.

Homebrew
~~~~~~~~

Homebrew_ est un gestionnaire de paquet source à la manière d'emerge_. Il permet
de compiler et d'installer de nombreux utilitaires UNIX.

Il existe aussi macport_ et fink_ mais ils sont plus difficiles à mettre en
place, et les règles d'installation des autotools ont été écrites spécifiquement
pour homebrew.

Après avoir ouvert Terminal.app (par l'intermédiaire de Spotlight ou par Finder
dans le dossier /Applications/Utilities), exécuter la commande suivante ::

    ruby -e "$(curl -fsSL https://gist.github.com/raw/323731/install_homebrew.rb)"

Ce script s'occupe de :

- changer les permissions par défaut pour laisser l'utilisateur installer des programmes (nécessite le mot de passe root)
- récupérer les dernières sources de homebrew et de configurer le PATH de l'utilisateur.

Le programme ``brew`` est maintenant disponible. Ses commandes principales
sont :

- ``install`` : installe un nouveau paquet
- ``search`` : recherche dans la base un paquet
- ``uninstall`` : supprime un paquet

On peut tester l'installation en récupérant git qui nous servira pour la gestion
des sources ::

brew install git


Homebrew installe tous les programmes demandés dans un répertoire qui lui est
propre. Des liens symboliques sont ensuite réalisés pour obtenir les binaires
dans ``/usr/local/bin``. Le ``$PATH`` par défaut de Mac OS X place en premier
les répertoires ``/usr/bin/`` et ``/usr/sbin`` avant le répertoire
``/usr/bin/local``. On désire exécuter les programmes nouvellement installé en
priorité. Il faut donc modifier la variable d'environnement.

Éditer le fichier ``~/.profile`` en y copiant la ligne suivante ::
    
    export PATH=/usr/local/bin:$PATH

Faire les modifications dans le shell courant ::
    
    source ~/.profile

Cette étape est nécessaire pour la compilation des autotools.

Autotools
~~~~~~~~~
FreeRDP nécessite Automake 1.11, la version fournie par défaut par Mac OS X est
la 1.10. 

On va utiliser ``brew`` pour installer les autotools.

Se rendre dans le répertoire ``~/code/homebrew`` et exécuter les commandes
suivantes :

- ``brew install ./m4.rb``
- ``brew install ./autoconf.rb``
- ``brew install ./automake.rb``
- ``brew install ./libtool.rb``

OpenSSL
~~~~~~~
FreeRDP nécessite OpenSSL pour fonctionner. Une bibliothèque statique est à
disposition dans le répertoire ``OpenSSL-for-iPhone`` aussi bien pour le
simulateur que pour un périphérique.

Les autotools chargés de la compilation de FreeRDP ont besoin de vérifier la
présence d'OpenSSL à l'aide de macros. Ces macros échouent à cause d'une
incompatibilité entre la bibliothèque standard issue de BSD.

Le seul moyen de résoudre ce problème est de faire un lien symbolique dans le
SDK de l'iPhone Simulator vers le répertoire ``include`` d'OpenSSL ::

    cd /Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator4.3.sdk/usr/include 
    ln -s ~/code/OpenSSL-for-iPhone/include/openssl

Dans le cas d'un changement de version de SDK, il faut refaire cette
manipulation.

Ce lien symbolique permet à la chaine de compilation de générer des
bibliothèques statiques aussi bien pour l'architecture du simulateur (i386) que
pour les périphériques (arm). 


.. _Xcode: http://developer.apple.com/technologies/tools/whats-new.html
.. _Homebrew: https://github.com/mxcl/homebrew
.. _emerge: http://fr.wikipedia.org/wiki/Emerge
.. _macport: http://www.macports.org/
.. _fink: http://www.finkproject.org/
