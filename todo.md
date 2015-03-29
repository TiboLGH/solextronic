## Roadmap ##
  * **0.1** : **Done** :
    * Firmware : mesure du RPM, mesures des temperatures, tension batterie et position papillon.
    * Simulation : integration en simulation des mesures externes
    * Carte : carte de test simplifiée sur plaque à pastilles et generateurs externes de signaux de tests
    * Software de controle : console série
  * **0.2** :
    * Firmware : génération des signaux d'injection et d'allumage avec un timing fixe
    * Systeme de trace (printf RS232) et assert
    * Simulation : analyseur de timing allumage et injection
    * Carte : test sur carte avec oscilloscope
    * Software de controle : adoption du protocole de Megasquirt, integration avec TunerStudio
  * **0.3** :
    * Firmware : gestion d'un table d'avance pour l'allumage
    * Calcul : logiciel de calcul des temps/timings d'injection en fonction des signaux d'entrée
    * Firmware : Gestion de l'afficheur LCD et boutons
    * Simulation : couplage simulation et logiciel de calcul timing/durée
  * **0.4** :
    * Firmware : gestion de la table de VE pour l'injection
    * Carte : design carte complete sur base Arduino Nano + traversants
  * **0.5** :
    * Firmware : gestion de compensation en phase d'accélération
    * Boitier
    * Construction du faisceau
  * **0.6** :
    * Firmware : mesure de la quantité de carburant, gestion dynamique de la pompe
  * **0.7** :
    * Firmware : gestion de l'alimentation Flyback
    * Carte : schema et carte d'alimentation haute-tension
  * **0.8** : intégration sur moteur
    * Etape 1 : remplacement de l'allumage par Solextronic, affinage table d'avance
  * **0.9** : intégration sur moteur
    * Etape 2 : remplacement du carburateur par le circuit d'injection
  * **1.0** : carte définitive
    * Carte : Debug et test carte d'integration en CMS avec AVR standalone
  * **1.1** :
    * Firmware : service de logs en EEPROM pour mesure de performances

## ToDo List ##
  * Makefile et 1er programme de communication USART : DONE
  * Description des principes généraux
  * Bloc diagramme général : DONE
  * Bloc diagramme électronique
  * Bloc diagramme carburant
  * Choix techniques
  * Choix des composants
  * Schéma faisceau
  * Relation avec Solexbench
  * Affichage LCD et boutons
  * Détails des commandes série : DONE
  * Simulation : DONE