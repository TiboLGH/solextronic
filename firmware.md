Le firmware est concu pour tourner sur un Atmel ATMega328 (sur une carte Arduino Nano v3). Il est écrit en C et compile avec avr-gcc et la avr-libc.

Neanmoins, la plupart du code specifique à l'AVR est regroupé dans le fichier platform.c. Utiliser un autre MCU doit se limiter a une réadaptation de ce fichier + changement des headers.


## 1. Description generale ##
Le firmware est compose de plusieurs taches :
  * une tache de fond basse priorité
  * une tache de mesure du signal PMH et de génération des signaux d'allumage et d'injection à l'aide des modules capture/compare des timers hardware.
  * une tache périodique (1ms) : pseudo-RTC, trigger des conversions analogique-numerique.

**Tache de fond**
Cette tache implémente :
  * gestion de l'EEPROM
  * gestion des commandes USB/RS232
  * calcul des timings d'allumage et d'injection
  * Dans un 2e temps : gestion de la face avant LCD/boutons


**Tache "PMH"**
Cette tache se base sur un timer hardware (Timer 1) et est cadencée sur le top PMH.
Une rotation moteur se déroule de la façon suivante :
  1. au top PMH, l'interruption INT0 est déclenchée : le contenu du timer 1 est capturé (et permet de calculer le régime courant). C'est le début du cycle. Attention, depuis la version 0.2, le contenu du timer n'est pas remis à 0: tous les timings sont en relatif.
  1. les registres output compare A (allumage) et B (injection) sont chargés avec les valeurs permettant de déclencher l'allumage et le début d'injection. Ces valeurs sont calculées en tache de fond. Note : ces calculs sont assez rapides pour tenir dans un cycle moteur.
  1. quand le timer 1 atteint une des valeurs des OC, la pin correspondante change d'état et une interruption est déclenchée : dans cette interruption, le registre OC correspondant est rechargé avec la valeur de fin d'evenement (durée d'injection ou d'allumage).
  1. le cycle reprends au prochain top PMH.

Le timer 1 a une granularité de 4µs et un overflow de 262ms (~230 tr/min). On considère le moteur "calé" si on ne reçoit pas de top PMH pour un cycle complet .


**Tache périodique (1ms)**
Cette tache se base sur un timer hardware (Timer 2) réglé pour déclencher une interruption chaque ms.
  * Mise à jour des timers software
  * Mise à jour de la pseudo-RTC (tous les 100msec)
  * Déclenchement des conversions analogique/numérique (tous les 10msec)
  * Déclenchement des cycles allumage/injection dans les modes de tests.


**Autres taches**
  * Emission/reception des caracteres sur l'USART sur interruption.


## 2. Fichiers sources ##
Il y a 6 fichiers principaux :

**main.c**
  * Initialisation
  * Tache de fond (incluant calcul de l'injection et l'allumage)

**command.c/.h**
  * Interpretation des commandes RS232

**helper.c/.h**
  * Regroupe les functions d'interpolation, de conversions...
  * A la fois executé par le firmware et la simulation.

**platform.c/.h**
  * Interface USART
  * Gestion EEPROM
  * Service de timer software
  * interface I²C
  * interface ADC
  * interface PWM
  * Gestion des timings allumage/injection

**common.h**
  * Définitions des structures communes
  * Version

**varDef.h**
  * Mapping de l'EEPROM
  * Mapping de l'état interne en RAM
  * Ce fichier est également utilisé par la simulation

**frontPanel.c/.h**
  * Gestion de l'écran LCD et des bouttons accessibles
  * Gestion des menus du LCD
  * Gestion de la couleur du rétroéclairage poour donner l'état global

## 3. Platforme ##
Cette section décrit les ressources matérielles utilisées dans le microcontroleur.

### 3.1 General ###
  * Horloge principale a 16MHz
  * RS232 : 57600, 8bits, pas de parité


### 3.2 Timers ###
  * Timer 0 (8 bits) : génération PWM pour le controle de l'injecteur
  * Timer 1 (16 bits): Mesure RPM et generation des signaux allumage et injection
  * Timer 2 (8 bits) : horloge 1ms pour la pseudo RTC et les timers software


### 3.3 Interruptions externes ###
**TODO**

### 3.4 ADC ###

<blockquote><table cellpadding='5' border='1'>
<tr><th>ADC</th><th>Utilisation</th></tr>
<tr><td>ADC1</td><td>Temperature CLT culasse<br />La conversion se fait à travers une table</td></tr>
<tr><td>ADC2</td><td>Temperature IAT admission<br />La conversion se fait à travers une table</td></tr>
<tr><td>ADC3</td><td>Papillon<br />La conversion en % se fait avec des bornes min/max</td></tr>
<tr><td>ADC6</td><td>Capteur de pression/debit<br />La conversion se fait à travers une table</td></tr>
<tr><td>ADC7</td><td>Niveau batterie<br />Le facteur de conversion est x3</td></tr>
</table></blockquote>

Le scheduler de conversion réponds aux critères suivants :
  * 1 seule fonction doit etre capable d'accéder à l'ADC
  * la mesure de la position du papillon se fait sur un timer 10ms + timestamp pour calculer la vitesse de variation. Le gestionnaire d'ADC fait le calcul de la vitesse de variation.
  * la mesure de la pression se fait de facon synchrone avec le top PMH (avec un angle d'avance reglable)
    * Pour l'instant, cette fonction n'est pas gérée, à voir en fct du besoin réel.

L'ADC est configuré avec un prescaler à 64, et donc une frequence de 250KHz (ok pour 8 bits), chaque conversion dure 52µs et donc 300µs pour les 6 conversions.


**Interface**:
  * Entrée
    * Toutes les 10ms, la 1ere acquisition est lancée. Les autres sont déclenchées "en rafale" à la suite.
  * Fonctions
    * ISR(ADC\_vect) : interruption de fin d'acquisition. On stocke le resultat courant et on passe au canal suivant.
    * AdcProcessing() : conversion des résultats. Cette fonction est à appeller régulièrement (en fait toutes les 10ms).

  * Sorties :
    * Résultats de conversions dans la structure gState mise à jour à chaque conversion. Les mesures brutes sur 8 bits sont aussi disponibles pour faciliter les calibrations.


## 4. Commandes ##
Les commandes RS232 sont basees sur le protocole de Megasquirt décrit dans la page [Commandes](commands.md). Les variables accessibles sont définies dans _varDef.h_ :
  * eData : parametres accessibles en écriture et stockées en EEPROM (+ image en RAM)
  * gState : variables internes accessibles en lecture et stockées en RAM


## 5. LCD et boutons ##
Le pilotage des boutons externes et du LCD se fait à travers un module Adafruit (http://www.adafruit.com/product/714).
Il comporte 5 boutons, un LCD 2 lignes/16 caractères et un rétroéclairage 8 couleurs le tout piloté par I2C.

### 5.1 Driver I2C et LCD ###


### 5.2 Menus et affichage ###