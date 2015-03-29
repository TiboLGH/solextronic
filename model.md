## Introduction ##

Cette page décrit le programme permettant d'appliquer le modele qui calcul les timings d'injection et d'allumage en fonction des grandeurs d'entrée :
  * tables d'avance et de remplissage
  * temperature moteur et air
  * accélérateur
  * tension de la batterie

Ce meme modele tourne dans le microcontroleur, ce programme permet de la debugger et de l'affiner "confortablement" (avec des sorties graphiques, des conditions stables, pas de contraintes de vitesse/taille de code...).


## Fonctionnalités ##

  * calcul de la taille d'injecteur optimal
  * calcul des timings d'avance à l'allumage pour comparaison avec la simulation
  * calcul des timings d'injection pour comparaison avec la simulation

## Compilation ##
Ce logiciel fonctionne sous Linux avec les bibliotheques suivantes :
  * [http://ndevilla.free.fr/iniparser/index.html](iniparser.md) et [http://ndevilla.free.fr/gnuplot/index.html](gnuplot_i.md) de Nicolas Devillard
  * [http://www.gnuplot.info/](GNUplot.md) installé

La compilation se fait avec la commande suivante :
```

cd model/
make
```

## Utilisation ##
### Fichier de configuration ###
Le fichier model.ini contient l'ensemble des parametres nécessaires au calcul

### Tables ###
Les tables sont typiquement stockées dans les fichiers ignition.csv et injection.csv.
Il doit y avoir exactement 11 lignes (1 par RPM + 1 d'en-tete) et 11 colonnes (1 par charge + 1 d'en-tete).
Le fichier ignition.csv doit comporter le keyword "Ignition" sur la 1ere ligne, 1ere colonne. Pareil pour le fichier injection.csv avec le keyword "Injection".

### Lancement ###

```

solextronic/solextronic/model} ./model.exe -h
usage : ./model.exe [options]
Les options valides sont :
-h             affiche ce message
-g             affichage des graphes (avec gnuplot)
-v             affichage des traces de debug
-f             calcul du debit de l'injecteur (calcul des tables de timings par default)
-i <inifile>   utilisation du fichier <inifile> a la place de model.ini
```
