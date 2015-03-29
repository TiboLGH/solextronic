# Introduction #

Ce bloc inclus 2 fonctions relatives a l'allumage :
  * la generation d'un "haute tension" (~300VDC) pour charger le condensateur d'allumage.
  * le conrole de ce condensateur d'allumage

Ilne reste donc plus qu'a installer une bobine haute tension peu couteuse ( 10-15 euros) pour alimenter la bougie.

Pour générer la haute tension, on va utiliser un convertisseur DC-DC de topologie Flyback à partir du 12VDC de la batterie.

# Schema et PCB : attention ! Non realise ! #

![http://solextronic.googlecode.com/git/hardware/DCDC/DCDC.png](http://solextronic.googlecode.com/git/hardware/DCDC/DCDC.png)

[En PDF](http://solextronic.googlecode.com/git/hardware/DCDC/DCDC.pdf).

Un 1er jet du PCB :

![http://solextronic.googlecode.com/git/hardware/DCDC/DCDC_brd.png](http://solextronic.googlecode.com/git/hardware/DCDC/DCDC_brd.png)

# Details #

Le montage se base sur les schema des sites proposes dans la page liens.
La 1ere approche consistait à utiliser l'AVR pour générer le signal PWM et faire fonctionner la boucle d'asservissement en tension. L'idée était de réduire le nombre de composants.

En regardant le problème plus en détail, il est apparut que réaliser ce genre d'alimentation avec un bon rendement nécessite des optimisations assez pointues sur la mesure et le controle ainsi qu'un montage relativement complexe (driver de FET, boucle de retour).

Des circuits spécialisés intègrent toutes ces problématiques et permettent de faire une solution plus facile à réaliser et à mettre au point, avec finalement moins de composants.
Parmi ces composants, on peut utiliser le UC3845 ou le LT3757.

Le montage doit avoir les caractéristiques suivantes :
  * Elévation de tension de 12VDC à ~300VDC
  * Puissance d'environ 5W (soit environ 500mA de conso sur le 12v)
  * Le meilleur rendement possible > 70%
  * Réalisation avec des composants "classiques", facilement trouvables ou récupérables sur une alim de PC par exemple.
  * Commande de l'alimentation pour forcer des periodes ON ou OFF.

# Liens #
  * [Schéma bloc CDI du site Transmic.net, basé sur un UC3845](http://www.transmic.net/DC-CDI-v20/DC-CDI-Schematic.pdf)
  * [Description du principe flyback](http://www.dos4ever.com/flyback/flyback.html)
  * [Schema simplifié sur base UC3845](http://www.sportdevices.com/ignition/inverter.htm)
  * [Application note TI pour le UC3845](http://www.ti.com/lit/an/slua143/slua143.pdf)
  * [Datasheet TI du UC3845](http://www.ti.com/lit/ds/symlink/uc3845.pdf)
  * [Datasheet et application note du LT3757](http://cds.linear.com/docs/en/datasheet/3757Afd.pdf)