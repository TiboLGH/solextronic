Le firmware est concu pour tourner sur un Atmel ATMega328 utilisé sur les cartes Arduino. Il est programme en C "pur" + AVR LibC, donc sans les librairies Arduino.

La simulation s'effectue grace à SimAvr, projet libre de simulation de core AVR. La particularite de ce simulateur est pouvoir être utilise de plusieurs façon :

  * simulateur classique avec un interface GDB intégrée pour faire du debug pas-à-pas, visualisation de variables...
  * generateur de fichiers waveform VCD des registres internes (voir un exemple dans les liens en fin de page)
  * librairie utilisable dans un programme de test : le coeur de simulation est piloté par ce programme et des périphériques peuvent être simulés.

C'est cette dernière approche qui sera utilisée pour permettre d'implementer des tests de régression automatiques. L'idée est d'avoir quelquechose de très automatique donc facile à lancer, ce qui incite à l'utiliser le plus souvent possible.

## 1. Architecture ##


## 2. Peripheriques simules ##
### 2.1. UART ###
Ce peripherique permet de creer un pont vers le port serie du microcontroleur.  Le port virtuel /dev/pts/5 est créé, on peut se connecter directement avec screen/minicom/picocom...

Ce composant reprends directement le composant uart\_pty.h livré par simavr.

Il est connecté au microcontroleur simulé par le port UART.

### 2.2. Pulse\_input ###
Ce composant émule un generateur de signal rectangulaire. Le but est d'émulé les impulsions PMH et compteur de vitesse.

Il est possible de régler le temps à l'état haut et le temps à l'état bas.

### 2.3. Analog\_driver ###
Ce composant émule les entrées analogiques : température, papillon de gaz, niveau de batterie...

### 2.4. Flyback\_supply ###
Ce composant émule l'alimentation haute tension Flyback. Il permet de valider le comportement de la coucle d'asservissement de l'alimentation HT.

L'entrée du composant est la valeur de PWM fournie par le microcontroleur, la sortie est un signal logique donnant l'état de la tension interne par rapport à un seuil prédéfini :

> 0 : tension interne sous le seuil

  1. : tension interne au-dessus du seuil

Ce seuil est réglable (par défaut il est à 400v).
Le comportement dynamique se base sur la formule suivante :

> V(int\_N) = 20% x (PWM / 256) x 500v + 80% x V(int\_N-1) x 95%

La formule est réévaluée toutes les 10 ms. La valeur est la tension interne est accessible pour tracer des graphes dans un fichier VCD.

### 2.4. Timing\_Analyzer ###
Ce composant se place sur les sorties allumage et injection. Il est aussi connecté au signal sortant du pulse\_input émulant le capteur PMH.

Il permet de calculer les timings des signaux d'allumage et d'injection :

  * temps à l'état ON
  * retard/avance par rapport au PMH

A partir de ces timings et des autres paramêtres (RPM, température...), le banc de test peut valider l'application de la loi de pilotage d'allumage et d'injection.

### 2.5. LCD\_Button\_i2c ###
Ce composant émule le "RGB LCD Shield Kit" de AdaFruit (http://www.adafruit.com/products/714). Il est interfacé en i2c et comporte un LCD 2x16 characters + 6 boutons.

Le LCD n'est pas rendu en OpenGL (possible avec SimAvr !), mais le buffer de texte affiché est disponible pour le banc de test.


## 3. Tests disponibles ##
### 3.1. Appel du sequenceur de test ###
```
./simMain.exe <option> elfFile

	-a			lance tous les tests disponibles

	-l			liste des tests disponibles

	-t "test"	lance le test "test"

	-m		mode manuel, le server GBD est activé, la communication UART disponible sur /dev/pts/5, le temps de simulation est infini

	-h 		message d'aide

	<elfFile>	chemin vers le fichier .elf 
```

### 3.2. Liste des tests ###

<blockquote><table cellpadding='5' border='1'>
<tr><th>Test</th><th>Description</th></tr>
<tr><td>Version</td><td>Lecture de la version avec la commande "v" et comparaison avec la version courante. Ce test permet de vérifier la communication série</td></tr>
<tr><td>RPM</td><td>Mesure du régime</td></tr>
<tr><td>Vitesse</td><td>Mesure de la vitesse</td></tr>
<tr><td>Entrées analogiques</td><td>Mesures des entrées temperatures, batterie, accélérateur</td></tr>
<tr><td>Alimentation DC/DC haute tension</td><td>Vérification de la boucle d'asservissement de l'alimenetation DC/DC</td></tr>
<tr><td>Allumage</td><td>Verification des timings d'allumage</td></tr>
<tr><td>Injection</td><td>Verification des timings d'allumage</td></tr>
<tr><td>Clavier/ecran</td><td>Verification du fonctionnement de l'interface LCD/boutons</td></tr>
</table></blockquote>


### 3.3. Compilation ###
Le makefile principale contient une cible "sim". Un simple "make sim" compile à la fois le firmware et le programme de simulation.
Le seul ajustement à faire est de faire pointer le makefile sur le chemin de simavr (qui doit bien sur avoir été compilé !).


## 4. Tests de regression automatiques ##

Pour faire de [l'integration continue](http://fr.wikipedia.org/wiki/Int%C3%A9gration_continue) du pauvre, un script est déclenché sur chaque commit pour effectuer les tests de base en simulation et publier les résultats sur ce wiki. Il y a pas mal d'avantage :
  * on a toujours un état des lieux de la version courante.
  * on voit rapidement les regressions et les problemes de compilation.
  * ça "oblige" à faire des commits propres
  * en ecrivant les tests avant le soft, on peut suivre l'avancement.


Le script _autoTest.sh_ se situe à la racine des sources. Il effectue les taches suivantes:
  * récupère la version courante depuis le serveur GIT dans un repertoire temporaire
  * compile le firmware et le banc de tests, enregistre les resultats des compilations avec l'occupation mémoire RAM/flash/EEPROM
  * lance l'ensemble des tests et enregistre les logs de tests
  * analyse les resultats de tests pour construire un tableau de résultats
  * modifie le fichier wiki de résultats et commit la mise à jour

## 5. Liens ##
http://ingo.orgizm.net/articles/2012-02-02-simulating-and-inspecting-code-with-gdb-and-simavr

https://github.com/buserror-uk/simavr