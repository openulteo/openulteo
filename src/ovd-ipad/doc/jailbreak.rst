Jailbreak
=========

Pour pouvoir tester une application sur un périphérique, il est nécessaire
d'avoir un compte développeur. Ce compte permet d'obtenir les certificats
nécessaires pour la signature du code. L'iPhone ou l'iPad refuseront d'exécuter
du code non signé.

Il est possible de patcher le mécanisme de vérification de signature à l'aide
d'un périphérique jailbreaké. 

Il est aussi possible de forcer Xcode à ne pas obliger l'utilisateur à signer
son code.

Périphérique
------------

Dans Cydia, ajouter la source suivante ::
    
    http://cydia.hackulo.us/

Un avertissement s'affiche pour indiquer que cette source permet de pirater les
applications. Valider quand même.

Installer alors le paquet *AppSync for 4.0+*. Ce paquet invalide la vérification
de la signature et permet de télécharger des applications crackées. Notre
application étant signée de manière invalide, elle pourra tout de même être
lancée.

Xcode
-----

La procédure_ donne des détails sur les différents fichiers de configuration à
modifier et sur la façon de patcher Xcode.

Il faut ré exécuter le patch lors d'une mise à jour d'Xcode.

.. _procédure: http://www.alexwhittemore.com/?p=398
