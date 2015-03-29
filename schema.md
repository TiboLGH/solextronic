# Schema #

![http://solextronic.googlecode.com/git/hardware/solextronic.png](http://solextronic.googlecode.com/git/hardware/solextronic.png)

[En PDF](http://solextronic.googlecode.com/git/hardware/solextronic.pdf).

## 1. Carte principale ##
### 1.1. Affectation broches Arduino Nano ###

<blockquote><table cellpadding='5' border='1'>
<tr><th>Pin Nano</th><th>Pin ATMega 328</th><th>Direction</th><th>Commentaire</th></tr>
<tr><td>TX/1</td><td>TX/PD1</td><td>Output</td><td>Directement connectee sur le convertisseur USB</td></tr>
<tr><td>RX/2</td><td>RX/PD0</td><td>Input</td><td>Directement connectee sur le convertisseur USB</td></tr>
<tr><td>Reset/3</td><td>Reset/PC6</td><td>Input</td><td>Reset HW. Non connecte</td></tr>
<tr><td>Gnd/4</td><td>Gnd</td><td>Alim</td><td>Masse generale</td></tr>
<tr><td>D2/5</td><td>INT0/PD2</td><td>Input</td><td>Impulsion moteur PMH</td></tr>
<tr><td>D3/6</td><td>INT1/PD3</td><td>Input</td><td>Impulsion roue</td></tr>
<tr><td>D4/7</td><td>PD4</td><td>Input</td><td>Bouton de demarrage</td></tr>
<tr><td>D5/8</td><td>OC0B/PD5</td><td>Output</td><td>Commande Alimentation HT </td></tr>
<tr><td>D6/9</td><td>OC0A/PD6</td><td>Output</td><td>PWM injecteur</td></tr>
<tr><td>D7/10</td><td>PD7</td><td>Output</td><td>Commande pompe</td></tr>
<tr><td>D8/11</td><td>PB0</td><td>Input</td><td>Retour mesure HT</td></tr>
<tr><td>D9/12</td><td>OC1A/PB1</td><td>Output</td><td>Commande allumage</td></tr>
<tr><td>D10/13</td><td>OC1B/PB2</td><td>Output</td><td>Commande injection</td></tr>
<tr><td>D11/14</td><td>PB3/MOSI</td><td>Output</td><td>Data out SPI</td></tr>
<tr><td>D12/15</td><td>PB4/MISO</td><td>Input</td><td>Data in SPI</td></tr>
<tr><td>D13/16</td><td>PB5/SCK</td><td>Output</td><td>Led interne / horloge SPI</td></tr>
<tr><td>3v3/17</td><td>/</td><td>Output</td><td>Non Connectee</td></tr>
<tr><td>AREF/18</td><td>Aref</td><td>Input</td><td>Connectee au +5v</td></tr>
<tr><td>A0/19</td><td>PC0</td><td>Output</td><td>SS pour SPI</td></tr>
<tr><td>A1/20</td><td>ADC1/PC1</td><td>Input</td><td>Temperature 1, culasse</td></tr>
<tr><td>A2/21</td><td>ADC2/PC2</td><td>Input</td><td>Temperature 2, admission</td></tr>
<tr><td>A3/22</td><td>ADC3/PC3</td><td>Input</td><td>Papillon</td></tr>
<tr><td>A4/23</td><td>SDA/PC4</td><td>Bidir</td><td>Connexion I2C vers afficheur/boutons</td></tr>
<tr><td>A5/24</td><td>SDL/PC5</td><td>Bidir</td><td>Connexion I2C vers afficheur/boutons</td></tr>
<tr><td>A6/25</td><td>ADC6</td><td>Input</td><td>Capteur de pression</td></tr>
<tr><td>A7/26</td><td>ADC7</td><td>Input</td><td>Niveau batterie</td></tr>
<tr><td>+5v/27</td><td>Vcc</td><td>Alim (out)</td><td>Alim 5v generale.<br />Le regulateur fournit 500mA</td></tr>
<tr><td>Reset/28</td><td>Reset/PC6</td><td>Input</td><td>Reset HW. Non connecte</td></tr>
<tr><td>Gnd/29</td><td>Gnd</td><td>Alim</td><td>Masse generale</td></tr>
<tr><td>Vin/30</td><td>/</td><td>Alim (in)</td><td>Alim generale +12v</td></tr>
</table></blockquote>

### 1.2. Montage élévateur DC/DC FlyBack ###
> Voir la page dédiée : [flyback](flyback.md).


### 1.3. Controle de l'injecteur ###
TODO :
  * pilotage de l'injecteur en hold/maintain.
  * rating FET, driver FET
  * injecteur high-Z / low-Z

### 1.4. Controle de l'allumage ###
TODO :
  * pilotage du trigger de bloc DC-CDI.


## 2. Faisceau ##

### 2.1. Alimentation ###
L'alimentation de SolexTronic se fait par une batterie 12v au plomb.

  * entree de +10v a +15v
  * consommation estimee : 1A
  * courant maximum : 4A
  * mesure de la tension d'entree, alarme batterie basse
  * Evolution possible : recharge de la batterie par un ensemble bobine/volant magnétique à travers un circuit de régulation.


### 2.2. Sorties ###

<h4>Commande allumage</h4>
Cette sortie commande l'allumage.

  * Niveau de sortie 0-5v
  * Controle du decalage par rapport a l'impulsion PMH
  * Controle de la duree de l'impulsion (dwell)
  * Polarite reglable


<h4>Commande injection</h4>
Cette sortie commande l'injection.

  * Niveau de sortie 0-5v
  * Controle du decalage par rapport a l'impulsion PMH
  * Controle de la duree de l'impulsion (temps d'injection)
  * Polarite reglable


<h4>Commande pompe</h4>
Cette sortie commande la pompe d'injection.

  * Sortie sur relais 12v
  * Possibilite de forcer le fonctionnement de la pompe



### 2.3 Entrees ###

<h4>Impulsion PMH</h4>
Cette entree est connecte sur le capteur de PMH.

  * niveau d'entree : 0-5v
  * frequence d'entree de 300 tr/min (4Hz) a 12000 tr/min (200 Hz). Resolution 100 tr/min.
  * synchro sur front montant ou descendant


Ce signal est la base de temps pour les mesures d'allumage et d'injection.

<h4>5.2 Impulsion roue</h4>
Cette entree est connecte sur le capteur de rotation de la roue.

  * niveau d'entree : 0-5v
  * Gamme de vitesse : 0-100 km/h. Resolution 0.1 km/h.
  * synchro sur front montant ou descendant


Ce signal est la base de temps pour les mesures d'allumage et d'injection.

<h4>Temperatures 1 & 2</h4>
Ces entrees sont connectees sur les capteurs de temperature. Le capteur 1 est sur la culasse, le capteur 2 est sur la boite a air.

  * niveau d'entree : 0-2.0v (0 to 200 deg)
  * detection haute impedance pour test default capteurs


<h4>Papillon d'accelerateur</h4>
Cette entree mesure la position du papillon d'acceleration.

  * niveau d'entree : 0-5.0v
  * detection haute impedance pour test default capteur


<h4>Capteur de pression</h4>
Cette entree mesure le capteur de pression d'admission. Ce signal n'est pas utilisé dans la 1ere version.

  * niveau d'entree : 0-10.0v ???
  * detection haute impedance pour test default capteur

## 3. Connections externes ##
La connexion vers le faisceau se fait a travers un connecteur DB25. Un faux faisceau permet de se connecter a SolexBench ou a une carte de test simplifiee.

<blockquote><table cellpadding='5' border='1'>
<tr><th>Pin</th><th>Nom</th><th>Type</th><th>Direction</th></tr>
<tr><td>1</td><td>Masse</td><td>Alim</td><td>in</td></tr>
<tr><td>2</td><td>Impulsion PMH</td><td>Logique</td><td>in</td></tr>
<tr><td>3</td><td>Masse</td><td>Alim</td><td>in</td></tr>
<tr><td>4</td><td>Impulsion roue</td><td>Logique</td><td>in</td></tr>
<tr><td>5</td><td>Papillon</td><td>Analogique</td><td>in</td></tr>
<tr><td>6</td><td>Temp 1</td><td>Analogique</td><td>in</td></tr>
<tr><td>7</td><td>Temp 2</td><td>Analogique</td><td>in</td></tr>
<tr><td>8</td><td>Pression</td><td>Analogique</td><td>in</td></tr>
<tr><td>9</td><td>RFU</td><td>RFU</td><td>RFU</td></tr>
<tr><td>10</td><td>RFU</td><td>RFU</td><td>RFU</td></tr>
<tr><td>11</td><td>+Batt</td><td>Alim</td><td>out</td></tr>
<tr><td>12</td><td>+5v</td><td>Alim</td><td>out</td></tr>
<tr><td>13</td><td>Masse</td><td>Alim</td><td>in</td></tr>
<tr><td>14</td><td>+Batt</td><td>Alim</td><td>out</td></tr>
<tr><td>15</td><td>+5v</td><td>Alim</td><td>out</td></tr>
<tr><td>16</td><td>Pompe</td><td>Loqique</td><td>out</td></tr>
<tr><td>17</td><td>Masse</td><td>Alim</td><td>in</td></tr>
<tr><td>18</td><td>I2C DATA</td><td>Logique</td><td>in/out</td></tr>
<tr><td>19</td><td>I2C CLK</td><td>Logique</td><td>in/out</td></tr>
<tr><td>20</td><td>Masse</td><td>Alim</td><td>in</td></tr>
<tr><td>21</td><td>+5v</td><td>Alim</td><td>out</td></tr>
<tr><td>22</td><td>RFU</td><td>RFU</td><td>RFU</td></tr>
<tr><td>23</td><td>RFU</td><td>RFU</td><td>RFU</td></tr>
<tr><td>24</td><td>Commande injection</td><td>Logique</td><td>out</td></tr>
<tr><td>25</td><td>Commande allumage</td><td>Logique</td><td>out</td></tr>
</table></blockquote>