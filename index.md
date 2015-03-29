# Specifications de SolexTronic #


## 1. Introduction ##
Solextronic est un boitier d'injection et d'allumage electronique pour Solex.

## 2. Design global ##


![http://wiki.solextronic.googlecode.com/git/global.png](http://wiki.solextronic.googlecode.com/git/global.png)


## 3. Principes ##
Le calcul des temps d'injection est base sur les principes decrit sur l'excellent site [MegaSquirt](http://www.megamanual.com/mtabcon.htm).

## 4. Simulation ##

Parametres d'entree :

  * RPM
  * Ouverture papillon
  * Temperature culasse
  * Temperature admission
  * Tension batterie
  * Debit injecteur
  * Avance admission
  * Courbe de remplissage en % (en fonction du RPM/charge)
  * Table d'avance (en fonction du RPM/charge)
  * Regime max
  * Temps d'injection maximal


Donnees de sortie :

  * Courbe d'avance a l'allumage (en us)
  * Courbe d'injection (temps et avance, en us)
  * Warning sur les temps d'injection min/max


## 7. Controle externe ##
SolexTronic est piloté à travers une application PC (ou un terminal) connecté sur un port série ou un bridge USB-série.
Les commandes sont au format texte et donc utilisable dans un terminal et sont detaillées dans la page [Firmware](firmware.md).