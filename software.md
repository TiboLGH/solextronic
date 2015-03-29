## Introduction ##

J'avais prévu au départ de développer un logiciel de controle sous Android. Finalement, après quelques recherches sur le web pour chercher l'inspiration ;-), il apparait que le soft de controle utilisé par [Megasquirt](http://www.megasquirt.info/), [TunerStudio](http://www.tunerstudio.com/) est en fait compatible avec plusieurs ECU (à commencer par la famille MegaSquirt). En fait, un "simple" fichier INI permet de parametrer l'interface et les commandes série.
Les softwares TunerStudio sont :
  * réellement bien faits et beaux,
  * multiplatformes (Java inside, pour le meilleur et pour le pire, mais compatible Windows, MacOs, Linux et même une version Android),
  * bien supportés
Inutile de réinventer la roue, j'adapte mon jeu de commandes pour être compatible et je vais produire des fichiers de configurations pour TunerStudio.
Seul défaut, il n'est pas libre : voir [MegaTunix](https://github.com/djandruczyk/MegaTunix) pour une alternative libre.

## Installation ##

## Configuration ##
TunerStudio utilise un fichier ini plutôt touffu pour s'adapter à chaque ECU. Il "suffit" en fait de décrire les commandes de l'ECU pour qu'il puissent lire et écrire les parametres, afficher les mesures...
Ces fichiers ini sont fournis par Megasquirt ou open5xxxecu et dépendent des modèles et/ou des versions firmware.

Pour Solextronic, le fichier ini est stocké dans les sources dans le repertoire /software.

Attention : le fichier ini doit correspondre au firmware (il y a une vérification de la version à la connection).
De plus, si vous modifiez le code de Solextronic touchant aux commandes RS232, il sera nécessaire de modifier le fichier ini.

## Logiciels alternatifs ##
Tuner Studio n'est pas libre : Solextronic essaie d'utiliser un maximum de composants et d'outils libres, c'est un peu gênant.
Comme alternative, j'ai trouvé :
  * [MegaTunix](https://github.com/djandruczyk/MegaTunix) : il est de plus écrit en C/GTK (plus de lourdeur Java/JVM), multiplatforme Linux/Windows/MacOS moyennant une recompilation. Pas encore essayé, je ne sais pas comment on paramêtre un nouvel ECU (fichier ini compatible TunerStudio ? ce serait super).