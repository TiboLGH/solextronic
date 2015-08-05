Solextronic est un ECU (Electronic Control Unit) pour Solex visant à controler l'injection et l'allumage.

## News
Le projet est en développement lent, les idées allant bien plus vites que le temps de réalisation disponible (je ne consacre que quelques heures par semaine à Solextronic...)
J'espère avoir un prototype plus ou moins fonctionnel d'ici quelques mois ;-).

### 5 Aout 2015 : Sortie de la version 0.2
Ca avance ! mais pas vite... Les principales nouveautées :
  * La 1ere carte de test est faite (sur plaque de test a trous). Elle s'accompagne du carte de test.
  * Le fichier de config de Tunerstudio a ete mis a jour, un script automatique est en cours de developpement
  * Nouvelle gestion des ADC, de la pompe a essence...
  * Ajout du LCD/bouton

### 12 Juillet 2015 
Pas mal de commits a droite a gauche et la definition du faisceau :
![https://github.com/TiboLGH/solextronic/blob/master/hardware/harness/harness.png]
(https://github.com/TiboLGH/solextronic/blob/master/hardware/harness/harness.png)


### 28 Mars 2015
Migration vers Github étant donné que Google Code va bientôt fermer !


### 20 Avril 2014 : 1er schéma du bloc CDI
Un peu de hardware : schema et PCB du bloc CDI associant une alimentation haute-tension et un allumage de type [CDI](http://fr.wikipedia.org/wiki/Allumage_%C3%A0_d%C3%A9charge_capacitive).
Attention ! ce sont juste les plans, pas encore réalisé/testé !

Voir la page flyback.


### 9 Décembre 2013 : Changement d'orientation pour le logiciel de controle
J'avais prévu au départ de développer un logiciel de controle sous Android. Finalement, après quelques recherches sur le web pour chercher l'inspiration ;-), il apparait que le soft de controle utilisé par [Megasquirt](http://www.megasquirt.info/), [TunerStudio](http://www.tunerstudio.com/) est en fait compatible avec plusieurs ECU (à commencer par la famille MegaSquirt). En fait, un "simple" fichier INI permet de parametrer l'interface et les commandes série.
Les software TunerStudio sont :
  * réellement bons et beaux,
  * multiplatformes (Java inside, pour le meilleur et pour le pire, mais compatible Windows, MacOs, Linux et même une version Android),
  * bien supportés
Inutile de réinventer la roue, j'adapte mon jeu de commandes pour être compatible et je vais produire des fichiers de configurations pour TunerStudio.
Seul défaut, il n'est pas libre : voir [MegaTunix](https://github.com/djandruczyk/MegaTunix) pour une alternative libre.

### 3 Décembre 2013 : Tag de la version 0.1
La version 0.1 est de sortie ! Au menu :
  * mesure du RPM, mesures des temperatures, tension batterie et position papillon.
  * Simulation : integration en simulation des mesures externes
  * Carte : carte de test simplifiée sur plaque à pastilles et generateurs externes de signaux de tests => les schemas sont fait mais pas les cartes !
  * Software de controle : console série
  * En bonus : j'ai démarré le programme de calcul des table d'injection/allumage. Pour l'instant il est capable de lire des fichiers CSV ! Il y a du taf...

Pour la suite, je m'attaque à la génération des signaux d'allumage et d'injection.


## Fonctionnalités
  * Allumage de type DC-CDI avec générateur haute-tension intégré à partir de la batterie.
    * Courbes d'avance dépendant du régime et de la charge
    * Limitation de régime par rupteur logiciel
    * Limitation des détonations par diminution de l'avance en cas de surchauffe
  * Injection de type synchronisée
    * Controle d'un injecteur haute ou basse impedance
    * Courbes d'avance dépendant du régime et de la charge
  * Surveillance de la temperature moteur
  * Surveillance de la tension batterie
  * Interfaces :
    * écran LCD 2x16 caracteres + 5 boutons
    * Liaison série sur USB pour pilotage par PC
    * Liaison Bluetooth pour pilotage par PC ou smartphone Android (avec TunerStudio)

