## Introduction ##

Le boitier électronique n'est qu'une petite partie du système d'injection. Il faut en effet plusieurs autres pièces très spécifiques pour faire un bon système d'injection et d'allumage.

Cette page décrit les pièces utilisées pour les essais de Solextronic.




## Choix des pièces ##

Plusieurs choix sont possibles pour trouver les pièces nécessaires :
  1. Utiliser des composants non-spécialisés pour l'automobile : ils sont globalement moins chers et plus faciles à approvisionner mais ne supportent pas très bien les contraintes de temperature, de vibration et de liquides corrosifs.
  1. Utiliser des pièces du secteur automobile : ces pièces sont assez facile à trouver, surtout dans les casses, la plupart des voitures des 15 dernières années sont à injection. Ces pièces sont robustes mais chères et souvent adaptées aux moteurs 4 temps à 4 cylindres, loin d'un 50cm3 2 temps.
  1. Utiliser des pièces du secteur moto : les pièces sont nettement plus adaptées mais sont plus difficiles à trouver. En neuf, les prix sont prohibitifs, reste les casses motos.

J'ai choisi la dernière solution en cherchant des pièces d'occasion. La fiabilité devrait être meilleure qu'avec des composants standards ce qui devrait éviter des séances d'arrachage de cheveux.
Quelques sites qui vendent des pièces d'occasions :
  * [www.surplusmotos.com](http://www.surplusmotos.com)
  * [www.agpl.net](http://www.agpl.net)


## Détails des pièces ##

Pour simplifier la recherche de pièces, j'ai cherché un modèle de moto/scooter 125cm3 très populaire, ça permet statistiquement de trouver plus de pièces.
Pour ma part, je suis parti sur le Yamaha XMAX 125 (le plus vendu en France) et YZF-[R125](https://code.google.com/p/solextronic/source/detail?r=125), mais d'autres modèles sont possibles comme le Peugeot Elystar,  Suzuki Burgman 125 ou Honda S-Wing 125.

### Injecteur ###
L'injecteur est celui d'une YZF-[R125](https://code.google.com/p/solextronic/source/detail?r=125), monté sur sa pipe d'admission.

![http://wiki.solextronic.googlecode.com/git/img/injecteur1.jpg](http://wiki.solextronic.googlecode.com/git/img/injecteur1.jpg)
![http://wiki.solextronic.googlecode.com/git/img/injecteur2.jpg](http://wiki.solextronic.googlecode.com/git/img/injecteur2.jpg)

#### Débit ####
Un banc de test de débit va être réalisé pour mesuré le débit effectif de cet injecteur. Le principe est le suivant :
  * l'injecteur est monté sur un couvercle de pot de confiture vide en verre (ou autre bocal).
  * le tout est connecté à la pompe à essence et à la carte Solextronic
  * Un mode de test spécial est déclenché : soit on ouvre l'injecteur en continu pour précisement 10sec, soit on réalise 1000 cycles de 1ms actif toutes les 10 ms.
  * pour chaque cas on pèse le bocal seul avant et après avec une balance de précision
  * 3 minutes de calcul plus tard on obtient le débit en g/min. On a 2 valeurs :
  * le débit en régime stabilisé (ouverture continue sur 10sec) : c'est la valeur données par les constructeurs
  * le débit sur 1ms avec les temps d'ouverture et de fermeture : c'est un débit "net", pour aider à déterminer le débit dans la phase d'ouverture.

#### Caractéristiques électriques ####
Il est nécessaire de connaitre les caractéristiques électriques de l'injecteur pour adapter le driver et la loi de commande. Pour cela, il est nécessaire de mesurer la tension et le courant

### Capteur de pression d'admission, température d'air (IAT) et papillon ###
Le corps d'admission de l'YZF-[R125](https://code.google.com/p/solextronic/source/detail?r=125) contient à la fois le capteur de pression, le capteur de température d'air (IAT) et le papillon. Il inclus aussi la valve de ralenti rapide qui ne sera pas utilisée dans un 1er temps.

L'ensemble est très compacte ce qui va aider pour l'intégration sur le moteur.

Il n'y a qu'un seul connecteur 5 points pour les 3 capteurs. Voir la page [faisceau](harness.md) pour les détails.

### Capteur de temperature moteur CLT ###

### Pompe à essence & régulateur de pression ###
La pompe à essence et le régulateur de pression de XMAX 125 sont regroupés dans le reservoir. On obtient donc une pression régulée à la sortie de l'ensemble. Pour les modèles Yamaha, on a 2.5bar de pression.
L'ensemble est très compacte, mais il va falloir un peu d'astuce pour le caser dans le réservoir rond du Solex et s'assurer que la crépine est toujours au point le plus bas.
L'alimentation se fait directement en 12v à travers un relais externe.

![http://wiki.solextronic.googlecode.com/git/img/pompe-regulateur.jpg](http://wiki.solextronic.googlecode.com/git/img/pompe-regulateur.jpg)

![http://wiki.solextronic.googlecode.com/git/img/regulateur.jpg](http://wiki.solextronic.googlecode.com/git/img/regulateur.jpg)

### Jauge à essence ###
La jauge à essence vient aussi du XMAX 125. C'est le grand tube noir associé à la pompe à essence. Là encore, il va être difficile de l'adapter au réservoir du Solex.

### Allumage ###
Le schema de Solextronic inclut déjà une grande partie des composants du bloc d'allumage. Il suffit donc d'ajouter une bobine haute-tension, un faisceau HT et un capuchon/antiparasite pour la bougie.
L'ensemble vient aussi d'un XMAX 125 et sera monté au plus proche de la bougie.

![http://wiki.solextronic.googlecode.com/git/img/bobine.jpg](http://wiki.solextronic.googlecode.com/git/img/bobine.jpg)