

# Introduction #

Le faisceau permet de connecter le boitier Solextronic aux capteurs et actionneurs situés sur le moteur :
  * Injecteur
  * Boitier CDI
  * Pompe a essence
  * Capteurs de temperature
  * Capteur de PMH et rotation de la roue
  * Capteur de pression tubulure
  * Papillon des gaz


# Schema general #


&lt;TODO&gt;



## Connexion au boitier ##
Le connexion au boitier principal se fait par un connecteur DB25 male, le pinout est définit dans la page [schema](schema.md).

## Protection des entrées/sorties ##


&lt;TODO&gt;



## Capteurs ##

### 1. Capteur TPS (accélerateur) ###
Le capteur est un simple potentiometre rotatif. La mise en forme consiste en un montage suiveur + filtre passe-bas autour de 200Hz (suffisant pour mesurer la vitesse d'ouverture/fermeture du papillon).
Pour la plpart des capteurs du commerce, la sortie se fait sur une plage réduite comprise entre 0 et 5V. Il peut être intéressant d'ajouter un circuit de mise en forme pour exploiter au mieux la dynamique de l'ADC.

### 2. Capteur MAP (pression) ###
Le capteur de pression est le cas le plus difficile. En utilisant un capteur dédié, la sortie se fait de 0 à 5V. Comme pour le TPS et l'IAT,  la mise en forme est un montage amplificateur non-inverseur permettant d'exploiter au mieux la dynamique de l'ADC.
Une table de calibration sera intégrée au FW pour convertir la tension de sortie en temperature.

### 3. Capteur IAT (température air) ###
Le capteur de temperature est une thermistance CTN. La mise en forme est un montage amplificateur non-inverseur permettant d'exploiter au mieux la dynamique de l'ADC.
Une table de calibration sera intégrée au FW pour convertir la tension de sortie en temperature.

### 4. Capteur CLT (température moteur) ###
Il est analogue au capteur IAT, la plage de fonctionnement étant seulement plus large.

### 5. Capteur de PMH ###


&lt;TODO&gt;



### 6. Capteur de vitesse ###


&lt;TODO&gt;



## Pompe à essence ##


&lt;TODO&gt;



## Injecteur ##


&lt;TODO&gt;



# Cablage #


&lt;TODO&gt;

